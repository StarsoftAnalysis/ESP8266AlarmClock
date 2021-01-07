// timers.h

#ifndef HEADER_TIMERS
#define HEADER_TIMERS

#include <stddef.h>

namespace timers {

    // One-shot Timers -- as many as needed, with names to avoid confusion
#   define TIMERCOUNT 1
#   define TIMER_KEEP_LIGHT_ON 0  

    typedef void (*TimerFn)(void);
    void setTimer (int id, unsigned int delay, TimerFn fn, bool force = true);
    void cancelTimer (int id);

    // NOTE: these two must be used for timers to work
    void setup ();  // in the main setup()
    void loop ();   // in the main loop()

} // namespace timers

#endif
