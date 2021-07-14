#ifndef FRAMETIMER_H
#define FRAMETIMER_H

#include <unistd.h>
#include <assert.h>
#include <chrono>

using std::chrono::steady_clock;


/**
* @class FrameTimer
*
* @brief Includes wait_for_frame() with a sleep and reset, and a frame_elapsed()
* time check since last reset (with optional reset if affirmative).
*
* @author  Jakob Garde
* @date    210224
*/
class FrameTimer {
  int interval_ms;
  steady_clock::time_point tick;
  int timelost_ms;
  int timewaited_ms;
public:
  // setup and start the time / reset the tick
  FrameTimer(int interval_ms = 33, int offset_ms=0) {
    this->interval_ms = interval_ms;
    this->timelost_ms = 0;
    this->timewaited_ms = 0;
    this->tick = steady_clock::now() - std::chrono::milliseconds(offset_ms);
  }
  // reset the tick to current time
  void reset() {
    this->tick = steady_clock::now();
  }
  double _get_duration_since_tick_ms() {
    return ((steady_clock::now() - this->tick).count()) / 1000000.0;
  }
  // returns true if frametime has elapsed since last tick
  bool frame_elapsed(bool autoreset_on_success=true, int* duration_ms=NULL) {
    double duration = this->_get_duration_since_tick_ms();

    // time warp - system time was probably reset, reset timer and return immediately
    if (duration < 0) {
      this->reset();
      return true;
    }

    // frame has elapsed - count, reset time and return
    if (duration > this->interval_ms) {
      this->timelost_ms += duration - this->interval_ms;
      if (autoreset_on_success) this->reset();
      return true;
    }

    // output the exact duration in milliseconds
    if (duration_ms != NULL)
      *duration_ms = duration;

    // frame has not yet run out
    return false;
  }
  // returns true if a wait was executed, otherwise false
  bool wait_for_frame() {
    double duration = this->_get_duration_since_tick_ms();
    double rem_ftime = this->interval_ms - duration;

    // time warp - system time was probably reset, reset timer and return immediately
    if (duration < 0) {
      this->reset();
      return false;
    }

    // we are behind - count, reset time and return immediately
    if (rem_ftime < 0) {
      this->timelost_ms += - rem_ftime;
      this->reset();
      return false;
    }

    // frame has not yet run out - wait for frame end (µs*1000), record, reset and return
    usleep(1000 * rem_ftime);
    this->timewaited_ms += rem_ftime;
    this->reset();
    return true;
  }
};


#endif // FRAMETIMER_H
