const char js[] PROGMEM = R"=====(

function saveAlarm() {
    var parms = "";
    for (var dy = 0; dy < 7; dy++) {
        var alarmTime = $("#time"+dy).val();
        if (!alarmTime) {
            alarmTime = "00:00";
        }
        parms += "&alarmTime" + dy + "=" + alarmTime + ";";
        parms += "&alarmSet"  + dy + "=" + ($("#set"+dy).prop('checked') ? "1" : "0") + ";"
    }
    $.ajax({"url": "setAlarm?" + parms, "success": alarmSuccess, "error": ajaxError});
}

function saveSettings() {
    var parms = "volume=" + $("#volume").val();
    parms += "&melody=" + encodeURIComponent($("#melody").val());
    parms += "&tz=" + encodeURIComponent($("#tz").val());
    $.ajax({"url": "saveSettings?" + parms, "success": settingsSuccess, "error": ajaxError});
}

function alarmSuccess (data, status) {
    $("#status").html("Settings saved");
    console.log("setAlarm ajax success: data=" + data + ", status=" + status);
    window.setTimeout(function(){ $("#status").html("Saved") }, 5000);
    getAlarm();	// update values (in case server constrained results)
}

function settingsSuccess (data, status) {
    $("#status").html("Settings saved");
    console.log("setAlarm ajax success: data=" + data + ", status=" + status);
    window.setTimeout(function(){ $("#status").html("Saved") }, 5000);
    getSettings();	// update values (in case server constrained results)
}

function ajaxError (data, status) {
    $("#status").html("ERROR! data=" + data + ", status=" + status);
    window.setTimeout(function(){ $("#status").html("Failed") }, 5000);
    //getAlarm();	// update values (in case server constrained results)
}

function displayAlarm(data) {
    var obj = JSON.parse(data);
    //console.log("displayAlarm got %s i.e. %s", data, obj);
    for (var dy = 0; dy < 7; dy++) {
        $("#time"+dy).val(obj["alarmDay"][dy]["alarmTime"]);
        let alarmSet = obj["alarmDay"][dy]["alarmSet"] == "1";
        $("#set"+dy).prop("checked", alarmSet);
    }
}

function displaySettings(data) {
    var obj = JSON.parse(data);
    //console.log("displaySettings got %s i.e. %s", data, obj);
    $("#volume").val(obj["volume"]);
    $("#volumeOutput").text(obj["volume"]);
    $("#melody").val(obj["melody"]);
    $("#tz").val(obj["tz"]);
    volumeOutput.textContent = volume.value;
}

function getAlarm(){
    $.ajax({"url": "getAlarm",
            "success": displayAlarm});
}

function getSettings(){
    $.ajax({"url": "getSettings",
            "success": displaySettings});
}


function getData() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("ADCValue").innerHTML = this.responseText;
        }
    };
    xhttp.open("GET", "readADC", true);
    xhttp.send();
}

function getWiFi() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("WiFiValue").innerHTML = this.responseText;
        }
    };
    xhttp.open("GET", "getWiFi", true);
    xhttp.send();
}

function getTime() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("currentTime").innerHTML = this.responseText;
        }
    };
    xhttp.open("GET", "getTime", true);
    xhttp.send();
}

$(document).ready(function() {

    const saveAlarmButton = document.getElementById('saveAlarm');
    const saveSettingsButton = document.getElementById('saveSettings');
    const volumeInput  = document.querySelector('#volume');
    const volumeOutput = document.querySelector('#volumeOutput');

    getWiFi();
    getData();
    if (window.location.pathname == "/") {
        getAlarm();
        getTime();
        saveAlarmButton.addEventListener("click", saveAlarm);
        setInterval(function() {
            getData();
            getWiFi();
            getTime();
        }, 2000);
    } else {
        volumeOutput.textContent = volumeInput.value;
        volumeInput.addEventListener('input', function() {
            //const parms = "volume=" + volume.value.toString();
            //$.ajax({"url": "setvolume?" + parms, "success": setVolume, "error": ajaxError});
            volumeOutput.textContent = volume.value;
        });
        getSettings();
        saveSettingsButton.addEventListener("click", saveSettings);
        setInterval(function() {
            getData();
            getWiFi();
        }, 2000);
    }

});
)=====";
