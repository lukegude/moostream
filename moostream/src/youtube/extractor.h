#pragma once

#include "track.h"
#include "http_client.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <future>
#include <map>

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

    // OAuth authentication
    std::string get_access_token();

private:
    std::string execute_ytdlp(const std::vector<std::string>& args);
    Track parse_track_json(const std::string& json);
    std::vector<Track> parse_search_results(const std::string& json);
    std::vector<Track> parse_youtube_api_search_results(const std::string& json);
    Track parse_single_search_result(const std::string& json_line);
    std::string decode_unicode_escapes(const std::string& input);
    std::string url_encode(const std::string& input);

    // YouTube API methods
    std::string build_search_url(const std::string& query, int max_results);
    std::string build_video_url(const std::string& video_id);
    Track parse_api_search_item(const nlohmann::json& item);
    Track parse_api_video_item(const nlohmann::json& item);

    // OAuth methods
    bool authenticate_oauth();
    bool refresh_access_token();

    std::string ytdlp_path_;
    HttpClient http_client_;
    std::map<std::string, std::string> stream_url_cache_;  // Cache for stream URLs
};

} // namespace ytui
