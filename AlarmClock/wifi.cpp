// wifi.cpp

// TODO go straight to wifi page in AP mode
//    -- and no internet available, so can't style with BS4 online!


#include <ESP8266WiFi.h>
//#include <WString.h>
/* in .h #include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h> */
#include "debug.h"
#include "wifi.h"
#include "timers.h"
#include "config.h"
//#include "json.h"

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

// These handlers have to be here (i.e., global and not static) so that they exist when needed.
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
	WiFi.softAP(configw.hostname);
	//WiFi.printDiag(Serial);
	PRINTF("started AP mode at %s\n", wifi_ip4().c_str());
	// If there are any known SSIDs, revert to station mode after a while
	if (ssidCount() > 0) {
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

// Return the index (into configw.wifi) of the current best network.
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
		for (int j = 0; j < wifimax; j++) {
			//PRINTF("matching '%s' with '%s'\n", WiFi.SSID(i).c_str(), configw.wifi[j].ssid);
			if (strcmp(WiFi.SSID(i).c_str(), configw.wifi[j].ssid) == 0) {
				PRINTF("rsr got matching network %s with strength %i\r\n", configw.wifi[j].ssid, WiFi.RSSI(i));
				if (rssi > best_rssi) {
					best_rssi = rssi;
					best_ssid_index = j;
				}
				// got a match
				//WiFi.mode(WIFI_STA);
				//WiFi.begin(configw.wifi[j].ssid, configw.wifi[j].psk);
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
		WiFi.begin(configw.wifi[best_ssid_index].ssid, configw.wifi[best_ssid_index].psk);
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
	unsigned long delay = 10000; // default delay until running this function again
	//	WiFi.mode(m): set mode to WIFI_AP, WIFI_STA, WIFI_AP_STA or WIFI_OFF
	switch (WiFi.getMode()) { // : return current Wi-Fi mode (one out of four modes above)
		case WIFI_OFF:
			// Shouldn't be off -- turn it on
			PRINTLN("WiFi is off -- going to STA");
			WiFi.mode(WIFI_STA);
			delay = 2000; // don't wait so long before trying again
			break;
		case WIFI_AP_STA:
			// Shouldn't be that - go to just STA
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
				if (ssidCount() == 0) {
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

static void onStationModeDisconnected (const WiFiEventStationModeDisconnected& evt) {
	PRINTF("Station disconnected from SSID %s with reason code %d\n", evt.ssid.c_str(), evt.reason);
	//PRINTLN(macToString(evt.mac));
}

// See https://github.com/esp8266/Arduino/blob/master/cores/esp8266/IPAddress.h re IPAddress class
static void onStationModeGotIP (const WiFiEventStationModeGotIP& evt) {
	PRINTF("Station got IP %s\n", evt.ip.toString().c_str());
	// also has evt.gw and evt.mask
}

// from https://github.com/me-no-dev/ESPAsyncWebServer#arduinojson-basic-response
void handleWifiGet (AsyncWebServerRequest* req) {
	AsyncResponseStream *response = req->beginResponseStream("application/json");
	StaticJsonDocument<200> jsonDoc;
	for (int i = 0; i < wifimax; i++) {
        JsonObject obj = jsonDoc.createNestedObject();
		obj["wifissid"] = configw.wifi[i].ssid;
		// FIXME don't send psk -- send some sort of hash that we can compare to see if user has changed it
		obj["wifipass"] = "(hidden)"; // configw.wifi[i].psk;
    }
	//PRINTF("hWG: jsonDoc size is %d\n", jsonDoc.memoryUsage()); // 168
	serializeJson(jsonDoc, *response);
	req->send(response);
}

/*
// This for dev only
static void list_all_parameters (AsyncWebServerRequest *req) {
	//List all parameters
	int params = req->params();
	PRINTF("URL: %s\n", req->url().c_str());
	for (int i = 0; i < params; i++) {
		AsyncWebParameter *p = req->getParam(i);
		if (p->isFile()) { //p->isPost() is also true
			PRINTF("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
		} else if (p->isPost()) {
			PRINTF("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
		} else {
			PRINTF("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
		}
	}
	PRINTF("	= %d params\n", params);
}
*/

// Receive AP / wifi update from web page.
// NOTE That no authentication and very little validation is done.
// (.toInt() returns 0 for an invalid numeric string)
/*
// NOTE this version gets data as 'body' (from new-fangled 'fetch' in javascript)
void handleWifiSet (AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  if (!index){
    Serial.printf("BodyStart: %u B\n", total);
  }
  for (size_t i = 0; i < len; i++) {
    Serial.write(data[i]);
  }
  if (index + len == total){
    Serial.printf("BodyEnd: %u B\n", total);
  }
}
*/

// FIXME better name:
void handleWifiSetJSON2 (AsyncWebServerRequest *req, JsonVariant json) {
	JsonArray jsonArray = json.as<JsonArray>();

	//PRINTLN("hWSJ starting");
	// Start with old config
	configw_t new_configw = configw;

	// pack the wifis into the config (move any empty ones to the end)
	int j = 0;
	// for (int i = 0; i < wifimax; i++) {
	for (JsonVariant item: jsonArray) {
		//Serial.println(value.as<char*>());
		//String ssidkey = "wifissid" + String(i);
		//String passkey = "wifipass" + String(i);
		//PRINTF("hWSJ: i=%d ssidkey=%s passkey=%s\n", i, ssidkey.c_str(), passkey.c_str());
		if (item["wifissid"] != "") {
			//if (wifi[i].ssid != "") {
			//PRINTF("hWSJ: i=%d key=%s value=%s\n", i, ssidkey.c_str(), jsonObj[ssidkey].as<char*>());
			strlcpy(new_configw.wifi[j].ssid, item["wifissid"], sizeof(new_configw.wifi[j].ssid));
			strlcpy(new_configw.wifi[j].psk,  item["wifipass"], sizeof(new_configw.wifi[j].psk));
			j += 1;
		}
		for (int i = j; i < wifimax; i++) {
			new_configw.wifi[i].ssid[0] = '\0';
			new_configw.wifi[i].psk[0] = '\0';
		}
	}
	storeConfigW(&new_configw);
	handleWifiGet(req);

}

// Body is JSON:
// FIXME this is main line -- should be in AlarmClock.ino??
AsyncCallbackJsonWebHandler* handleWifiSetJSON = new AsyncCallbackJsonWebHandler("/wifiSet", handleWifiSetJSON2);

/*
void handleWifiSetOld (AsyncWebServerRequest* req) {

	PRINTLN("handleWifiSet...");
	list_all_parameters(req);

	// Start with old config
	configw_t new_configw = configw;

	struct {
		String ssid;
		String pass;
	} wifi[wifimax];

	for (int i = 0; i < wifimax; i++) {
		String ssidstring = "wifissid" + String(i);
		String passstring = "wifipass" + String(i);
		wifi[i].ssid = "";
		wifi[i].pass = "";
		if (req->hasParam(ssidstring, true)) {  // true for POST
			String ssid = req->getParam(ssidstring, true)->value();
			if (ssid == "") {
				// ssid cleared -- delete from config
			} else {
				wifi[i].ssid = ssid;
				if (req->hasParam(passstring, true)) {
					wifi[i].pass = req->getParam(passstring, true)->value();
				}
			}
			PRINTF("\t%s = %s\n", ssidstring.c_str(), ssid.c_str());
		}
	}

	// pack the wifis into the config
	int j = 0;
	for (int i = 0; i < wifimax; i++) {
		if (wifi[i].ssid != "") {
			strlcpy(new_configw.wifi[j].ssid, wifi[i].ssid.c_str(), sizeof(new_configw.wifi[i].ssid));
			strlcpy(new_configw.wifi[j].psk,  wifi[i].pass.c_str(), sizeof(new_configw.wifi[i].psk));
			j += 1;
		}
	}
	for (int i = j; i < wifimax; i++) {
		new_configw.wifi[i].ssid[0] = '\0';
		new_configw.wifi[i].psk[0] = '\0';
	}

	storeConfigW(&new_configw);
	//req->send(200, "text/plain", "done");
	handleWifiGet(req);

	// Try to connect to one from the updated list of networks
	timers::setTimer(TIMER_NEW_SSID_DELAY, 5000, start_sta_mode);
}
*/

// exported
void wifi_setup() {

	// Set WIFI module to STA mode
	WiFi.setAutoConnect(false); // we'll handle the connection
	if (ssidCount() == 0) {
		start_ap_mode();
	} else {
		WiFi.mode(WIFI_STA);
	}

	timers::setTimer(TIMER_MONITOR_WIFI, 5000, monitor_wifi);

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

}

//void wifi_loop () {
//}

// vim: set ts=4 sw=4 tw=0 noet : 
