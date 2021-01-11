// timers.cpp

// Simple timer routines to replace all that tedious messing about with
//   millis() - startTime > duration

#include "Arduino.h"
#include <stddef.h>
#include <string.h>
#include "debug.h"
#include "timers.h"

namespace timers {

	// The function passed to setTimer must take no arguments, and return nothing,
	// i.e. void fn (void);

	typedef struct {
		unsigned long start;
		unsigned long delay;
		TimerFn fn;
	} timer_t;

	timer_t timers[TIMERCOUNT];

	// 'force': default to true; if false, don't restart if already running
	void setTimer (int id, unsigned int delay, TimerFn fn, bool force) {
		if (id < 0 || id >= TIMERCOUNT) {
			Serial.printf("setTimer: invalid id %i ignored\r\n", id);
			return;
		}
		if (timers[id].fn == NULL  ||      // not already set 
				force                     ) {  // restart even if running
			timers[id].start = millis();
			timers[id].delay = delay;
			timers[id].fn = fn;
		}
	}

	void cancelTimer (int id) {
		if (id < 0 || id >= TIMERCOUNT) {
			Serial.printf("cancelTimer: invalid id %i ignored\r\n", id);
			return;
		}
		timers[id].fn = NULL;
	}

	// Initialise the timers (as all empty)
	void setup () {
		for (int i = 0; i < TIMERCOUNT; i++) {
			timers[i].fn = NULL;
		}
	}

	// The utils loop runs the timers
	void loop () {
		for (int i = 0; i < TIMERCOUNT; i++) {
			if (timers[i].fn != NULL) {
				if (millis() - timers[i].start > timers[i].delay) {
					TimerFn fncopy = timers[i].fn;
					timers[i].fn = NULL;    // just run it once
					fncopy();
				}
			}
		}
	}

} // namespace timers

// vim: set ts=4 sw=4 tw=0 noet : 
