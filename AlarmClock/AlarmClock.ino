/*******************************************************************
    BLough Alarm Clock
    An Alarm clock that gets its time from the internet

    Features:
    - Web interface for setting the alarm
    - Captive portal for setting WiFi Details
    - Automatically adjust for DST

    By Brian Lough

    For use with Brian Lough's alarm clock kit at https://www.tindie.com/products/15402/
    (or similar ESP8266-based circuit)
    
    This version by Chris Dennis

    Modified from Brian Lough's code at https://github.com/witnessmenow/arduino-alarm-clock,
    with additional ideas and code from James Brown's version at https://github.com/jbrown123/arduino-alarm-clock 

    Features:
    - snooze and repeat alarm
    - RTTTL tunes
    - display is completely off at night, and comes on if any button is pressed

    Features planned:
    - set and cancel alarm time via buttons
    - likewise for current time (in case internet is not available)

****************************************************************/
    

// TODO:
// * use ezt namespace
// * Use NVS instead of EEPROM?
//    - and/or use EZTime's cache
// * display sometimes hunts -- depends on light level -- do running average?
// * debounce buttons -- just one pause at a time:
//   else it does this if l button is held:
/*
22:32:00.022 -> cFA: start ringing for alarm 22:32
22:32:00.055 -> starting alarm tune

22:32:09.897 -> starting alarm tune

22:32:16.615 -> cFA: ringing -> snooze no. 1 (lButton)
22:32:16.615 -> stopped playing

22:32:16.615 -> cFA: snoozing -> snooze no. 2 (lButton)
22:32:16.648 -> cFA: snoozing -> snooze no. 3 (lButton)
22:32:16.681 -> cFA: snoozing -> snooze no. 4 (lButton)
22:32:16.681 -> cFA: snoozing -> snooze no. 5 (lButton)
22:32:16.714 -> cFA: snoozing -> snooze no. 6 (lButton)
22:32:16.748 -> cFA: snoozing -> snooze no. 7 (lButton)
*/
// * button functions --  'config' more
//   - cancel / enable next alarm
//   - set alarm time etc. e.g. left button held to cycle through modes: hr, min, alrmH, or use libraryr, alrmMin, alrmSet, exit (for next 24h), left=- right=+, hold for next mode.    e.g. setting hour, flash left 2 numbers; add colon when doing alarm, just show colon in alrmSet modei (+/- turns it on/off)
// * check summer time etc (i.e. NTPClient vs Timezone) - use UK. function, not direct from ntpclient
// * get RTTTL to adjust volume -- use analogWriteRange or Resolution??
//   - need to combine https://bitbucket.org/teckel12/arduino-timer-free-tone/downloads/ with the non-blocking RTTTL
//   - alarm to get louder gradually
// * test RTTTL by playing it???  No -- give a link to a site to test it https://adamonsoon.github.io/rtttl-play/
// NO
// - use ezTime events to trigger alarm?
// * store alarm time as minutes since midnight instead of hour/minute
// * ? define alarms as repeating or one-off.  or separate set of one-off alarms
// DONE
// * off completely if ldr < min -- not just on level
// * colon if seconds % 2
// * get rid if mdNS stuff
// * can't get dots to work -- try a different TM1637 library? -- nothing seems to help
// * bug? 
//   - rebooted when button pressed for too long to see time at night
//   - rebooted at alarm time
// * light on when button pressed -- keep it on for e.g. 5 seconds
// * EEPROM instead of SPIFFS
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

//Included with ESP8266 Arduino Core
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
//#include <DNSServer.h>
//#include "FS.h"

#include <ezTime.h>
// Library used for getting the time and adjusting for DST
// Search for "ezTime" in the Arduino Library manager
// https://github.com/ropg/ezTime

#include <TM1637Display.h>
// Library used for controlling the 7 Segment display
// Search for "TM1637" in the Arduino Library manager
// https://github.com/avishorp/TM1637

#include "ArduinoJson.h"
// Local v6
// Library used for parsing & saving Json to config
// NOTE: There is a breaking change in the 6.x.x version,
// install the 5.x.x version instead
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

// --- TimeZone (Change me!) ---

// You should be able to use the country code for
// countries that only span one timezone, but I'm having
// issues with it:
// https://github.com/ropg/ezTime/issues/47
//#define MYTIMEZONE "ie"

// Or to set a specific timezone, use this list:
// https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
#define MYTIMEZONE "Europe/London"

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

//static int alarmHour = 0;
//static int alarmMinute = 0;
//static bool alarmActive = false;
//static bool alarmHandled = false;
//static bool buttonPressed = false;

#define KEEP_LIGHT_ON_TIME 5000
static bool keepingLightOn = false;

/* Now as config
// here is all the alarm info
typedef struct {
    byte alarmHour;
    byte alarmMinute;
    bool alarmSet;
} alarmDetails;
struct {
    int volume;	// 1-100% (really only works in 10% increments; 10-100)   FIXME really doesn't work at all!
    alarmDetails alarmDay[7];	// one for each day, 0=Sun ... 6=Sat
} alarmInfo;
*/

struct {
    bool          lButtonPressed;    // left button
    long unsigned lButtonPressedTime;
    bool          lButtonLong;
    bool          rButtonPressed;    // right button
    long unsigned rButtonPressedTime;
    bool          rButtonLong;
    bool          aButtonPressed;    // any button
    // TODO lButtonLongStart etc.
} buttonState;
#define LONG_BUTTON_PRESS_TIME 2000

// Alarm state etc.
enum class alarmStateEnum { Off, Ringing, Snoozed, Paused };
static alarmStateEnum alarmState = alarmStateEnum::Off;
static long unsigned alarmStateStart;
static const long unsigned alarmRingTime = 60 * 1000;  // ms
static int alarmRepeatCount;
static const int alarmRepeatMax = 3;
static const long unsigned alarmPauseTime = 60 * 1000;  // ms
static int alarmSnoozeCount;
static const int alarmSnoozeMax = 3;
static const long unsigned alarmSnoozeTime = 60 * 1000;  // ms
 
//const char *alarmTune = "Futurama:d=8,o=5,b=112:e,4e,4e,a,4a,4d,4d,e,4e,4e,e,4a,4g#,4d,d,f#,f#,4e,4e,e,4a,4g#,4b,16b,16b,g,g,f#,f#,4e,4e,a,4a,4d,4d,e,g,f#,4e,e,4a,4g#,4d,d,f#,f#,4e,4e,e,4a,4g#,4b,16b,16b,g,g,f#,f#,p,16e,16e,e,d#,d,d,c#,c#,2p";

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
    String rssiValue = String(rssi);    //sprintf("%d%  (Compiled %s)", rssi, dateTime(compileTime() - (TZ.getOffset() * 60)).c_str());
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

    Serial.println("hSA: args: ");
    for (int i = 0; i < server.args(); i++) {
        //if (server.argName(i) == "alarm") 
        Serial.printf("\t%d: %s = %s\n", i, server.argName(i).c_str(), server.arg(i).c_str());
        if (server.argName(i).substring(0,9) == "alarmTime") {
            //Serial.printf("\t\tdaystring = %s\n", server.argName(i).substring(9).c_str());
            int dy = constrain(server.argName(i).substring(9).toInt(), 0, 6);  // returns 0 on error!
            String alarmTime = server.arg(i);
            int indexOfColon = alarmTime.indexOf(":");
            int alarmHour = constrain(alarmTime.substring(0, indexOfColon).toInt(), 0, 23);
            int alarmMinute = constrain(alarmTime.substring(indexOfColon + 1).toInt(), 0, 59);
            Serial.printf("\t\td=%d  %d:%d\n", dy, alarmHour, alarmMinute);
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


/*
void handleDeleteAlarm() {
    Serial.println("Deleting Alarm");
    alarmHour = 0;
    alarmMinute = 0;
    alarmActive = false;
    storeConfig();
    Serial.print("Alarm deleted");
    server.send(200, "text/plain", "--:--");
}
*/

/*
void oldhandleGetAlarm() {
    char alarmString[5];
    if (alarmActive) {
        sprintf(alarmString, "%02d:%02d", alarmHour, alarmMinute);
    } else {
        sprintf(alarmString, "--:--");
    }
    server.send(200, "text/plain", alarmString);
}
*/

void handleGetAlarm() {
    // single threaded
    if (webActive) {
        server.send(503, "text/html", "busy - single threaded");
        return;
    }
    webActive = true;

    //StaticJsonBuffer<2048> jsonBuffer;
    //JsonObject& json = jsonBuffer.createObject();
    //JsonArray& dayArray = jsonBuffer.createArray();

    DynamicJsonDocument json(2000);

    json["volume"] = config.volume;
    json["melody"] = String(config.melody);
    json["tz"] = String(config.tz);

    JsonArray days = json.createNestedArray("alarmDay");
    for (int i = 0; i < 7; i++) {
        JsonObject obj = days.createNestedObject();
        //obj["alarmHour"] = config.alarmDay[i].hour;
        //obj["alarmMinute"] = config.alarmDay[i].minute;
        char timeString[6];
        sprintf(timeString, "%02d:%02d", config.alarmDay[i].hour, config.alarmDay[i].minute);
        obj["alarmTime"] = timeString;
        obj["alarmSet"] = (config.alarmDay[i].set ? "1" : "0");
        //days.add(obj);
    }
    //json["alarmDay2"] = days;

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
    static bool lButtonPressedBefore = false;
    static bool rButtonPressedBefore = false;
    long unsigned now = millis();
    buttonState.lButtonPressed = (digitalRead(LBUTTON_PIN) == LOW);
    buttonState.rButtonPressed = (digitalRead(RBUTTON_PIN) == LOW);
    buttonState.aButtonPressed = (buttonState.lButtonPressed || buttonState.rButtonPressed);
    buttonState.lButtonLong = false;
    buttonState.rButtonLong = false;
    if (buttonState.lButtonPressed) {
        if (lButtonPressedBefore) {
            if (now - buttonState.lButtonPressedTime > LONG_BUTTON_PRESS_TIME) {
                // FIXME need to know when long press finishes?  or rather when long press has been acknowledged/dealt with
                // Could reset xButtonLong every loop, so that it's only true once for other functions called from the loop that want to test it, rather than calling other functions from here.
                if (!buttonState.lButtonLong) {
                    Serial.println("sBS: lButtonLong becomes true");
                }
                buttonState.lButtonLong = true;
            }
        } else {
            buttonState.lButtonPressedTime = now;
        }
    } else {
        buttonState.lButtonLong = false;
    }
    lButtonPressedBefore = buttonState.lButtonPressed;
    if (buttonState.rButtonPressed) {
        if (rButtonPressedBefore) {
            if (now - buttonState.rButtonPressedTime > LONG_BUTTON_PRESS_TIME) {
                if (!buttonState.rButtonLong) {
                    Serial.println("sBS: rButtonLong becomes true");
                }
                buttonState.rButtonLong = true;
            }
        } else {
            buttonState.rButtonPressedTime = now;
        }
    } else {
        buttonState.rButtonLong = false;
    }
    rButtonPressedBefore = buttonState.rButtonPressed;
}

// Get details of next alarm (whether it's set or not)
alarmDetails_t nextAlarm () {
    alarmDetails_t next;
    // Get 'now' as day of week, hour, minute
    int wd = weekday()-1;    // 0 = Sunday  from ezTime
    int hr = hour();
    int mn = minute();
    if (second() > 10) {
        // Hack so that alarms only get done once  -- may need tuning, especially if this isn't called every loop FIXME
        mn += 1;    // Don't need to adjust if mn>59
    }
    alarmDetails_t today = config.alarmDay[wd];
    alarmDetails_t tomorrow = config.alarmDay[(wd+1) % 7];
    if ((today.hour > hr) || ((today.hour == hr) && (today.minute >= mn))) {
        next = today;
    } else if ((tomorrow.hour < hr) || ((tomorrow.hour == hr) && (tomorrow.minute < mn))) {
        next = tomorrow;
    }
    //Serial.printf("alarmDue: dow=%d hr=%d min=%d  today: %d %02d:%02d  tmrw: %d %02d:%02d  due=%d\n", dow, hour, mins, today.set, today.hour, today.minute, tomorrow.set, tomorrow.hour, tomorrow.minute, due); 
    return next;
}

/*
// notes in the melody:
static int melody[] = {
    NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4, 0
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
static int noteDurations[] = {
    4, 8, 8, 4, 4, 4, 4, 4, 4
};
*/

// FIXME non-blocking version of this, or use library
/*
static void soundAlarm() {
    for (int thisNote = 0; thisNote < 8; thisNote++) {
        // to calculate the note duration, take one second divided by the note type.
        //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
        int noteDuration = 1000 / noteDurations[thisNote];
        tone(BUZZER_PIN, melody[thisNote], noteDuration);

        // to distinguish the notes, set a minimum time between them.
        // the note's duration + 30% seems to work well:
        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        // stop the tone playing:
        noTone(BUZZER_PIN);
    }
}
*/

static void adjustBrightness() {
    int sensorValue = analogRead(LDR_PIN);
    int level = sensorValue / 256;
    //Serial.printf("aB: ldr=%d level=%d button=%d\n", sensorValue, level, buttonState.aButtonPressed);
    display.setBrightness(level, sensorValue > 128 || buttonState.aButtonPressed || keepingLightOn);
}

static void checkForAlarm () {
    int hr = hour();
    int mn = minute();
    long unsigned now = millis();
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
                    // fully off
                    alarmState = alarmStateEnum::Off;
                    alarmStateStart = now;  // not needed
                    PRINTLN("cFA: ringing -> off (rButton)");
                }
            } else if (now - alarmStateStart > alarmRingTime) {
                if (alarmRepeatCount >= alarmRepeatMax) {
                    // alarm ignored for long enough -- off
                    alarmState = alarmStateEnum::Off;
                    PRINTLN("cFA: ringing -> off (timeout)");
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
                    // fully off
                    alarmState = alarmStateEnum::Off;
                    PRINTLN("cFA: snoozing -> off (rButton)");
                }
            } else if (now - alarmStateStart > alarmSnoozeTime) {
                if (alarmSnoozeCount >= alarmSnoozeMax) {
                    // alarm ignored for long enough -- off
                    alarmState = alarmStateEnum::Off;
                    PRINTLN("cFA: snoozing -> off (enough snoozes))");
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
                    // fully off
                    alarmState = alarmStateEnum::Off;
                    PRINTLN("cFA: paused -> off (rButton)");
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
    }
        
    //TODO if now off, silence it immediately
    //PRINTLN("cFA: need to silence tune now");  only if ringing
    
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
    bool showColon = (second() % 2) || nextAlarm().set;
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
    //setDebug(INFO);
    //waitForSync();
    setInterval(60);
    setServer(String("uk.pool.ntp.org"));

    Serial.println("\nUTC: " + UTC.dateTime());
    TZ.setLocation(config.tz);
    Serial.print("Local: " + TZ.dateTime());

    timers::setup();

    alarmState = alarmStateEnum::Off;
}

void loop() {

    timers::loop();

    // ezTime events
    events();

    setButtonStates();
    adjustBrightness();

    // Light the display for a while even if it's dark
    if (buttonState.aButtonPressed) {
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

    //unsigned long now = millis();
    if (buttonState.lButtonLong) {
        display.setSegments(SEG_CONF);
        delay(1000);
        PRINTLN("Config mode -- would be here...");
        // perhaps set buttons off so that not acted on again this loop FIXME
    }
    /*
    } else if (digitalRead(LBUTTON_PIN) == LOW && digitalRead(RBUTTON_PIN) == LOW) {
        int sensorValue = analogRead(LDR_PIN);
        display.showNumberDec(sensorValue, false);
    } else if (digitalRead(LBUTTON_PIN) == LOW) {
        IPAddress ipAddress = WiFi.localIP();
        display.showNumberDec(ipAddress[3], false);
    } else {
    */
        displayTime();
        checkForAlarm();
    //}

    server.handleClient();

    //delay(100); // TESTING ONLY!
}
