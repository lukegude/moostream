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
    std::string get_youtube_client_id() const { return youtube_client_id_; }
    std::string get_youtube_client_secret() const { return youtube_client_secret_; }
    std::string get_youtube_access_token() const { return youtube_access_token_; }
    std::string get_youtube_refresh_token() const { return youtube_refresh_token_; }

    // Setters
    void set_volume(double volume);
    void set_shuffle(bool shuffle);
    void set_repeat(bool repeat);
    void set_ytdlp_path(const std::string& path);
    void set_youtube_client_id(const std::string& client_id);
    void set_youtube_client_secret(const std::string& client_secret);
    void set_youtube_access_token(const std::string& token);
    void set_youtube_refresh_token(const std::string& token);

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
    std::string youtube_client_id_;
    std::string youtube_client_secret_;
    std::string youtube_access_token_;
    std::string youtube_refresh_token_;
};

} // namespace ytui
