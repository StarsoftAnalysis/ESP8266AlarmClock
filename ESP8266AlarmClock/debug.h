// debug.h
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

// macros for turning debugging statements on and off
// (and maybe later for redirecting to other outputs...)

#define DEBUG

#ifdef DEBUG
#   define PRINT(...)   Serial.print(__VA_ARGS__)
#   define PRINTLN(...) Serial.println(__VA_ARGS__)
#   define PRINTF(...)  Serial.printf(__VA_ARGS__)
#else
#   define PRINT(...)
#   define PRINTLN(...)
#   define PRINTF(...)
#endif

#if defined DEBUG
#   define BEGIN(speed, debug) Serial.begin((speed)); while (!Serial) {}; Serial.setDebugOutput(debug)
#else
#   define BEGIN(speed, debug)
#endif
