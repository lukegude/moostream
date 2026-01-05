#include "core/state_manager.h"
#include "utils/logger.h"
#include <algorithm>
#include <random>

namespace ytui {

StateManager::StateManager()
    : current_index_(-1), shuffle_enabled_(false), repeat_enabled_(false) {
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

} // namespace ytui
