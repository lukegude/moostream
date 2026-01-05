#include "audio/mpv_player.h"
#include "utils/logger.h"
#include <cstring>

namespace ytui {

MpvPlayer::MpvPlayer() : mpv_(nullptr), state_(PlaybackState::Stopped) {
    mpv_ = mpv_create();
    if (!mpv_) {
        Logger::error("Failed to create MPV instance");
        return;
    }

    // Set options
    mpv_set_option_string(mpv_, "video", "no");  // Audio only
    mpv_set_option_string(mpv_, "ytdl", "yes");  // Enable youtube-dl/yt-dlp
    mpv_set_option_string(mpv_, "terminal", "no");
    mpv_set_option_string(mpv_, "msg-level", "all=error");

    // Initialize
    if (mpv_initialize(mpv_) < 0) {
        Logger::error("Failed to initialize MPV");
        mpv_destroy(mpv_);
        mpv_ = nullptr;
        return;
    }

    // Observe properties
    mpv_observe_property(mpv_, 0, "pause", MPV_FORMAT_FLAG);
    mpv_observe_property(mpv_, 0, "core-idle", MPV_FORMAT_FLAG);

    Logger::info("MPV player initialized");
}

MpvPlayer::~MpvPlayer() {
    if (mpv_) {
        mpv_terminate_destroy(mpv_);
    }
}

bool MpvPlayer::load(const std::string& url) {
    if (!mpv_) return false;

    const char* cmd[] = {"loadfile", url.c_str(), nullptr};
    int result = mpv_command(mpv_, cmd);

    if (result < 0) {
        Logger::error("Failed to load URL: " + url);
        return false;
    }

    state_ = PlaybackState::Buffering;
    Logger::info("Loading: " + url);
    return true;
}

void MpvPlayer::play() {
    if (!mpv_) return;
    int flag = 0;
    mpv_set_property(mpv_, "pause", MPV_FORMAT_FLAG, &flag);
    state_ = PlaybackState::Playing;
}

void MpvPlayer::pause() {
    if (!mpv_) return;
    int flag = 1;
    mpv_set_property(mpv_, "pause", MPV_FORMAT_FLAG, &flag);
    state_ = PlaybackState::Paused;
}

void MpvPlayer::stop() {
    if (!mpv_) return;
    const char* cmd[] = {"stop", nullptr};
    mpv_command(mpv_, cmd);
    state_ = PlaybackState::Stopped;
}

void MpvPlayer::seek(double position) {
    if (!mpv_) return;
    mpv_set_property(mpv_, "time-pos", MPV_FORMAT_DOUBLE, &position);
}

void MpvPlayer::set_volume(double volume) {
    if (!mpv_) return;
    double vol = volume * 100.0;  // MPV uses 0-100
    mpv_set_property(mpv_, "volume", MPV_FORMAT_DOUBLE, &vol);
}

double MpvPlayer::get_volume() const {
    if (!mpv_) return 0.0;
    double volume = 0.0;
    mpv_get_property(mpv_, "volume", MPV_FORMAT_DOUBLE, &volume);
    return volume / 100.0;
}

PlaybackState MpvPlayer::get_state() const {
    return state_;
}

double MpvPlayer::get_position() const {
    if (!mpv_) return 0.0;
    double pos = 0.0;
    mpv_get_property(mpv_, "time-pos", MPV_FORMAT_DOUBLE, &pos);
    return pos;
}

double MpvPlayer::get_duration() const {
    if (!mpv_) return 0.0;
    double duration = 0.0;
    mpv_get_property(mpv_, "duration", MPV_FORMAT_DOUBLE, &duration);
    return duration;
}

void MpvPlayer::set_end_callback(std::function<void()> callback) {
    end_callback_ = callback;
}

void MpvPlayer::set_error_callback(std::function<void(const std::string&)> callback) {
    error_callback_ = callback;
}

void MpvPlayer::update() {
    process_events();
}

void MpvPlayer::process_events() {
    if (!mpv_) return;

    while (true) {
        mpv_event* event = mpv_wait_event(mpv_, 0);
        if (event->event_id == MPV_EVENT_NONE) break;

        switch (event->event_id) {
            case MPV_EVENT_PLAYBACK_RESTART:
                state_ = PlaybackState::Playing;
                Logger::info("Playback started");
                break;

            case MPV_EVENT_END_FILE: {
                auto* ef = (mpv_event_end_file*)event->data;
                if (ef->reason == MPV_END_FILE_REASON_EOF) {
                    state_ = PlaybackState::Stopped;
                    if (end_callback_) end_callback_();
                } else if (ef->reason == MPV_END_FILE_REASON_ERROR) {
                    state_ = PlaybackState::Stopped;
                    if (error_callback_) error_callback_("Playback error");
                }
                break;
            }

            case MPV_EVENT_PROPERTY_CHANGE:
                handle_property_change((mpv_event_property*)event->data);
                break;

            case MPV_EVENT_LOG_MESSAGE: {
                auto* msg = (mpv_event_log_message*)event->data;
                if (std::strcmp(msg->level, "error") == 0) {
                    Logger::error(std::string("MPV: ") + msg->text);
                }
                break;
            }

            default:
                break;
        }
    }
}

void MpvPlayer::handle_property_change(mpv_event_property* prop) {
    if (!prop->data) return;

    if (std::strcmp(prop->name, "pause") == 0) {
        int paused = *(int*)prop->data;
        if (paused && state_ == PlaybackState::Playing) {
            state_ = PlaybackState::Paused;
        }
    }
}

} // namespace ytui
