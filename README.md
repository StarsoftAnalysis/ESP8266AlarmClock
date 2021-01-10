# arduino-alarm-clock

ESP8266 code to run a simple alarm clock, running on [Brian Lough](http://blough.ie/bac/)'s
alarm clock kit, which is available from [Tindie](https://www.tindie.com/products/brianlough/blough-alarm-clock-shield-for-wemos-d1-mini/).

This is StarSoftAnalysis's version, based on [Brian](https://github.com/witnessmenow/arduino-alarm-clock)'s, and incorporating
features from [James Brown's fork](https://github.com/jbrown123/arduino-alarm-clock).

## Features

* Gets the current time from the internet using the network time protocol.
* Seven separate alarms -- one for every day of the week.
* 


## To do

* Allow tune selection on web page.
* Volume control
* Set alarm (and time) via the buttons.

## Possibly later

* WiFi connection details on the webpage.

## Customisation

Some adjustments require changes to the source code...

## Instructions

* Initially, or when no know WiFi networks are available, the clock will run as a separate WiFi network, and 
its interface will be available on your local network at [192.168.4.1](http://192.168.4.1).
Browse to that page and enter your WiFi details.  After that, the clock
will connect to your network, and try to update the time via NTP.

* ...
