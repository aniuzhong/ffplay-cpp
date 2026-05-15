#pragma once

#include <cmath>

constexpr double AV_NOSYNC_THRESHOLD = 10.0;

class Clock {
public:
    Clock();

    double get(int queue_serial) const;
    void set(double pts, int serial);
    void set_at(double pts, int serial, double time);
    void set_speed(double speed);

    double speed() const { return speed_; }
    int serial() const { return serial_; }
    double pts() const { return pts_; }
    bool paused() const { return paused_; }
    void set_paused(bool paused) { paused_ = paused; }
    double last_updated() const { return last_updated_; }

private:
    double pts_ = NAN;
    double pts_drift_ = 0;
    double last_updated_ = 0;
    double speed_ = 1.0;
    int serial_ = -1;
    bool paused_ = false;
};

void sync_clock_to_slave(Clock *c, int c_queue_serial, Clock *slave, int slave_queue_serial);
