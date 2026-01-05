#include "youtube/extractor.h"
#include "utils/logger.h"
#include "core/config.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <array>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

namespace ytui {

YouTubeExtractor::YouTubeExtractor() : ytdlp_path_("yt-dlp") {
    // TODO: Could add logic to find yt-dlp or youtube-dl in PATH
}

std::string YouTubeExtractor::execute_ytdlp(const std::vector<std::string>& args) {
    std::string command = ytdlp_path_;
    for (const auto& arg : args) {
        command += " " + arg;
    }
    command += " 2>/dev/null";

    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("Failed to execute yt-dlp");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

Track YouTubeExtractor::extract_info(const std::string& url) {
    try {
        std::vector<std::string> args = {
            "--dump-json",
            "--no-playlist",
            "--quiet",
            "--js-runtimes", "deno",
            "--cookies", "../www.youtube.com_cookies.txt",
            url
        };

        std::string json_output = execute_ytdlp(args);
        return parse_track_json(json_output);
    } catch (const std::exception& e) {
        Logger::error("Failed to extract info: " + std::string(e.what()));
        return Track();
    }
}

std::future<Track> YouTubeExtractor::extract_info_async(const std::string& url) {
    return std::async(std::launch::async, [this, url]() {
        return extract_info(url);
    });
}

    std::vector<Track> YouTubeExtractor::search(const std::string& query, int max_results) {
    Logger::info("Starting YouTube API search for: " + query);
    try {
        std::string access_token = get_access_token();
        if (access_token.empty()) {
            Logger::error("No access token available for YouTube API search, falling back to yt-dlp");
            // Fallback to yt-dlp search
            std::string search_query = "ytsearch" + std::to_string(max_results) + ":" + query;
            std::vector<std::string> args = {
                "--dump-json",
                "--quiet",
                "--no-warnings",
                "--js-runtimes", "deno",
                "--cookies", "../www.youtube.com_cookies.txt",
                search_query
            };
            std::string json_output = execute_ytdlp(args);
            return parse_search_results(json_output);
        }

        Logger::info("Using YouTube API for search with access token");
        // YouTube Data API v3 search
        std::string api_url = "https://www.googleapis.com/youtube/v3/search?part=snippet&type=video&maxResults=" +
                             std::to_string(max_results) + "&q=" + url_encode(query);

        Logger::info("YouTube API URL: " + api_url);
        Logger::info("Making YouTube API call with access token");
        std::string response = http_client_.get(api_url, access_token);
        Logger::info("YouTube API response length: " + std::to_string(response.length()));

        if (response.empty()) {
            Logger::error("Empty response from YouTube Data API, falling back to yt-dlp");
            // Fallback to yt-dlp
            std::string search_query = "ytsearch" + std::to_string(max_results) + ":" + query;
            std::vector<std::string> args = {
                "--dump-json",
                "--quiet",
                "--no-warnings",
                "--js-runtimes", "deno",
                "--cookies", "../www.youtube.com_cookies.txt",
                search_query
            };
            std::string json_output = execute_ytdlp(args);
            return parse_search_results(json_output);
        }

        Logger::info("YouTube API search response received, parsing results");
        return parse_youtube_api_search_results(response);
    } catch (const std::exception& e) {
        Logger::error("YouTube API search failed: " + std::string(e.what()) + ", falling back to yt-dlp");
        // Fallback to yt-dlp search
        std::string search_query = "ytsearch" + std::to_string(max_results) + ":" + query;
        std::vector<std::string> args = {
            "--dump-json",
            "--quiet",
            "--no-warnings",
            "--js-runtimes", "deno",
            "--cookies", "../www.youtube.com_cookies.txt",
            search_query
        };
        std::string json_output = execute_ytdlp(args);
        return parse_search_results(json_output);
    }
}

std::future<std::vector<Track>> YouTubeExtractor::search_async(const std::string& query, int max_results) {
    return std::async(std::launch::async, [this, query, max_results]() {
        return search(query, max_results);
    });
}

    void YouTubeExtractor::search_streaming(const std::string& query, int max_results,
                                        std::function<void(const Track&)> result_callback) {
    Logger::info("search_streaming called with query: " + query);
    try {
        // Use YouTube API for streaming search
        std::vector<Track> results = search(query, max_results);
        Logger::info("search_streaming got " + std::to_string(results.size()) + " results");

        for (const auto& track : results) {
            result_callback(track);
        }
    } catch (const std::exception& e) {
        Logger::error("Streaming search failed: " + std::string(e.what()));
    }
}

    std::string YouTubeExtractor::get_stream_url(const std::string& video_url) {
    try {
        std::vector<std::string> args = {
            "--get-url",
            "--format", "bestaudio",
            "--no-playlist",
            "--quiet",
            "--js-runtimes", "deno",
            "--cookies", "../www.youtube.com_cookies.txt",
            video_url
        };

        std::string url = execute_ytdlp(args);

        // Remove trailing newline
        if (!url.empty() && url.back() == '\n') {
            url.pop_back();
        }

        return url;
    } catch (const std::exception& e) {
        Logger::error("Failed to get stream URL: " + std::string(e.what()));
        return "";
    }
}

Track YouTubeExtractor::parse_track_json(const std::string& json) {
    Track track;

    // Simple string-based JSON parsing for key fields
    // This extracts: id, title, uploader, duration, is_live, webpage_url

    // Extract id
    size_t id_pos = json.find("\"id\":");
    if (id_pos != std::string::npos) {
        size_t start = json.find('"', id_pos + 6);
        size_t end = json.find('"', start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            track.id = json.substr(start + 1, end - start - 1);
        }
    }

    // Extract title
    size_t title_pos = json.find("\"title\":");
    if (title_pos != std::string::npos) {
        size_t start = json.find('"', title_pos + 9);
        size_t end = json.find('"', start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            std::string title = json.substr(start + 1, end - start - 1);
            // Decode Unicode escapes
            track.title = decode_unicode_escapes(title);
        }
    }

    // Extract uploader (channel)
    size_t uploader_pos = json.find("\"uploader\":");
    if (uploader_pos != std::string::npos) {
        size_t start = json.find('"', uploader_pos + 12);
        size_t end = json.find('"', start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            track.channel = json.substr(start + 1, end - start - 1);
        }
    }

    // Extract duration
    size_t duration_pos = json.find("\"duration\":");
    if (duration_pos != std::string::npos) {
        size_t start = json.find_first_not_of(" \t", duration_pos + 12);
        size_t end = json.find_first_of(",}", start);
        if (start != std::string::npos && end != std::string::npos) {
            try {
                track.duration = std::stod(json.substr(start, end - start));
            } catch (const std::exception&) {
                track.duration = 0;
            }
        }
    }

    // Check if it's live
    size_t live_pos = json.find("\"is_live\":");
    if (live_pos != std::string::npos) {
        size_t start = json.find_first_not_of(" \t", live_pos + 11);
        size_t end = json.find_first_of(",}", start);
        if (start != std::string::npos && end != std::string::npos) {
            std::string live_str = json.substr(start, end - start);
            track.is_live = (live_str.find("true") != std::string::npos);
        }
    }

    // Extract webpage_url as the main URL
    size_t url_pos = json.find("\"webpage_url\":");
    if (url_pos != std::string::npos) {
        size_t start = json.find('"', url_pos + 15);
        size_t end = json.find('"', start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            track.url = json.substr(start + 1, end - start - 1);
        }
    }

    return track;
}

std::vector<Track> YouTubeExtractor::parse_youtube_api_search_results(const std::string& json) {
    std::vector<Track> results;

    try {
        nlohmann::json api_response = nlohmann::json::parse(json);

        if (!api_response.contains("items")) {
            Logger::error("YouTube API response missing 'items' field");
            return results;
        }

        for (const auto& item : api_response["items"]) {
            Track track;

            if (item.contains("id") && item["id"].contains("videoId")) {
                track.id = item["id"]["videoId"];
            }

            if (item.contains("snippet")) {
                const auto& snippet = item["snippet"];

                if (snippet.contains("title")) {
                    track.title = snippet["title"];
                }

                if (snippet.contains("channelTitle")) {
                    track.channel = snippet["channelTitle"];
                }

                if (snippet.contains("liveBroadcastContent")) {
                    std::string live_status = snippet["liveBroadcastContent"];
                    track.is_live = (live_status == "live");
                }

                if (!track.id.empty()) {
                    track.url = "https://www.youtube.com/watch?v=" + track.id;
                }
            }

            if (!track.id.empty()) {
                results.push_back(track);
            }
        }
    } catch (const std::exception& e) {
        Logger::error("Failed to parse YouTube API search results: " + std::string(e.what()));
    }

    return results;
}

std::vector<Track> YouTubeExtractor::parse_search_results(const std::string& json) {
    std::vector<Track> results;

    // Split the JSON output by newlines (each line is a separate JSON object)
    std::istringstream iss(json);
    std::string line;

    while (std::getline(iss, line)) {
        if (line.empty()) continue;

        Track track = parse_single_search_result(line);
        if (!track.id.empty()) {
            results.push_back(track);
        }
    }

    return results;
}

Track YouTubeExtractor::parse_single_search_result(const std::string& json_line) {
    Track track;

    // Simple string-based JSON parsing for key fields
    // This extracts: id, title, uploader, duration, webpage_url

    // Extract id
    size_t id_pos = json_line.find("\"id\":");
    if (id_pos != std::string::npos) {
        size_t start = json_line.find('"', id_pos + 6);
        size_t end = json_line.find('"', start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            track.id = json_line.substr(start + 1, end - start - 1);
        }
    }

    // Extract title
    size_t title_pos = json_line.find("\"title\":");
    if (title_pos != std::string::npos) {
        size_t start = json_line.find('"', title_pos + 9);
        size_t end = json_line.find('"', start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            std::string title = json_line.substr(start + 1, end - start - 1);
            // Decode Unicode escapes like \u2615\ufe0f
            track.title = decode_unicode_escapes(title);
        }
    }

    // Extract uploader
    size_t uploader_pos = json_line.find("\"uploader\":");
    if (uploader_pos != std::string::npos) {
        size_t start = json_line.find('"', uploader_pos + 12);
        size_t end = json_line.find('"', start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            track.channel = json_line.substr(start + 1, end - start - 1);
        }
    }

    // Extract duration
    size_t duration_pos = json_line.find("\"duration\":");
    if (duration_pos != std::string::npos) {
        size_t start = json_line.find_first_not_of(" \t", duration_pos + 12);
        size_t end = json_line.find_first_of(",}", start);
        if (start != std::string::npos && end != std::string::npos) {
            try {
                track.duration = std::stod(json_line.substr(start, end - start));
            } catch (const std::exception&) {
                track.duration = 0;
            }
        }
    }

    // Extract webpage_url
    size_t url_pos = json_line.find("\"webpage_url\":");
    if (url_pos != std::string::npos) {
        size_t start = json_line.find('"', url_pos + 15);
        size_t end = json_line.find('"', start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            track.url = json_line.substr(start + 1, end - start - 1);
        }
    }

    // Extract is_live
    size_t live_pos = json_line.find("\"is_live\":");
    if (live_pos != std::string::npos) {
        size_t start = json_line.find_first_not_of(" \t", live_pos + 10);
        if (start != std::string::npos) {
            std::string live_str = json_line.substr(start, 4);
            track.is_live = (live_str == "true");
        }
    }

    return track;
}

std::string YouTubeExtractor::decode_unicode_escapes(const std::string& input) {
    std::string result = input;

    // Simple Unicode escape decoding (\uXXXX)
    size_t pos = 0;
    while ((pos = result.find("\\u", pos)) != std::string::npos) {
        if (pos + 5 < result.size()) {
            std::string hex = result.substr(pos + 2, 4);
            try {
                int code = std::stoi(hex, nullptr, 16);
                if (code >= 0x20 && code <= 0x7E) {
                    // ASCII range
                    result.replace(pos, 6, 1, static_cast<char>(code));
                } else {
                    // For now, just remove the escape for non-ASCII
                    result.erase(pos, 6);
                }
            } catch (const std::exception&) {
                pos += 2; // Skip this invalid escape
            }
        } else {
            break;
        }
    }

    return result;
}

bool YouTubeExtractor::authenticate_oauth() {
    // Implementation based on Google OAuth 2.0 Device Authorization Grant
    // https://developers.google.com/identity/protocols/oauth2/limited-input-device

    // Check if client ID is configured
    std::string client_id = Config::instance().get_youtube_client_id();
    if (client_id.empty()) {
        Logger::error("YouTube OAuth client ID not configured");
        return false;
    }

    // Step 1: Request device and user codes
    std::string device_url = "https://oauth2.googleapis.com/device/code";
    std::string device_data = "client_id=" + client_id + "&scope=https://www.googleapis.com/auth/youtube.readonly";

    std::string device_response = http_client_.post(device_url, device_data);

    if (device_response.empty()) {
        Logger::error("Failed to get device code - empty response");
        return false;
    }

    Logger::info("Device code response: " + device_response);

    nlohmann::json device_json;
    try {
        device_json = nlohmann::json::parse(device_response);
    } catch (const std::exception& e) {
        Logger::error("Failed to parse device code response: " + std::string(e.what()));
        return false;
    }

    if (!device_json.contains("device_code") || !device_json.contains("user_code") ||
        !device_json.contains("verification_url")) {
        Logger::error("Invalid device code response format");
        return false;
    }

    std::string device_code = device_json["device_code"];
    std::string user_code = device_json["user_code"];
    std::string verification_url = device_json["verification_url"];

    // Step 2: Tell user to authorize
    std::cout << "Please visit: " << verification_url << std::endl;
    std::cout << "Enter this code when prompted: " << user_code << std::endl;
    std::cout << std::endl;
    std::cout << "Complete the authorization in your browser." << std::endl;
    std::cout << "This program will automatically check for completion every 5 seconds..." << std::endl;
    std::cout << std::endl;

    // Step 3: Poll for token
    std::string token_url = "https://oauth2.googleapis.com/token";
    std::string client_secret = Config::instance().get_youtube_client_secret();
    std::string token_data = "client_id=" + client_id + "&device_code=" + device_code +
                            "&grant_type=urn:ietf:params:oauth:grant-type:device_code";
    if (!client_secret.empty()) {
        token_data += "&client_secret=" + client_secret;
    }

    for (int i = 0; i < 60; ++i) {  // Poll for up to 5 minutes
        std::cout << "Checking authorization status... (" << (i + 1) << "/60)" << std::endl;

        std::string token_response = http_client_.post(token_url, token_data);
        Logger::info("Token polling response: " + token_response);

        if (!token_response.empty()) {
            try {
                nlohmann::json token_json = nlohmann::json::parse(token_response);

                if (token_json.contains("access_token")) {
                    std::string access_token = token_json["access_token"];
                    std::string refresh_token = token_json.value("refresh_token", "");

                    Config::instance().set_youtube_access_token(access_token);
                    if (!refresh_token.empty()) {
                        Config::instance().set_youtube_refresh_token(refresh_token);
                    }
                    Config::instance().save();

                    std::cout << std::endl << "✓ Authorization successful!" << std::endl;
                    Logger::info("YouTube OAuth authentication successful");
                    return true;
                } else if (token_json.contains("error")) {
                    std::string error = token_json["error"];
                    Logger::info("Token polling error: " + error);

                    if (error == "authorization_pending") {
                        std::cout << "Still waiting for authorization..." << std::endl;
                    } else if (error == "slow_down") {
                        std::cout << "Slowing down polling..." << std::endl;
                        std::this_thread::sleep_for(std::chrono::seconds(10));
                        continue;
                    } else if (error == "access_denied") {
                        std::cout << "Authorization denied by user." << std::endl;
                        return false;
                    } else if (error == "invalid_grant") {
                        std::cout << "Invalid device code or authorization expired." << std::endl;
                        return false;
                    } else {
                        std::cout << "Authorization error: " << error << std::endl;
                    }
                } else {
                    Logger::error("Unexpected token response format");
                }
            } catch (const std::exception& e) {
                Logger::error("Failed to parse token response: " + std::string(e.what()));
            }
        } else {
            Logger::error("Empty token response from polling");
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    Logger::error("YouTube OAuth authentication timed out");
    return false;
}

bool YouTubeExtractor::refresh_access_token() {
    std::string refresh_token = Config::instance().get_youtube_refresh_token();
    std::string client_id = Config::instance().get_youtube_client_id();

    if (refresh_token.empty() || client_id.empty()) {
        return false;
    }

    std::string token_url = "https://oauth2.googleapis.com/token";
    std::string client_secret = Config::instance().get_youtube_client_secret();
    std::string token_data = "client_id=" + client_id + "&refresh_token=" + refresh_token +
                            "&grant_type=refresh_token";
    if (!client_secret.empty()) {
        token_data += "&client_secret=" + client_secret;
    }

    std::string token_response = http_client_.post(token_url, token_data);

    if (!token_response.empty()) {
        try {
            nlohmann::json token_json = nlohmann::json::parse(token_response);

            if (token_json.contains("access_token")) {
                std::string access_token = token_json["access_token"];

                Config::instance().set_youtube_access_token(access_token);
                Config::instance().save();

                Logger::info("YouTube access token refreshed successfully");
                return true;
            } else if (token_json.contains("error")) {
                Logger::error("Token refresh failed: " + std::string(token_json["error"]));
            }
        } catch (const std::exception& e) {
            Logger::error("Failed to parse refresh token response: " + std::string(e.what()));
        }
    }

    return false;
}

    std::string YouTubeExtractor::get_access_token() {
    std::string access_token = Config::instance().get_youtube_access_token();

    Logger::info(std::string("Checking for access token: ") + (access_token.empty() ? "NOT FOUND" : "FOUND"));

    if (!access_token.empty()) {
        // TODO: Check if token is expired and refresh if needed
        return access_token;
    }

    std::string client_id = Config::instance().get_youtube_client_id();
    if (client_id.empty()) {
        Logger::error("No YouTube client ID configured");
        return "";
    }

    // No token, try to authenticate
    if (authenticate_oauth()) {
        return Config::instance().get_youtube_access_token();
    }

    return "";
}

std::string YouTubeExtractor::url_encode(const std::string& input) {
    std::string result;
    CURL* curl = curl_easy_init();
    if (curl) {
        char* encoded = curl_easy_escape(curl, input.c_str(), input.length());
        if (encoded) {
            result = encoded;
            curl_free(encoded);
        }
        curl_easy_cleanup(curl);
    }
    return result;
}

} // namespace ytui
