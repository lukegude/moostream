#include "youtube/extractor.h"
#include "utils/logger.h"
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
    try {
        std::string search_query = "ytsearch" + std::to_string(max_results) + ":" + query;

        std::vector<std::string> args = {
            "--dump-json",
            "--quiet",
            "--no-warnings",
            search_query
        };

        std::string json_output = execute_ytdlp(args);
        return parse_search_results(json_output);
    } catch (const std::exception& e) {
        Logger::error("Search failed: " + std::string(e.what()));
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
    try {
        std::string search_query = "ytsearch" + std::to_string(max_results) + ":" + query;

        std::vector<std::string> args = {
            "--dump-json",
            "--quiet",
            "--no-warnings",
            search_query
        };

        std::string json_output = execute_ytdlp(args);

        // Process each line incrementally
        std::istringstream iss(json_output);
        std::string line;

        while (std::getline(iss, line)) {
            if (line.empty()) continue;

            Track track = parse_single_search_result(line);
            if (!track.id.empty()) {
                result_callback(track);
            }
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
    // TODO: Implement proper JSON parsing using nlohmann/json
    // For now, this is a placeholder
    Track track;

    // Simple parsing - in real implementation use nlohmann/json
    // This is just to make it compile
    if (json.find("\"id\"") != std::string::npos) {
        track.id = "placeholder_id";
        track.title = "Placeholder Title";
        track.channel = "Placeholder Channel";
        track.duration = 0;
        track.is_live = false;
    }

    return track;
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

} // namespace ytui
