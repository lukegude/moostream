#include "ui/application.h"
#include "audio/mpv_player.h"
#include "core/config.h"
#include "utils/logger.h"

#include "imtui/imtui.h"
#include "imtui/imtui-impl-ncurses.h"

#include <cstring>
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <vector>
#include <future>

namespace ytui {

Application::Application()
    : screen_(nullptr),
      selected_search_result_(0),
      selected_queue_item_(0),
      show_search_(true),
      running_(false),
      search_in_progress_(false),
      incremental_search_active_(false) {
    std::memset(search_buffer_, 0, sizeof(search_buffer_));
}

Application::~Application() {
    shutdown();
}

bool Application::initialize() {
    // Initialize logger
    Logger::init("/tmp/youtube-tui.log");
    Logger::info("Initializing YouTube TUI");

    // Load config
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
    player_->set_volume(Config::instance().get_volume());

    // Initialize ImTui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

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
    ImTui_ImplNcurses_NewFrame();
    ImTui_ImplText_NewFrame();
    ImGui::NewFrame();

    // Main window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::Begin("YouTube TUI", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoCollapse);

    render_menu_bar();

    if (show_search_) {
        render_search_view();
    }

    render_queue_view();
    render_player_view();
    render_status_bar();

    ImGui::End();

    ImGui::Render();
    ImTui_ImplText_RenderDrawData(ImGui::GetDrawData(), screen_);
    ImTui_ImplNcurses_DrawScreen();
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
    ImGui::Text("Search YouTube:");

    if (ImGui::InputText("##search", search_buffer_, sizeof(search_buffer_),
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
        search_youtube(search_buffer_);
    }

    ImGui::SameLine();
    if (ImGui::Button("Search") && std::strlen(search_buffer_) > 0) {
        search_youtube(search_buffer_);
    }

    if (incremental_search_active_) {
        ImGui::Text("Searching...");
    }

    // Show incremental results as they come in
    if (incremental_search_active_ || !incremental_results_.empty()) {
        ImGui::Text("Results:");
        ImGui::BeginChild("SearchResults", ImVec2(0, 10), true);

        std::lock_guard<std::mutex> lock(search_mutex_);
        for (size_t i = 0; i < incremental_results_.size(); ++i) {
            const auto& track = incremental_results_[i];
            bool selected = (i == static_cast<size_t>(selected_search_result_));

            if (ImGui::Selectable(track.title.c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick)) {
                selected_search_result_ = static_cast<int>(i);
                if (ImGui::IsMouseDoubleClicked(0)) {
                    state_manager_->add_to_queue(track);
                    Logger::info("Added to queue: " + track.title);
                }
            }
        }

        ImGui::EndChild();

        if (ImGui::Button("Add to Queue") && selected_search_result_ >= 0 &&
            selected_search_result_ < static_cast<int>(incremental_results_.size())) {
            std::lock_guard<std::mutex> lock(search_mutex_);
            state_manager_->add_to_queue(incremental_results_[selected_search_result_]);
        }
    }

    ImGui::Separator();
}

void Application::render_queue_view() {
    ImGui::Text("Queue:");
    ImGui::BeginChild("Queue", ImVec2(0, 15), true);

    const auto& queue = state_manager_->get_queue();
    for (size_t i = 0; i < queue.size(); ++i) {
        const auto& track = queue[i];
        bool is_current = (i == static_cast<size_t>(state_manager_->get_current_index()));
        bool selected = (i == static_cast<size_t>(selected_queue_item_));

        std::string label = (is_current ? "> " : "  ") + track.title;

        if (ImGui::Selectable(label.c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick)) {
            selected_queue_item_ = static_cast<int>(i);
            if (ImGui::IsMouseDoubleClicked(0)) {
                state_manager_->play_at_index(i);
                play_current_track();
            }
        }
    }

    ImGui::EndChild();
}

void Application::render_player_view() {
    ImGui::Separator();
    ImGui::Text("Now Playing:");

    const Track* current = state_manager_->get_current_track();
    if (current) {
        ImGui::Text("%s", current->title.c_str());
        ImGui::Text("Channel: %s", current->channel.c_str());

        double pos = player_->get_position();
        double dur = player_->get_duration();

        ImGui::ProgressBar(dur > 0 ? static_cast<float>(pos / dur) : 0.0f);
        ImGui::Text("%.0f:%.0f / %.0f:%.0f",
                   std::floor(pos / 60), std::fmod(pos, 60),
                   std::floor(dur / 60), std::fmod(dur, 60));
    } else {
        ImGui::Text("No track playing");
    }

    // Controls
    if (ImGui::Button("Previous")) previous_track();
    ImGui::SameLine();

    PlaybackState state = player_->get_state();
    if (state == PlaybackState::Playing) {
        if (ImGui::Button("Pause")) player_->pause();
    } else {
        if (ImGui::Button("Play")) player_->play();
    }

    ImGui::SameLine();
    if (ImGui::Button("Next")) next_track();

    // Volume control
    float volume = static_cast<float>(player_->get_volume());
    if (ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f)) {
        player_->set_volume(volume);
        Config::instance().set_volume(volume);
    }
}

void Application::render_status_bar() {
    ImGui::Separator();
    PlaybackState state = player_->get_state();
    const char* state_str = "Stopped";
    switch (state) {
        case PlaybackState::Playing: state_str = "Playing"; break;
        case PlaybackState::Paused: state_str = "Paused"; break;
        case PlaybackState::Buffering: state_str = "Buffering"; break;
        default: break;
    }
    ImGui::Text("Status: %s | Queue: %zu tracks", state_str, state_manager_->get_queue().size());
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
        extractor_->search_streaming(query, 10, [this](const Track& track) {
            // This callback will be called for each result as it comes in
            std::lock_guard<std::mutex> lock(search_mutex_);
            incremental_results_.push_back(track);
        });
        // Mark search as complete
        std::lock_guard<std::mutex> lock(search_mutex_);
        search_in_progress_ = false;
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

} // namespace ytui
