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
u32 StopTimer(bool print = false, bool highres = false) {
  double res = 1000000.0;
  char* unit = (char*) " ms";
  if (highres == true) {
    res = 1000.0;
    unit = (char*) " µs";
  }
  auto retval = ((std::chrono::steady_clock::now() - g_tick).count()) / res;
  if (print == true)
    std::cout << retval << unit << std::endl;
  return retval;
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
