const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en" class="h-100">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
    <link rel="shortcut icon" href="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAyZpVFh0WE1MOmNvbS5hZG9iZS54bXAAAAAAADw/eHBhY2tldCBiZWdpbj0i77u/IiBpZD0iVzVNME1wQ2VoaUh6cmVTek5UY3prYzlkIj8+IDx4OnhtcG1ldGEgeG1sbnM6eD0iYWRvYmU6bnM6bWV0YS8iIHg6eG1wdGs9IkFkb2JlIFhNUCBDb3JlIDUuNi1jMTQ1IDc5LjE2MzQ5OSwgMjAxOC8wOC8xMy0xNjo0MDoyMiAgICAgICAgIj4gPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4gPHJkZjpEZXNjcmlwdGlvbiByZGY6YWJvdXQ9IiIgeG1sbnM6eG1wPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvIiB4bWxuczp4bXBNTT0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL21tLyIgeG1sbnM6c3RSZWY9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZVJlZiMiIHhtcDpDcmVhdG9yVG9vbD0iQWRvYmUgUGhvdG9zaG9wIENDIDIwMTkgKFdpbmRvd3MpIiB4bXBNTTpJbnN0YW5jZUlEPSJ4bXAuaWlkOjcyM0IyNEUzNzU1NTExRTlCMzkxRjQ2OEExQ0JBRDQyIiB4bXBNTTpEb2N1bWVudElEPSJ4bXAuZGlkOjcyM0IyNEU0NzU1NTExRTlCMzkxRjQ2OEExQ0JBRDQyIj4gPHhtcE1NOkRlcml2ZWRGcm9tIHN0UmVmOmluc3RhbmNlSUQ9InhtcC5paWQ6NzIzQjI0RTE3NTU1MTFFOUIzOTFGNDY4QTFDQkFENDIiIHN0UmVmOmRvY3VtZW50SUQ9InhtcC5kaWQ6NzIzQjI0RTI3NTU1MTFFOUIzOTFGNDY4QTFDQkFENDIiLz4gPC9yZGY6RGVzY3JpcHRpb24+IDwvcmRmOlJERj4gPC94OnhtcG1ldGE+IDw/eHBhY2tldCBlbmQ9InIiPz6w1L5cAAACMUlEQVR42sRX3ZGCQAwW5hqwBa4EeLqne8ASsAQoQUvQEqSEowR5uKd7ckuQFizBS5ys8xF3dUEcMxNxdDf58h+i8/k8eyd98EcURUGHs6/vOT1y4kSeSC1xx8/D3+8pRB4bH10+HgAgxaysJC4CDWuIawLSPgVALN4NUOwCUvk8chcAKU/psSeew88sqCY2zCS4k7McklS4dNxZ0FkTDIAElmI5ClmTkDowV/j+RgGp9H0nAIflHMely42SGzNXrCV8P5CsN564ASCXDpLlM0mkymPlTtw95ByHLLPGsO5YnV+BcuMTKpR4vvdIZBg4t8L/Y0CKfzLC5YT9ZikyL0aKrj4AcBPT1mb4FCSyti5dCKBQ9Ts1NS5dMbjfuqWd0nrlBVstiQ2D9UCqevqrCGWnCACzuHshgE5XTuw4dHohgBvZ8USCc4rpasxFF4B0RDyZNgTigDUeItsCMCFdTWU113Wl3MoKGEQR0D3NFYAaJvmA0uLplikD5nf2hxzutjoEDdRoMaS+iTPodCfV9WyrL8ADjSsHWk9bDgWy5nErc984jpQuXXocHwHlWuL8NEmFbGwvILmfvnFcqaxOJ1CegnKtow9AEgOt3tutZ6TyXLYrnLK98vXthHtVDYPDodxuh9widCl1reOdbMS1b82We6Vw8mg9D3kv0FZgEzEwXHAt1+T1XtCbESTR0FxoRbl5+tUMgBQCxFcdRhQ39xSPAuB7J9CtdQhdAbyT/gUYAIfcPyOnTyPUAAAAAElFTkSuQmCC" />
    <!-- Bootstrap CSS -->
    <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css" integrity="sha384-ggOyR0iXCbMQv3Xipma34MD+dH/1fQ784/j6cY/iJTQUOhcWr7x9JvoRxT2MZw1T" crossorigin="anonymous">
    <!-- Font Awesome -->
    <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.8.2/css/all.css" integrity="sha384-oS3vJWv+0UjzBfQzYUhtDYW+Pj2yciDJxpsK1OYPAYjqT085Qq/1cq5FLXAZQ7Ay" crossorigin="anonymous">
    <title>Alarm Clock</title>

    <script src="https://code.jquery.com/jquery-3.3.1.slim.min.js" integrity="sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo" crossorigin="anonymous"></script>
    <!-- <script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.7/umd/popper.min.js" integrity="sha384-UO2eT0CpHqdSJQ6hJty5KVphtPhzWj9WO1clHTMGa3JDZwrnQq4sF86dIHNDz0W1" crossorigin="anonymous"></script> -->
    <script src="https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/js/bootstrap.min.js" integrity="sha384-JjSmVgyd0p3pXB1rRibZUAYoIIy6OrQ6VrjIEaFf/nJGzIxFDsf4x0xIM+B07jRM" crossorigin="anonymous"></script>
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js"></script>
</head>

<body class="d-flex flex-column h-100">
<nav class="navbar navbar-expand-md navbar-dark fixed-top bg-dark">
    <div class="container">
        <a class="navbar-brand" href="#"><i class="far fa-clock"></i> Alarm Clock</a>
    </div>
</nav>
<main role="main" class="flex-shrink-0 pb-4">
  <div class="container pt-5">
    <div class="form-row pt-3">
        <div class="col-auto">
            <h1>Set the alarm</h1>
            <div>
              <table>
                <tr><th>Day</th><th>Alarm Time</th><th>Set?</th></tr>
                <tr><td>Sunday</td><td><input id="time0" type="time" class="form-control" aria-label="Alarm time Sunday"></td><td><input id="set0" class="form-control" type="checkbox" aria-label="Alarm set Sunday"</td></tr> 
                <tr><td>Monday</td><td><input id="time1" type="time" class="form-control" aria-label="Alarm time Monday"></td><td><input id="set1" class="form-control" type="checkbox" aria-label="Alarm set Monday"</td></tr> 
                <tr><td>Tuesday</td><td><input id="time2" type="time" class="form-control" aria-label="Alarm time Tuesday"></td><td><input id="set2" class="form-control" type="checkbox" aria-label="Alarm set Tuesday"</td></tr> 
                <tr><td>Wednesday</td><td><input id="time3" type="time" class="form-control" aria-label="Alarm time Wednesday"></td><td><input id="set3" class="form-control" type="checkbox" aria-label="Alarm set Wednesday"</td></tr> 
                <tr><td>Thursday</td><td><input id="time4" type="time" class="form-control" aria-label="Alarm time Thursday"></td><td><input id="set4" class="form-control" type="checkbox" aria-label="Alarm set Thursday"</td></tr> 
                <tr><td>Friday</td><td><input id="time5" type="time" class="form-control" aria-label="Alarm time Friday"></td><td><input id="set5" class="form-control" type="checkbox" aria-label="Alarm set Friday"</td></tr> 
                <tr><td>Saturday</td><td><input id="time6" type="time" class="form-control" aria-label="Alarm time Saturday"></td><td><input id="set6" class="form-control" type="checkbox" aria-label="Alarm set Saturday"</td></tr> 
              </table>
              <button class="btn btn-primary" type=button id=sendAlarm onclick='saveAlarm()'>Save</button>
            </div>
        </div>
    </div>
  </div>
</main>

<footer class="footer mt-auto bg-light text-dark py-3">
    <div class="container">
      <div class="row">
        <div class="col-sm">
        <i class='fas fa-fw fa-code-branch'></i> Version: 1.0.2cd<br>
        <i class='fas fa-fw fa-wifi'></i> Signal Strength: <span id="WiFiValue">...</span> Light level: <span id="ADCValue">0</span>
        </div>
        <div class="col-sm text-left text-sm-right">
            <i class="fab fa-github"></i> <a href="https://github.com/witnessmenow/arduino-alarm-clock/" target="_blank">GitHub</a><br>
            <i class="far fa-question-circle"></i> <a href="http://blough.ie/bac/" target="_blank">Help & Support</a>
        </div>
      </div>
    </div>
</footer>

<script>
$(document).ready(function() {
        getWiFi();
        getData();
        getAlarm();
        });

function saveAlarm() {
    var parms = "volume=" + $("#volume").val();
    //parms += "&alarmSound=" + encodeURIComponent($("#alarmSound").val());
    for (var dy = 0; dy < 7; dy++) {
        var alarmTime = $( "#time"+dy ).val();
        if (!alarmTime) {
            alarmTime = "00:00";
        }
        parms += "&alarmTime" + dy + "=" + alarmTime + ";";
        parms += "&alarmSet"  + dy + "=" + ($("#set"+dy).prop('checked') ? "1" : "0") + ";"
    }
    $.ajax({"url": "setAlarm?" + parms, "success": ajaxSuccess, "error": ajaxError});
}

function ajaxSuccess (data, status) {
    $("#status").html("Settings saved");
    console.log("setAlarm ajax sucess:  data=" + data + ", status=" + status);
    window.setTimeout(function(){$("#status").html("")}, 5000);
    getAlarm();	// update values (in case server constrained results)
}

function ajaxError (data, status) {
    $("#status").html("ERROR!  data=" + data + ", status=" + status);
    //window.setTimeout(function(){$("#status").html("")}, 2000);
    getAlarm();	// update values (in case server constrained results)
}

//    window.setInterval(getAlarm, 2000);


function displayAlarm(data) {
    var obj = JSON.parse(data);
    //console.log("displayAlarm got %s i.e. %s", data, obj);
    $("#volume").val(obj["volume"]);
    //$("#alarmSound").val(obj["alarmSound"]);
    for (var dy = 0; dy < 7; dy++) {
        //var alarmTime = obj["alarmDay"][dy]["alarmHour"].padStart(2, "0") + ":" + obj["alarmDay"][dy]["alarmMinute"].padStart(2, "0");
        $("#time"+dy).val(obj["alarmDay"][dy]["alarmTime"]);
        let alarmSet = obj["alarmDay"][dy]["alarmSet"] == "1";
        $("#set"+dy).prop("checked", alarmSet);
    }
}
function OlddisplayAlarm(data) {

    $("#currentAlarm").html(data);
    $("#deleteAlarm").removeClass('invisible');
    if(data!="--:--"){
        $("#deleteAlarm").show();
    }
    else{
        $("#deleteAlarm").hide();
    }
}

function getAlarm(){
    $.ajax({"url": "getAlarm",
            "success": displayAlarm});
}
//function deleteAlarm(){
//  $.ajax({"url": "deleteAlarm",
//  "success": displayAlarm});
//}

// Call a function repetatively with 2 Second interval
setInterval(function() {
        getData();
        getWiFi();
}, 2000);

function getData() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("ADCValue").innerHTML =
                this.responseText;
        }
    };
    xhttp.open("GET", "readADC", true);
    xhttp.send();
}
function getWiFi() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("WiFiValue").innerHTML =
                this.responseText;
        }
    };
    xhttp.open("GET", "getWiFi", true);
    xhttp.send();
}
</script>
  </body>
</html>
)=====";
