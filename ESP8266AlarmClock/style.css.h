const char stylecss[] PROGMEM = R"=====(

th:nth-child(1) { text-align: left; }
td:nth-child(2) { text-align: center; }

label[for=volume] { 
    font-weight: bold; 
    margin-top: 1rem;
}

input[type=range] { vertical-align: bottom; margin-left: 2em; }

h1 { 
    background-color: black; color: white; 
    padding: 0.2rem;
    margin-bottom: 0;
}
h1 svg { 
    vertical-align: baseline;
    padding: 0.2rem 0 0 0.2rem;
}
h2 { margin-bottom: 0; }
h3 { margin-bottom: 0; }
h4 { margin-bottom: 0; }

#currentTime { font-weight: bold; font-size: 200%; }

p { margin-top: 0.1rem; margin-bottom: 0.1rem; }

#setAlarm, #saveSettings { margin-top: 1rem; font-size: 140%; }

footer { 
    margin-top: 1rem; 
}
.footeritem { 
    margin-right: 1rem; 
    display: inline-block;
}

)=====";
