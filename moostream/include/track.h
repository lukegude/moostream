#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <cstddef>

namespace ytui {

struct Track {
    std::string id;              // YouTube video ID
    std::string url;             // Full YouTube URL
    std::string title;           // Video title
    std::string channel;         // Channel name
    int duration;                // Duration in seconds (0 for livestreams)
    bool is_live;                // Is this a livestream?
    std::string thumbnail_url;   // Thumbnail URL
    std::string stream_url;      // Extracted audio stream URL (filled by extractor)

    Track() : duration(0), is_live(false) {}

    bool is_valid() const {
        return !id.empty() && !title.empty();
    }
};

struct Playlist {
    std::string name;
    std::string id;  // Unique identifier
    std::vector<Track> tracks;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point modified_at;

    Playlist() : created_at(std::chrono::system_clock::now()),
                 modified_at(std::chrono::system_clock::now()) {}

    Playlist(const std::string& name) : name(name),
                                       id(std::to_string(std::chrono::system_clock::now().time_since_epoch().count())),
                                       created_at(std::chrono::system_clock::now()),
                                       modified_at(std::chrono::system_clock::now()) {}

    bool is_valid() const {
        return !name.empty() && !id.empty();
    }

    size_t size() const {
        return tracks.size();
    }

    void add_track(const Track& track) {
        tracks.push_back(track);
        modified_at = std::chrono::system_clock::now();
    }

    void remove_track(size_t index) {
        if (index < tracks.size()) {
            tracks.erase(tracks.begin() + index);
            modified_at = std::chrono::system_clock::now();
        }
    }
};

} // namespace ytui
