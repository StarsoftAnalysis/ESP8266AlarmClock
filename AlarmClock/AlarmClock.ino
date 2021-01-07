/*******************************************************************
    BLough Alarm Clock
    An Alarm clock that gets it's time from the internet

    Features:
    - Web interface for setting the alarm
    - Captive portal for setting WiFi Details
    - Automatically adjust for DST

    By Brian Lough

    For use with: https://www.tindie.com/products/15402/
    
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.twitch.tv/brianlough
    Twitter: https://twitter.com/witnessmenow

    This version by Chris Dennis

    TODO:
     - use ezTime events to trigger alarm?
     - JSON6
 *******************************************************************/
// * max alarm time? needs fixing
// * store alarm time as minutes since midnight instead of hour/minute
// * snooze
// * repeat e.g. alarm of after 5 minutes, then on again at 10 minutes, etc.
// * button functions:
//   - cancel / enable next alarm
//   - set alarm time etc. e.g. left button held to cycle through modes: hr, min, alrmH, or use libraryr, alrmMin, alrmSet, exit (for next 24h), left=- right=+, hold for next mode.    e.g. setting hour, flash left 2 numbers; add colon when doing alarm, just show colon in alrmSet modei (+/- turns it on/off)
// * check summer time etc (i.e. NTPClient vs Timezone) - use UK. function, not direct from ntpclient
// * get RTTTL to adjust volume -- use analogWriteRange or Resolution??
//   - need to combine https://bitbucket.org/teckel12/arduino-timer-free-tone/downloads/ with the non-blocking RTTTL
//   - alarm to get louder gradually
// * SPIFFS for HTML and JS, perhaps using https://github.com/winder/ESPTemplateProcessor
// * config in flash?
// * web page 
//   - choose from list of tunes
//   - choose NTP server, time zone, DST rule
// * home-grown timers instead of pseudothreads
// * home-grown wifi library?
// * JSON v6
// DONE
// * off completely if ldr < min -- not just on level
// * colon if seconds % 2
// * get rid if mdNS stuff
// * can't get dots to work -- try a different TM1637 library? -- nothing seems to help
// * bug? 
//   - rebooted when button pressed for too long to see time at night
//   - rebooted at alarm time
// * light on when button pressed -- keep it on for e.g. 5 seconds

//Included with ESP8266 Arduino Core
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
//#include <DNSServer.h>
#include "FS.h"

#include <ezTime.h>
// Library used for getting the time and adjusting for DST
// Search for "ezTime" in the Arduino Library manager
// https://github.com/ropg/ezTime

#include <TM1637Display.h>
// Library used for controlling the 7 Segment display
// Search for "TM1637" in the Arduino Library manager
// https://github.com/avishorp/TM1637

#include <ArduinoJson.h>
// Library used for parsing & saving Json to config
// NOTE: There is a breaking change in the 6.x.x version,
// install the 5.x.x version instead
// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson

#include <WiFiManager.h>
// Library used for creating the captive portal for entering WiFi Details
// Search for "Wifimanager" in the Arduino Library manager
// https://github.com/tzapu/WiFiManager

#include "timers.h"
// Home-made timers

// Extra bits of code
#include "secret.h"
#include "pitches.h"
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

static int alarmHour = 0;
static int alarmMinute = 0;
static bool alarmActive = false;
static bool alarmHandled = false;
static bool buttonPressed = false;

#define KEEP_LIGHT_ON_TIME 5000
static bool keepingLightOn = false;

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

static bool saveConfig() {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["alarmHour"] = alarmHour;
    json["alarmMinute"] = alarmMinute;
    json["alarmActive"] = alarmActive;

    File configFile = SPIFFS.open("/alarm.json", "w");
    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        return false;
    }

    json.printTo(configFile);
    return true;
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

void handleLED() {
    String ledState = "OFF";
    String t_state = server.arg("LEDstate"); //Refer  xhttp.open("GET", "setLED?LEDstate="+led, true);
    Serial.println(t_state);
    if (t_state == "1") {
        digitalWrite(LED_PIN,LOW); //LED ON
        ledState = "<i class=\"fas fa-lightbulb\"></i> ON"; //Feedback parameter
    } else if (t_state == "2") {
        //  digitalRead(LED_PIN); //READ LED
        if (digitalRead(LED_PIN) == 1) {
            ledState = "<i class=\"far fa-lightbulb\"></i> OFF"; //Feedback parameter  
        } else {
            ledState = "<i class=\"fas fa-lightbulb\"></i> ON";
        }
    } else {
        digitalWrite(LED_PIN,HIGH); //LED OFF
        ledState = "<i class=\"far fa-lightbulb\"></i> OFF"; //Feedback parameter  
    }
    server.send(200, "text/html", ledState); //Send web page
}

void handleSetAlarm() {
    Serial.println("Setting Alarm");
    for (uint8_t i = 0; i < server.args(); i++) {
        if (server.argName(i) == "alarm") {
            String alarm = server.arg(i);
            int indexOfColon = alarm.indexOf(":");
            alarmHour = alarm.substring(0, indexOfColon).toInt();
            alarmMinute = alarm.substring(indexOfColon + 1).toInt();
            alarmActive = true;
            saveConfig();
            Serial.print("Setting Alarm to: ");
            Serial.print(alarmHour);
            Serial.print(":");
            Serial.println(alarmMinute);
        }
    }
    char alarmString[5];
    sprintf(alarmString, "%02d:%02d", alarmHour, alarmMinute);
    server.send(200, "text/plain", alarmString);
}

void handleDeleteAlarm() {
    Serial.println("Deleting Alarm");
    alarmHour = 0;
    alarmMinute = 0;
    alarmActive = false;
    saveConfig();
    Serial.print("Alarm deleted");
    server.send(200, "text/plain", "--:--");
}

void handleGetAlarm() {
    char alarmString[5];
    if (alarmActive) {
        sprintf(alarmString, "%02d:%02d", alarmHour, alarmMinute);
    } else {
        sprintf(alarmString, "--:--");
    }
    server.send(200, "text/plain", alarmString);
}

void configModeCallback (WiFiManager *myWiFiManager) {
    Serial.println("Entered config mode");
    Serial.println(WiFi.softAPIP());
    display.setSegments(SEG_CONF);
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

/* later
// Get details of next alarm (whether it's set or not)
alarmDetails nextAlarm () {
    alarmDetails next = nullptr;
    // Get 'now' as day of week, hour, minute
    int dow = timeClient.getDay();    // 0 = Sunday  from ezTime
    int hour = timeClient.getHours();
    int mins = timeClient.getMinutes();
    alarmDetails today = alarmInfo.alarmDay[dow];
    alarmDetails tomorrow = alarmInfo.alarmDay[(dow+1) % 7];
    if ((today.alarmHour > hour) || ((today.alarmHour == hour) && (today.alarmMinute > mins))) {
        next = today;
    } else if ((tomorrow.alarmHour < hour) || ((tomorrow.alarmHour == hour) && (tomorrow.alarmMinute < mins))) {
        due = tomorrow;
    }
    //Serial.printf("alarmDue: dow=%d hr=%d min=%d  today: %d %02d:%02d  tmrw: %d %02d:%02d  due=%d\n", dow, hour, mins, today.alarmSet, today.alarmHour, today.alarmMinute, tomorrow.alarmSet, tomorrow.alarmHour, tomorrow.alarmMinute, due); 
    return next;
}
*/

static bool loadConfig() {
    File configFile = SPIFFS.open("/alarm.json", "r");
    if (!configFile) {
        Serial.println("Failed to open config file");
        return false;
    }

    size_t size = configFile.size();
    if (size > 1024) {
        Serial.println("Config file size is too large");
        return false;
    }

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    configFile.readBytes(buf.get(), size);

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(buf.get());

    if (!json.success()) {
        Serial.println("Failed to parse config file");
        return false;
    }

    alarmHour = json["alarmHour"];
    alarmMinute = json["alarmMinute"];
    alarmActive = json["alarmActive"];
    return true;
}

// notes in the melody:
static int melody[] = {
    NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4, 0
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
static int noteDurations[] = {
    4, 8, 8, 4, 4, 4, 4, 4, 4
};

// FIXME non-blocking version of this, or use library
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

static void adjustBrightness() {
    int sensorValue = analogRead(LDR_PIN);
    int level = sensorValue / 256;
    //Serial.printf("aB: ldr=%d level=%d button=%d\n", sensorValue, level, buttonState.aButtonPressed);
    display.setBrightness(level, sensorValue > 128 || buttonState.aButtonPressed || keepingLightOn);
}

static int timeHour;
static int timeMinutes;

static void checkForAlarm() {
    if (alarmActive && timeHour == alarmHour && timeMinutes == alarmMinute) {
        if (!alarmHandled) {
            soundAlarm();
        }
    } else {
        alarmHandled = false;
    }
}

ICACHE_RAM_ATTR void interuptButton() {
    buttonPressed = true;
    return;
}

static void displayTime (void) {

    //Requesting the time in a specific format
    // G = hours without leading 0
    // i = minutes with leading 0
    // so 9:34am would come back " 934"
    String timeString = TZ.dateTime("Gi");

    timeHour = timeString.substring(0,2).toInt();
    timeMinutes = timeString.substring(2).toInt();
    int timeDecimal = timeHour * 100 + timeMinutes;
    uint8_t dots = (second() % 2) << 6; //(alarmDue() << 7) | (dotsVisible << 5);
    display.showNumberDecEx(timeDecimal, dots, false);
    
}

void setup() {
    Serial.begin(115200);

    //Onboard LED port Direction output
    pinMode(LED_PIN,OUTPUT); 
    digitalWrite(LED_PIN,HIGH); //LED OFF

    display.setBrightness(2);
    display.setSegments(SEG_BOOT);
    if (!SPIFFS.begin()) {
        Serial.println("Failed to mount FS");
        display.setSegments(SEG_FILE);
        while(true){
            // loop as we want to show there is an error
            // as it possible couldn't load an alarm
        }
    } else {
        loadConfig(); 
    }

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    pinMode(RBUTTON_PIN, INPUT_PULLUP);
    pinMode(LBUTTON_PIN, INPUT_PULLUP);

    attachInterrupt(RBUTTON_PIN, interuptButton, RISING);  // TODO both buttons, or probably not at all

    WiFiManager wifiManager;
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.autoConnect("AlarmClock", "password");

    Serial.println("");
    Serial.print("WiFi Connected");
    Serial.println("");
    Serial.print("IP address: ");

    IPAddress ipAddress = WiFi.localIP();
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
    server.on("/deleteAlarm", handleDeleteAlarm);
    server.on("/setLED", handleLED);
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
    TZ.setLocation(F(MYTIMEZONE));
    Serial.print("Local: " + TZ.dateTime());

    timers::setup();
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

    //unsigned long now = millis();
    if (buttonState.lButtonLong) {
        display.setSegments(SEG_CONF);
        delay(1000);
    } else if (digitalRead(LBUTTON_PIN) == LOW && digitalRead(RBUTTON_PIN) == LOW) {
        int sensorValue = analogRead(LDR_PIN);
        display.showNumberDec(sensorValue, false);
    } else if (digitalRead(LBUTTON_PIN) == LOW) {
        IPAddress ipAddress = WiFi.localIP();
        display.showNumberDec(ipAddress[3], false);
    } else {
//        if (now > oneSecondLoopDue) {
            displayTime();
            checkForAlarm();
            if (buttonPressed) { // FIXME
                alarmHandled = true;
                buttonPressed = false;
            }
//            oneSecondLoopDue = now + 1000;
//        }
    }

    server.handleClient();

    delay(500); // TESTING ONLY!
}
