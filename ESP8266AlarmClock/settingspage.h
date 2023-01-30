const char settingspage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en" class="h-100">
<head>
<!--

    Copyright 2021 Chris Dennis

    This file is part of ESP8266AlarmClock.

    ESP8266AlarmClock is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ESP8266AlarmClock is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ESP8266AlarmClock.  If not, see <https://www.gnu.org/licenses/>.

-->
<title>Alarm Clock - Settings</title>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
<link rel="shortcut icon" href='data:image/svg+xml;utf8,<!-- clock.svg --><svg version="1.1" xmlns="http://www.w3.org/2000/svg" width="24" height="28" viewBox="0 0 24 28"><path d="M14 8.5v7c0 0.281-0.219 0.5-0.5 0.5h-5c-0.281 0-0.5-0.219-0.5-0.5v-1c0-0.281 0.219-0.5 0.5-0.5h3.5v-5.5c0-0.281 0.219-0.5 0.5-0.5h1c0.281 0 0.5 0.219 0.5 0.5zM20.5 14c0-4.688-3.813-8.5-8.5-8.5s-8.5 3.813-8.5 8.5 3.813 8.5 8.5 8.5 8.5-3.813 8.5-8.5zM24 14c0 6.625-5.375 12-12 12s-12-5.375-12-12 5.375-12 12-12 12 5.375 12 12z"></path></svg>'>

<link href="/bootstrapcss" rel=stylesheet>

<script src="/bootstrapjs"></script>
<script src="/settingsjs" xdefer></script>

</head>

<body class="d-flex flex-column h-100">
    <nav class="navbar navbar-expand-md navbar-dark bg-dark">
        <div class="container">
            <a href="/"><h1 class="text-white mb-0"><!-- clock.svg --><svg version="1.1" xmlns="http://www.w3.org/2000/svg" width="24" height="28" viewBox="0 0 24 28"><path d="M14 8.5v7c0 0.281-0.219 0.5-0.5 0.5h-5c-0.281 0-0.5-0.219-0.5-0.5v-1c0-0.281 0.219-0.5 0.5-0.5h3.5v-5.5c0-0.281 0.219-0.5 0.5-0.5h1c0.281 0 0.5 0.219 0.5 0.5zM20.5 14c0-4.688-3.813-8.5-8.5-8.5s-8.5 3.813-8.5 8.5 3.813 8.5 8.5 8.5 8.5-3.813 8.5-8.5zM24 14c0 6.625-5.375 12-12 12s-12-5.375-12-12 5.375-12 12-12 12 5.375 12 12z"></path></svg> Alarm Clock - Settings</h1></a>
        </div>
    </nav>
    <main role="main">
        <div class="container">
            <div class=form-group>
                <div class="form-row">
                    <div class="col-auto">
                        <label for=melody class="h3 mt-3">RTTTL Melody</label>
                        <textarea id=melody class="form-control" rows="5" cols="60" maxlength=500></textarea>
                    </div>
                    <div class="col-auto">
                        <label for=tz class="h3 mt-3">Timezone</label>
                        <input type=text id=tz class="form-control" maxlength=32>
                    </div>
                </div>

                <div class=form-row>
                    <p>Enter the name (SSID) and password (key) for up to three WiFi networks.
                    <p>Clear an SSID field to delete it.
                    <p>When you click on 'Save' with at least one SSID entered, the plotter will revert to, or stay in, 'station' mode and attempt to connect to the network.

                    <h4>WiFi Network 1</h4>
                    <p><label for="wifissid0">SSID:</label> <input type="text" name=wifissid0 id="wifissid0">
                    <label for="wifipass0">Password:</label> <input type="password" name=wifipass0 id="wifipass0">
                    <h4>WiFi Network 2</h4>
                    <p><label for="wifissid1">SSID:</label> <input type="text" name=wifissid1 id="wifissid1"> 
                    <label for="wifipass1">Password:</label> <input type="password" name=wifipass1 id="wifipass1">
                    <h4>WiFi Network 3</h4>
                    <p><label for="wifissid2">SSID:</label> <input type="text" name=wifissid2 id="wifissid2">
                    <label for="wifipass2">Password:</label> <input type="password" name=wifipass2 id="wifipass2">

                <div class="form-row">
                    <div class="col-auto">
                        <button class="btn btn-primary mt-3" type=button id=saveSettings>Save</button>
                        <span id="statusMsg"></span>
                    </div>
                </div>
                </div>

            </div>
            <a href="/">Main Page</a>

        </div>
    </main>

</body>
</html>
)=====";
