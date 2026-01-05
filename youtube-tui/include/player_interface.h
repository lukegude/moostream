#pragma once

#include <functional>
#include <string>

namespace ytui {

enum class PlaybackState {
    Stopped,
    Playing,
    Paused,
    Buffering
};

class IPlayer {
public:
    virtual ~IPlayer() = default;

    virtual bool load(const std::string& url) = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void seek(double position) = 0;

    virtual void set_volume(double volume) = 0;
    virtual double get_volume() const = 0;

    virtual PlaybackState get_state() const = 0;
    virtual double get_position() const = 0;
    virtual double get_duration() const = 0;

    virtual void set_end_callback(std::function<void()> callback) = 0;
    virtual void set_error_callback(std::function<void(const std::string&)> callback) = 0;

    virtual void update() = 0;
};

} // namespace ytui
