// wifi.h

#ifndef HEADER_WIFI
#define HEADER_WIFI

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>

void wifi_setup (void);

//void wifi_loop (void);

String wifi_ssid (void);
String wifi_ip4 (void);
String wifi_ip6 (void);
String wifi_status (void);
int wifi_status_code ();
void stop_ap_mode ();
void start_sta_mode ();
void handleWifiGet (AsyncWebServerRequest* req);
//void handleWifiSet (AsyncWebServerRequest* req);
extern AsyncCallbackJsonWebHandler* handleWifiSetJSON;
//int best_wifi (int networksFound);

#endif

// vim: set ts=4 sw=4 tw=0 noet :
