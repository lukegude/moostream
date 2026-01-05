#pragma once

#include "player_interface.h"
#include <mpv/client.h>
#include <memory>

namespace ytui {

class MpvPlayer : public IPlayer {
public:
    MpvPlayer();
    ~MpvPlayer() override;

    // IPlayer interface
    bool load(const std::string& url) override;
    void play() override;
    void pause() override;
    void stop() override;
    void seek(double position) override;

    void set_volume(double volume) override;
    double get_volume() const override;

    PlaybackState get_state() const override;
    double get_position() const override;
    double get_duration() const override;

    void set_end_callback(std::function<void()> callback) override;
    void set_error_callback(std::function<void(const std::string&)> callback) override;

    void update() override;

private:
    mpv_handle* mpv_;
    PlaybackState state_;
    std::function<void()> end_callback_;
    std::function<void(const std::string&)> error_callback_;

    void process_events();
    void handle_property_change(mpv_event_property* prop);
};

} // namespace ytui
