#ifndef __PERFTIMER_H__
#define __PERFTIMER_H__


#include <chrono>
#include <iostream>


/** 
* Function based convenient, but inaccurate, perftimer and sleep.
*/
void Sleep(u32 ms) {
  usleep(1000 * ms);
}
std::chrono::steady_clock::time_point g_tick;
void StartTimer() {
  g_tick = std::chrono::steady_clock::now();
}
u32 StopTimer() {
  return ((std::chrono::steady_clock::now() - g_tick).count()) / 1000.0;
}


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


#endif // FRAMETIMER_H
