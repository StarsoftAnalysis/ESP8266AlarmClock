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
    <script src="/js" xdefer></script>

</head>

<body class="d-flex flex-column h-100">
<nav class="navbar navbar-expand-md navbar-dark bg-dark">
  <div class="container">
      <h1 class="text-white mb-0"><!-- clock.svg --><svg version="1.1" xmlns="http://www.w3.org/2000/svg" width="24" height="28" viewBox="0 0 24 28"><path d="M14 8.5v7c0 0.281-0.219 0.5-0.5 0.5h-5c-0.281 0-0.5-0.219-0.5-0.5v-1c0-0.281 0.219-0.5 0.5-0.5h3.5v-5.5c0-0.281 0.219-0.5 0.5-0.5h1c0.281 0 0.5 0.219 0.5 0.5zM20.5 14c0-4.688-3.813-8.5-8.5-8.5s-8.5 3.813-8.5 8.5 3.813 8.5 8.5 8.5 8.5-3.813 8.5-8.5zM24 14c0 6.625-5.375 12-12 12s-12-5.375-12-12 5.375-12 12-12 12 5.375 12 12z"></path></svg> Alarm Clock</h1>
  </div>
</nav>
<main role="main">
  <div class="container">
    <div class=form-group>
      <div class="form-row">
        <div class="col-auto">
          <label for=melody class="h3 mt-3">RTTTL Melody</label>
          <textarea id=melody class="form-control" rows="5" cols="60" maxlength=500></textarea>
          <p class="h4 mt-3"><label for="volume">Volume:</label> <output class="xxform-control" for="volume" id="volumeOutput"></output>
          <input class="form-control" type="range" name="volume" id="volume" min="0" max="11" step="1" value="5">
        </div>
        <div class="col-auto">
          <label for=tz class="h3 mt-3">Timezone</label>
          <input type=text id=tz class="form-control" maxlength=32>
        </div>
      </div>
      <div class="form-row">
        <div class="col-auto">
          <button class="btn btn-primary mt-3" type=button id=saveSettings>Save</button>
          <span id="statusMsg"></span>
        </div>
      </div>
    </div>

    <a href="/">Main Page</a>

  </div>
</main>

<footer class="footer mt-auto bg-light text-dark py-3">
    <div class="container">
      <div class="row">
        <div class="col-sm">
            <!-- wifi.svg --><svg version="1.1" xmlns="http://www.w3.org/2000/svg" height="1em" viewBox="0 0 32 28"> <path d="M16 23.797c-0.406 0-2.594-2.188-2.594-2.609 0-0.766 2-1.188 2.594-1.188s2.594 0.422 2.594 1.188c0 0.422-2.188 2.609-2.594 2.609zM20.219 19.563c-0.219 0-1.922-1.563-4.219-1.563-2.312 0-3.984 1.563-4.219 1.563-0.375 0-2.641-2.25-2.641-2.625 0-0.141 0.063-0.266 0.156-0.359 1.672-1.656 4.391-2.578 6.703-2.578s5.031 0.922 6.703 2.578c0.094 0.094 0.156 0.219 0.156 0.359 0 0.375-2.266 2.625-2.641 2.625zM24.484 15.313c-0.125 0-0.266-0.063-0.359-0.125-2.578-2-4.75-3.187-8.125-3.187-4.719 0-8.313 3.313-8.484 3.313-0.359 0-2.609-2.25-2.609-2.625 0-0.125 0.063-0.25 0.156-0.344 2.797-2.797 7.016-4.344 10.938-4.344s8.141 1.547 10.938 4.344c0.094 0.094 0.156 0.219 0.156 0.344 0 0.375-2.25 2.625-2.609 2.625zM28.719 11.078c-0.125 0-0.25-0.063-0.344-0.141-3.609-3.172-7.5-4.937-12.375-4.937s-8.766 1.766-12.375 4.937c-0.094 0.078-0.219 0.141-0.344 0.141-0.359 0-2.625-2.25-2.625-2.625 0-0.141 0.063-0.266 0.156-0.359 3.953-3.922 9.656-6.094 15.188-6.094s11.234 2.172 15.188 6.094c0.094 0.094 0.156 0.219 0.156 0.359 0 0.375-2.266 2.625-2.625 2.625z"></path></svg>
            Signal Strength: <span id="WiFiValue">...</span>%<br>
            
            <!-- lightbulb-o.svg --><svg version="1.1" xmlns="http://www.w3.org/2000/svg" height=1em viewBox="0 0 16 28"><path d="M11.5 9c0 0.266-0.234 0.5-0.5 0.5s-0.5-0.234-0.5-0.5c0-1.078-1.672-1.5-2.5-1.5-0.266 0-0.5-0.234-0.5-0.5s0.234-0.5 0.5-0.5c1.453 0 3.5 0.766 3.5 2.5zM14 9c0-3.125-3.172-5-6-5s-6 1.875-6 5c0 1 0.406 2.047 1.062 2.812 0.297 0.344 0.641 0.672 0.953 1.031 1.109 1.328 2.047 2.891 2.203 4.656h3.563c0.156-1.766 1.094-3.328 2.203-4.656 0.313-0.359 0.656-0.688 0.953-1.031 0.656-0.766 1.062-1.813 1.062-2.812zM16 9c0 1.609-0.531 3-1.609 4.188s-2.5 2.859-2.625 4.531c0.453 0.266 0.734 0.766 0.734 1.281 0 0.375-0.141 0.734-0.391 1 0.25 0.266 0.391 0.625 0.391 1 0 0.516-0.266 0.984-0.703 1.266 0.125 0.219 0.203 0.484 0.203 0.734 0 1.016-0.797 1.5-1.703 1.5-0.406 0.906-1.313 1.5-2.297 1.5s-1.891-0.594-2.297-1.5c-0.906 0-1.703-0.484-1.703-1.5 0-0.25 0.078-0.516 0.203-0.734-0.438-0.281-0.703-0.75-0.703-1.266 0-0.375 0.141-0.734 0.391-1-0.25-0.266-0.391-0.625-0.391-1 0-0.516 0.281-1.016 0.734-1.281-0.125-1.672-1.547-3.344-2.625-4.531s-1.609-2.578-1.609-4.188c0-4.25 4.047-7 8-7s8 2.75 8 7z"></path></svg>
            Light level: <span id="ADCValue">...</span>
        </div>
        <div class="col-sm text-left text-sm-right">

            <!-- code-fork.svg --><svg version="1.1" xmlns="http://www.w3.org/2000/svg" height="1em" viewBox="0 0 16 28"><path d="M4.5 23c0-0.828-0.672-1.5-1.5-1.5s-1.5 0.672-1.5 1.5 0.672 1.5 1.5 1.5 1.5-0.672 1.5-1.5zM4.5 5c0-0.828-0.672-1.5-1.5-1.5s-1.5 0.672-1.5 1.5 0.672 1.5 1.5 1.5 1.5-0.672 1.5-1.5zM14.5 7c0-0.828-0.672-1.5-1.5-1.5s-1.5 0.672-1.5 1.5 0.672 1.5 1.5 1.5 1.5-0.672 1.5-1.5zM16 7c0 1.109-0.609 2.078-1.5 2.594-0.047 5.641-4.047 6.891-6.703 7.734-2.484 0.781-3.297 1.156-3.297 2.672v0.406c0.891 0.516 1.5 1.484 1.5 2.594 0 1.656-1.344 3-3 3s-3-1.344-3-3c0-1.109 0.609-2.078 1.5-2.594v-12.812c-0.891-0.516-1.5-1.484-1.5-2.594 0-1.656 1.344-3 3-3s3 1.344 3 3c0 1.109-0.609 2.078-1.5 2.594v7.766c0.797-0.391 1.641-0.656 2.406-0.891 2.906-0.922 4.562-1.609 4.594-4.875-0.891-0.516-1.5-1.484-1.5-2.594 0-1.656 1.344-3 3-3s3 1.344 3 3z"></path>
</svg>
            Version: 0.2.0dev<br>

            <!-- github.svg --><svg version="1.1" xmlns="http://www.w3.org/2000/svg" height="1em" viewBox="0 0 32 32"><path d="M16 0.396c-8.84 0-16 7.164-16 16 0 7.071 4.584 13.067 10.94 15.18 0.8 0.151 1.093-0.344 1.093-0.769 0-0.38-0.013-1.387-0.020-2.72-4.451 0.965-5.389-2.147-5.389-2.147-0.728-1.847-1.78-2.34-1.78-2.34-1.449-0.992 0.112-0.972 0.112-0.972 1.607 0.112 2.451 1.648 2.451 1.648 1.427 2.447 3.745 1.74 4.66 1.331 0.144-1.035 0.556-1.74 1.013-2.14-3.553-0.4-7.288-1.776-7.288-7.907 0-1.747 0.62-3.173 1.647-4.293-0.18-0.404-0.72-2.031 0.14-4.235 0 0 1.34-0.429 4.4 1.64 1.28-0.356 2.64-0.532 4-0.54 1.36 0.008 2.72 0.184 4 0.54 3.040-2.069 4.38-1.64 4.38-1.64 0.86 2.204 0.32 3.831 0.16 4.235 1.020 1.12 1.64 2.547 1.64 4.293 0 6.147-3.74 7.5-7.3 7.893 0.56 0.48 1.080 1.461 1.080 2.96 0 2.141-0.020 3.861-0.020 4.381 0 0.42 0.28 0.92 1.1 0.76 6.401-2.099 10.981-8.099 10.981-15.159 0-8.836-7.164-16-16-16z"></path></svg>
            <a href="https://github.com/starsoftanalysis/ESP8266AlarmClock/" target="_blank">GitHub</a><br>
        </div>
      </div>
    </div>
</footer>

  </body>
</html>
)=====";
