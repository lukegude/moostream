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
#include <ctime>

namespace ytui {

YouTubeExtractor::YouTubeExtractor() {
}



Track YouTubeExtractor::extract_info(const std::string& url) {
    Logger::error("extract_info not implemented - YouTube API does not provide stream URLs");
    return Track();
}

std::future<Track> YouTubeExtractor::extract_info_async(const std::string& url) {
    return std::async(std::launch::async, [this, url]() {
        return extract_info(url);
    });
}

    std::vector<Track> YouTubeExtractor::search(const std::string& query, int max_results) {
    Logger::info("Starting YouTube API search for: '" + query + "'");
    try {
        std::string access_token = get_access_token();
        if (access_token.empty()) {
            Logger::error("No access token available for YouTube API search");
            return {};
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
            Logger::error("Empty response from YouTube Data API");
            return {};
        }

        Logger::info("YouTube API search response received, parsing results");
        return parse_youtube_api_search_results(response);
    } catch (const std::exception& e) {
        Logger::error("YouTube API search failed: " + std::string(e.what()));
        return {};
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
    Logger::error("get_stream_url not implemented - YouTube API does not provide stream URLs");
    return "";
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
                     std::string raw_title = snippet["title"];
                     Logger::info("YouTube API raw title: '" + raw_title + "'");
                     track.title = sanitize_title(raw_title);
                     Logger::info("YouTube API sanitized title: '" + track.title + "'");
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





std::string YouTubeExtractor::sanitize_title(const std::string& input) {
    std::string result;
    for (char c : input) {
        // Keep only ASCII printable characters (32-126) and basic whitespace
        if ((c >= 32 && c <= 126) || c == '\t' || c == '\n' || c == '\r') {
            result += c;
        }
        // Skip unicode characters that would display as question marks
    }
    return result;
}

std::string YouTubeExtractor::decode_unicode_escapes(const std::string& input) {
    std::string result = input;

    // Unicode escape decoding (\uXXXX)
    size_t pos = 0;
    while ((pos = result.find("\\u", pos)) != std::string::npos) {
        if (pos + 5 < result.size()) {
            std::string hex = result.substr(pos + 2, 4);
            try {
                int codepoint = std::stoi(hex, nullptr, 16);

                // Convert Unicode codepoint to UTF-8
                std::string utf8_bytes;
                if (codepoint <= 0x7F) {
                    // 1-byte UTF-8
                    utf8_bytes = static_cast<char>(codepoint);
                } else if (codepoint <= 0x7FF) {
                    // 2-byte UTF-8
                    utf8_bytes = static_cast<char>(0xC0 | (codepoint >> 6));
                    utf8_bytes += static_cast<char>(0x80 | (codepoint & 0x3F));
                } else if (codepoint <= 0xFFFF) {
                    // 3-byte UTF-8
                    utf8_bytes = static_cast<char>(0xE0 | (codepoint >> 12));
                    utf8_bytes += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                    utf8_bytes += static_cast<char>(0x80 | (codepoint & 0x3F));
                } else if (codepoint <= 0x10FFFF) {
                    // 4-byte UTF-8
                    utf8_bytes = static_cast<char>(0xF0 | (codepoint >> 18));
                    utf8_bytes += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
                    utf8_bytes += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                    utf8_bytes += static_cast<char>(0x80 | (codepoint & 0x3F));
                } else {
                    // Invalid codepoint, skip
                    pos += 6;
                    continue;
                }

                result.replace(pos, 6, utf8_bytes);
                pos += utf8_bytes.length();
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
    std::cout << "Press Enter when you've authorized the app to start checking for completion..." << std::endl;

    // Wait for user to press Enter
    std::cin.get();

    std::cout << "Press Enter again when you've completed authorization to check for tokens..." << std::endl;

    // Wait for user to press Enter again
    std::cin.get();

    // Step 3: Check for token once
    std::string token_url = "https://oauth2.googleapis.com/token";
    std::string client_secret = Config::instance().get_youtube_client_secret();
    std::string token_data = "client_id=" + client_id + "&device_code=" + device_code +
                            "&grant_type=urn:ietf:params:oauth:grant-type:device_code";
    if (!client_secret.empty()) {
        token_data += "&client_secret=" + client_secret;
    }

    std::cout << "Checking authorization status..." << std::endl;

    std::string token_response = http_client_.post(token_url, token_data);
    Logger::info("Token response: " + token_response);

    if (!token_response.empty()) {
        try {
            nlohmann::json token_json = nlohmann::json::parse(token_response);

            if (token_json.contains("access_token")) {
                std::string access_token = token_json["access_token"];
                std::string refresh_token = token_json.value("refresh_token", "");

                // Parse expiration time
                int expires_in = token_json.value("expires_in", 3600); // Default 1 hour
                time_t now = std::time(nullptr);
                time_t expiry = now + expires_in;

                Config::instance().set_youtube_access_token(access_token);
                if (!refresh_token.empty()) {
                    Config::instance().set_youtube_refresh_token(refresh_token);
                }
                Config::instance().set_youtube_token_expiry(expiry);
                Config::instance().save();

                std::cout << std::endl << "✓ Authorization successful!" << std::endl;
                Logger::info("YouTube OAuth authentication successful");
                return true;
            } else if (token_json.contains("error")) {
                std::string error = token_json["error"];
                Logger::info("Token error: " + error);

                if (error == "authorization_pending") {
                    std::cout << "Authorization not completed yet. Run the auth command again when ready." << std::endl;
                    return false;
                } else if (error == "access_denied") {
                    std::cout << "Authorization denied by user." << std::endl;
                    return false;
                } else if (error == "invalid_grant") {
                    std::cout << "Invalid device code or authorization expired." << std::endl;
                    return false;
                } else {
                    std::cout << "Authorization error: " << error << std::endl;
                    return false;
                }
            } else {
                Logger::error("Unexpected token response format");
                return false;
            }
        } catch (const std::exception& e) {
            Logger::error("Failed to parse token response: " + std::string(e.what()));
            return false;
        }
    } else {
        Logger::error("Empty token response");
        return false;
    }
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

                // Parse expiration time
                int expires_in = token_json.value("expires_in", 3600); // Default 1 hour
                time_t now = std::time(nullptr);
                time_t expiry = now + expires_in;

                Config::instance().set_youtube_access_token(access_token);
                Config::instance().set_youtube_token_expiry(expiry);
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
        // Check if token is expired and refresh if needed
        time_t now = std::time(nullptr);
        time_t expiry = Config::instance().get_youtube_token_expiry();

        if (now >= expiry) {
            Logger::info("Access token expired, attempting refresh");
            if (refresh_access_token()) {
                Logger::info("Token refresh successful");
                return Config::instance().get_youtube_access_token();
            } else {
                Logger::error("Token refresh failed, will re-authenticate");
                // Fall through to authentication below
            }
        } else {
            return access_token;
        }
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
