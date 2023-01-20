// timers.h
/*

    Copyright 2021 Chris Dennis

    This file is part of AlarmClock.

    AlarmClock is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AlarmClock is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AlarmClock.  If not, see <https://www.gnu.org/licenses/>.

*/

#ifndef HEADER_TIMERS
#define HEADER_TIMERS

#include <stddef.h>

namespace timers {

    // One-shot Timers -- as many as needed, with names to avoid confusion
#   define TIMERCOUNT 4
#   define TIMER_KEEP_LIGHT_ON 0  
#   define TIMER_TRY_AP_MODE   1  
#   define TIMER_STOP_AP_MODE  2
#   define TIMER_MONITOR_WIFI  3

    typedef void (*TimerFn)(void);
    void setTimer (int id, unsigned int delay, TimerFn fn, bool force = true);
    void cancelTimer (int id);

    // NOTE: these two must be used for timers to work
    void setup ();  // in the main setup()
    void loop ();   // in the main loop()

} // namespace timers

#endif
