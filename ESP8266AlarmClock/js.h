const char js[] PROGMEM = R"=====(


// FIXME !  saving alarm/settings needs to take priority over getData -- make it wait 5 seconds for fetching to be available
 
// Fields by id   TODO how does vplot do it now?
let volume, volumeOutput, melody, tz, statusMsg, saveAlarmButton, saveSettingsButton, adcValue, wifiValue, currentTime;
let time = [];
let set = [];

// Flag to make sure only one 'fetch' is active at a time
let fetchingTimer = 0;

// Check that fetching is not already happening, and 
// initialise the timer.
function alreadyFetching (label) {
    if (fetchingTimer) {
        console.log("Already fetching in", label);
        return true;
    }
    fetchingTimer = setTimeout(function() { 
        console.log("fetch timed out in", label); 
        fetchingDone("timeout"); 
    }, 5000);
    console.log(label, "fetchingTimer =", fetchingTimer);
    return false;
}

function fetchingDone (label) {
    console.log("fetchingDone for %s, t=%d", label, fetchingTimer);
    clearTimeout(fetchingTimer);
    fetchingTimer = 0;
}

function saveAlarm() {

    if (alreadyFetching("saveAlarm")) return;

    var parms = "";
    for (var dy = 0; dy < 7; dy++) {
        var alarmTime = time[dy].value;
        if (!alarmTime) {
            alarmTime = "00:00";
        }
        parms += "&alarmTime" + dy + "=" + alarmTime + ";";
        parms += "&alarmSet"  + dy + "=" + (set[dy].checked ? "1" : "0") + ";"
    }
    //fetch("/setAlarm", { method: 'POST', body: parms, headers: { 'Content-Type': 'text/plain' } })
    fetch("/setAlarm?" + parms) // , { method: 'GET', headers: { 'Content-Type': 'text/plain' } })
    .then(alarmSuccess);

}

// NOTE: calls same endpoint as setAlarm -- even though we're only supplying some of the information
// TODO: JSON or like this:
// fetch('https://example.com?' + new URLSearchParams({
//     foo: 'value',
//     bar: 2,
// }))
function saveSettings () {
    var parms = "volume=" + volume.value;
    parms += "&melody=" + encodeURIComponent(melody.value);
    parms += "&tz=" + encodeURIComponent(tz.value);
    //fetch("/setAlarm", { method: 'POST', body: parms, headers: { 'Content-Type': 'text/plain' } })
    fetch("/setAlarm?" + parms, { method: 'GET', headers: { 'Content-Type': 'text/plain' } })
    .then(alarmSuccess);
}

function alarmSuccess () {
    statusMsg.textContent = "Settings saved";
    fetchingDone("alarmSuccess");
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

// FIXME separate alarm and settings
function getAlarm(){
    if (alreadyFetching("getAlarm")) return;
    fetch("/getAlarm")
    .then(response => response.json())
    .then((json) => {
        fetchingDone("getAlarm");
        if (window.location.pathname == "/") {
            displayAlarm(json);
        } else {
            displaySettings(json);
        }
    });
}

function getSettings(){
    if (alreadyFetching("getSettings")) return;
    fetch("/getSettings")
    .then(response => response.json())
    .then((json) => {
        fetchingDone("getSettings");
        displaySettings(json);
    });
}

function getData() {
    if (alreadyFetching("getData")) return;
    fetch("/getData")
    .then(response => response.json())
    .then((json) => {
        fetchingDone("getData");
        adcValue.textContent = json.lightLevel;
        wifiValue.textContent = json.wifiQuality;
        currentTime.textContent = json.time;
    });
}

/*
function getWiFi() {
    if (alreadyFetching("getWiFi")) return;
    fetch("/getWiFi")
    .then(response => response.text())
    .then(text => {
        fetchingDone("getWiFi");
        wifiValue.textContent = text;
    });
}

function getTime() {
    if (alreadyFetching("getTime")) return;
    fetch("/getTime")
    .then(response => response.text())
    .then(text => {
        fetchingDone("getTime");
        currentTime.textContent = text;
    });
}
*/

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

    if (window.location.pathname == "/") {
        getAlarm();
        saveAlarmButton.addEventListener("click", saveAlarm);
    } else {
        volumeOutput.textContent = volume.value;
        volume.addEventListener('input', function() {
            volumeOutput.textContent = volume.value;
        });
        getSettings();
        saveSettingsButton.addEventListener("click", saveSettings);
    }
    setInterval(function() {
        getData();
    }, 2000);

}
)=====";
