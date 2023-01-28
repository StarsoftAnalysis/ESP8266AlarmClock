// wifi.cpp

#include <ESP8266WiFi.h>
#include <WString.h>

#include "debug.h"
#include "wifi.h"
#include "timers.h"
#include "config.h"

namespace wifi {

// These are defined in ESP8266WiFi/src/includes/wl_definitions.h
//typedef enum {
//    WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
//    WL_IDLE_STATUS      = 0,
//    WL_NO_SSID_AVAIL    = 1,
//    WL_SCAN_COMPLETED   = 2,
//    WL_CONNECTED        = 3,
//    WL_CONNECT_FAILED   = 4,
//    WL_CONNECTION_LOST  = 5,
//    WL_DISCONNECTED     = 6
//} wl_status_t;

// (see wifi.h for wifi_mode)

// These handlers have to be here so that they exist when needed.
// says https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/WiFiEvents/WiFiEvents.ino
WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;
WiFiEventHandler stationGotIPHandler;

// TODO change these to return C strings?
String wifi_ssid () {
	return WiFi.SSID();
}

String wifi_ip4 () {
	return WiFi.localIP().toString();
}

String wifi_ip6 () {
	return "(unknown)";
	//return WiFi.localIPv6().toString();
}

// exported
int wifi_status_code () {
	return (int)WiFi.status();
}

// exported
String wifi_status () {
	switch (WiFi.status()) {  
		case WL_CONNECTED: 
			//return "Connected to " + wifi_ssid() + " as " + wifi_ip4();
			return F("Connected");
			break;
		case WL_NO_SHIELD: 
			return F("No WiFi shield is present");
			break;
		case WL_IDLE_STATUS: 
			return F("Idle (trying to connect)");
			break;
		case WL_NO_SSID_AVAIL: 
			return F("No SSIDs are available");
			break;
		case WL_SCAN_COMPLETED: 
			return F("Scan of networks is completed");
			break;
		case WL_CONNECT_FAILED: 
			return F("Connection failed");
			break;
		case WL_CONNECTION_LOST: 
			return F("Connection lost");
			break;
		case WL_DISCONNECTED: 
			return F("Disconnected");
			break;
		default:  
			break;
	}
	return F("Unknown status"); 
}

static void start_ap_mode () {
	// go to AP mode
	PRINTLN("starting AP mode");
	WiFi.mode(WIFI_AP);
	WiFi.softAP(config::config.hostname);
	//WiFi.printDiag(Serial);
	PRINTF("started AP mode at 192.168.4.1\n");	// %s\n", wifi_ip4().c_str());
	// If there are any known SSIDs, revert to station mode after a while
	if (config::ssidCount() > 0) {
		timers::setTimer(TIMER_STOP_AP_MODE, 15*60*1000, stop_ap_mode);
	}
}

// This is called from server.cpp when the user enters one or mode SSIDs.
// exported
void stop_ap_mode () {
	PRINTLN("stopping AP mode");
	WiFi.softAPdisconnect(false);  // leave WiFi on
	WiFi.mode(WIFI_STA);  // FIXME call start_sta  ??
}

// Called when SSID settings may have changed
	void newSSIDs () {
		/*
	mode = WiFi.getMode();
		if (config::ssidCount() == 0) {
			go to AP mode if not already
		} else {
			if (in STA mode)
				if current SSID is still in config
					stay where we are
				else
					try the new SSIDs
			else (in AP mode) -- try one of the new SSIDs
			*/
	}

// Return the index (into config::config.wifi) of the current best network.
// (or -1 if none available)
// This can/should be used as the callback from WiFi.scanNetworksAsync,
// which supplies the total number of networks currently available.
static int best_wifi (int networksFound) { 
	//PRINTF("rsr: %d network(s) found\n", networksFound);
	int best_rssi = -999999;
	int best_ssid_index = -1;
	for (int i = 0; i < networksFound; i++) { 
		int rssi = WiFi.RSSI(i);
		PRINTF("%d: %s, Ch:%d (%ddBm) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), rssi, WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
		for (int j = 0; j < WIFI_MAX; j++) {
			//PRINTF("matching '%s' with '%s'\n", WiFi.SSID(i).c_str(), config::config.wifi[j].ssid);
			if (strcmp(WiFi.SSID(i).c_str(), config::config.wifi[j].ssid) == 0) {
				PRINTF("rsr got matching network %s with strength %i\r\n", config::config.wifi[j].ssid, WiFi.RSSI(i));
				if (rssi > best_rssi) {
					best_rssi = rssi;
					best_ssid_index = j;
				}
				// got a match
				//WiFi.mode(WIFI_STA);
				//WiFi.begin(config::config.wifi[j].ssid, config::config.wifi[j].pass);
				//return; // don't look any further
			}
		}
	}
	return best_ssid_index;
}

// Connect to the current best network.
// This can/should be used as the callback from WiFi.scanNetworksAsync,
// which supplies the total number of networks currently available.
static void connect_best_wifi (int networksFound) {
	int best_ssid_index = best_wifi(networksFound);
	if (best_ssid_index > -1) {
		WiFi.mode(WIFI_STA);
		WiFi.begin(config::config.wifi[best_ssid_index].ssid, config::config.wifi[best_ssid_index].pass);
	} else {
		// what? stay in whatever mode we're in...
	}
}

// exported
void start_sta_mode () {
	WiFi.scanNetworksAsync(connect_best_wifi);
}

// Check the wifi and try to connect
// (called via a timer, not direct from the loop)
static void monitor_wifi () {
	//PRINTF("monitor_wifi: mode is %i:%s, wifi_status is %i:%s\r\n", WiFi.getMode(), wifi_mode().c_str(), WiFi.status(), wifi_status().c_str());
	PRINTF("monitor_wifi: mode is %i:%s, wifi_status is %i:%s\r\n", WiFi.getMode(), "??", WiFi.status(), wifi_status().c_str());
	unsigned long delay = 10000; // default delay until running this function again
	switch (WiFi.getMode()) {
		case WIFI_OFF:
			// Shouldn't be off -- turn it on
			PRINTLN("WiFi is off -- going to STA");
			WiFi.mode(WIFI_STA);
			delay = 2000; // don't wait so long before trying again
			break;
		case WIFI_AP_STA:
			// Shouldn't be that - go to just STA		// FIXME or maybe allow this mode
			PRINTLN("WiFi is AP_STA, going to STA");
			start_sta_mode();
			break;
		case WIFI_AP:
			// carry on in AP mode until user supplies details (or timeout)
			break;
		case WIFI_STA:
			if (WiFi.isConnected()) {
				delay = 30000;
				// cancel timer to try AP mode
				timers::cancelTimer(TIMER_TRY_AP_MODE);
			} else {
				timers::setTimer(TIMER_TRY_AP_MODE, 1*60*1000, start_ap_mode, false); // false means don't restart if already running
				if (config::ssidCount() == 0) {
					// No known SSIDs -- go to AP mode
					// no -- wait for TIMER_TRY_AP_MODE  start_ap_mode();  // NO == may be too soon - wait a bit in case somethung comes into range
				} else {
					// Find best available SSID and connect to it (async)
					start_sta_mode();
					delay = 20000;
				}
			}
			break;
		default:
			break;
	}
	// repeat...
	timers::setTimer(TIMER_MONITOR_WIFI, delay, monitor_wifi);
}

// See https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/ESP8266WiFiType.h
// for what's in the various evt structs
static void onStationModeConnected (const WiFiEventStationModeConnected& evt) {
	PRINTF("Station connected to SSID %s\n", evt.ssid.c_str());
	//PRINTLN(macToString(evt.mac));
}

// enum WiFiDisconnectReason { 
//     WIFI_DISCONNECT_REASON_UNSPECIFIED              = 1, 
//     WIFI_DISCONNECT_REASON_AUTH_EXPIRE              = 2, 
//     WIFI_DISCONNECT_REASON_AUTH_LEAVE               = 3, 
//     WIFI_DISCONNECT_REASON_ASSOC_EXPIRE             = 4, 
//     WIFI_DISCONNECT_REASON_ASSOC_TOOMANY            = 5, 
//     WIFI_DISCONNECT_REASON_NOT_AUTHED               = 6, 
//     WIFI_DISCONNECT_REASON_NOT_ASSOCED              = 7, 
//     WIFI_DISCONNECT_REASON_ASSOC_LEAVE              = 8, 
//     WIFI_DISCONNECT_REASON_ASSOC_NOT_AUTHED         = 9, 
//     WIFI_DISCONNECT_REASON_DISASSOC_PWRCAP_BAD      = 10,  /* 11h */ 
//     WIFI_DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD     = 11,  /* 11h */ 
//     WIFI_DISCONNECT_REASON_IE_INVALID               = 13,  /* 11i */ 
//     WIFI_DISCONNECT_REASON_MIC_FAILURE              = 14,  /* 11i */ 
//     WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT   = 15,  /* 11i */ 
//     WIFI_DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT = 16,  /* 11i */ 
//     WIFI_DISCONNECT_REASON_IE_IN_4WAY_DIFFERS       = 17,  /* 11i */ 
//     WIFI_DISCONNECT_REASON_GROUP_CIPHER_INVALID     = 18,  /* 11i */ 
//     WIFI_DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID  = 19,  /* 11i */ 
//     WIFI_DISCONNECT_REASON_AKMP_INVALID             = 20,  /* 11i */ 
//     WIFI_DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION    = 21,  /* 11i */ 
//     WIFI_DISCONNECT_REASON_INVALID_RSN_IE_CAP       = 22,  /* 11i */ 
//     WIFI_DISCONNECT_REASON_802_1X_AUTH_FAILED       = 23,  /* 11i */ 
//     WIFI_DISCONNECT_REASON_CIPHER_SUITE_REJECTED    = 24,  /* 11i */ 
//     WIFI_DISCONNECT_REASON_BEACON_TIMEOUT           = 200, 
//     WIFI_DISCONNECT_REASON_NO_AP_FOUND              = 201, 
//     WIFI_DISCONNECT_REASON_AUTH_FAIL                = 202, 
//     WIFI_DISCONNECT_REASON_ASSOC_FAIL               = 203, 
//     WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT        = 204, 
// };
static void onStationModeDisconnected (const WiFiEventStationModeDisconnected& evt) {
	PRINTF("Station disconnected from SSID %s with reason code %d\n", evt.ssid.c_str(), evt.reason);
	// TODO count disconnections -- if too many, go back to AP mode e.g. password is wrong seems to give 15
	//PRINTLN(macToString(evt.mac));
}

// See https://github.com/esp8266/Arduino/blob/master/cores/esp8266/IPAddress.h re IPAddress class
static void onStationModeGotIP (const WiFiEventStationModeGotIP& evt) {
	PRINTF("Station got IP %s\n", evt.ip.toString().c_str());
	// also has evt.gw and evt.mask
}

// exported
void setup() {

	// Set WIFI module to STA mode
	// // FIXME if ssidCount > 0, but there are no valid ones, we get nowhere (no PRINTF output...)
	WiFi.setAutoConnect(false); // we'll handle the connection
	if (config::ssidCount() == 0) {
		start_ap_mode();
	} else {
		WiFi.mode(WIFI_STA);
	}

	// DON'T FORGET event handlers https://github.com/esp8266/Arduino/blob/master/doc/esp8266wifi/generic-class.rst#wifieventhandler
	// TODO use events instead of the monitor thing
	// e.g.:
	//disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event)
	//WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event) {
	////WiFi.onStationModeGotIP([](){
	//	//PRINTF("Station connected, IP: %s", WiFi.localIP().toString().c_str());
	//	PRINTF("Station connected, IP: %s", wifi_ip4().c_str());
	//});
	stationConnectedHandler    = WiFi.onStationModeConnected(&onStationModeConnected);
	stationDisconnectedHandler = WiFi.onStationModeDisconnected(&onStationModeDisconnected);
	stationGotIPHandler        = WiFi.onStationModeGotIP(&onStationModeGotIP);

	// FIXME this PRINTF causes repeated crashes: //PRINTF("wifi::setup: ssidCount = %n\n", config::ssidCount());
	monitor_wifi();
	//timers::setTimer(TIMER_MONITOR_WIFI, 5000, monitor_wifi);

}

//void loop () {
//}

} // namespace

// vim: set ts=4 sw=4 tw=0 noet : 
