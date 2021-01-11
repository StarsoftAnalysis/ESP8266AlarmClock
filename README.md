# arduino-alarm-clock

ESP8266 code to run a simple alarm clock, running on [Brian Lough](http://blough.ie/bac/)'s
alarm clock kit, which is available from [Tindie](https://www.tindie.com/products/brianlough/blough-alarm-clock-shield-for-wemos-d1-mini/).

This is StarSoftAnalysis's version of the software, based on [Brian](https://github.com/witnessmenow/arduino-alarm-clock)'s, and incorporating
features from [James Brown's fork](https://github.com/jbrown123/arduino-alarm-clock).

## Features

* Gets the current time from the internet using the network time protocol, automatically adjusting for daylight savings in the local timezone.
* Seven separate alarms -- one for every day of the week.
* Colon flashes every second, and stays on if the next alarm is set.
* RTTTL alarm melody.


## To do

* Allow tune selection on web page.
* Volume control
* Set alarm (and time) via the buttons.

## Possibly later

* WiFi connection details on the webpage.

## Customisation

Some adjustments require changes to the source code...

* Timezone (defaults to Europe/London)
* Alarm tune.  

## Instructions

* Initially, or when no known WiFi networks are available, the clock will run as a separate WiFi network called
"Alarm Clock", and 
its interface will be available as [http://192.168.4.1](http://192.168.4.1).
Connect to that network, and browse to that page and enter your WiFi details.  After that, the clock
will connect to your network, and try to update the time via NTP.
For further information on this process, see the documentation for the [WiFiManager](https://github.com/tzapu/WiFiManager) library.

* When the clock has connect to your network, it will display the last digit of its IP address for a few seconds.
For example, if the clock gets IP address 192.168.1.55, it will display '55'.  It will also display this number
after the 'boot' message each time it is connected to the power supply.

* Browse to the alarm clock's webpage: setting the alarm for each day of the week is easy.

* You can paste in the RTTTL code for a different melody of required -- melodies can be found
on the web ([Picaxe](https://picaxe.com/rtttl-ringtones-for-tune-command/), for example), or you can create your own.
To test the melody, copy-and-paste it into a site such as [Adam Rahwane](https://adamonsoon.github.io/rtttl-play/)'s.

* Choose your timezone from the list on [Wikipedia](https://en.wikipedia.org/wiki/List_of_tz_database_time_zones).
