#ifndef _TIMER_HPP
#define _TIMER_HPP

#include <ogc/lwp_watchdog.h>

class Timer
{
    public:
        Timer() { starttick = gettime(); };
        ~Timer() { };
        float elapsed() { return (float) (gettime()-starttick)/(1000.0f*TB_TIMER_CLOCK); };
        float elapsed_millisecs() { return 1000.0f*elapsed(); };
        void reset() { starttick = gettime(); }
    protected:
        u64 starttick;
};

#endif