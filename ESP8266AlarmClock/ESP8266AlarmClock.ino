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

    AlarmClock is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AlarmClock.  If not, see <https://www.gnu.org/licenses/>.
    ---

    Features:
    - Web interface for setting the alarm
    - Captive portal for setting WiFi Details
    - Automatically adjust for DST
    - snooze and repeat alarm
    - RTTTL tunes
    - display is completely off at night, and comes on if any button is pressed

    Features planned:
    - set and cancel alarm time via buttons
    - likewise for current time (in case internet is not available)

****************************************************************/

// 21/9/21: This is dev branch -- pre-async, new progress from here as 0.2.0...

// TODO:
// * separate 'Settings' web page'
//   - need to separate .js file and use it for both pages
// * Maybe use SPIFFS or the new thing -- NO! will break OTA updates -- see https://github.com/jandrassy/ArduinoOTA#esp8266-and-esp32-support
// * make use of PRINTLN vs Serial.println consistent.
// * if no internet, prompt for manual time setting
// * simple config: set/unset alarm on long left; show alarm time on long right (what if no alarm in next 24hours)
// * redo nextAlarm (to allow for more than one alarm per day): get list of alarms in next 24h, choose first one that is set (may return 'none')
// * more aria labels in html
// * allow user to set light level for turning display off
// * allow user to set repeat/snooze times etc.
// * use #defines to set e.g. SNOOZE = LBUTTON etc. for ease of customisation
// * Use NVS instead of EEPROM?
//    - and/or use EZTime's cache
// * button functions --  'config' more
//   - cancel / enable next alarm
//   - set alarm time etc. e.g. left button held to cycle through modes: hr, min, alrmH, or use libraryr, alrmMin, alrmSet, exit (for next 24h), left=- right=+, hold for next mode.    e.g. setting hour, flash left 2 numbers; add colon when doing alarm, just show colon in alrmSet modei (+/- turns it on/off)
// * alarm to get louder gradually
// * HTTPS? -- see http://www.iotsharing.com/2017/08/how-to-use-https-in-arduino-esp32.html
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

#include <WiFiManager.h>
// Library used for creating the captive portal for entering WiFi Details
// Search for "Wifimanager" in the Arduino Library manager
// https://github.com/tzapu/WiFiManager

// OTA Updates — ESP8266 Arduino Core 2.4.0 documentation
// https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html
#include <ArduinoOTA.h>

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
// Defines "const char js[]":
#include "js.h"

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
 
// converts the dBm to a range between 0 and 100%
static int getWifiQuality() {
    int32_t dbm = WiFi.RSSI();
    if (dbm <= -100) {
        return 0;
    } else if (dbm >= -50) {
        return 100;
    } else {
        return 2 * (dbm + 100);
    }
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
    PRINTF("Sending '/js' length %d\n", strlen_P(js));
    server.send(200, "text/html", js);
}

void handleNotFound() {
    char message[] = "File Not Found";
    PRINTLN(message);
    server.send(404, "text/plain", message);
}

void handleGetData () {
    // Light level
    int a = analogRead(LDR_PIN);
    // WiFi signal
    int rssi = getWifiQuality();
    // Current time
    String time = TZ.dateTime("H:i");

    DynamicJsonDocument json(512); // FIXME tune this

    json["lightLevel"] = String(a);
    json["wifiQuality"] = String(rssi);
    json["time"] = String(time);

    String s;
    serializeJson(json, s);
    //Serial.printf("hGD: sending %s\n", s.c_str());
    server.send(200, "text/plain", s);
}

/*
void handleWiFi() {
    int rssi = getWifiQuality();
    //String rssiValue = String(rssi);
    char buffer[8];  
    snprintf(buffer, sizeof(buffer), "%d", rssi);
    //PRINTF("Sending WiFi value length %d\n", strlen(buffer));
    server.send(200, "text/plain", buffer);
}

void handleTime() {
    String time = TZ.dateTime("H:i");
    server.send(200, "text/plain", time);
}
*/

void handleSetAlarm() {

    config_t newConfig = config;
    bool melodyChanged = false;

    Serial.println("hSA: args: ");
    for (int i = 0; i < server.args(); i++) {
        Serial.printf("\t%d: %s = %s\n", i, server.argName(i).c_str(), server.arg(i).c_str());
        if (server.argName(i).substring(0,9) == "alarmTime") {
            Serial.printf("\t\tdaystring = %s\n", server.argName(i).substring(9).c_str());
            int dy = constrain(server.argName(i).substring(9).toInt(), 0, 6);  // returns 0 on error!
            String alarmTime = server.arg(i);
            int indexOfColon = alarmTime.indexOf(":");
            int alarmHour = constrain(alarmTime.substring(0, indexOfColon).toInt(), 0, 23);
            int alarmMinute = constrain(alarmTime.substring(indexOfColon + 1).toInt(), 0, 59);
            Serial.printf("\t\td=%d  %d:%d\n", dy, alarmHour, alarmMinute);
            newConfig.alarmDay[dy].hour = alarmHour;
            newConfig.alarmDay[dy].minute = alarmMinute;
        } else if (server.argName(i).substring(0,8) == "alarmSet") {
            Serial.printf("\t\tdaystring = %s   %s\n", server.argName(i).c_str(), server.argName(i).substring(8).c_str());
            int dy = constrain(server.argName(i).substring(8).toInt(), 0, 6);  // returns 0 on error!
            bool alarmSet = (server.arg(i) == "1");
            Serial.printf("\t\td=%d  %s\n", dy, alarmSet ? "y" : "n");
            newConfig.alarmDay[dy].set = alarmSet;
        } else if (server.argName(i) == "volume") {
            newConfig.volume = e8rtp::setVolume(server.arg(i).toInt()); // returns validated value
        } else if (server.argName(i) == "melody") {
            melodyChanged = (strcmp(newConfig.melody, server.arg(i).substring(0, MELODY_MAX-1).c_str()) == 0);
            strlcpy(newConfig.melody, server.arg(i).substring(0, MELODY_MAX-1).c_str(), MELODY_MAX);
        } else if (server.argName(i) == "tz") {
            strlcpy(newConfig.tz, server.arg(i).substring(0, MELODY_MAX-1).c_str(), TZ_MAX);
        }
    }

    if (melodyChanged) {
        if (e8rtp::state() == e8rtp::Playing) {
            e8rtp::stop();
        }
        e8rtp::setup(BUZZER_PIN, newConfig.volume, newConfig.melody);
    }

    // Set timezone if it's changed
    if (newConfig.tz != config.tz) {
        TZ.setLocation(newConfig.tz);
    }

    storeConfig(&newConfig);    // also copies newConfig to config
//FIXME if unexpected stuff sent, return non-200
// FIXME supply sensible defaults e.g. to melody
// FIXME separate settings and alarm?
    server.send(200, "text/html", "Alarm Set");
}

void handleGetAlarm() {

    DynamicJsonDocument json(2000); // FIXME tune this

    JsonArray days = json.createNestedArray("alarmDay");
    for (int i = 0; i < 7; i++) {
        JsonObject obj = days.createNestedObject();
        char timeString[6];
        sprintf(timeString, "%02d:%02d", config.alarmDay[i].hour, config.alarmDay[i].minute);
        obj["alarmTime"] = timeString;
        obj["alarmSet"] = (config.alarmDay[i].set ? "1" : "0");
    }

    String s;
    serializeJson(json, s);
    Serial.printf("hGA: sending %s\n", s.c_str());
    server.send(200, "text/plain", s);

}

void handleGetSettings() {

    DynamicJsonDocument json(2000); // FIXME tune this

    json["volume"] = config.volume;
    json["melody"] = String(config.melody);
    json["tz"] = String(config.tz);

    String s;
    serializeJson(json, s);
    Serial.printf("hGA: sending %s\n", s.c_str());
    server.send(200, "text/plain", s);

}

void configModeCallback (WiFiManager *myWiFiManager) {
    Serial.println("Entered WiFi config mode");
    Serial.println(WiFi.softAPIP());
    display.setSegments(SEG_WIFI);
}

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

// Get the index (into config.alarmDay) of the next alarm. 
// Returns -1 if there is no alarm in the next 24 hours.
static int nextAlarmIndex (void) {
    int next = -1;
    // Get 'now' as day of week, hour, minute
    int wd = TZ.weekday() - 1;    // 0 = Sunday  (ezTime has 1=Sunday)
    int wd1 = (wd + 1) % 7;
    int hr = TZ.hour();
    int mn = TZ.minute();
    alarmDetails_t today = config.alarmDay[wd];
    alarmDetails_t tomorrow = config.alarmDay[wd1];
    if ((today.hour > hr) || ((today.hour == hr) && (today.minute >= mn))) {
        next = wd;
    } else if ((tomorrow.hour < hr) || ((tomorrow.hour == hr) && (tomorrow.minute < mn))) {
        next = wd1;
    }
    //PRINTF("nAI: wd=%d wd1=%d hr=%d mn=%d today=%d:%d.%d tmrw=%d:%d.%d next=%d\n", wd, wd1, hr, mn, today.hour, today.minute, today.set, tomorrow.hour, tomorrow.minute, tomorrow.set, next);
    return next;
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
    int hr = TZ.hour();
    int mn = TZ.minute();
    long unsigned now = millis();
    static int stopMinute = -1; // The minute when the alarm was stopped
    //PRINTF("cFA: alarmState is %d   time is %02d:%02d\n", alarmState, hr, mn);
    switch (alarmState) {
        case alarmStateEnum::Off: {
            int i = nextAlarmIndex();
            if (i >= 0) {
                alarmDetails_t alarm = config.alarmDay[i];
                //PRINTF("cFA: off, next alarm is %02d:%02d %d\n", alarm.hour, alarm.minute, alarm.set);
                if (alarm.set && hr == alarm.hour && mn == alarm.minute) {
                    // set time -- start ringing
                    alarmState = alarmStateEnum::Ringing;
                    alarmStateStart = now;  
                    alarmRepeatCount = 0;
                    alarmSnoozeCount = 0;
                    e8rtp::start();
                    PRINTF("cFA: start ringing for alarm %02d:%02d\n", alarm.hour, alarm.minute);
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
                    alarmStateStart = now;
                    alarmSnoozeCount = 1;
                    e8rtp::pause();
                    PRINTF("cFA: ringing -> snooze no. %d (lButton)\n", alarmSnoozeCount);
                } else if (buttonState.rButtonPressed) {   // TODO better choices of long/short/both button presses
                    // stop
                    alarmState = alarmStateEnum::Stopped;
                    stopMinute = TZ.minute();
                    alarmStateStart = now;  // not needed
                    e8rtp::stop();
                    PRINTLN("cFA: ringing -> stopped (rButton)");
                }
            } else if (now - alarmStateStart > alarmRingTime) {
                if (alarmRepeatCount >= alarmRepeatMax) {
                    // alarm ignored for long enough -- stop
                    alarmState = alarmStateEnum::Stopped;
                    stopMinute = TZ.minute();
                    e8rtp::stop();
                    PRINTLN("cFA: ringing -> stopped (timeout)");
                } else {
                    // pause
                    alarmState= alarmStateEnum::Paused;
                    alarmStateStart = now;
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
                    alarmStateStart = now;
                    alarmSnoozeCount += 1;
                    PRINTF("cFA: snoozing -> snooze no. %d (lButton)\n", alarmSnoozeCount);
                } else if (buttonState.rButtonPressed) {
                    // stop
                    alarmState = alarmStateEnum::Stopped;
                    stopMinute = TZ.minute();
                    e8rtp::stop();
                    PRINTLN("cFA: snoozing -> stopped (rButton)");
                }
            } else if (now - alarmStateStart > alarmSnoozeTime) {
                if (alarmSnoozeCount >= alarmSnoozeMax) {
                    // alarm ignored for long enough -- stop
                    alarmState = alarmStateEnum::Stopped;
                    stopMinute = TZ.minute();
                    e8rtp::stop();
                    PRINTLN("cFA: snoozing stopped (enough snoozes))");
                } else {
                    // back to ringing (but don't reset the counts)
                    alarmState = alarmStateEnum::Ringing;
                    alarmStateStart = now;  
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
                    alarmStateStart = now;
                    alarmSnoozeCount += 1;
                    e8rtp::pause(); // (should be paused already)
                    PRINTF("cFA: paused -> snooze no. %d (lButton)\n", alarmSnoozeCount);
                } else if (buttonState.rButtonPressed) {
                    // stop
                    alarmState = alarmStateEnum::Stopped;
                    stopMinute = TZ.minute();
                    e8rtp::stop();
                    PRINTLN("cFA: paused -> stopped (rButton)");
                }
            } else if (now - alarmStateStart > alarmPauseTime) {
                // back to ringing 
                alarmState = alarmStateEnum::Ringing;
                alarmStateStart = now;  
                alarmRepeatCount += 1;
                e8rtp::resume();
                PRINTF("cFA: paused -> ringing, repeat %d\n", alarmRepeatCount);
            }
            break;
        }
        case alarmStateEnum::Stopped: {
            // Go to Off once the minute has changed
            if (TZ.minute() != stopMinute) {
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
    int hr = TZ.hour();
    int mn = TZ.minute();
    digits[0] = (hr > 9) ? display.encodeDigit(hr / 10) : 0;    // 0 means blank
    digits[1] = display.encodeDigit(hr % 10);
    digits[2] = display.encodeDigit(mn / 10);
    digits[3] = display.encodeDigit(mn % 10);

    // Toggle colon every second, but keep it on if the alarm is set
    int i = nextAlarmIndex();
    bool set = (i >= 0) && (config.alarmDay[i].set);
    bool showColon = set || (TZ.second() % 2);
    if (showColon) {
        digits[1] |= 1 << 7;
    }

    display.setSegments(digits);
    
}

static void toggleNextAlarm (void) {
    int i = nextAlarmIndex();
    if (i >= 0) {
       config.alarmDay[i].set ^= true;
    }
}

void setup() {

    Serial.begin(115200);
    Serial.println("\n=======================");

    //Onboard LED port Direction output
    pinMode(LED_PIN, OUTPUT); 
    digitalWrite(LED_PIN, HIGH); //LED OFF

    display.setBrightness(2);
    display.setSegments(SEG_BOOT);

    configSetup();
    loadConfig(); 

    pinMode(RBUTTON_PIN, INPUT_PULLUP);
    pinMode(LBUTTON_PIN, INPUT_PULLUP);

    //attachInterrupt(RBUTTON_PIN, interuptButton, RISING);  // TODO both buttons, or probably not at all

    WiFiManager wifiManager;
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.autoConnect("ESP8266AlarmClock");  // could check boolean rc from this
    IPAddress ipAddress = WiFi.localIP();

    Serial.println("\nWiFi Connected");
    Serial.print("IP address: ");
    Serial.println(ipAddress);

    display.showNumberDec(ipAddress[3], false);
    delay(1000);

    // HTTP Server
    server.on("/", handleMain);
    server.on("/settings", handleSettings);
    server.on("/js", handleJS);
    server.on("/setAlarm", handleSetAlarm);
    server.on("/getAlarm", handleGetAlarm);
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
    ezt::waitForSync();
    Serial.println("\nUTC: " + UTC.dateTime());
    TZ.setLocation(config.tz);
    Serial.println("Local: " + TZ.dateTime());

    // OTA Port defaults to 8266
    // ArduinoOTA.setPort(8266);
    // OTA hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname("alarmclock");
    ArduinoOTA.begin();	// start the OTA responder
    PRINTLN("OTA server started");

    timers::setup();

    // Load the alarm tune
    e8rtp::setup(BUZZER_PIN, config.volume, config.melody);

    alarmState = alarmStateEnum::Off;
}

void loop() {

    // Library loops
    timers::loop();
    ezt::events();
    e8rtp::loop();
	ArduinoOTA.handle();
    server.handleClient();

    setButtonStates();
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
        toggleNextAlarm();
    }

    if ((alarmState == alarmStateEnum::Ringing) && (e8rtp::state() != e8rtp::Playing)) {
        // Repeat the tune
        e8rtp::start();
    }

    displayTime();
    checkForAlarm();

}
