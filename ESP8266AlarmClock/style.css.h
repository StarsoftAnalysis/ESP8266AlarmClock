const char stylecss[] PROGMEM = R"=====(

th:nth-child(1) { text-align: left; }
td:nth-child(2) { text-align: center; }

label[for=volume] { 
    font-weight: bold; 
    margin-top: 1rem;
}

input[type=range] { vertical-align: bottom; margin-left: 2em; }

h1 { background-color: black; color: white; }
h2 { margin-bottom: 0; }
h4 { margin-bottom: 0; }

#currentTime { font-weight: bold; font-size: 200%; }

p { margin-top: 0.1rem; margin-bottom: 0.1rem; }

#setAlarm { margin-top: 1rem; font-size: 140%; }

footer { 
    margin-top: 1rem; 
}
.footeritem { 
    margin-right: 1rem; 
    display: inline-block;
}

)=====";
