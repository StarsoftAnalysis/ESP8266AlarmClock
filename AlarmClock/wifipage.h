const char wifipage[] PROGMEM = R"=====(
<html>
<head>
  <meta charset='utf-8'>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <!-- script src="microajax.js"></script -->
  <script>

  // TODO factor out these functions that are duplicated in index.html
  //      style it like the main page

    // 'fetch' -- see https://developer.mozilla.org/en-US/docs/Web/API/Fetch_API
    // Example POST method implementation:
    async function postData(url = '', data = {}) {
      // Default options are marked with *
      console.log("postData url=", url, " data=", data);
      console.log(" json version=", JSON.stringify(data));
      const response = await fetch(url, {
        method: 'POST', // *GET, POST, PUT, DELETE, etc.
        mode: 'cors', // no-cors, *cors, same-origin
        cache: 'no-cache', // *default, no-cache, reload, force-cache, only-if-cached
        credentials: 'same-origin', // include, *same-origin, omit
        headers: {
          'Content-Type': 'application/json'
          // 'Content-Type': 'application/x-www-form-urlencoded',
        },
        redirect: 'follow', // manual, *follow, error
        referrerPolicy: 'no-referrer', // no-referrer, *no-referrer-when-downgrade, origin, origin-when-cross-origin, same-origin, strict-origin, strict-origin-when-cross-origin, unsafe-url
        body: JSON.stringify(data) // body data type must match "Content-Type" header
      });
      return response.json(); // parses JSON response into native JavaScript objects
    }

  // Set span text
  function settext (id, text) {
    let element = document.getElementById(id);
    if (element) {
      element.textContent = text;
    }
  }

  // Set input value
  function setvalue (id, text) {
    let element = document.getElementById(id);
    if (element) {
      element.value = text;
    }
  }

  // Get input value
  function getvalue (id) {
    let element = document.getElementById(id);
    if (element) {
      return element.value
    }
    return '';
  }

  function showapsettings (json) {
    console.log("showapsettings json=", json);
    //let json = JSON.parse(res.responseText);
    for (let key in json) {
      //if (key.indexOf('pass') >= 0) {
      //  //setvalue(key, '****'); // don't show, rely on placeholder texti
      //} else {
        setvalue(key, json[key]);
      //}
    }
    // It's now an array
    for (var i = 0; i < json.length; i++) {
        setvalue("wifissid"+i, json[i].wifissid);
        setvalue("wifipass"+i, json[i].wifipass);
    }
  }

  function getapsettings () {
    postData('/wifiGet', {})
    .then(data => {
      console.log("/wifiGet sent data:", data); // JSON data parsed by `data.json()` call
      showapsettings(data);
    });
  }

  function submitform () {
    /*const data = {
      wifissid0: getvalue('wifissid0'),
      wifipass0: getvalue('wifipass0'),
      wifissid1: getvalue('wifissid1'),
      wifipass1: getvalue('wifipass1'),
      wifissid2: getvalue('wifissid2'),
      wifipass2: getvalue('wifipass2'),
    };*/
    const data = [
        {wifissid: getvalue('wifissid0'), wifipass: getvalue('wifipass0')},
        {wifissid: getvalue('wifissid1'), wifipass: getvalue('wifipass1')},
        {wifissid: getvalue('wifissid2'), wifipass: getvalue('wifipass2')},
    ];
    console.log('sending form data: ', data);
    postData('/wifiSet', data)
    .then(data => {
        console.log('wifiSet response: ', data);
        showapsettings(data);
        settext("submitresult", "Done");
        setTimeout(function () { settext("submitresult", ""); }, 5000);
    });
  }

  window.onload = function () {

    getapsettings();

    document.getElementById("submit").onclick = submitform;

  } // .onload
  </script>
</head>
<body>
<h1>Alarm Clock Access Point</h1>

<p>Enter name (SSID) and password (key) for up to three WiFi networks.
<p>Clear an SSID field to delete it.
<p>When you click on 'Save' with at least one SSID entered, the plotter will revert to, or stay in, 'station' mode and attempt to connect to the network.

<form action="" method="get">

    <fieldset>
      <legend>WiFi Network 0</legend>
      <p><label for="wifissidi0">SSID:</label>
      <input type="text" name=wifissid0 id="wifissid0">
      <p><label for="wifipass0">Password:</label>
      <input type="password" name=wifipass0 id="wifipass0">
    </fieldset>

    <fieldset>
      <legend>WiFi Network 1</legend>
      <p><label for="wifissid1">SSID:</label>
      <input type="text" name=wifissid1 id="wifissid1">
      <p><label for="wifipass1">Password:</label>
      <input type="password" name=wifipass1 id="wifipass1">
    </fieldset>

    <fieldset>
      <legend>WiFi Network 2</legend>
      <p><label for="wifissid2">SSID:</label>
      <input type="text" name=wifissid2 id="wifissid2">
      <p><label for="wifipass2">Password:</label>
      <input type="password" name=wifipass2 id="wifipass2">
    </fieldset>

    <p><button type="button" id=submit>Save</button>&nbsp; <span id=save></span>

  </form>

  <fieldset>
    <legend>Other Pages</legend>
    <p><a href="/"><button type=button>Main Page</button></a>
    <p><a href="/settings"><button type=button>Settings Page</button></a>
  </fieldset>

</body>
</html>
)=====";
