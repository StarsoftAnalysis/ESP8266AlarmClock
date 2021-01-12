// config.h
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

#ifndef HEADER_CONFIG
#define HEADER_CONFIG

typedef struct {
	int  hour;
	int  minute;
	bool set;
} alarmDetails_t;

#define MELODY_MAX 500
#define TZ_MAX 33
typedef struct {
	uint32_t       signature;
	int            volume;      // 1-100% (really only works in 10% increments; 10-100)   FIXME really doesn't work at all!
	alarmDetails_t alarmDay[7]; // one for each day, 0=Sun ... 6=Sat
	char		   tz[TZ_MAX];
	char           melody[MELODY_MAX];
} config_t;

extern config_t config;         // stored in config.cpp

void configSetup();
void loadConfig (void);
void storeConfig (config_t *new_config);
void printConfig ();	//config_t *c = &config);

#endif

// vim: set ts=4 sw=4 tw=0 noet : 
