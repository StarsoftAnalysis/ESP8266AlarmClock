/*******************************************************************
    ESP8266AlarmClock
    An Alarm clock that gets its time from the internet
    for use with Brian Lough's alarm clock kit at https://www.tindie.com/products/15402/
    (or similar ESP8266-based circuit)

    This version by Chris Dennis
    Copyright 2021 Chris Dennis

    Modified from Brian Lough's code at https://github.com/witnessmenow/arduino-alarm-clock,
    with additional ideas and code from James Brown's version at https://github.com/jbrown123/arduino-alarm-clock 

    ---
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ESP8266AlarmClock is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ESP8266AlarmClock.  If not, see <https://www.gnu.org/licenses/>.
    ---

    Features:
    - Web interface for setting the alarm
    - Captive portal for setting WiFi Details
    - Set your local timezone
    - Automatically adjusts for DST
    - snooze and repeat alarm
    - RTTTL tunes
    - display is completely off at night, and comes on if any button is pressed
    - cancel (and uncancel) next alarm  via buttons

    Features planned:
    - set alarm times via buttons -- really?  there are seven! -- perhaps just the next one
    - likewise for current time (in case internet is not available)

****************************************************************/

// TODO:
// OH!  Can have two alarms in next 24 hours -- e.g. now 8pm, alarms for 11pm and 11am tomorrow -- how does this affect nextAlarmIndex etc.?
// * cancelling an upcoming alarm with the buttons cancels it for next week too!
        // DONE html/js: show if next alarm is overridden (and time to next alarm)
        //          allow nextAlarmOverridden to be toggled
        // NO put nextAlarmOverridden in the config
        // DONE remove the override when the alarm rings! so that it doesn't apply to the next alarm
        // BETTER PLAN -- nextAlarmCancelled -- subtle difference
        // Deal with two alarms in next 24 hours
        // OPtimise by calling nextSetAlarm... etc. every loop and storing the answer (displayAlarm calls it, and gets called every loop anyway)
// * getNextAlarmIn() doesn't deal with daylight savings start/end -- does ezTime have the relevant functions?
// * show version on display
// * more delay on brightness to stop flashing
// * send version to HTML via JS
// * put alarmPauseTime etc onto settings page and into config
// * --:-- is not 00:00 - need to completely unset an alarm so that the button doesn't set it
// * getData to also get display status, not just time e.g. boot Wifi 55 etc.  -- send what's on the display as text e.g. "12:34"
//     and  display status e.g. 'Alarm ringing', and 'Time to next alarm' on webpage
// * update all License bits with full name and dates
// * allow user to set repeat/snooze times etc.
// * get rid of bootstrap
// * separate 'Settings' web page'
//   - need to separate .js file and use it for both pages
// * move svgs etc. into a folder
// * make use of PRINTLN vs Serial.println consistent.
// * if no internet, prompt for manual time setting
// * cope with losing wifi -- gets stuck on WIFI display
// * redo nextAlarm (to allow for more than one alarm per day): get list of alarms in next 24h, choose first one that is set (may return 'none')
// * more aria labels in html
// * allow user to set light level for turning display off in settings
// * use #defines to set e.g. SNOOZE = LBUTTON etc. for ease of customisation
// * Use NVS instead of EEPROM?
//    - and/or use EZTime's cache
// * button functions --  'config' more
//   - set alarm time etc. e.g. left button held to cycle through modes: hr, min, alrmH, or use libraryr, alrmMin, alrmSet, exit (for next 24h), left=- right=+, hold for next mode.    e.g. setting hour, flash left 2 numbers; add colon when doing alarm, just show colon in alrmSet modei (+/- turns it on/off)
// * alarm to get louder gradually
// * HTTPS? -- see http://www.iotsharing.com/2017/08/how-to-use-https-in-arduino-esp32.html
// * ezt::setServer(String("pool.ntp.org")); // TODO make this a web page option?

// NO
// - use ezTime events to trigger alarm?
// * store alarm time as minutes since midnight instead of hour/minute
// * ? define alarms as repeating or one-off.  or separate set of one-off alarms
// * maybe use bool ezt's secondChanged() to display the time less often - no, frequent updates are good
// * test RTTTL by playing it???  No -- give a link to a site to test it https://adamonsoon.github.io/rtttl-play/

// DONE
// * async web server?? or more consistent use of webActive??
// * off completely if ldr < min -- not just on level
// * colon if seconds % 2
// * get rid if mdNS stuff
// * bug? 
//   - rebooted when button pressed for too long to see time at night
//   - rebooted at alarm time
// * light on when button pressed -- keep it on for e.g. 5 seconds
// * EEPROM instead of SPIFFS for config
// * JSON v6
// * home-grown timers instead of pseudothreads
// * need to not re-trigger alarm if turned off or timed out within the minute
// * config in flash?
// * snooze
// * repeat e.g. alarm of after 5 minutes, then on again at 10 minutes, etc.
// * snooze/pause times and counts in config and webpage
// * web page 
//   - choose from list of tunes
//   - choose NTP server, time zone, DST rule
// * GPL
// * debounce buttons -- just one pause at a time:
// * use ezt namespace
// * 0:03 displays as '  : 3' !!
// * only check for alarm once a minute (will avoid the nasty hack in nextAlarm())
// * change time zone when it's changed on the gui
// * get RTTTL to adjust volume -- use analogWriteRange or Resolution??
// * display sometimes flickers -- depends on light level -- do running average?  maybe it only flickers while uploading software
// * Updating the tune -- need to do e8rtp re-setup etc.
// * OTA updates
// * get rid of OTA updates (to allow async etc.)
// * allow user to cancel alarm before it goes off
// * put volume on main alarm page

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include <ezTime.h>
// Library used for getting the time and adjusting for DST
// Search for "ezTime" in the Arduino Library manager
// https://github.com/ropg/ezTime

#include <TM1637Display.h>
// Library used for controlling the 7 Segment display
// Search for "TM1637" in the Arduino Library manager
// https://github.com/avishorp/TM1637

#include "ArduinoJson.h"
// v6
// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson

#include "wifi.h"
//#include <WiFiManager.h>
//// Library used for creating the captive portal for entering WiFi Details
//// Search for "Wifimanager" in the Arduino Library manager
//// https://github.com/tzapu/WiFiManager

//#include <TimerFreeTone.h>
//#include <NonBlockingRtttl.h>
#include <ESP8266RTTTLPlus.h>

#include "debug.h"

// Home-made timers
#include "timers.h"

// Config -- stored in EEPROM
#include "config.h"

// Letters as display segments
#include "displayConf.h"

// Web pages and local javascript.
// Defines "const char mainpage[]":
#include "mainpage.h"
// Defines "const char settingspage[]":
#include "settingspage.h"
// Defines "const char mainjs[]":
#include "mainjs.h"
// Defines "const char settingsjs[]":
#include "settingsjs.h"
// Bootstrap 5 CSS and JS
// Defines "const char bootstrapcss[]":
#include "bootstrap.min.css.h"
// Defines "const char bootstrapjs[]":
#include "bootstrap.bundle.min.js.h"

// -----------------------------

// --- Pin Configuration ---
#define CLK_PIN     D6
#define DIO_PIN     D5
#define BUZZER_PIN  D1
#define RBUTTON_PIN D2   // SW1
#define LBUTTON_PIN D3   // SW2
#define LDR_PIN     A0
#define LED_PIN     LED_BUILTIN

// ------------------------

// Set this in mainpage.h as well
#define VERSION "0.2.3dev"

// Globals ------------------------

TM1637Display display(CLK_PIN, DIO_PIN);

ESP8266WebServer server(80);

static Timezone TZ;

#define KEEP_LIGHT_ON_TIME 5000
static bool keepingLightOn = false;

struct {
    bool lButtonPressed;    // left button
    bool lButtonLong;
    bool rButtonPressed;    // right button
    bool rButtonLong;
    bool aButtonPressed;    // any button
    bool bButtonPressed;    // both buttons
} buttonState;
#define LONG_BUTTON_PRESS_TIME 2000

// Alarm state etc.
enum class                 alarmStateEnum { Off, Ringing, Snoozed, Paused, Stopped };
static                     alarmStateEnum alarmState = alarmStateEnum::Off;
static long unsigned       alarmStateStart;
static int                 alarmRepeatCount;
static int                 alarmSnoozeCount;
// These could be user-configurable:    TODO
static const long unsigned alarmPauseTime  = 60 * 1000;  // ms
static const long unsigned alarmRingTime   = 60 * 1000;  // ms
static const int           alarmRepeatMax  = 3;
static const int           alarmSnoozeMax  = 3;
static const long unsigned alarmSnoozeTime = 60 * 1000;  // ms

// 'Loop globals' -- get calculated each time round the loop in calcLoopGlobals() ------------------------

static int  nextSetAlarmIndex  = -1;          // -1 means no alarm set
static long nextSetAlarmIn     = -1;          // in minutes; -1 means no alarm set
static bool nextAlarmCancelled = false;       // true if the next alarm has been cancelled
static int  today    = 0;                     // today.  0 = Sunday  (ezTime has 1=Sunday)
static int  tomorrow = 0;                     // tomorrow
static int  nowHr    = 0;                     // current hour
static int  nowMn    = 0;                     // current minute
static config::alarmDetails_t *nextSetAlarm  = nullptr; // next alarm
static config::alarmDetails_t *alarmToday    = nullptr; // today's alarm
static config::alarmDetails_t *alarmTomorrow = nullptr; // tomorrow's alarm

// ------------------------
 
// converts the dBm to a range between 0 and 100%
static int getWifiQuality() {
    int32_t dbm = WiFi.RSSI();
    if (dbm <= -100) {
        return 0;
    } else if (dbm >= -50) {
        return 100;
    }
    return 2 * (dbm + 100);
}

void dontKeepLightOn (void) {
    keepingLightOn = false;
}

// HTTP request handlers:

void handleMain() {
    PRINTF("Sending '/' length %d\n", strlen_P(mainpage));
    server.send(200, "text/html", mainpage);
}
void handleSettings() {
    PRINTF("Sending '/settings' length %d\n", strlen_P(settingspage));
    server.send(200, "text/html", settingspage);
}
void handleJS() {
    PRINTF("Sending '/mainjs' length %d\n", strlen_P(mainjs));
    server.send(200, "text/javascript", mainjs);
}
void handleSettingsJS() {
    PRINTF("Sending '/settingsjs' length %d\n", strlen_P(settingsjs));
    server.send(200, "text/javascript", settingsjs);
}
void handleBootstrapJS() {
    server.send(200, "text/javascript", bootstrapjs);
}
void handleBootstrapCSS() {
    server.send(200, "text/css", bootstrapcss);
}

void handleNotFound() {
    char message[] = "hNF: 404";
    PRINTLN(message);
    server.send(404, "text/plain", message);
}

// Get the index (into config.alarmDay) of the next set alarm.
// Returns -1 if there is no alarm set in the next 24 hours.
// This does not take nextAlarmCancelled into account (and nor should it).
static int getNextSetAlarmIndex (void) {
    int next = -1;
    // Get 'now' as day of week, hour, minute
    //int tdy = TZ.weekday() - 1;  // today.  0 = Sunday  (ezTime has 1=Sunday)
    //int tmw = (tdy + 1) % 7;     // tomorrow
    //int hr = TZ.hour();
    //int mn = TZ.minute();
    //config::alarmDetails_t today = config::config.alarmDay[tdy];
    //config::alarmDetails_t tomorrow = config::config.alarmDay[tmw];
    if ((alarmToday->set) && ((alarmToday->hour > nowHr) || ((alarmToday->hour == nowHr) && (alarmToday->minute >= nowMn)))) {
        // Next set alarm is later today
        //if (!nextAlarmCancelled) {
            next = today;
        //}
    }
    if ((next == -1) && (alarmTomorrow->set) && ((alarmTomorrow->hour < nowHr) || ((alarmTomorrow->hour == nowHr) && (alarmTomorrow->minute < nowMn)))) {
        // Next set alarm is tomorrow
        //if (!nextAlarmCancelled) {
            next = tomorrow;
        //}
    }
    //PRINTF("nSAI: tdy=%d tmw=%d hr=%d mn=%d today=%d:%d.%d tmrw=%d:%d.%d next=%d nac=%d\n", 
    //    tdy, tmw, hr, mn, today.hour, today.minute, today.set, tomorrow.hour, tomorrow.minute, tomorrow.set, next, nextAlarmCancelled);
    return next;
}

// Return number of minutes until next set alarm (or -1 if none due in the next 24 hours)
// FIXME allow for daylight saving start or end between now and the alarm times.
static long getNextSetAlarmIn (void) {
    if (nextSetAlarmIndex >= 0) {
        //config::alarmDetails_t alarm = config::config.alarmDay[nextSetAlarmIndex];
        //int today = TZ.weekday() - 1;  // today  0 = Sunday  (ezTime has 1=Sunday)
        //int hr = TZ.hour();
        //int mn = TZ.minute();
        if (nextSetAlarmIndex == today) {
            // alarm is later today
            long minutes = (nextSetAlarm->hour - nowHr) * 60 + (nextSetAlarm->minute - nowMn);
            return minutes;
        } else {
            // alarm is tomorrow
            long minutes = (23 - nowHr) * 60 + (59 - nowMn);         // time left today
            minutes += nextSetAlarm->hour * 60 + nextSetAlarm->minute; // time until alarm tomorrow
            return minutes;
        } 
    } 
    return -1;
}

// Calculate and store next alarm details etc.
// This gets called from the loop, so that other functions such as displayTime
// can access the results quickly.
static void calcLoopGlobals (void) {
    // Need these 'now' values...
    today    = TZ.weekday() - 1;  // today.  0 = Sunday  (ezTime has 1=Sunday)
    tomorrow = (today + 1) % 7;   // tomorrow
    nowHr    = TZ.hour();
    nowMn    = TZ.minute();
    alarmToday    = &(config::config.alarmDay[today]);
    alarmTomorrow = &(config::config.alarmDay[tomorrow]);
    // ...before calling these functions:
    nextSetAlarmIndex = getNextSetAlarmIndex();
    nextSetAlarm      = &(config::config.alarmDay[nextSetAlarmIndex]);
    nextSetAlarmIn    = getNextSetAlarmIn();
    // If there's no alarm in the next 24 hours, the cancelled
    // flag is meaningless, so clear it to keep things unambiguous.
    if (nextSetAlarmIndex < 0) {
        nextAlarmCancelled = false;
    }
}

void addAlarmToJsonDoc(DynamicJsonDocument *json) {
    JsonArray days = json->createNestedArray("alarmDay");
    for (int i = 0; i < 7; i++) {
        JsonObject obj = days.createNestedObject();
        char timeString[6];
        sprintf(timeString, "%02d:%02d", config::config.alarmDay[i].hour, config::config.alarmDay[i].minute);
        obj["alarmTime"] = timeString;
        obj["alarmSet"] = (config::config.alarmDay[i].set ? "1" : "0");
    }
    (*json)["volume"] = config::config.volume;
}

void handleGetData () {
    // Light level
    int a = analogRead(LDR_PIN);
    // WiFi signal
    int rssi = getWifiQuality();
    // Current time
    String time = TZ.dateTime("H:i");
    // Next alarm overriden
    // Next alarm time

    DynamicJsonDocument json(1512); // FIXME tune this

    json["lightLevel"] = String(a);
    json["wifiQuality"] = String(rssi);
    json["time"] = String(time);
    json["nextSetAlarmIndex"] = nextSetAlarmIndex;
    json["nextSetAlarmIn"] = nextSetAlarmIn;
    json["nextAlarmCancelled"] = nextAlarmCancelled;

    // Also send alarm data
    addAlarmToJsonDoc(&json);

    String s;
    serializeJson(json, s);
    //Serial.printf("hGD: sending %s\n", s.c_str());
    server.send(200, "text/plain", s);
}

void handleSetAlarm() {

    config::config_t newConfig = config::config;    // copy of old config

    DynamicJsonDocument doc(512);
    deserializeJson(doc, server.arg("plain"));   // body
    PRINTF("hSA: args: %s\n", server.arg("plain").c_str());

    // Test for existence to avoid losing old values
    if (doc.containsKey("alarmTime")) {
        for (int i = 0; i < 7; i++) {
            String time = doc["alarmTime"][i];
            int indexOfColon = time.indexOf(":");
            newConfig.alarmDay[i].hour   = constrain(time.substring(0,indexOfColon).toInt(), 0, 23);
            newConfig.alarmDay[i].minute = constrain(time.substring(indexOfColon+1).toInt(), 0, 59);
        }
    }
    if (doc.containsKey("alarmSet")) {
        for (int i = 0; i < 7; i++) {
            newConfig.alarmDay[i].set = (doc["alarmSet"][i] == "1");
        }
    }
    if (doc.containsKey("nextAlarmCancelled")) {
        // This flag is just global, not in the config.
        //PRINTF("hSA: nextAlarmCancelled was %d, setting to %s\n", nextAlarmCancelled, doc["nextAlarmCancelled"].as<String>().c_str());
        nextAlarmCancelled = doc["nextAlarmCancelled"]; //.as<boolean>;
        //PRINTF("hSA: nextAlarmCancelled now %d\n", nextAlarmCancelled);
    }
    if (doc.containsKey("volume")) {
        newConfig.volume = doc["volume"];
    }
    storeConfig(&newConfig);    // also copies newConfig to config

    server.send(200, "text/html", "OK");
}

void handleSetSettings() {

    config::config_t newConfig = config::config;    // copy old config

    DynamicJsonDocument doc(1512);  // FIXME tune this
    deserializeJson(doc, server.arg("plain"));   // body
    //Serial.println("hGA: args: ", server.arg("plain"));

    // Test for existence to avoid losing old values
    if (doc.containsKey("melody")) {
        strlcpy(newConfig.melody, doc["melody"], MELODY_MAX);
    }
    if (doc.containsKey("tz")) {
        strlcpy(newConfig.tz, doc["tz"], TZ_MAX);
    }

    // Play the new melody if it's changed and we're currently playing
    if (strcmp(newConfig.melody, config::config.melody) == 0) {
        if (e8rtp::state() == e8rtp::Playing) {
            e8rtp::stop();
        }
        e8rtp::setup(BUZZER_PIN, newConfig.volume, newConfig.melody);
    }

    // Set timezone if it's changed
    if (newConfig.tz != config::config.tz) {
        TZ.setLocation(newConfig.tz);
    }

    // Validate and store wifi details
    int ssidCount = 0;
    bool wifiSettingsChanged = false;
    if (doc.containsKey("wifissid")) {  // assume that "wifipass" is there too
/*
        struct {
            String ssid;
            String pass;
        } wifi[WIFI_MAX];
*/
        JsonArray ssids = doc["wifissid"].as<JsonArray>();
        JsonArray passes = doc["wifipass"].as<JsonArray>();
        for (size_t i = 0; i < WIFI_MAX; i++) {
            newConfig.wifi[i].ssid[0] = '\0';
            newConfig.wifi[i].pass[0] = '\0';
            //wifi[i].ssid = "";
            //wifi[i].pass = "";
            if (ssids[i].as<String>() == "") {
                // ssid cleared -- delete from config
            } else {
                strlcpy(newConfig.wifi[ssidCount].ssid, ssids[i].as<String>().c_str(), sizeof(newConfig.wifi[i].ssid));
                if (passes[i] == nullptr) {
                    // keep the old password
                    strlcpy(newConfig.wifi[ssidCount].pass, config::config.wifi[ssidCount].pass, sizeof(newConfig.wifi[i].pass));
                } else {
                    // store the new password
                    strlcpy(newConfig.wifi[ssidCount].pass, passes[i].as<String>().c_str(), sizeof(newConfig.wifi[i].pass));
                }
                if (strcmp(newConfig.wifi[ssidCount].ssid, config::config.wifi[ssidCount].ssid) != 0 ||
                    strcmp(newConfig.wifi[ssidCount].pass, config::config.wifi[ssidCount].pass) != 0    ) {
             //   if (wifi[i].ssid != config::config.wifi[i].ssid ||
               //     wifi[i].pass != config::config.wifi[i].pass    ) {
                    wifiSettingsChanged = true;
                }
                ssidCount += 1;
            }
        }
/*
        // pack the wifis into the config
        for (int i = 0; i < WIFI_MAX; i++) {
            if (i < ssidCount) {
                strlcpy(newConfig.wifi[i].ssid, wifi[i].ssid.c_str(), sizeof(newConfig.wifi[i].ssid));
                strlcpy(newConfig.wifi[i].pass, wifi[i].pass.c_str(), sizeof(newConfig.wifi[i].pass));
            } else {
                newConfig.wifi[i].ssid[0] = '\0';
                newConfig.wifi[i].pass[0] = '\0';
            }
        }
*/
    }
    storeConfig(&newConfig);    // also copies newConfig to config

    if (wifiSettingsChanged) {
        // Tell wifi to try again
        wifi::newSSIDs();
    }

    //FIXME if unexpected stuff sent, return non-200
    // FIXME supply sensible defaults e.g. to melody
    server.send(200, "text/html", "OK");
}

void handleGetAlarm() {
    DynamicJsonDocument json(1000); // FIXME tune this
    addAlarmToJsonDoc(&json);
    String s;
    serializeJson(json, s);
    Serial.printf("hGA: sending %s\n", s.c_str());
    server.send(200, "text/plain", s);
}

void handleGetSettings() {

    DynamicJsonDocument json(1512); // FIXME tune this

    json["melody"] = String(config::config.melody);
    json["tz"] = String(config::config.tz);

    JsonArray wifissid = json.createNestedArray("wifissid");
    JsonArray wifipass = json.createNestedArray("wifipass");
    // Don't send the passwords! -- just the length
    for (int i = 0; i < WIFI_MAX; i++) {
        wifissid[i] = config::config.wifi[i].ssid;
        wifipass[i] = strlen(config::config.wifi[i].pass);
    }

    String s;
    serializeJson(json, s);
    Serial.printf("hGS: sending %s\n", s.c_str());
    server.send(200, "text/plain", s);

}

//void configModeCallback (WiFiManager *myWiFiManager) {
//    Serial.println("Entered WiFi config mode");
//    Serial.println(WiFi.softAPIP());
//    display.setSegments(SEG_WIFI);
//}

static void setButtonStates (void) {
    // Check for button presses, with 'debouncing' so that a single press is only detected for one loop.
    // 'before' means in a previous loop.
    // This function must be called early in the loop, before other functions that use buttonState.
    static bool lButtonPressedBefore = false;
    static bool rButtonPressedBefore = false;
    static bool lButtonLongPressedBefore = false;
    static bool rButtonLongPressedBefore = false;
    static long unsigned lButtonPressedTime = 0;
    static long unsigned rButtonPressedTime = 0;
    long unsigned now = millis();

    bool lButtonPressed = (digitalRead(LBUTTON_PIN) == LOW);
    bool rButtonPressed = (digitalRead(RBUTTON_PIN) == LOW);

    // Reset all buttonState flags first.
    buttonState.lButtonPressed = false;
    buttonState.rButtonPressed = false;
    buttonState.lButtonLong = false;
    buttonState.rButtonLong = false;

    // Consider combinations for button pressed now and before:
    //   Neither: do nothing
    //   Before, not now: set both flags false, and flag in struct false
    //   Not before, but now: set flag in struct true, 
    //   Before and now: same 'event': set flag in struct false so that other functions won't act on it again
    // At end, set before = now
    if (lButtonPressed) {
        if (lButtonPressedBefore) {
            // Not a new press
            buttonState.lButtonPressed = false;
            if (now - lButtonPressedTime > LONG_BUTTON_PRESS_TIME) {
                // button has been pressed for long enough to count as a long press
                buttonState.lButtonLong = !lButtonLongPressedBefore; // true;
                lButtonLongPressedBefore = true;
            }
        } else {
            buttonState.lButtonPressed = true;
            // Start timer for a long press
            lButtonPressedTime = now;
            lButtonLongPressedBefore = false;
        }
    }
    lButtonPressedBefore = lButtonPressed;
    if (rButtonPressed) {
        if (rButtonPressedBefore) {
            // Not a new press
            buttonState.rButtonPressed = false;
            if (now - rButtonPressedTime > LONG_BUTTON_PRESS_TIME) {
                // button has been pressed for long enough to count as a long press
                buttonState.rButtonLong = !rButtonLongPressedBefore; // true;
                rButtonLongPressedBefore = true;
            }
        } else {
            buttonState.rButtonPressed = true;
            // Start timer for a long press
            rButtonPressedTime = now;
            rButtonLongPressedBefore = false;
        }
    }
    rButtonPressedBefore = rButtonPressed;

    buttonState.aButtonPressed = (buttonState.lButtonPressed || buttonState.rButtonPressed);
    buttonState.bButtonPressed = (buttonState.lButtonPressed && buttonState.rButtonPressed);
}

// Calculate exponential moving average to smooth the light level readings.
// From https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average
static int exponentialMovingAverage (int value) {
    static float ema = 1023.0 / 2.0;  // Initialise to half the maximum sensor value
    static const float alpha = 0.1;   // Tune this from 0.0 to 0.1: small values for lots of smoothing, bigger for quicker response
    ema = alpha * (float)value + (1.0 - alpha) * ema;
    return round(ema);
}

// Adjust the display brightness.
// The LDR returns a value between 0 and 1023: apply smoothing,
// and convert to the range 0..3, which is all the TM1637 seems to handle.
// If the light is very low, turn off the display completely,
// unless a button has been pressed recently, or the alarm is ringing.
static void adjustBrightness() {
    int sensorValue = exponentialMovingAverage(analogRead(LDR_PIN));
    int level = sensorValue / 256;
    //Serial.printf("aB: ldr=%d level=%d button=%d\n", sensorValue, level, buttonState.aButtonPressed);
    display.setBrightness(level, sensorValue > 16 || keepingLightOn || alarmState == alarmStateEnum::Ringing);
}

static void checkForAlarm () {
    //int hr = TZ.hour();
    //int mn = TZ.minute();
    long unsigned nowMillis = millis();
    static int stopMinute = -1; // The minute when the alarm was stopped
    //PRINTF("cFA: alarmState is %d   time is %02d:%02d\n", alarmState, nowHr, nowMn);
    switch (alarmState) {
        case alarmStateEnum::Off: {
            if ((nextSetAlarmIndex >= 0) && !nextAlarmCancelled)  {
                //config::alarmDetails_t alarm = config::config.alarmDay[i];
                //PRINTF("cFA: off, next alarm is %02d:%02d %d\n", alarm.hour, alarm.minute, alarm.set);
                if (nowHr == nextSetAlarm->hour && nowMn == nextSetAlarm->minute) {
                    // we're at the set time -- start ringing
                    alarmState = alarmStateEnum::Ringing;
                    alarmStateStart = nowMillis;  
                    alarmRepeatCount = 0;
                    alarmSnoozeCount = 0;
                    e8rtp::start();
                    // cancel alarm override (whichever way it was toggled before)
                    nextAlarmCancelled = false;
                    PRINTF("cFA: start ringing for alarm %02d:%02d\n", nextSetAlarm->hour, nextSetAlarm->minute);
                }
            }
            break;
        }
        case alarmStateEnum::Ringing: {
            // FIXME consider the order of checking button presses here and in other cases
            if (buttonState.aButtonPressed) {
                if (buttonState.lButtonPressed) {   // TODO better choices of long/short/both button presses
                    // snooze
                    alarmState = alarmStateEnum::Snoozed;
                    alarmStateStart = nowMillis;
                    alarmSnoozeCount = 1;
                    e8rtp::pause();
                    PRINTF("cFA: ringing -> snooze no. %d (lButton)\n", alarmSnoozeCount);
                } else if (buttonState.rButtonPressed) {   // TODO better choices of long/short/both button presses
                    // stop
                    alarmState = alarmStateEnum::Stopped;
                    stopMinute = nowMn;
                    alarmStateStart = nowMillis;  // not needed
                    e8rtp::stop();
                    PRINTLN("cFA: ringing -> stopped (rButton)");
                }
            } else if (nowMillis - alarmStateStart > alarmRingTime) {
                if (alarmRepeatCount >= alarmRepeatMax) {
                    // alarm ignored for long enough -- stop
                    alarmState = alarmStateEnum::Stopped;
                    stopMinute = nowMn;
                    e8rtp::stop();
                    PRINTLN("cFA: ringing -> stopped (timeout)");
                } else {
                    // pause
                    alarmState= alarmStateEnum::Paused;
                    alarmStateStart = nowMillis;
                    e8rtp::pause();
                    PRINTLN("cFA: ringing -> paused");
                }   
            }
            break;
        }
        case alarmStateEnum::Snoozed: {
            if (buttonState.aButtonPressed) {
                if (buttonState.lButtonPressed) {   // TODO better choices of long/short/both button presses
                    // restart the snooze from now, but increase the count
                    alarmStateStart = nowMillis;
                    alarmSnoozeCount += 1;
                    PRINTF("cFA: snoozing -> snooze no. %d (lButton)\n", alarmSnoozeCount);
                } else if (buttonState.rButtonPressed) {
                    // stop
                    alarmState = alarmStateEnum::Stopped;
                    stopMinute = nowMn;
                    e8rtp::stop();
                    PRINTLN("cFA: snoozing -> stopped (rButton)");
                }
            } else if (nowMillis - alarmStateStart > alarmSnoozeTime) {
                if (alarmSnoozeCount >= alarmSnoozeMax) {
                    // alarm ignored for long enough -- stop
                    alarmState = alarmStateEnum::Stopped;
                    stopMinute = nowMn;
                    e8rtp::stop();
                    PRINTLN("cFA: snoozing stopped (enough snoozes))");
                } else {
                    // back to ringing (but don't reset the counts)
                    alarmState = alarmStateEnum::Ringing;
                    alarmStateStart = nowMillis;  
                    e8rtp::resume();
                    PRINTLN("cFA: snoozing -> ringing (timeout)");
                }
            }
            break;
        }
        case alarmStateEnum::Paused: {
            if (buttonState.aButtonPressed) {
                if (buttonState.lButtonPressed) {   // TODO better choices of long/short/both button presses
                    // start the snooze from now
                    alarmState = alarmStateEnum::Snoozed;
                    alarmStateStart = nowMillis;
                    alarmSnoozeCount += 1;
                    e8rtp::pause(); // (should be paused already)
                    PRINTF("cFA: paused -> snooze no. %d (lButton)\n", alarmSnoozeCount);
                } else if (buttonState.rButtonPressed) {
                    // stop
                    alarmState = alarmStateEnum::Stopped;
                    stopMinute = nowMn;
                    e8rtp::stop();
                    PRINTLN("cFA: paused -> stopped (rButton)");
                }
            } else if (nowMillis - alarmStateStart > alarmPauseTime) {
                // back to ringing 
                alarmState = alarmStateEnum::Ringing;
                alarmStateStart = nowMillis;  
                alarmRepeatCount += 1;
                e8rtp::resume();
                PRINTF("cFA: paused -> ringing, repeat %d\n", alarmRepeatCount);
            }
            break;
        }
        case alarmStateEnum::Stopped: {
            // Go to Off once the minute has changed
            if (nowMn != stopMinute) {
                alarmState = alarmStateEnum::Off; 
                PRINTLN("cFA: stopped -> off");
            }
            break;
        }
    }
    
}

//ICACHE_RAM_ATTR void interuptButton() {
//    buttonPressed = true;
//    return;
//}

static void displayTime (void) {
    uint8_t digits[4];
    //int hr = TZ.hour();
    //int mn = TZ.minute();
    digits[0] = (nowHr > 9) ? display.encodeDigit(nowHr / 10) : 0;    // 0 means blank, just for this digit
    digits[1] = display.encodeDigit(nowHr % 10);
    digits[2] = display.encodeDigit(nowMn / 10);
    digits[3] = display.encodeDigit(nowMn % 10);
    // Toggle colon every second, but keep it on if the alarm is set (and not cancelled)
    bool showColon = ((nextSetAlarmIndex >= 0) && !nextAlarmCancelled) || (TZ.second() % 2);
    if (showColon) {
        digits[1] |= 1 << 7;
    }
    display.setSegments(digits);
}

// Toggle the nextAlarmCancelled flag when user has pressed the relevant button.
// BUT: don't set it to on if there is no alarm set in the next 24 hours.
// FIXME should we set it off even if there is no alarm due??
static void toggleNextAlarmCancelled (void) {
    if (nextSetAlarmIndex >= 0) {
        nextAlarmCancelled ^= true;
        PRINTF("tNAC: nextAlarmCancelled toggled to %oi (nSAI=%d)\n", nextAlarmCancelled, nextSetAlarmIndex);
    }
}

void setup() {

    timers::setup();

    Serial.begin(115200);
    Serial.println("\n=======================");
    Serial.setDebugOutput(false);

    //Onboard LED port Direction output
    pinMode(LED_PIN, OUTPUT); 
    digitalWrite(LED_PIN, HIGH); //LED OFF

    display.setBrightness(2);
    display.setSegments(SEG_BOOT);

    config::setup();
    config::loadConfig(); 

    pinMode(RBUTTON_PIN, INPUT_PULLUP);
    pinMode(LBUTTON_PIN, INPUT_PULLUP);

    //attachInterrupt(RBUTTON_PIN, interuptButton, RISING);  // TODO both buttons, or probably not at all

	wifi::setup();
    //WiFiManager wifiManager;
    //wifiManager.setAPCallback(configModeCallback);
    //wifiManager.autoConnect("ESP8266AlarmClock");  // could check boolean rc from this
    //IPAddress ipAddress = WiFi.localIP();
    //Serial.println("\nWiFi Connected");
    //Serial.print("IP address: ");
    //Serial.println(ipAddress);
    //display.showNumberDec(ipAddress[3], false);
    //delay(1000);

    // HTTP Server
    server.on("/", handleMain);
    server.on("/settings", handleSettings);
    server.on("/mainjs", handleJS);
    server.on("/settingsjs", handleSettingsJS);
    server.on("/bootstrapjs", handleBootstrapJS);
    server.on("/bootstrapcss", handleBootstrapCSS);
    server.on("/setAlarm", handleSetAlarm);
    server.on("/getAlarm", handleGetAlarm);
    server.on("/setSettings", handleSetSettings);
    server.on("/getSettings", handleGetSettings);
    server.on("/getData", handleGetData);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP Server Started");
    // TODO:
    // if(!server.authenticate(username, password)){
    //  server.requestAuthentication();
    // }

    // EZ Time
    //ezt::setDebug(INFO);
    ezt::setInterval(60);
    ezt::setServer(String("pool.ntp.org")); // TODO make this a web page option?
    // NO!, no wifi yet... ezt::waitForSync();
    //Serial.println("\nUTC: " + UTC.dateTime());
    TZ.setLocation(config::config.tz);
    //Serial.println("Local: " + TZ.dateTime());

    // Load the alarm tune
    e8rtp::setup(BUZZER_PIN, config::config.volume, config::config.melody);

    alarmState = alarmStateEnum::Off;
}

void loop() {

    // Library loops
    timers::loop();
    ezt::events();
    e8rtp::loop();
    server.handleClient();

    setButtonStates();

    if (Serial.available() > 0) {
        String input = Serial.readString();
        input.trim();
        // emulate buttons
        if (input == "r") {
            buttonState.rButtonPressed = true;
            buttonState.aButtonPressed = true;
        } else if (input == "l") {
            buttonState.lButtonPressed = true;
            buttonState.aButtonPressed = true;
        } else if (input == "rl") {
            buttonState.rButtonLong = true;
            buttonState.rButtonPressed = true;
            buttonState.aButtonPressed = true;
        } else if (input == "ll") {
            buttonState.lButtonLong = true;
            buttonState.lButtonPressed = true;
            buttonState.aButtonPressed = true;
        } else {
            PRINTF("echo: '%s'\n", input.c_str());
        }
    }

    // debug button states:
    if (buttonState.lButtonPressed) {
        PRINTLN("l button pressed");
    }
    if (buttonState.rButtonPressed) {
        PRINTLN("r button pressed");
    }
    if (buttonState.lButtonLong) {
        PRINTLN("l button long");
    }
    if (buttonState.rButtonLong) {
        PRINTLN("r button long");
    }

    calcLoopGlobals();

    adjustBrightness();

    if (buttonState.aButtonPressed) {
        // Light the display for a while even if it's dark
        keepingLightOn = true;
        timers::setTimer(TIMER_KEEP_LIGHT_ON, KEEP_LIGHT_ON_TIME, dontKeepLightOn); 
    }

    if (buttonState.lButtonLong) {
        display.setSegments(SEG_CONF);
        PRINTLN("Config mode -- would be here...");
        delay(1000);
    }

    if (buttonState.rButtonLong) {
        toggleNextAlarmCancelled();
    }

    if ((alarmState == alarmStateEnum::Ringing) && (e8rtp::state() != e8rtp::Playing)) {
        // Repeat the tune
        e8rtp::start();
    }

    displayTime();
    checkForAlarm();

}
