// config.h

#ifndef HEADER_CONFIG
#define HEADER_CONFIG

// config data
typedef struct {
	int  hour;
	int  minute;
	bool set;
} alarmDetails_t;

typedef struct {
	uint32_t       signature;
	int            volume;      // 1-100% (really only works in 10% increments; 10-100)   FIXME really doesn't work at all!
	alarmDetails_t alarmDay[7]; // one for each day, 0=Sun ... 6=Sat
} config_t;

extern config_t config;         // stored in config.cpp

void configSetup();
void loadConfig (void);
void storeConfig (config_t *new_config);
void printConfig ();	//config_t *c = &config);

#endif

// vim: set ts=4 sw=4 tw=0 noet : 
