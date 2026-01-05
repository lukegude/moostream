#pragma once

#include <string>
#include <chrono>

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

} // namespace ytui
