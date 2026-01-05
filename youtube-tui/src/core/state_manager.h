#pragma once

#include "track.h"
#include <deque>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace ytui {

class StateManager {
public:
    StateManager();

    // Queue management
    void add_to_queue(const Track& track);
    void add_to_queue(const std::vector<Track>& tracks);
    void remove_from_queue(size_t index);
    void clear_queue();
    void move_in_queue(size_t from, size_t to);

    // Playback control
    void play_next();
    void play_previous();
    void play_at_index(size_t index);

    // Getters
    const std::deque<Track>& get_queue() const { return queue_; }
    const Track* get_current_track() const;
    int get_current_index() const { return current_index_; }

    // History
    void add_to_history(const Track& track);
    const std::vector<Track>& get_history() const { return history_; }

    // Saved playlists
    void save_playlist(const Playlist& playlist);
    void delete_playlist(const std::string& id);
    Playlist* get_playlist(const std::string& id);
    const std::unordered_map<std::string, Playlist>& get_playlists() const { return playlists_; }
    void load_playlist_to_queue(const std::string& id);
    void save_queue_as_playlist(const std::string& name);

    // Persistence
    void load_playlists();
    void save_playlists();

    // Shuffle and repeat
    void set_shuffle(bool enabled);
    void set_repeat(bool enabled);
    bool is_shuffle_enabled() const { return shuffle_enabled_; }
    bool is_repeat_enabled() const { return repeat_enabled_; }

private:
    std::deque<Track> queue_;
    std::vector<Track> history_;
    std::unordered_map<std::string, Playlist> playlists_;
    int current_index_;
    bool shuffle_enabled_;
    bool repeat_enabled_;
    mutable std::mutex mutex_;

    // Helper methods
    std::string get_playlists_path() const;
    std::string escape_json_string(const std::string& str) const;

    size_t get_next_index() const;
    size_t get_previous_index() const;
};

} // namespace ytui
