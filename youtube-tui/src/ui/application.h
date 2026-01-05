#pragma once

#include "player_interface.h"
#include "youtube/extractor.h"
#include "core/state_manager.h"
#include <memory>
#include <string>
#include <mutex>
#include <thread>

namespace ImTui {
struct TScreen;
}

namespace ytui {

class Application {
public:
    Application();
    ~Application();

    bool initialize();
    void run();
    void shutdown();

private:
    void update();
    void render();
    void handle_input();

    // UI rendering
    void render_menu_bar();
    void render_search_view();
    void render_queue_view();
    void render_player_view();
    void render_status_bar();

    // Commands
    void search_youtube(const std::string& query);
    void play_url(const std::string& url);
    void play_current_track();
    void next_track();
    void previous_track();

    // State
    std::unique_ptr<IPlayer> player_;
    std::unique_ptr<YouTubeExtractor> extractor_;
    std::unique_ptr<StateManager> state_manager_;

    // ImTui state
    ImTui::TScreen* screen_;

    // UI state
    char search_buffer_[256];
    std::vector<Track> search_results_;
    int selected_search_result_;
    int selected_queue_item_;
    bool show_search_;
    bool running_;

    // Async search
    std::future<std::vector<Track>> search_future_;
    bool search_in_progress_;

    // Incremental search results
    std::vector<Track> incremental_results_;
    bool incremental_search_active_;
    std::mutex search_mutex_;
};

} // namespace ytui
