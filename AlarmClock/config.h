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

// Max no. of WiFi networks known
const int wifimax = 3;

// WiFi config data
typedef struct {
	uint32_t signature;
	char hostname[32];      // Arbitrary length
	struct {
		char ssid[33];		// keep these empty when not in use
		char psk[64];
	} wifi[wifimax];
} configw_t;

typedef struct {
	int  hour;
	int  minute;
	bool set;
} alarmDetails_t;

#define MELODY_MAX 500
#define TZ_MAX 33
typedef struct {
	uint32_t       signature;
	int            volume;
	alarmDetails_t alarmDay[7]; // one for each day, 0=Sun ... 6=Sat
	char		   tz[TZ_MAX];
	char           melody[MELODY_MAX];
} config_t;

extern config_t config;         // stored in config.cpp
extern configw_t configw;

void configSetup (void);
void loadConfigs (void);
void storeConfig (config_t *new_config);
void storeConfigW (configw_t *new_configw);
void printConfig (void);
void printConfigW (void);
int ssidCount (void);

#endif

// vim: set ts=4 sw=4 tw=0 noet : 
