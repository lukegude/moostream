#include "core/state_manager.h"
#include "utils/logger.h"
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace ytui {

StateManager::StateManager()
    : current_index_(-1), shuffle_enabled_(false), repeat_enabled_(false) {
    load_playlists();
}

void StateManager::add_to_queue(const Track& track) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push_back(track);
    Logger::info("Added to queue: " + track.title);
}

void StateManager::add_to_queue(const std::vector<Track>& tracks) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& track : tracks) {
        queue_.push_back(track);
    }
    Logger::info("Added " + std::to_string(tracks.size()) + " tracks to queue");
}

void StateManager::remove_from_queue(size_t index) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (index >= queue_.size()) return;

    queue_.erase(queue_.begin() + index);

    if (current_index_ >= static_cast<int>(index)) {
        current_index_--;
    }
}

void StateManager::clear_queue() {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.clear();
    current_index_ = -1;
}

void StateManager::move_in_queue(size_t from, size_t to) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (from >= queue_.size() || to >= queue_.size()) return;

    Track track = queue_[from];
    queue_.erase(queue_.begin() + from);
    queue_.insert(queue_.begin() + to, track);

    // Update current index if needed
    if (current_index_ == static_cast<int>(from)) {
        current_index_ = static_cast<int>(to);
    } else if (current_index_ > static_cast<int>(from) && current_index_ <= static_cast<int>(to)) {
        current_index_--;
    } else if (current_index_ < static_cast<int>(from) && current_index_ >= static_cast<int>(to)) {
        current_index_++;
    }
}

void StateManager::play_next() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) return;

    current_index_ = get_next_index();
}

void StateManager::play_previous() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) return;

    current_index_ = get_previous_index();
}

void StateManager::play_at_index(size_t index) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (index >= queue_.size()) return;

    current_index_ = static_cast<int>(index);
}

const Track* StateManager::get_current_track() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (current_index_ < 0 || current_index_ >= static_cast<int>(queue_.size())) {
        return nullptr;
    }
    return &queue_[current_index_];
}

void StateManager::add_to_history(const Track& track) {
    std::lock_guard<std::mutex> lock(mutex_);
    history_.push_back(track);

    // Keep history limited to last 100 tracks
    if (history_.size() > 100) {
        history_.erase(history_.begin());
    }
}

void StateManager::set_shuffle(bool enabled) {
    shuffle_enabled_ = enabled;
    Logger::info(std::string("Shuffle ") + (enabled ? "enabled" : "disabled"));
}

void StateManager::set_repeat(bool enabled) {
    repeat_enabled_ = enabled;
    Logger::info(std::string("Repeat ") + (enabled ? "enabled" : "disabled"));
}

size_t StateManager::get_next_index() const {
    if (queue_.empty()) return 0;

    if (shuffle_enabled_) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, queue_.size() - 1);
        return dis(gen);
    }

    int next = current_index_ + 1;
    if (next >= static_cast<int>(queue_.size())) {
        return repeat_enabled_ ? 0 : current_index_;
    }
    return next;
}

size_t StateManager::get_previous_index() const {
    if (queue_.empty()) return 0;

    int prev = current_index_ - 1;
    if (prev < 0) {
        return repeat_enabled_ ? queue_.size() - 1 : 0;
    }
    return prev;
}

void StateManager::save_playlist(const Playlist& playlist) {
    std::lock_guard<std::mutex> lock(mutex_);
    playlists_[playlist.id] = playlist;
    Logger::info("Saved playlist: " + playlist.name);
    save_playlists();
}

void StateManager::delete_playlist(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = playlists_.find(id);
    if (it != playlists_.end()) {
        Logger::info("Deleted playlist: " + it->second.name);
        playlists_.erase(it);
        save_playlists();
    }
}

Playlist* StateManager::get_playlist(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = playlists_.find(id);
    return it != playlists_.end() ? &it->second : nullptr;
}

void StateManager::load_playlist_to_queue(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = playlists_.find(id);
    if (it != playlists_.end()) {
        const auto& playlist = it->second;
        queue_.clear();
        for (const auto& track : playlist.tracks) {
            queue_.push_back(track);
        }
        current_index_ = -1;
        Logger::info("Loaded playlist to queue: " + playlist.name);
    }
}

void StateManager::save_queue_as_playlist(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    Playlist playlist(name);
    for (const auto& track : queue_) {
        playlist.add_track(track);
    }
    playlists_[playlist.id] = playlist;
    Logger::info("Saved queue as playlist: " + name);
    save_playlists();
}

std::string StateManager::get_playlists_path() const {
    const char* home = std::getenv("HOME");
    if (!home) {
        return "playlists.json";
    }
    return std::string(home) + "/.config/youtube-tui/playlists.json";
}

void StateManager::load_playlists() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string path = get_playlists_path();
    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::info("No playlists file found, starting with empty playlists");
        return;
    }

    // Simple JSON parsing for playlists
    // Format: {"playlists":[{"name":"...", "id":"...", "tracks":[{...}, ...]}, ...]}
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Basic parsing - find playlist objects
    playlists_.clear();

    size_t pos = 0;
    while ((pos = content.find("\"name\":\"", pos)) != std::string::npos) {
        // Extract playlist data - simplified parsing
        Playlist playlist;
        // This is placeholder - proper JSON parsing would be better
        // For now, just initialize empty playlists
        Logger::info("Loaded playlist (parsing placeholder): " + playlist.name);
        pos += 8;
    }

    Logger::info("Loaded " + std::to_string(playlists_.size()) + " playlists");
}

void StateManager::save_playlists() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string path = get_playlists_path();

    // Create directory if needed
    size_t dir_pos = path.find_last_of('/');
    if (dir_pos != std::string::npos) {
        std::string dir = path.substr(0, dir_pos);
        std::string mkdir_cmd = "mkdir -p " + dir;
        system(mkdir_cmd.c_str());
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        Logger::error("Failed to save playlists to: " + path);
        return;
    }

    // Simple JSON serialization
    file << "{\"playlists\":[";

    bool first = true;
    for (const auto& pair : playlists_) {
        if (!first) file << ",";
        first = false;

        const Playlist& playlist = pair.second;
        file << "{\"name\":\"" << escape_json_string(playlist.name) << "\",";
        file << "\"id\":\"" << playlist.id << "\",";
        file << "\"tracks\":[";

        bool first_track = true;
        for (const auto& track : playlist.tracks) {
            if (!first_track) file << ",";
            first_track = false;

            file << "{\"id\":\"" << track.id << "\",";
            file << "\"url\":\"" << escape_json_string(track.url) << "\",";
            file << "\"title\":\"" << escape_json_string(track.title) << "\",";
            file << "\"channel\":\"" << escape_json_string(track.channel) << "\",";
            file << "\"duration\":" << track.duration << ",";
            file << "\"is_live\":" << (track.is_live ? "true" : "false") << ",";
            file << "\"thumbnail_url\":\"" << escape_json_string(track.thumbnail_url) << "\"}";
        }

        file << "]}";
    }

    file << "]}";
    file.close();
    Logger::info("Saved " + std::to_string(playlists_.size()) + " playlists to: " + path);
}

std::string StateManager::escape_json_string(const std::string& str) const {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    return result;
}

} // namespace ytui
