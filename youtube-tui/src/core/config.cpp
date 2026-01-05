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
    ytdlp_path_ = "yt-dlp";
}

std::string Config::get_config_path() const {
    const char* home = std::getenv("HOME");
    if (!home) {
        return ".youtube-tui.conf";
    }
    return std::string(home) + "/.config/youtube-tui/config";
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
        } else if (key == "ytdlp_path") {
            ytdlp_path_ = value;
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

    file << "# YouTube TUI Configuration\n";
    file << "volume=" << volume_ << "\n";
    file << "shuffle=" << (shuffle_ ? "true" : "false") << "\n";
    file << "repeat=" << (repeat_ ? "true" : "false") << "\n";
    file << "ytdlp_path=" << ytdlp_path_ << "\n";

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

void Config::set_ytdlp_path(const std::string& path) {
    ytdlp_path_ = path;
}

} // namespace ytui
