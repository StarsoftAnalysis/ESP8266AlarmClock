# ESP8266AlarmClock

ESP8266 code to run a simple alarm clock, running on [Brian Lough](http://blough.ie/bac/)'s
alarm clock kit, which is available from [Tindie](https://www.tindie.com/products/brianlough/blough-alarm-clock-shield-for-wemos-d1-mini/).

This is StarSoftAnalysis's version of the software, based on [Brian](https://github.com/witnessmenow/arduino-alarm-clock)'s, and incorporating
features from [James Brown's fork](https://github.com/jbrown123/arduino-alarm-clock).

    Copyright 2021 Chris Dennis

    This file is part of ESP8266AlarmClock.

    ESP8266AlarmClock is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AlarmClock is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ESP8266AlarmClock.  If not, see <https://www.gnu.org/licenses/>.

## Features

* Gets the current time from the internet using the network time protocol, automatically adjusting for daylight savings in the local timezone.
* Set the timezone via the web page.
* Seven separate alarms -- one for every day of the week.
* Colon flashes every second, but stays on if the next alarm is set.
* RTTTL alarm melody: set your own tune via the web page.
* Volume control via the web page.
* OTA software updates.

## To do

* Set alarm (and time) via the buttons.

## Instructions

* Initially, or when no known WiFi networks are available, the clock will run as a separate WiFi network called
"ESP8266AlarmClock", and 
its interface will be available as [http://192.168.4.1](http://192.168.4.1).
Connect to that network, and browse to that page and enter your WiFi details.  After that, the clock
will connect to your network, and try to update the time via NTP.
For further information on this process, see the documentation for the [WiFiManager](https://github.com/tzapu/WiFiManager) library.

* When the clock has connected to your network, it will display the last digit of its IP address for a few seconds.
For example, if the clock gets IP address 192.168.1.55, it will display '55'.  It will also display this number
after the 'boot' message each time it is connected to the power supply.

* Browse to the alarm clock's webpage: setting the alarm for each day of the week is easy.

* You can paste in the RTTTL code for a different melody of required -- melodies can be found
on the web ([Picaxe](https://picaxe.com/rtttl-ringtones-for-tune-command/), for example), or you can create your own.
To test the melody, copy-and-paste it into a site such as [Adam Rahwane](https://adamonsoon.github.io/rtttl-play/)'s
(although that site is a bit picky about the RTTTL syntax that it will accept).

* Choose your timezone from the list on [Wikipedia](https://en.wikipedia.org/wiki/List_of_tz_database_time_zones).

## Buttons

When alarm sounding:
* Right button cancels alarm
* Left button sleeps alarm
Otherwise
* Either button lights display for a few seconds if it's off
* Long press on right button cancels or sets next alarm -- colon flashes if alarm is due in the next 24 hours

## Dependencies

The code includes the following libraries:

| Library | Description |
| ------- | ----------- |
| ESP8266WiFi ||
| WiFiClient ||
| ESP8266WebServer ||
| ezTime | Library used for getting the time and adjusting for DST Search for "ezTime" in the Arduino Library manager https://github.com/ropg/ezTime |
| TM1637Display | Library used for controlling the 7 Segment display Search for "TM1637" in the Arduino Library manager https://github.com/avishorp/TM1637 |
| ArduinoJson version 6 ||
| WiFiManager | Library used for creating the captive portal for entering WiFi Details Search for "Wifimanager" in the Arduino Library manager https://github.com/tzapu/WiFiManager |
| ESP8266RTTTLPlus | Non-blocking RTTTL player with volume control. |


* The web page uses jQuery, Font Awesome, and Bootstrap 4 CSS, which it accesses directly from 
https://code.jquery.com/jquery-3.3.1.slim.min.js,
https://use.fontawesome.com/releases/v5.8.2/css/all.css, and 
https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css respectively.



