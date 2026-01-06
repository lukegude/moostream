#include "core/config.h"
#include "utils/logger.h"
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace ytui {

Config& Config::instance() {
    static Config instance;
    return instance;
}

Config::Config() {
    load_defaults();
}

void Config::load_defaults() {
    volume_ = 0.7;
    shuffle_ = false;
    repeat_ = false;
    youtube_client_id_ = "";
    youtube_client_secret_ = "";
    youtube_access_token_ = "";
    youtube_refresh_token_ = "";
}

std::string Config::get_config_path() const {
    const char* home = std::getenv("HOME");
    if (!home) {
        return ".moostream.conf";
    }
    return std::string(home) + "/.config/moostream/config";
}

void Config::load(const std::string& config_path) {
    config_path_ = config_path.empty() ? get_config_path() : config_path;

    std::ifstream file(config_path_);
    if (!file.is_open()) {
        Logger::info("No config file found, using defaults");
        return;
    }

    // Simple key=value parser
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        if (key == "volume") {
            volume_ = std::stod(value);
        } else if (key == "shuffle") {
            shuffle_ = (value == "true" || value == "1");
        } else if (key == "repeat") {
            repeat_ = (value == "true" || value == "1");
        } else if (key == "youtube_client_id") {
            youtube_client_id_ = value;
        } else if (key == "youtube_client_secret") {
            youtube_client_secret_ = value;
        } else if (key == "youtube_access_token") {
            youtube_access_token_ = value;
        } else if (key == "youtube_refresh_token") {
            youtube_refresh_token_ = value;
        }
    }

    Logger::info("Config loaded from: " + config_path_);
}

void Config::save() {
    std::ofstream file(config_path_);
    if (!file.is_open()) {
        Logger::error("Failed to save config");
        return;
    }

    file << "# Moostream Configuration\n";
    file << "volume=" << volume_ << "\n";
    file << "shuffle=" << (shuffle_ ? "true" : "false") << "\n";
    file << "repeat=" << (repeat_ ? "true" : "false") << "\n";
    file << "youtube_client_id=" << youtube_client_id_ << "\n";
    file << "youtube_client_secret=" << youtube_client_secret_ << "\n";
    file << "youtube_access_token=" << youtube_access_token_ << "\n";
    file << "youtube_refresh_token=" << youtube_refresh_token_ << "\n";

    Logger::info("Config saved to: " + config_path_);
}

void Config::set_volume(double volume) {
    volume_ = volume;
}

void Config::set_shuffle(bool shuffle) {
    shuffle_ = shuffle;
}

void Config::set_repeat(bool repeat) {
    repeat_ = repeat;
}



void Config::set_youtube_client_id(const std::string& client_id) {
    youtube_client_id_ = client_id;
}

void Config::set_youtube_client_secret(const std::string& client_secret) {
    youtube_client_secret_ = client_secret;
}

void Config::set_youtube_access_token(const std::string& token) {
    youtube_access_token_ = token;
}

void Config::set_youtube_refresh_token(const std::string& token) {
    youtube_refresh_token_ = token;
}

} // namespace ytui
