#ifndef __PERFTIMER_H__
#define __PERFTIMER_H__

#include <chrono>
#include <iostream>

/** 
* A convenient, but inaccurate, timer usable for many performance tasks.
*/
class PerfTimerScoped {
public:
  std::chrono::steady_clock::time_point tick;
  bool print_time;
  PerfTimerScoped(bool print_time = true) : print_time(print_time) {
    this->tick = std::chrono::steady_clock::now();
  }
  ~PerfTimerScoped() {
    if (this->print_time)
      std::cout << ((std::chrono::steady_clock::now() - this->tick).count()) / 1000.0 << " µs" << std::endl;
  }
  u32 GetTimeMicroS() {
    return ((std::chrono::steady_clock::now() - this->tick).count()) / 1000.0;
  }
};

void Sleep(u32 ms) {
  usleep(1000 * ms);
}

#endif // FRAMETIMER_H
