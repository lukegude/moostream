#pragma once

#include <string>
#include <map>

namespace ytui {

class Config {
public:
    static Config& instance();

    void load(const std::string& config_path = "");
    void save();

    // Getters
    double get_volume() const { return volume_; }
    bool get_shuffle() const { return shuffle_; }
    bool get_repeat() const { return repeat_; }
    std::string get_ytdlp_path() const { return ytdlp_path_; }

    // Setters
    void set_volume(double volume);
    void set_shuffle(bool shuffle);
    void set_repeat(bool repeat);
    void set_ytdlp_path(const std::string& path);

private:
    Config();
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    std::string get_config_path() const;
    void load_defaults();

    std::string config_path_;
    double volume_;
    bool shuffle_;
    bool repeat_;
    std::string ytdlp_path_;
};

} // namespace ytui
