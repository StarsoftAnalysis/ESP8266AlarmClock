// config.cpp
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

#include <Arduino.h>
#include <stddef.h>
#include <string.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>	// for WiFi.mac()
#include "debug.h"
#include "config.h"

#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

const uint32_t CONFIGSIG = 0xacac0002ul; 
const uint32_t CONFIGWSIG = 0xacac0002ul; 

config_t config;
configw_t configw;
const size_t configw_offset = ROUND_UP(sizeof(config), 4);
const size_t config_size = configw_offset + sizeof(configw);

void printConfig (void) { //(config_t *c) {	// *c defaults to &config
	PRINTF("config: signature %x  volume %d  timezone '%s'  melody '%.10s...'  alarm details:\n", config.signature, config.volume, config.tz, config.melody);
	for (size_t i = 0; i < 7; i++) {
		PRINTF("\t%u: %02d:%02d %d\n", i, config.alarmDay[i].hour, config.alarmDay[i].minute, config.alarmDay[i].set);
	}
}

void printConfigW (void) {
	PRINTF("configw: %x hostname = '%s'\r\n", 
			configw.signature, 
			configw.hostname 
			);
	PRINTF("\twifi: '%s'/'%s' '%s'/'%s' '%s'/'%s'\r\n",
			configw.wifi[0].ssid, "***", //configw.wifi[0].psk,
			configw.wifi[1].ssid, "***", //configw.wifi[1].psk,
			configw.wifi[2].ssid, "***"  //configw.wifi[2].psk
			);
}

/*
static void check_isnan (float *n, float goodvalue = 0.0) {
	if (isnan(*n)) {
		*n = goodvalue;
	}
}
*/

static void setDefaults () {
	PRINTLN("c:sD setting defaults");
	config.signature = CONFIGSIG;
	config.volume = 0;
	for (size_t i = 0; i < (sizeof(config.alarmDay)/sizeof(config.alarmDay[0])); i++) {
		config.alarmDay[i].hour = 0;
		config.alarmDay[i].minute = 0;
		config.alarmDay[i].set = false;
	}
	strlcpy(config.melody, "WalkLike:d=4,o=5,b=120:p,8f#,8d#,d#,d#,d#.,c#,f#.,d#,8d#,e,8d#,b4,b4.,a4,8a4,b4", MELODY_MAX);
	strlcpy(config.tz, "Europe/London", TZ_MAX);
}

// Return the number of SSIDs currently known
int ssidCount () {
	int count = 0;
	for (int i = 0; i < wifimax; i++) {
		if (strlen(configw.wifi[i].ssid) > 0) {
			count += 1;
		}
	}
	return count;
}

// Get the WiFi MAC address as "abcdef123456"
static void mac_address (String& macstring) {
	unsigned char mac[6];
	char machex[7]; // 13 for all 6 digits
	WiFi.macAddress(mac);
	snprintf(machex, sizeof(machex),
			//"%.2x%.2x%.2x%.2x%.2x%.2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			"%.2x%.2x%.2x", mac[3], mac[4], mac[5]);
	macstring = machex;
}

static void setWifiDefaults () {
	PRINTLN(F("configw: setting wifi defaults"));
	configw.signature = CONFIGWSIG;
	String macstring = "";
	mac_address(macstring);
	macstring = "AlarmClock-" + macstring;
	strlcpy(configw.hostname, macstring.c_str(), sizeof(configw.hostname));
	configw.wifi[0].ssid[0] = '\0';
	configw.wifi[1].ssid[0] = '\0';
	configw.wifi[2].ssid[0] = '\0';
	configw.wifi[0].psk[0] = '\0';
	configw.wifi[1].psk[0] = '\0';
	configw.wifi[2].psk[0] = '\0';
	// TEMP hard-coded defaults
	/*
	strlcpy(configw.wifi[0].ssid, "NETGEAR07",         sizeof(configw.wifi[0].ssid));
	strlcpy(configw.wifi[0].psk,  "joyouslotus469",    sizeof(configw.wifi[0].psk));
	strlcpy(configw.wifi[1].ssid, "76awd",             sizeof(configw.wifi[1].ssid));
	strlcpy(configw.wifi[1].psk,  "OakFrogWoodpecker", sizeof(configw.wifi[1].psk));
	*/
}

// Get config from eeprom into the static struct.
// If it doesn't look valid, apply defaults
void loadConfigs () {
	EEPROM.get(0, config);
	if (config.signature != CONFIGSIG) {
		setDefaults();
		EEPROM.put(0, config);
		EEPROM.commit();
	}
	PRINTF("c:lC loaded config (size %d):\n", sizeof(config));
	printConfig();
	// WiFi config too
	EEPROM.get(configw_offset, configw);
	if (configw.signature != CONFIGWSIG) {
		setWifiDefaults();
		EEPROM.put(configw_offset, configw);
		EEPROM.commit();
	}
	PRINTF("c:lC loaded wifi config (size %d):\n", sizeof(configw));
	printConfigW();
}

// Set the configurations
void storeConfig (config_t *new_config) {
	config = *new_config;
	config.signature = CONFIGSIG;
	PRINTLN("c:sC storing new config:");
	printConfig();
	EEPROM.put(0, config);
	EEPROM.commit();
}
void storeConfigW (configw_t *new_configw) {
	configw = *new_configw;
	configw.signature = CONFIGWSIG;
	PRINTLN("c:sC storing new wifi config:");
	printConfigW();
	EEPROM.put(configw_offset, configw);
	EEPROM.commit();
}

void configSetup () {
	EEPROM.begin(sizeof(config) + sizeof(configw));
}

// vim: set ts=4 sw=4 tw=0 noet : 
