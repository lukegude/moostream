#pragma once

#include "track.h"
#include <deque>
#include <vector>
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

    // Shuffle and repeat
    void set_shuffle(bool enabled);
    void set_repeat(bool enabled);
    bool is_shuffle_enabled() const { return shuffle_enabled_; }
    bool is_repeat_enabled() const { return repeat_enabled_; }

private:
    std::deque<Track> queue_;
    std::vector<Track> history_;
    int current_index_;
    bool shuffle_enabled_;
    bool repeat_enabled_;
    mutable std::mutex mutex_;

    size_t get_next_index() const;
    size_t get_previous_index() const;
};

} // namespace ytui
