// settingsjs.h -- Javascript for the settings page -- simpler because no getData() updates

const char settingsjs[] PROGMEM = R"=====(

const fetchTimeout = 5000;
//const fetchWaitTimeout = 10000;
const msgFadeTime = 5000;
const wifi_max = 3; // TODO make sure this matches WIFI_MAX

// Fields by id 
let volume, volumeOutput, melody, tz, statusMsg, saveSettingsButton;
let wifissid = Array(wifi_max);
let wifipass = Array(wifi_max);
let pwchanged = Array(wifi_max);
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
    }, fetchTimeout);
    //console.log(label, "fetchingTimer =", fetchingTimer);
    return false;
}

function fetchingDone (label) {
    //console.log("fetchingDone for %s, t=%d", label, fetchingTimer);
    clearTimeout(fetchingTimer);
    fetchingTimer = 0;
}

function showWaiting (flag) {
    if (flag) {
        statusMsg.textContent = '...';
        document.body.style.cursor = 'wait';
    } else {
        statusMsg.textContent = '';
        document.body.style.cursor = '';
    }
}

function saveSettings () {
    if (fetchingTimer) {
        showWaiting(true);
        // Try again in a moment
        setTimeout(saveSettings, 0);
        return;
    }
    alreadyFetching("saveSettings");
    showWaiting(false);

    let wifissidvalues = new Array(wifi_max);    
    let wifipassvalues = new Array(wifi_max);    
    for (let i = 0; i < wifi_max; i++) {
        wifissidvalues[i] = wifissid[i].value;
        if (pwchanged[i]) {
            wifipassvalues[i] = wifipass[i].value;
        }
    }
    const jsonObject = {
        melody: melody.value,
        volume: volume.value,
        tz: tz.value,
        wifissid: wifissidvalues,
        wifipass: wifipassvalues,
    }
    console.log("sending settings: ", jsonObject);
    fetch("/setSettings", { method: 'POST', body: JSON.stringify(jsonObject), headers: { 'Content-Type': 'application/json' } })
    .then(() => {
        fetchingDone("saveSettings");
        statusMsg.textContent = "Settings saved";
        window.setTimeout(function(){ statusMsg.textContent = ""; }, msgFadeTime);
        getSettings();	// update values (in case server constrained results)
    });
}

function displaySettings (json) {
    //var json = JSON.parse(data);
    //console.log("displaySettings got %s i.e. %s", data, json);
    volume.value = json["volume"];
    volumeOutput.textContent = json["volume"];
    melody.value = json["melody"];
    tz.value = json["tz"];
    for (let i = 0; i < wifi_max; i++) {
         wifissid[i].value = json.wifissid[i];
         wifipass[i].value = "*".repeat(json.wifipass[i]);
    }
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

window.onload = function () {

    const id = document.getElementById.bind(document);
    const cname = document.getElementsByClassName.bind(document);

    volume = id("volume");
    volumeOutput = id("volumeOutput");
    melody = id("melody");
    tz = id("tz");
    saveSettingsButton = id("saveSettings");
    for (let i = 0; i < wifi_max; i++) {   // FIXME get WIFI_MAX somehow
        wifissid[i] = id("wifissid" + i);
        wifipass[i] = id("wifipass" + i);
        pwchanged[i] = false;
        wifipass[i].addEventListener("change", () => { pwchanged[i] = true; });
    }
    statusMsg = id("statusMsg");

    volumeOutput.textContent = volume.value;
    volume.addEventListener('input', function() {
        volumeOutput.textContent = volume.value;
    });

    saveSettingsButton.addEventListener("click", saveSettings);
    getSettings();

}
)=====";
