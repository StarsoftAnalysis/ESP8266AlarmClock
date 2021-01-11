// config.cpp

#include "Arduino.h"
#include <stddef.h>
#include <string.h>
#include <EEPROM.h>
#include "debug.h"
#include "config.h"

const uint32_t CONFIGSIG = 0xacac0002ul; // Something unique to the current version of
// the config structure -- if it changes later, 
// build up a list of previous signatures if migration is required.

// Configuration is stored here  //
// ----------------------------- //
// (it's 'extern' in the header) //
config_t config;

void printConfig (void) { //(config_t *c) {	// *c defaults to &config
	PRINTF("config: signature %x  volume %d  timezone '%s'  melody '%.10s...'  alarm details:\n", config.signature, config.volume, config.tz, config.melody);
	for (size_t i = 0; i < 7; i++) {
		PRINTF("\t%u: %02d:%02d %d\n", i, config.alarmDay[i].hour, config.alarmDay[i].minute, config.alarmDay[i].set);
	}
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

// Get config from eeprom into the static struct.
// If it doesn't look valid, apply defaults
void loadConfig () {
	EEPROM.get(0, config);
	if (config.signature != CONFIGSIG) {
		setDefaults();
		EEPROM.put(0, config);
		EEPROM.commit();
	}
	PRINTLN("c:lC loaded config:");
	printConfig();
}

// Set the configuration
void storeConfig (config_t *new_config) {
	config = *new_config;
	config.signature = CONFIGSIG;
	PRINTLN("c:sC storing new config:");
	printConfig();
	EEPROM.put(0, config);
	EEPROM.commit();
}

void configSetup () {
	EEPROM.begin(sizeof(config));
}

// vim: set ts=4 sw=4 tw=0 noet : 
