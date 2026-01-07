#include "ui/application.h"
#include "audio/mpv_player.h"
#include "player_interface.h"
#include "core/config.h"
#include "utils/logger.h"
#include "image_utils.h"

#include "imtui/imtui.h"
#include "imtui/imtui-impl-ncurses.h"

#include <cstring>
#include <clocale>
#include <ncurses.h>
#include <unistd.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <vector>
#include <future>
#include <functional>

namespace ytui {

Application::Application()
    : screen_(nullptr),
      selected_search_result_(0),
      selected_queue_item_(0),
      show_search_(true),
      live_only_(false),
      running_(false),
      current_focus_(FocusArea::Search),
      vim_mode_enabled_(false),
      search_in_progress_(false),
      incremental_search_active_(false),
      thumbnail_fetch_in_progress_(false),
      current_volume_(0.7f) {
    std::memset(search_buffer_, 0, sizeof(search_buffer_));
    std::memset(url_buffer_, 0, sizeof(url_buffer_));
}

Application::~Application() {
    shutdown();
}

bool Application::initialize() {
    // CRITICAL: Set UTF-8 locale BEFORE ncurses initialization for proper unicode support
    setlocale(LC_ALL, "");

    // MPV requires C locale for LC_NUMERIC to avoid parsing issues
    setlocale(LC_NUMERIC, "C");

    Logger::init("/tmp/moostream.log");
    Config::instance().load();

    // Create components
    player_ = std::make_unique<MpvPlayer>();
    extractor_ = std::make_unique<YouTubeExtractor>();
    state_manager_ = std::make_unique<StateManager>();

    // Set up player callbacks
    player_->set_end_callback([this]() {
        Logger::info("Track ended, playing next");
        next_track();
    });

    player_->set_error_callback([](const std::string& error) {
        Logger::error("Player error: " + error);
    });

    // Initialize volume from config
    current_volume_ = static_cast<float>(Config::instance().get_volume());
    player_->set_volume(current_volume_);

    // Initialize ImTui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Set up modern theme
    setup_theme();

    screen_ = ImTui_ImplNcurses_Init(true);
    ImTui_ImplText_Init();

    running_ = true;
    Logger::info("Application initialized successfully");

    return true;
}

void Application::run() {
    while (running_) {
        update();
        render();
        handle_input();
    }
}

void Application::shutdown() {
    Logger::info("Shutting down application");

    // Save config
    Config::instance().save();

    // Cleanup ImTui
    ImTui_ImplText_Shutdown();
    if (screen_) {
        ImTui_ImplNcurses_Shutdown();
        screen_ = nullptr;
    }

    ImGui::DestroyContext();

    Logger::shutdown();
}

void Application::update() {
    // Update player
    if (player_) {
        player_->update();
    }

    // Check if incremental search is complete
    if (incremental_search_active_ && !search_in_progress_) {
        incremental_search_active_ = false;
        Logger::info("Search completed: " + std::to_string(incremental_results_.size()) + " results");
    }
}

void Application::render() {
    // Ensure ImTui screen is valid before proceeding
    if (!screen_) {
        Logger::error("ImTui screen is null, reinitializing...");
        screen_ = ImTui_ImplNcurses_Init(true);
        if (!screen_) {
            Logger::error("Failed to reinitialize ImTui screen");
            return;
        }
    }

    ImTui_ImplNcurses_NewFrame();
    ImTui_ImplText_NewFrame();

    // Ensure ImGui context is valid
    if (!ImGui::GetCurrentContext()) {
        Logger::error("ImGui context lost, recreating...");
        ImGui::CreateContext();
        setup_theme();
    }

    ImGui::NewFrame();

    // Ensure clean state for each render
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

    for (auto it = toasts_.begin(); it != toasts_.end(); ) {
        it->time_remaining -= ImGui::GetIO().DeltaTime;
        if (it->time_remaining <= 0.0f) {
            it = toasts_.erase(it);
        } else {
            ++it;
        }
    }

    // Main window with modern styling

    ImGui::Begin("YouTube TUI", nullptr,
                  ImGuiWindowFlags_NoTitleBar |
                  ImGuiWindowFlags_NoResize |
                  ImGuiWindowFlags_NoMove |
                  ImGuiWindowFlags_NoCollapse);

    render_menu_bar();

    if (ImGui::BeginTabBar("MainTabs")) {
        if (ImGui::BeginTabItem("Search")) {
            render_search_view();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("URL")) {
            render_url_view();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    render_queue_view();
    render_player_view();
    render_status_bar();

    ImGui::End();

    ImGui::Render();
    ImTui_ImplText_RenderDrawData(ImGui::GetDrawData(), screen_);
    ImTui_ImplNcurses_DrawScreen();

    // Force immediate screen refresh to prevent blank screen during operations
    refresh();
    doupdate(); // Ensure ncurses updates the physical screen

    // Small delay to prevent UI freezing during background operations
    usleep(1000); // 1ms delay
}

void Application::render_menu_bar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Quit", "q")) {
                running_ = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Search", "s", &show_search_);
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void Application::render_search_view() {
    ImGui::BeginChild("SearchPanel", ImVec2(0, 12), true,
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "[SEARCH] YouTube");
    draw_visual_separator();

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.18f, 0.25f, 1.0f));
    bool enter_pressed = ImGui::InputText("##search", search_buffer_, sizeof(search_buffer_), ImGuiInputTextFlags_EnterReturnsTrue);
    if (enter_pressed) {
        search_youtube(search_buffer_);
        show_toast("Searching YouTube...", 2.0f);
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::Checkbox("Live", &live_only_);
    if (live_only_) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "(API MODE)");
    }

    ImGui::SameLine();
    if (ImGui::Button("Search") && std::strlen(search_buffer_) > 0) {
        search_youtube(search_buffer_);
        show_toast("Searching YouTube...", 2.0f);
        // Ensure UI stays responsive after action
        ImGui::SetWindowFocus(nullptr);
    }

    if (incremental_search_active_) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "[Searching...]");
    }

    if (incremental_search_active_ || !incremental_results_.empty()) {
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Results: (%zu)", incremental_results_.size());
        ImGui::BeginChild("SearchResults", ImVec2(0, 0), true);

        std::lock_guard<std::mutex> lock(search_mutex_);
        for (size_t i = 0; i < incremental_results_.size(); ++i) {
            const auto& track = incremental_results_[i];
            bool selected = (i == static_cast<size_t>(selected_search_result_));

            std::string duration_str = track.duration > 0 ?
                std::to_string(track.duration / 60) + ":" + std::to_string(track.duration % 60) : "LIVE";
            std::string display_text = track.title + " - " + track.channel + " (" + duration_str + ")";

            if (ImGui::Selectable(display_text.c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick)) {
                selected_search_result_ = static_cast<int>(i);
                if (ImGui::IsMouseDoubleClicked(0)) {
                    state_manager_->add_to_queue(track);
                    show_toast("Added to queue: " + track.title, 3.0f);
                    Logger::info("Added to queue: " + track.title);
                    // Ensure UI stays responsive after action
                    ImGui::SetWindowFocus(nullptr);
                }
            }
        }

        ImGui::EndChild();

        if (ImGui::Button("Add to Queue") && selected_search_result_ >= 0 &&
            selected_search_result_ < static_cast<int>(incremental_results_.size())) {
            std::lock_guard<std::mutex> lock(search_mutex_);
            const auto& track = incremental_results_[selected_search_result_];
            state_manager_->add_to_queue(track);
            show_toast("Added to queue: " + track.title, 3.0f);
            // Ensure UI stays responsive after action
            ImGui::SetWindowFocus(nullptr);
        }
    }

    ImGui::EndChild();
}

void Application::render_url_view() {
    ImGui::BeginChild("URLPanel", ImVec2(0, 12), true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "[URL] YouTube Direct");
    draw_visual_separator();

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.18f, 0.25f, 1.0f));
    bool enter_pressed = ImGui::InputText("##url", url_buffer_, sizeof(url_buffer_), ImGuiInputTextFlags_EnterReturnsTrue);
    if (enter_pressed && std::strlen(url_buffer_) > 0) {
        handle_url_input(url_buffer_);
        show_toast("Playing URL...", 2.0f);
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (ImGui::Button("Play URL") && std::strlen(url_buffer_) > 0) {
        handle_url_input(url_buffer_);
        show_toast("Playing URL...", 2.0f);
        // Ensure UI stays responsive after action
        ImGui::SetWindowFocus(nullptr);
    }

    ImGui::EndChild();
}

void Application::render_queue_view() {
    ImGui::BeginChild("QueuePanel", ImVec2(0, 15), true);

    if (ImGui::BeginTabBar("QueueTabs")) {
        if (ImGui::BeginTabItem("Queue")) {
            render_queue_tab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Playlists")) {
            render_playlists_tab();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::EndChild();
}

void Application::render_queue_tab() {
    const auto& queue = state_manager_->get_queue();

    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "Queue: (%zu tracks)", queue.size());
    draw_visual_separator();

    if (queue.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), "No tracks in queue. Search and add some!");
    } else {
        ImGui::BeginChild("QueueList", ImVec2(0, 0), false);

        for (size_t i = 0; i < queue.size(); ++i) {
            const auto& track = queue[i];
            bool is_current = (i == static_cast<size_t>(state_manager_->get_current_index()));
            bool selected = (i == static_cast<size_t>(selected_queue_item_));

            std::string icon = is_current ? ">" : "  ";
            std::string duration_str = track.duration > 0 ?
                std::to_string(track.duration / 60) + ":" +
                (track.duration % 60 < 10 ? "0" : "") + std::to_string(track.duration % 60) : "LIVE";

            std::string label = icon + " " + track.title + " - " + track.channel + " (" + duration_str + ")";

            if (is_current) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.2f, 1.0f));
            }

            if (ImGui::Selectable(label.c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick)) {
                selected_queue_item_ = static_cast<int>(i);
                if (ImGui::IsMouseDoubleClicked(0)) {
                    state_manager_->play_at_index(i);
                    play_current_track();
                    show_toast("Now playing: " + track.title, 2.0f);
                }
            }

            if (is_current) {
                ImGui::PopStyleColor();
            }
        }

        ImGui::EndChild();
    }
}

void Application::render_playlists_tab() {
    const auto& playlists = state_manager_->get_playlists();

    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "Saved Playlists: (%zu playlists)", playlists.size());
    draw_visual_separator();

    if (ImGui::Button("Save Current Queue as Playlist")) {
        // TODO: Show input dialog for playlist name
        std::string name = "My Playlist"; // Placeholder
        state_manager_->save_queue_as_playlist(name);
        show_toast("Saved queue as playlist: " + name, 2.0f);
    }

    if (playlists.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), "No saved playlists. Save your current queue!");
    } else {
        ImGui::BeginChild("PlaylistsList", ImVec2(0, 0), false);

        for (const auto& pair : playlists) {
            const Playlist& playlist = pair.second;

            std::string label = playlist.name + " (" + std::to_string(playlist.size()) + " tracks)";

            if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                if (ImGui::IsMouseDoubleClicked(0)) {
                    state_manager_->load_playlist_to_queue(playlist.id);
                    show_toast("Loaded playlist: " + playlist.name, 2.0f);
                }
            }

            // Context menu for delete
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Delete Playlist")) {
                    state_manager_->delete_playlist(playlist.id);
                    show_toast("Deleted playlist: " + playlist.name, 2.0f);
                }
                ImGui::EndPopup();
            }
        }

        ImGui::EndChild();
    }
}

void Application::render_player_view() {
    ImGui::BeginChild("PlayerPanel", ImVec2(0, 0), true);

    float windowWidth = ImGui::GetWindowWidth();
    const char* headerText = "NOW PLAYING";
    float headerWidth = ImGui::CalcTextSize(headerText).x;
    ImGui::SetCursorPosX((windowWidth - headerWidth) * 0.5f);
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "%s", headerText);
    
    draw_visual_separator();

    const Track* current = state_manager_->get_current_track();
    if (current) {
        if (current->id != last_fetched_track_id_ && !current->thumbnail_url.empty()) {
            fetch_thumbnail_async(current->id, current->thumbnail_url);
        }

        {
            std::lock_guard<std::mutex> lock(thumbnail_mutex_);
            float artPixelWidth = 56.0f * ImGui::CalcTextSize(" ").x;
            float artStartX = (windowWidth - artPixelWidth) * 0.5f;
            if (artStartX < 0.0f) artStartX = 0.0f;

            if (!current_colored_art_.empty()) {
                for (const auto& row : current_colored_art_.rows) {
                    ImGui::SetCursorPosX(artStartX);
                    for (size_t i = 0; i < row.size(); ++i) {
                        const auto& cc = row[i];
                        if (i > 0) ImGui::SameLine(0, 0);
                        char buf[2] = {cc.ch, '\0'};
                        ImGui::TextColored(ImVec4(cc.r, cc.g, cc.b, 1.0f), "%s", buf);
                    }
                }
            } else {
                for (int i = 0; i < 16; ++i) {
                    ImGui::SetCursorPosX(artStartX);
                    if (i == 0 || i == 15) {
                        ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.4f, 1.0f), "+------------------------------------------------------+");
                    } else if (i == 7) {
                        ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.4f, 1.0f), "|                      Loading...                      |");
                    } else {
                        ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.4f, 1.0f), "|                                                      |");
                    }
                }
            }
        }

        ImGui::Spacing();
        ImGui::Spacing();

        float titleWidth = ImGui::CalcTextSize(current->title.c_str()).x;
        ImGui::SetCursorPosX((windowWidth - titleWidth) * 0.5f);
        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.6f, 1.0f), "%s", current->title.c_str());

        std::string artistStr = "by " + current->channel;
        float artistWidth = ImGui::CalcTextSize(artistStr.c_str()).x;
        ImGui::SetCursorPosX((windowWidth - artistWidth) * 0.5f);
        ImGui::TextColored(ImVec4(0.7f, 0.8f, 0.9f, 1.0f), "%s", artistStr.c_str());

        ImGui::Spacing();

        double pos = player_->get_position();
        double dur = player_->get_duration();
        float progress = dur > 0 ? static_cast<float>(pos / dur) : 0.0f;

        float barWidth = windowWidth * 0.8f;
        ImGui::SetCursorPosX((windowWidth - barWidth) * 0.5f);
        ImGui::PushItemWidth(barWidth);
        draw_progress_bar(progress);
        ImGui::PopItemWidth();

        char timeBuf[64];
        snprintf(timeBuf, sizeof(timeBuf), "%.0f:%.0f / %.0f:%.0f",
                   std::floor(pos / 60), std::fmod(pos, 60),
                   std::floor(dur / 60), std::fmod(dur, 60));
        float timeWidth = ImGui::CalcTextSize(timeBuf).x;
        ImGui::SetCursorPosX((windowWidth - timeWidth) * 0.5f);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.7f, 1.0f), "%s", timeBuf);

        ImGui::Spacing();

        const char* prevLabel = "[Prev]";
        const char* nextLabel = "[Next]";
        const char* playLabel = (player_->get_state() == PlaybackState::Playing) ? "[Pause]" : " [Play] ";

        float btnSpacing = ImGui::GetStyle().ItemSpacing.x;
        float controlsWidth = ImGui::CalcTextSize(prevLabel).x + ImGui::CalcTextSize(nextLabel).x + ImGui::CalcTextSize(playLabel).x + (btnSpacing * 2) + 20.0f;
        
        ImGui::SetCursorPosX((windowWidth - controlsWidth) * 0.5f);
        
        if (ImGui::Button(prevLabel)) {
            previous_track();
            show_toast("Previous track", 1.5f);
        }
        ImGui::SameLine();
        
        if (ImGui::Button(playLabel)) {
             if (player_->get_state() == PlaybackState::Playing) {
                player_->pause();
                show_toast("Paused", 1.5f);
            } else {
                player_->play();
                show_toast("Playing", 1.5f);
            }
        }
        ImGui::SameLine();

        if (ImGui::Button(nextLabel)) {
            next_track();
            show_toast("Next track", 1.5f);
        }

        ImGui::Spacing();

        char volLabel[32];
        snprintf(volLabel, sizeof(volLabel), "Vol: %.0f%%", current_volume_ * 100.0f);
        
        float volTextWidth = ImGui::CalcTextSize(volLabel).x;
        float btnWidth = ImGui::CalcTextSize("[-]").x + 8.0f;
        float totalWidth = btnWidth + 10.0f + volTextWidth + 10.0f + btnWidth;
        ImGui::SetCursorPosX((windowWidth - totalWidth) * 0.5f);
        
        if (ImGui::Button("[-]")) {
            current_volume_ = std::max(0.0f, current_volume_ - 0.05f);
            player_->set_volume(current_volume_);
            Config::instance().set_volume(current_volume_);
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.9f, 1.0f), "%s", volLabel);
        ImGui::SameLine();
        if (ImGui::Button("[+]")) {
            current_volume_ = std::min(1.0f, current_volume_ + 0.05f);
            player_->set_volume(current_volume_);
            Config::instance().set_volume(current_volume_);
        }

    } else {
        std::string msg1 = "No track playing";
        float w1 = ImGui::CalcTextSize(msg1.c_str()).x;
        ImGui::SetCursorPosX((windowWidth - w1) * 0.5f);
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), "%s", msg1.c_str());
        
        std::string msg2 = "Search and add tracks to get started!";
        float w2 = ImGui::CalcTextSize(msg2.c_str()).x;
        ImGui::SetCursorPosX((windowWidth - w2) * 0.5f);
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.5f, 1.0f), "%s", msg2.c_str());
    }

    ImGui::EndChild();
}


void Application::render_status_bar() {
    draw_visual_separator("STATUS");
    PlaybackState state = player_->get_state();
    const char* state_icon = "Stopped";
    const char* state_str = "Stopped";
    ImVec4 state_color = ImVec4(0.6f, 0.6f, 0.7f, 1.0f);

    switch (state) {
        case PlaybackState::Playing:
            state_icon = ">";
            state_str = "Playing";
            state_color = ImVec4(0.4f, 1.0f, 0.6f, 1.0f);
            break;
        case PlaybackState::Paused:
            state_icon = "[Pause]";
            state_str = "Paused";
            state_color = ImVec4(1.0f, 0.8f, 0.4f, 1.0f);
            break;
        case PlaybackState::Buffering:
            state_icon = "Buffering";
            state_str = "Buffering";
            state_color = ImVec4(1.0f, 0.6f, 0.4f, 1.0f);
            break;
        default:
            break;
    }

    ImGui::TextColored(state_color, "%s %s", state_icon, state_str);
    ImGui::SameLine();

    size_t queue_size = state_manager_->get_queue().size();
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "| Results: %zu tracks", queue_size);

    if (state_manager_->is_shuffle_enabled()) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.8f, 1.0f), "| Shuffle: Shuffle ON");
    }

    if (state_manager_->is_repeat_enabled()) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.8f, 0.6f, 1.0f, 1.0f), "| Repeat: Repeat ON");
    }
}

void Application::handle_input() {
    // Additional keyboard shortcuts can be handled here
}

void Application::search_youtube(const std::string& query) {
    if (search_in_progress_ || incremental_search_active_) return;

    Logger::info("Searching for: " + query);
    search_in_progress_ = true;
    incremental_search_active_ = true;
    incremental_results_.clear();
    selected_search_result_ = 0;

    // Start streaming search in background
    std::thread([this, query]() {
        extractor_->search_streaming(query, 50, live_only_, [this](const Track& track) {
            // This callback will be called for each result as it comes in
            std::lock_guard<std::mutex> lock(search_mutex_);
            // When live_only is enabled, API already returns only live results
            incremental_results_.push_back(track);
        });
        // Mark search as complete
        std::lock_guard<std::mutex> lock(search_mutex_);
        search_in_progress_ = false;
    }).detach();
}

void Application::handle_url_input(const std::string& url) {
    Logger::info("Handling URL input: " + url);

    // Create a basic track for immediate playback - skip slow metadata extraction
    Track track;
    track.url = url;
    track.title = "Loading... (" + url.substr(url.find_last_of('/') + 1) + ")"; // Use video ID as temp title
    track.channel = "YouTube";
    track.duration = 0; // Unknown
    track.id = url; // Use URL as ID

    // Add to queue and play immediately
    state_manager_->add_to_queue(track);
    state_manager_->play_next();
    play_current_track();

    // Optionally extract metadata in background for UI update (but don't block playback)
    std::thread([this, url]() {
        try {
            Track metadata = extractor_->extract_info(url);
            if (metadata.is_valid()) {
                // TODO: Update the track in queue with metadata if needed
                Logger::info("Metadata extracted for: " + metadata.title);
            }
        } catch (const std::exception& e) {
            Logger::error("Failed to extract metadata: " + std::string(e.what()));
        }
    }).detach();
}

void Application::play_url(const std::string& url) {
    Logger::info("Playing URL: " + url);
    player_->load(url);
    player_->play();
}

void Application::play_current_track() {
    const Track* track = state_manager_->get_current_track();
    if (!track) return;

    // Use MPV's built-in yt-dlp support
    player_->load(track->url);
    player_->play();

    state_manager_->add_to_history(*track);
}

void Application::next_track() {
    state_manager_->play_next();
    play_current_track();
}

void Application::previous_track() {
    state_manager_->play_previous();
    play_current_track();
}

void Application::setup_theme() {
    ImGuiStyle& style = ImGui::GetStyle();

    // Retro-futuristic color scheme
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.12f, 1.00f);     // Dark blue-black
    colors[ImGuiCol_Border] = ImVec4(0.30f, 0.40f, 0.60f, 0.50f);       // Electric blue border
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Text colors - neon style
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.95f, 1.00f, 1.00f);         // Bright white
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.60f, 1.00f); // Muted gray

    // Interactive elements
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);       // Dark blue-gray
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.40f, 0.60f, 1.00f); // Electric blue hover
    colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.50f, 0.80f, 1.00f);  // Bright blue active

    // Selection colors
    colors[ImGuiCol_Header] = ImVec4(0.25f, 0.30f, 0.45f, 1.00f);        // Selected item
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.45f, 0.65f, 1.00f); // Selected hover
    colors[ImGuiCol_HeaderActive] = ImVec4(0.45f, 0.55f, 0.85f, 1.00f);  // Selected active

    // Frame colors (inputs, progress bars)
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.18f, 0.25f, 1.00f);       // Dark input background
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.30f, 0.45f, 1.00f);

    // Progress bar colors
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.60f, 1.00f, 1.00f); // Electric blue progress

    // Style adjustments for terminal
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.FrameRounding = 0.0f;  // Sharp corners for retro look
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;

    // Spacing for terminal readability
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.WindowPadding = ImVec2(8.0f, 8.0f);
}

void Application::show_toast(const std::string& message, float duration) {
    toasts_.push_back({message, duration, duration});
}

void Application::draw_progress_bar(float fraction, const char* label) {
    ImGui::ProgressBar(fraction, ImVec2(-1, 0), label);
}

void Application::draw_visual_separator(const char* title) {
    if (title) {
        ImGui::TextColored(ImVec4(0.7f, 0.8f, 1.0f, 1.0f), "--- %s %s", title,
                          std::string(50 - strlen(title) - 5, '=').c_str());
    } else {
        ImGui::TextColored(ImVec4(0.4f, 0.5f, 0.7f, 1.0f), "%s", std::string(60, '-').c_str());
    }
}

void Application::fetch_thumbnail_async(const std::string& track_id, const std::string& thumbnail_url) {
    if (thumbnail_fetch_in_progress_) return;
    
    Logger::info("Fetching thumbnail for track: " + track_id + " from: " + thumbnail_url);
    last_fetched_track_id_ = track_id;
    
    {
        std::lock_guard<std::mutex> lock(thumbnail_mutex_);
        auto it = thumbnail_cache_.find(track_id);
        if (it != thumbnail_cache_.end()) {
            current_colored_art_ = it->second;
            Logger::info("Using cached thumbnail for: " + track_id);
            return;
        }
    }
    
    thumbnail_fetch_in_progress_ = true;
    current_colored_art_ = ColoredAsciiArt();
    
    std::thread([this, track_id, thumbnail_url]() {
        std::vector<unsigned char> image_data = download_image(thumbnail_url);
        Logger::info("Downloaded image data: " + std::to_string(image_data.size()) + " bytes");
        ColoredAsciiArt colored_art;
        
        if (!image_data.empty()) {
            colored_art = image_to_colored_ascii(image_data, 56, 16);
            Logger::info("Generated colored ASCII art");
        }
        
        {
            std::lock_guard<std::mutex> lock(thumbnail_mutex_);
            thumbnail_cache_[track_id] = colored_art;
            if (last_fetched_track_id_ == track_id) {
                current_colored_art_ = colored_art;
            }
        }
        
        thumbnail_fetch_in_progress_ = false;
    }).detach();
}

void Application::render_toast_notifications() {
    // Render toast notifications in bottom-right corner
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 300, ImGui::GetIO().DisplaySize.y - 100));
    ImGui::SetNextWindowSize(ImVec2(280, 80));

    if (!toasts_.empty()) {
        ImGui::Begin("Toasts", nullptr,
                    ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoScrollbar |
                    ImGuiWindowFlags_NoInputs);

        for (auto& toast : toasts_) {
            ImGui::TextWrapped("%s", toast.message.c_str());
        }

        ImGui::End();
    }
}

} // namespace ytui
