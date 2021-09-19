#ifndef __PERFTIMER_H__
#define __PERFTIMER_H__


#include <chrono>


/** 
* Function based convenient, but inaccurate, perftimer and sleep.
*/
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
    if (print == true) {
        printf("%f%s\n", retval, unit);
    }
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
        if (this->print_time) {
            printf("%f µs\n", ((std::chrono::steady_clock::now() - this->tick).count()) / 1000.0);
        }
    }
    u32 GetTimeMicroS() {
        return ((std::chrono::steady_clock::now() - this->tick).count()) / 1000.0;
    }
};


#endif // FRAMETIMER_H
