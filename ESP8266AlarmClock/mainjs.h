const char mainjs[] PROGMEM = R"=====(

// TODO:
//  set0.style.backgroundColor = "red"   i.e. show if alarm has been overridden
//  don't seem to need the whole alarmFieldChanged stuff -- but see NOTEs

// NOTE:
// * 'nextAlarmOverridden' is unusual -- it can get updated by button presses, so we get it in
//   getData.  But changes here get saved with setAlarm.
//   Oh!  If alarm can get set via buttons (future plans), then anything can change...

let nao, nextAlarmIn, volume, volumeOutput, statusMsg, setAlarmButton, adcValue, wifiValue, currentTime;
let time = [];
let set = [];
let nextAlarmOverridden = false;  // FIXME this is superfluous -- use nao.checked
let alarmFieldChanged = false;  // True if user has changed one of the alarm fields since last time they were set.
let alarmFieldTimeout = 0;

const fetchTimeout = 5000;
const fetchWaitTimeout = 10000;
const msgFadeTime = 5000;

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

function setAlarm() {
    if (fetchingTimer) {
        showWaiting(true);
        // Try again in a moment
        setTimeout(setAlarm, 0);
        return;
    }
    alreadyFetching("setAlarm");
    showWaiting(false);
    nextAlarmOverridden = nao.checked;
    const jsonObject = { 
        alarmTime: [],
        alarmSet: [],
        volume: volume.value,
        nextAlarmOverridden: nextAlarmOverridden,
    };
    for (var dy = 0; dy < 7; dy++) {
        var alarmTime = time[dy].value;
        if (!alarmTime) {
            alarmTime = "00:00";
        }
        jsonObject.alarmTime[dy] = alarmTime;
        jsonObject.alarmSet[dy] = (set[dy].checked ? "1" : "0"); 
    }
    
    fetch("/setAlarm", { method: 'POST', body: JSON.stringify(jsonObject), headers: { 'Content-Type': 'application/json' } })
    .then(alarmSuccess);
}

function alarmSuccess () {
    statusMsg.textContent = "alarms saved";
    fetchingDone("alarmSuccess");
    window.setTimeout(function(){ statusMsg.textContent = ""; }, msgFadeTime);
    getAlarm();	// update values (in case server constrained results)
 }

function displayAlarm (json) {
    if (Object.hasOwn(json, "alarmDay")) {
        for (var dy = 0; dy < 7; dy++) {
            time[dy].value = json["alarmDay"][dy]["alarmTime"];
            set[dy].checked = (json["alarmDay"][dy]["alarmSet"] == "1");
        }
    }
    volume.value = json["volume"];
    volumeOutput.textContent = json["volume"];
 }

function getAlarm(){
    if (alreadyFetching("getAlarm")) return;
    fetch("/getAlarm")
    .then(response => response.json())
    .then((json) => {
        fetchingDone("getAlarm");
        displayAlarm(json);
        alarmFieldChanged = false;
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
        nextAlarmIndex = json.nextAlarmIndex;
        if (!alarmFieldChanged) {
            nextAlarmOverridden = json.nextAlarmOverridden;
            nao.checked = nextAlarmOverridden;
            naiMins = json.nextAlarmIn;
            if (naiMins < 0) {
                nextAlarmIn.textContent = "No alarm set in next 24 hours";
            } else if (naiMins == 0) {
                nextAlarmIn.textContent = "Alarm currently ringing";
            } else {
                naiHrs = naiMins / 60 | 0;  // integer division(!)
                naiMins %= 60
                naiString = "Next alarm in ";
                if (naiHrs > 0) {
                    naiString += `${naiHrs} hours and `;
                }
                naiString += `${naiMins} minutes`;
                nextTime = document.getElementById("time" + nextAlarmIndex);
                naiString += ` at ${nextTime.value}`;
                nextAlarmIn.textContent = naiString;
            }
        }

        // Also update alarms stuff -- could have been changed via buttons.
        // But don't if user is entering things.
        if (!alarmFieldChanged) {
            displayAlarm(json);
        }
    })
    .catch(() => {});  // do nothing if the fetch failed
}

function setAlarmFieldChanged () {
    alarmFieldChanged = true;
    clearTimeout(alarmFieldTimeout);
    alarmFieldTimeout = setTimeout(() => {
        // Clear the flag (and lose user input) if they do nothing for a while
        alarmFieldChanged = false;
    }, 5*60*1000); // 5 mins
}

window.onload = function () {

    const id    = document.getElementById.bind(document);
    const cname = document.getElementsByClassName.bind(document);

    nao            = id("nao");
    nextAlarmIn    = id("nextAlarmIn");
    volume         = id("volume");
    volumeOutput   = id("volumeOutput");
    statusMsg      = id("statusMsg");
    setAlarmButton = id("setAlarm");
    adcValue       = id("ADCValue");
    wifiValue      = id("WiFiValue");
    currentTime    = id("currentTime");
    for (let i = 0; i < 7; i++) {
        time[i]    = id("time" + i);
        set[i]     = id("set" + i);
    }
    alarmFieldChanged = false;

    getAlarm();
    setAlarmButton.addEventListener("click", setAlarm);

    volumeOutput.textContent = volume.value;
    volume.addEventListener('input', function() {
        volumeOutput.textContent = volume.value;
        setAlarmFieldChanged();
    });

    // Detect any changes to alarm settings, to avoid overwriting user 
    // input with values from getData()
    const alarmFields = document.querySelectorAll('input.alarmField');
    alarmFields.forEach((af) => {
        // 'input' works except for the checkbox    ?
        //af.addEventListener('input', setAlarmFieldChanged);
        af.addEventListener('change', setAlarmFieldChanged);
    });

    // Call getData repeatedly
    setInterval(function() {
        getData();
    }, 2000);  // 2s

}
)=====";
