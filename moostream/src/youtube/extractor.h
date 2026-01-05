#pragma once

#include "track.h"
#include <vector>
#include <string>
#include <future>

namespace ytui {

class YouTubeExtractor {
public:
    YouTubeExtractor();

    // Synchronous operations
    Track extract_info(const std::string& url);
    std::vector<Track> search(const std::string& query, int max_results = 10);

    // Asynchronous operations
    std::future<Track> extract_info_async(const std::string& url);
    std::future<std::vector<Track>> search_async(const std::string& query, int max_results = 10);

    // Streaming search that processes results incrementally
    void search_streaming(const std::string& query, int max_results,
                         std::function<void(const Track&)> result_callback);

    // Get direct stream URL from video
    std::string get_stream_url(const std::string& video_url);

private:
    std::string execute_ytdlp(const std::vector<std::string>& args);
    Track parse_track_json(const std::string& json);
    std::vector<Track> parse_search_results(const std::string& json);
    Track parse_single_search_result(const std::string& json_line);
    std::string decode_unicode_escapes(const std::string& input);

    std::string ytdlp_path_;
};

} // namespace ytui
