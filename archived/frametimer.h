#ifndef FRAMETIMER_H
#define FRAMETIMER_H

#include <unistd.h>
#include <assert.h>
#include <chrono>

using std::chrono::steady_clock;


class FrameTimer {
public:
    int interval_ms;
    std::chrono::steady_clock::time_point tick;
    int timelost_ms;
    int timewaited_ms;
    u32 frames_ticked = 0;
    FrameTimer(int interval_ms = 33, int offset_ms=0) {
        this->interval_ms = interval_ms;
        this->timelost_ms = 0;
        this->timewaited_ms = 0;
        this->tick = std::chrono::steady_clock::now() - std::chrono::milliseconds(offset_ms);
    }
    void Reset() {
        this->tick = std::chrono::steady_clock::now();
    }
    double _GetDurationSinceTickMs() {
        return ((std::chrono::steady_clock::now() - this->tick).count()) / 1000000.0;
    }
    bool FrameElapsed(bool autoreset_on_success=true, int* duration_ms=NULL) {
        // returns true if frametime has elapsed since last tick
        double duration = this->_GetDurationSinceTickMs();

        // time warp
        if (duration < 0) {
            this->Reset();
            ++this->frames_ticked;
            return true;
        }

        if (duration > this->interval_ms) {
            this->timelost_ms += duration - this->interval_ms;
            if (autoreset_on_success) {
                this->Reset();
                ++this->frames_ticked;
            }
            return true;
        }

        if (duration_ms != NULL)
        *duration_ms = duration;

        return false;
    }
    bool WaitForFrame() {
        // returns true if a wait was executed, otherwise false
        double duration = this->_GetDurationSinceTickMs();
        double rem_ftime = this->interval_ms - duration;

        // time warp
        if (duration < 0) {
            this->Reset();
            return false;
        }

        if (rem_ftime < 0) {
            this->timelost_ms += - rem_ftime;
            this->Reset();
            return false;
        }

        usleep(1000 * rem_ftime);
        this->timewaited_ms += rem_ftime;
        this->Reset();
        ++this->frames_ticked;
        return true;
    }
};

#endif // FRAMETIMER_H
