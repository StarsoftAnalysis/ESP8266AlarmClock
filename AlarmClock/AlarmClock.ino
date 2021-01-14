/*******************************************************************
    Alarm Clock
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

// TODO:
// * 0:03 displays as '  : 3' !!
// * only check for alarm once a minute (will avoid the nasty hack in nextAlarm())
// OR call minuteChanged in nextAlarm and cache its answer
// * change time zone when it's changed on the gui
// * OTA updates
// * simple config: set/unset alarm on long left; show alarm time on long right (what if no alarm in next 24hours)
// * redo nextAlarm (to allow for more than one alarm per day): get list of alarms in next 24h, choose first one that is set (may return 'none')
// * more aria labels in html
// * use #defines to set e.g. SNOOZE = LBUTTON etc. for ease of customisation
// * Use NVS instead of EEPROM?
//    - and/or use EZTime's cache
// * async web server?? or more consistent use of webActive??
// * display sometimes hunts -- depends on light level -- do running average?
// * maybe use bool ezt's secondChanged() to display the time less often
// * button functions --  'config' more
//   - cancel / enable next alarm
//   - set alarm time etc. e.g. left button held to cycle through modes: hr, min, alrmH, or use libraryr, alrmMin, alrmSet, exit (for next 24h), left=- right=+, hold for next mode.    e.g. setting hour, flash left 2 numbers; add colon when doing alarm, just show colon in alrmSet modei (+/- turns it on/off)
// * get RTTTL to adjust volume -- use analogWriteRange or Resolution??
//   - need to combine https://bitbucket.org/teckel12/arduino-timer-free-tone/downloads/ with the non-blocking RTTTL
//   - alarm to get louder gradually
// * HTTPS? -- see http://www.iotsharing.com/2017/08/how-to-use-https-in-arduino-esp32.html
// * test RTTTL by playing it???  No -- give a link to a site to test it https://adamonsoon.github.io/rtttl-play/
// NO
// - use ezTime events to trigger alarm?
// * store alarm time as minutes since midnight instead of hour/minute
// * ? define alarms as repeating or one-off.  or separate set of one-off alarms
// DONE
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

//#include <TimerFreeTone.h>
#include <NonBlockingRtttl.h>

#include "debug.h"

// Home-made timers
#include "timers.h"

// Config -- stored in EEPROM
#include "config.h"

// Letters as display segments
#include "displayConf.h"

// Declare 'webpage': 
#include "alarmWeb.h"

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
enum class alarmStateEnum { Off, Ringing, Snoozed, Paused, Stopped };
static alarmStateEnum alarmState = alarmStateEnum::Off;
static long unsigned alarmStateStart;
static const long unsigned alarmRingTime = 60 * 1000;  // ms
static int alarmRepeatCount;
static const int alarmRepeatMax = 3;
static const long unsigned alarmPauseTime = 60 * 1000;  // ms
static int alarmSnoozeCount;
static const int alarmSnoozeMax = 3;
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

void handleRoot() {
    server.send(200, "text/html", webpage);
}

void handleNotFound() {
    String message = "File Not Found\n";
    server.send(404, "text/plain", message);
}

void handleADC() {
    int a = analogRead(LDR_PIN);
    String adcValue = String(a);
    server.send(200, "text/plain", adcValue); //Send ADC value only to client ajax request
}

void handleWiFi() {
    int rssi = getWifiQuality();
    String rssiValue = String(rssi);    //sprintf("%d%  (Compiled %s)", rssi, TZ.dateTime(compileTime() - (TZ.getOffset() * 60)).c_str());
    server.send(200, "text/plain", rssiValue);
}

static bool webActive = false;
void handleSetAlarm() {
    // single threaded
    if (webActive) {
        server.send(503, "text/html", "busy - single threaded");
        return;
    }
    webActive = true;
    config_t newConfig;

    //Serial.println("hSA: args: ");
    for (int i = 0; i < server.args(); i++) {
        //Serial.printf("\t%d: %s = %s\n", i, server.argName(i).c_str(), server.arg(i).c_str());
        if (server.argName(i).substring(0,9) == "alarmTime") {
            //Serial.printf("\t\tdaystring = %s\n", server.argName(i).substring(9).c_str());
            int dy = constrain(server.argName(i).substring(9).toInt(), 0, 6);  // returns 0 on error!
            String alarmTime = server.arg(i);
            int indexOfColon = alarmTime.indexOf(":");
            int alarmHour = constrain(alarmTime.substring(0, indexOfColon).toInt(), 0, 23);
            int alarmMinute = constrain(alarmTime.substring(indexOfColon + 1).toInt(), 0, 59);
            //Serial.printf("\t\td=%d  %d:%d\n", dy, alarmHour, alarmMinute);
            newConfig.alarmDay[dy].hour = alarmHour;
            newConfig.alarmDay[dy].minute = alarmMinute;
        } else if (server.argName(i).substring(0,8) == "alarmSet") {
            //Serial.printf("\t\tdaystring = %s   %s\n", server.argName(i).c_str(), server.argName(i).substring(8).c_str());
            int dy = constrain(server.argName(i).substring(8).toInt(), 0, 6);  // returns 0 on error!
            bool alarmSet = (server.arg(i) == "1");
            //Serial.printf("\t\td=%d  %s\n", dy, alarmSet ? "y" : "n");
            newConfig.alarmDay[dy].set = alarmSet;
        } else if (server.argName(i) == "volume") {
            newConfig.volume = constrain(server.arg(i).toInt(), 10, 100);
        } else if (server.argName(i) == "melody") {
            strlcpy(newConfig.melody, server.arg(i).substring(0, MELODY_MAX-1).c_str(), MELODY_MAX);
        } else if (server.argName(i) == "tz") {
            strlcpy(newConfig.tz, server.arg(i).substring(0, MELODY_MAX-1).c_str(), TZ_MAX);
        }
    }

    storeConfig(&newConfig);
    server.send(200, "text/html", "Alarm Set");
    webActive=false;
}

void handleGetAlarm() {
    // single threaded
    if (webActive) {
        server.send(503, "text/html", "busy - single threaded");
        return;
    }
    webActive = true;

    DynamicJsonDocument json(2000);

    json["volume"] = config.volume;
    json["melody"] = String(config.melody);
    json["tz"] = String(config.tz);

    JsonArray days = json.createNestedArray("alarmDay");
    for (int i = 0; i < 7; i++) {
        JsonObject obj = days.createNestedObject();
        char timeString[6];
        sprintf(timeString, "%02d:%02d", config.alarmDay[i].hour, config.alarmDay[i].minute);
        obj["alarmTime"] = timeString;
        obj["alarmSet"] = (config.alarmDay[i].set ? "1" : "0");
    }

    String alarmString;
    serializeJson(json, alarmString);
    Serial.printf("hGA: sending %s\n", alarmString.c_str());
    server.send(200, "text/plain", alarmString);

    webActive = false;
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

    // Reset all buttonState buttons first.
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

// Get details of next alarm (whether it's set or not),
// 
alarmDetails_t nextAlarm () {
    static alarmDetails_t next = {0, 0, false};
    //static int lastMinute = 0;
    //int thisMinute = TZ.minute()
    //if (thisMinute != lastMinute) {
        // Get 'now' as day of week, hour, minute
        int wd = TZ.weekday()-1;    // 0 = Sunday  (ezTime has 1=Sunday)
        int hr = TZ.hour();
        int mn = TZ.minute();
    //    if (TZ.second() > 1) { //10) {
    //        // FIXME maybe a first-time flag instead -- would need to reset the flag every minute?  or do it in checkForAlarm?
    //        // Hack so that alarms only get done once  -- may need tuning, especially if this isn't called every loop FIXME
    //        mn += 1;    // Don't need to adjust if mn>59
    //    }
        alarmDetails_t today = config.alarmDay[wd];
        alarmDetails_t tomorrow = config.alarmDay[(wd+1) % 7];
        if ((today.hour > hr) || ((today.hour == hr) && (today.minute >= mn))) {
            next = today;
        } else if ((tomorrow.hour < hr) || ((tomorrow.hour == hr) && (tomorrow.minute < mn))) {
            next = tomorrow;
        }
        //Serial.printf("alarmDue: dow=%d hr=%d min=%d  today: %d %02d:%02d  tmrw: %d %02d:%02d  due=%d\n", dow, hour, mins, today.set, today.hour, today.minute, tomorrow.set, tomorrow.hour, tomorrow.minute, due); 
    //    lastMinute = thisMinute;
    //}
    return next;
}

static void adjustBrightness() {
    int sensorValue = analogRead(LDR_PIN);
    int level = sensorValue / 256;
    //Serial.printf("aB: ldr=%d level=%d button=%d\n", sensorValue, level, buttonState.aButtonPressed);
    display.setBrightness(level, sensorValue > 128 || buttonState.aButtonPressed || keepingLightOn);
}

static void checkForAlarm () {
    int hr = TZ.hour();
    int mn = TZ.minute();
    long unsigned now = millis();
    static int stopMinute = -1; // The minute when the alarm was stopped
    //PRINTF("cFA: alarmState is %d   time is %02d:%02d\n", alarmState, hr, mn);
    switch (alarmState) {
        case alarmStateEnum::Off: {
            alarmDetails_t alarm = nextAlarm();
            //PRINTF("cFA: off, next alarm is %02d:%02d %d\n", alarm.hour, alarm.minute, alarm.set);
            if (alarm.set && hr == alarm.hour && mn == alarm.minute) {
                // set time -- start ringing
                alarmState = alarmStateEnum::Ringing;
                alarmStateStart = now;  
                alarmRepeatCount = 0;
                alarmSnoozeCount = 0;
                PRINTF("cFA: start ringing for alarm %02d:%02d\n", alarm.hour, alarm.minute);
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
                    PRINTF("cFA: ringing -> snooze no. %d (lButton)\n", alarmSnoozeCount);
                } else if (buttonState.rButtonPressed) {   // TODO better choices of long/short/both button presses
                    // stop
                    alarmState = alarmStateEnum::Stopped;
                    stopMinute = TZ.minute();
                    alarmStateStart = now;  // not needed
                    PRINTLN("cFA: ringing -> stopped (rButton)");
                }
            } else if (now - alarmStateStart > alarmRingTime) {
                if (alarmRepeatCount >= alarmRepeatMax) {
                    // alarm ignored for long enough -- stop
                    alarmState = alarmStateEnum::Stopped;
                    stopMinute = TZ.minute();
                    PRINTLN("cFA: ringing -> stopped (timeout)");
                } else {
                    // pause
                    alarmState= alarmStateEnum::Paused;
                    alarmStateStart = now;
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
                    PRINTLN("cFA: snoozing -> stopped (rButton)");
                }
            } else if (now - alarmStateStart > alarmSnoozeTime) {
                if (alarmSnoozeCount >= alarmSnoozeMax) {
                    // alarm ignored for long enough -- stop
                    alarmState = alarmStateEnum::Stopped;
                    stopMinute = TZ.minute();
                    PRINTLN("cFA: snoozing stopped (enough snoozes))");
                } else {
                    // back to ringing (but don't reset the counts)
                    alarmState = alarmStateEnum::Ringing;
                    alarmStateStart = now;  
                    PRINTLN("cFA: snoozing -> ringing (timeout)");
                }
            }
            break;
        }
        case alarmStateEnum::Paused: {
            if (buttonState.aButtonPressed) {
                if (buttonState.lButtonPressed) {   // TODO better choices of long/short/both button presses
                    // start the snooze from now
                    alarmState = alarmStateEnum::Ringing;
                    alarmStateStart = now;
                    alarmSnoozeCount += 1;
                    PRINTF("cFA: paused -> snooze no. %d (lButton)\n", alarmSnoozeCount);
                } else if (buttonState.rButtonPressed) {
                    // stop
                    alarmState = alarmStateEnum::Stopped;
                    stopMinute = TZ.minute();
                    PRINTLN("cFA: paused -> stopped (rButton)");
                }
            } else if (now - alarmStateStart > alarmPauseTime) {
                // back to ringing 
                alarmState = alarmStateEnum::Ringing;
                alarmStateStart = now;  
                alarmRepeatCount += 1;
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

    //Requesting the time in a specific format
    // H = hours with leading 0
    // i = minutes with leading 0
    // so 9:34am would come back "0934"
    String timeString = TZ.dateTime("Hi");
    int timeHour = timeString.substring(0,2).toInt();
    int timeMinutes = timeString.substring(2).toInt();
    int timeDecimal = timeHour * 100 + timeMinutes;
    //PRINTF("dT: timeString=%s h=%d m=%d\n", timeString.c_str(), timeHour, timeMinutes);
    // Pulse colon every second, but keep it on if the alarm is set
    bool showColon = (TZ.second() % 2) || nextAlarm().set;
    uint8_t dots = showColon << 6;
    display.showNumberDecEx(timeDecimal, dots, false);
    
}

void setup() {
    Serial.begin(115200);

    //Onboard LED port Direction output
    pinMode(LED_PIN, OUTPUT); 
    digitalWrite(LED_PIN, HIGH); //LED OFF

    display.setBrightness(2);
    display.setSegments(SEG_BOOT);

    configSetup();
    loadConfig(); 

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    pinMode(RBUTTON_PIN, INPUT_PULLUP);
    pinMode(LBUTTON_PIN, INPUT_PULLUP);

    //attachInterrupt(RBUTTON_PIN, interuptButton, RISING);  // TODO both buttons, or probably not at all

    WiFiManager wifiManager;
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.autoConnect("AlarmClock");  // could check boolean rc from this
    IPAddress ipAddress = WiFi.localIP();

    Serial.println("\nWiFi Connected");
    Serial.print("IP address: ");
    Serial.println(ipAddress);

    display.showNumberDec(ipAddress[3], false);
    delay(1000);

    //if (MDNS.begin("alarm")) {
    //    Serial.println("MDNS Responder Started");
    //}

    // HTTP Server
    server.on("/", handleRoot);
    server.on("/setAlarm", handleSetAlarm);
    server.on("/getAlarm", handleGetAlarm);
    //server.on("/deleteAlarm", handleDeleteAlarm);
    server.on("/getWiFi", handleWiFi);
    server.on("/readADC", handleADC);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP Server Started");

    // EZ Time
    //ezt::setDebug(INFO);
    ezt::setInterval(60);
    ezt::setServer(String("uk.pool.ntp.org"));
    ezt::waitForSync();
    Serial.println("\nUTC: " + UTC.dateTime());
    TZ.setLocation(config.tz);
    Serial.println("Local: " + TZ.dateTime());

    timers::setup();

    alarmState = alarmStateEnum::Off;
}

void loop() {

    timers::loop();

    ezt::events();

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

    if (alarmState == alarmStateEnum::Ringing) {
        if (rtttl::isPlaying()) {
            rtttl::play();	// keep playing stuff
        } else {
            PRINTLN("starting alarm tune");
            rtttl::begin(BUZZER_PIN, config.melody);
        }
    } else {
        if (rtttl::isPlaying()) {
            rtttl::stop();
            PRINTLN("stopped playing");
        }
    }

    if (buttonState.lButtonLong) {
        display.setSegments(SEG_CONF);
        PRINTLN("Config mode -- would be here...");
        delay(1000);
    }

    displayTime();
    checkForAlarm();
    
    server.handleClient();

    //delay(100); // TESTING ONLY!
}
