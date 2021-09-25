const char js[] PROGMEM = R"=====(
 
// Fields by id   TODO how does vplot do it now?
let volume, volumeOutput, melody, tz, statusMsg, saveAlarmButton, saveSettingsButton, adcValue, wifiValue, currentTime;
let time = [];
let set = [];

function saveAlarm() {
    var parms = "";
    for (var dy = 0; dy < 7; dy++) {
        var alarmTime = time[dy].value;
        if (!alarmTime) {
            alarmTime = "00:00";
        }
        parms += "&alarmTime" + dy + "=" + alarmTime + ";";
        parms += "&alarmSet"  + dy + "=" + (set[dy].checked ? "1" : "0") + ";"
    }
    fetch("/setAlarm", { method: 'POST', body: parms, headers: { 'Content-Type': 'text/plain' } })
    .then(alarmSuccess);
}

// NOTE: calls same endpoint as setAlarm -- even though we're only supplying some of the information
function saveSettings () {
    var parms = "volume=" + volume.value;
    parms += "&melody=" + encodeURIComponent(melody.value);
    parms += "&tz=" + encodeURIComponent(tz.value);
    fetch("/setAlarm", { method: 'POST', body: parms, headers: { 'Content-Type': 'text/plain' } })
    .then(alarmSuccess);
}

function alarmSuccess () {
    statusMsg.textContent = "Settings saved";
    window.setTimeout(function(){ statusMsg.textContent = ""; }, 5000);
    getAlarm();	// update values (in case server constrained results)
 }

function settingsSuccess (data, status) {
    statusMsg.textContent = "Settings saved";
    console.log("setAlarm ajax success: data=" + data + ", statusMsg=" + status);
    window.setTimeout(function(){ statusMsg.textContent = ""; }, 5000);
    getSettings();	// update values (in case server constrained results)
}

function displayAlarm (json) {
    for (var dy = 0; dy < 7; dy++) {
        time[dy].value = json["alarmDay"][dy]["alarmTime"];
        set[dy].checked = (json["alarmDay"][dy]["alarmSet"] == "1");
    }
 }

function displaySettings (json) {
    //var json = JSON.parse(data);
    //console.log("displaySettings got %s i.e. %s", data, json);
    volume.value = json["volume"];
    volumeOutput.textContent = json["volume"];
    melody.value = json["melody"];
    tz.value = json["tz"];
}

function getAlarm(){
    fetch("/getAlarm")
    .then(response => response.json())
    .then((json) => {
        if (window.location.pathname == "/") {
            displayAlarm(json);
        } else {
            displaySettings(json);
        }
    });
}

function getSettings(){
    fetch("/getSettings")
    .then(response => response.json())
    .then(displaySettings);
}

function getData() {
    fetch("/readADC")
    .then(response => response.text())
    .then(text => {
        adcValue.textContent = text;
    });
}

function getWiFi() {
    fetch("/getWiFi")
    .then(response => response.text())
    .then(text => {
        wifiValue.textContent = text;
    });
}

function getTime() {
    fetch("/getTime")
    .then(response => response.text())
    .then(text => {
        currentTime.textContent = text;
    });
}

window.onload = function () {

    const id = document.getElementById.bind(document);
    const cname = document.getElementsByClassName.bind(document);

    volume = id("volume");
    volumeOutput = id("volumeOutput");
    melody = id("melody");
    tz = id("tz");
    statusMsg = id("statusMsg");
    saveAlarmButton = id("saveAlarm");
    saveSettingsButton = id("saveSettings");
    adcValue = id("ADCValue");
    wifiValue = id("WiFiValue");
    currentTime = id("currentTime");
    for (let i = 0; i < 7; i++) {
        time[i] = id("time" + i);
        set[i] = id("set" + i);
    }

    getWiFi();
    getData();
    if (window.location.pathname == "/") {
        getAlarm();
        getTime();
        saveAlarmButton.addEventListener("click", saveAlarm);
        // TEMP !! setInterval(function() {
        //    getData();
        //    getWiFi();
        //    getTime();
        //}, 2000);
    } else {
        volumeOutput.textContent = volume.value;
        volume.addEventListener('input', function() {
            volumeOutput.textContent = volume.value;
        });
        getSettings();
        saveSettingsButton.addEventListener("click", saveSettings);
        // TEMP !! setInterval(function() {
        //    getData();
        //    getWiFi();
        //}, 2000);
    }

}
)=====";
