#define MYST2HTML_VERSION V1.2

#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

void setupDashboard() {
  // Main page route
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    String inputData = generateHTMLPage("/home");
  printSerialln(inputData,10);
    request->send(200, "text/html", inputData);
  });

  // Route to refresh input indicators
  server.on("/refreshInputs_dataHTML", HTTP_GET, [](AsyncWebServerRequest * request) {
    String inputData = generateInputIndicatorsHTML(NUM_DIGITAL_INPUTSA);
    request->send(200, "text/html", inputData);
  });

  server.on("/getMatrixData", HTTP_GET, [](AsyncWebServerRequest * request) {
    int indicators = (NUM_FLED_ADDLEDS / NUM_FLED_CHANNELS);
    String matrixHTML = generate_inputIndicatorHTML(NUM_FLED_CHANNELS, indicators);
    request->send(200, "text/html", matrixHTML);
  });

  // Route to refresh output data
  server.on("/refreshOutputsA_dataHTML", HTTP_GET, [](AsyncWebServerRequest * request) {
    String payload = generateOutputsA_Payload();
    request->send(200, "application/json", payload);
  });

  // Route to refresh output data
  server.on("/refreshOutputsB_dataHTML", HTTP_GET, [](AsyncWebServerRequest * request) {
    String payload = generateOutputsB_Payload();
    request->send(200, "application/json", payload);
  });

  server.on("/getAnalogInputs", HTTP_GET, [](AsyncWebServerRequest * request) {
    String inputs = "<div>";
    inputs += "<div class='input'>Analog 1: " + String(analogRead(34)) + "</div>";
    inputs += "</div>";
    request->send(200, "text/html", inputs);
  });

  server.on("/getLEDMatrix", HTTP_GET, [](AsyncWebServerRequest * request) {

    request->send(200, "text/html", "");
  });

  // Route to toggle output state
  server.on("/toggleOutputAState", HTTP_GET, [](AsyncWebServerRequest * request) {
    String outputNumberStr = request->getParam("output")->value();
    int outputNumber = outputNumberStr.toInt();

    printSerial("Toggle Output: ");
    printSerialln(String(outputNumber),0);

    toggleOutputStateGPIOA(outputNumber);
    request->send(200, "text/plain", "Output toggled");
  });

  // Route to toggle output state
  server.on("/toggleOutputBState", HTTP_GET, [](AsyncWebServerRequest * request) {
    String outputNumberStr = request->getParam("output")->value();
    int outputNumber = outputNumberStr.toInt();

    printSerial("Toggle Output: ");
    printSerialln(String(outputNumber),0);

    toggleOutputStateGPIOB(outputNumber);
    request->send(200, "text/plain", "Output toggled");
  });

  // Route to get output state
  server.on("/getOutputAState", HTTP_GET, [](AsyncWebServerRequest * request) {
    String outputNumberStr = request->getParam("output")->value();
    int outputNumber = outputNumberStr.toInt();

    if (outputNumber >= 0 && outputNumber < NUM_DIGITAL_OUTPUTSA) {
      bool state = getDigitalInputStateGPIO(outputPinsA[outputNumber]);
      request->send(200, "text/plain", state ? "HIGH" : "LOW");
    } else {
      request->send(400, "text/plain", "Invalid output number");
    }
  });

  // Route to get output state
  server.on("/getOutputBState", HTTP_GET, [](AsyncWebServerRequest * request) {
    String outputNumberStr = request->getParam("output")->value();
    int outputNumber = outputNumberStr.toInt();

    if (outputNumber >= 0 && outputNumber < NUM_DIGITAL_OUTPUTSB) {
      bool state = getDigitalInputStateGPIO(outputPinsB[outputNumber]);

      request->send(200, "text/plain", state ? "HIGH" : "LOW");
    } else {
      request->send(400, "text/plain", "Invalid output number");
    }
  });

  // Route to get input state
  server.on("/getInputState", HTTP_GET, [](AsyncWebServerRequest * request) {
    String inputNumberStr = request->getParam("input")->value();
    int inputNumber = inputNumberStr.toInt();

    if (inputNumber >= 0 && inputNumber < NUM_DIGITAL_INPUTSA) {
      int pin = inputDigitalPinsB[inputNumber];
      String state = getDigitalInputStateGPIO(pin) ? "HIGH" : "LOW";
      request->send(200, "text/plain", state);
    } else {
      request->send(400, "text/plain", "Invalid input number");
    }
  });

  // Admin page route
  server.on("/admin", HTTP_GET, [](AsyncWebServerRequest * request) {
    String inputData = generateHTMLPage("/admin");
    request->send(200, "text/html", inputData);
  });

  // Route to save WiFi configuration
  server.on("/saveWiFi", HTTP_POST, [](AsyncWebServerRequest * request) {
    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
      String newSSID = request->getParam("ssid", true)->value();
      String newPassword = request->getParam("password", true)->value();

      wifiSettings.ssid = newSSID;
      wifiSettings.password = newPassword;
      saveWiFiSettings();

      String confirmationPage = "<html><head><title>Confirmation</title></head><body>";
      confirmationPage += "<h1>Configuration Saved</h1>";
      confirmationPage += "<p>New SSID: " + wifiSettings.ssid + "</p>";
      confirmationPage += "<p>New Password: " + wifiSettings.password + "</p>";
      confirmationPage += "<a href='/admin'>Back to Admin Panel</a>";
      confirmationPage += "<p>Restarting...</p>";
      confirmationPage += "</body></html>";

      request->send(200, "text/html", confirmationPage);

      delay(1000);  // Allow page load before restarting
      ESP.restart();
    } else {
      request->send(400, "text/plain", "Missing parameters.");
    }
  });

  // Route to save RedNode settings
  server.on("/saveRedNode", HTTP_POST, [](AsyncWebServerRequest * request) {
    if (request->hasParam("nrTab", true) &&
        request->hasParam("nrGroup", true) &&
        request->hasParam("mqttPassword", true) &&
        request->hasParam("mqttBroker", true) &&
        request->hasParam("mqttUsername", true)) {

      nodeSettings.nrTab = request->getParam("nrTab", true)->value();
      nodeSettings.nrGroup = request->getParam("nrGroup", true)->value();
      mqttSettings.mqttUsername = request->getParam("mqttUsername", true)->value();
      mqttSettings.mqttBroker = request->getParam("mqttBroker", true)->value();
      mqttSettings.mqttPassword = request->getParam("mqttPassword", true)->value();

      saveGlobalSettings();
      saveMQTTSettings();

      request->send(200, "text/plain", "Global settings saved successfully.");
    } else {
      request->send(400, "text/plain", "Missing parameters.");
    }
  });

  // Edit config route
  server.on("/editConfig", HTTP_POST, [](AsyncWebServerRequest * request) {
    String action = request->getParam("action", true)->value();
    String inputData = generateHTMLPage(action);
    request->send(200, "text/html", inputData);
  });

  // Start the server
  server.begin();
  printSerialln("Server started", 0);
}




String generateHTMLPage(String scriptname) {

  styleHTML = "";
  onloadHTML = "";
  setIntervalHTML = "";
  bodyDivHTML = "";
  scriptHTML = "";
  navButtonHTML = "";

  String page = getHTML_headerHTML();
  page += getHead_headerHTML();
  page += getHead_metaHTML();
  page += getStyle_headerHTML();

  bool blackTrue = false;
  String newScriptHTML;
  if (scriptname == "/admin") {
    newScriptHTML = refreshAdmin_dataHTML();
  } else if (scriptname == "/config") {  // Fixed this line
    newScriptHTML = refreshConfig_dataHTML();
  } else if (scriptname == "/nodered") {  // Fixed this line
    newScriptHTML = refreshNodeRed_dataHTML();
  } else if (scriptname == "/wifi") {  // Fixed this line
    newScriptHTML = refreshWiFi_dataHTML();
  } else if (scriptname == "/home") {
    blackTrue = true;

    if (NUM_ANALOG_INPUTPAIRS >= 1) {
       newScriptHTML += refreshDialsLEDs_dataHTML(); // example. lost > 3 dials > addressable led
    } 
    if (NUM_DIGITAL_INPUTSA >= 1) {
        newScriptHTML += refreshInputs_dataHTML(); // Refresh inputs
        newScriptHTML += updateInputIndicatorsFunctionality();
    }
    if (NUM_DIGITAL_INPUTSB >= 1) {
        newScriptHTML += refreshInputs_dataHTML(); // Refresh inputs
        newScriptHTML += updateInputIndicatorsFunctionality();
    }
    if (NUM_DIGITAL_OUTPUTSA >= 1) {
        newScriptHTML += refreshOutputsA_dataHTML(); // Refresh outputs
    }
    if (NUM_DIGITAL_OUTPUTSB >= 1) {
        newScriptHTML += refreshOutputsB_dataHTML(); // Refresh outputs
    }
    if (NUM_FLED_CHANNELS >= 1) {
      newScriptHTML += refreshOutputsMatrixSection_dataHTML();
    }
  }
  
  
  navButtonHTML = refreshNavigationButtons_dataHTML(scriptname);

  page += styleHTML;
  page += getStyle_body_backgroundBlack(blackTrue);
  page += getStyle_footerHTML();
  page += getScript_headerHTML();
  page += scriptHTML;
  page += newScriptHTML;
  page += onloadHTML;
  page += setIntervalHTML;
  page += getScript_footerHTML();
  page += getHead_footerHTML();
  page += getBody_headerHTML();
  page += bodyDivHTML;
  page += navButtonHTML;

  page += getBody_footerHTML();
  page += getHTML_footerHTML();
  return page;
}

//// ANALOG INPUTS

String generateAnalogInputDisplayHTML() {

  for (int i = 0; i < NUM_ANALOG_INPUTPAIRS; i++) {
    
  }
  return "";
}

String generate_inputIndicatorHTML(int channels, int indicators) {
  String indicatorsHTML = "<div id='indicatorMatrixGroup'>";

  for (int c = 0; c < channels; c++) {
    for (int i = 0; i < indicators; i++) {
      indicatorsHTML += "<div id='indicatorDot" + String(c*i) + "' class='indicator red'></div>";
    }
  }
  indicatorsHTML += "</div>";
  return indicatorsHTML;
}

String generateInputIndicatorsHTML(int indicators) {
  String indicatorsHTML = "<div id='inputs'>";
  for (int i = 0; i < indicators; i++) {
    
    indicatorsHTML += "<div id='inputIndicator" + String(i) + "' class='indicator red'></div>";
  }
  indicatorsHTML += "</div>";
  return indicatorsHTML;
}

String generateOutputsA_Payload() {
 DynamicJsonDocument doc(MQTT_MAX_PACKET_SIZE); 
  doc["puzzleName"] = PUZZLE_NAME;    // Example
  doc["mac"] = WiFi.macAddress();     // MAC address of the ESP32

  String outputStatus = "[";
  for (int i = 0; i < NUM_DIGITAL_OUTPUTSA; i++) {
    outputStatus += (digitalRead(outputPinsA[i]) != outputPins_initStateA[i]) ? "'green'" : "'red'";
    if (i < NUM_DIGITAL_OUTPUTSA - 1) outputStatus += ", ";  // Formatting between items
  }
  outputStatus += "]";
  doc["outputs"] = outputStatus;

  String payload;
  serializeJson(doc, payload);       // Serialize JSON to string
  return payload;
}

String generateOutputsB_Payload() {
  
 DynamicJsonDocument doc(MQTT_MAX_PACKET_SIZE); 
  doc["puzzleName"] = PUZZLE_NAME;    // Example
  doc["mac"] = WiFi.macAddress();     // MAC address of the ESP32

  String outputStatus = "[";
  for (int i = 0; i < NUM_DIGITAL_OUTPUTSB; i++) {
    outputStatus += (digitalRead(outputPinsB[i]) != outputPins_initStateB[i]) ? "'green'" : "'red'";
    if (i < NUM_DIGITAL_OUTPUTSB - 1) outputStatus += ", ";  // Formatting between items
  }
  outputStatus += "]";
  doc["outputs"] = outputStatus;

  

  String payload;
  serializeJson(doc, payload);       // Serialize JSON to string
  return payload;
}

String generateOutputButtonsAHTML() {
  
  String buttonsHTML = "<div id='buttonsSectionA'>";
  //buttonsHTML += "<h3>" + String(buttonsHTMLTitle) + "</h3>";

  int numColumns = NUM_DIGITAL_OUTPUTSA / NUM_DIGITAL_OUTPUT_ROWSA;  // Calculate number of columns based on rows

  // Ensure even distribution of buttons across rows
  for (int row = 0; row < NUM_DIGITAL_OUTPUT_ROWSA; row++) {
    
    buttonsHTML += "<div class='buttonRow'>"; // Start a new row

    for (int col = 0; col < numColumns; col++) {
      int outputIndex = row * numColumns + col;  // Calculate the index of the button
      if (outputIndex < NUM_DIGITAL_OUTPUTSA) { // Ensure we do not exceed the number of outputs

        String initialColor = (digitalRead(outputPinsA[outputIndex]) == outputPins_initStateA[outputIndex]) ? "red" : "green";
        
    buttonsHTML += "<button id='outputButtonA" + String(outputIndex) + "' class='" + initialColor +
                       "' onclick='toggleOutputA(" + String(outputIndex) + ")'>" + globalSettings.outputNames[outputIndex] + "</button>";
      }
    }
    
    buttonsHTML += "</div><br>"; // End the row and add line break for clarity
  }

  buttonsHTML += "</div>";
  return buttonsHTML;
}

String generateOutputButtonsBHTML() {
  String buttonsHTML = "<div id='buttonsSectionB'>";
  //buttonsHTML += "<h3>" + String(buttonsHTMLTitle) + "</h3>";

  int numColumns = NUM_DIGITAL_OUTPUTSB / NUM_DIGITAL_OUTPUT_ROWSB;  // Calculate number of columns based on rows

  // Ensure even distribution of buttons across rows
  for (int row = 0; row < NUM_DIGITAL_OUTPUT_ROWSB; row++) {
    
    buttonsHTML += "<div class='buttonRow'>"; // Start a new row

    for (int col = 0; col < numColumns; col++) {
      int outputIndex = row * numColumns + col;  // Calculate the index of the button
      if (outputIndex < NUM_DIGITAL_OUTPUTSB) { // Ensure we do not exceed the number of outputs

        String initialColor = (digitalRead(outputPinsB[outputIndex]) == outputPins_initStateB[outputIndex]) ? "red" : "green";
        
    buttonsHTML += "<button id='outputButtonB" + String(outputIndex) + "' class='" + initialColor +
                       "' onclick='toggleOutputB(" + String(outputIndex) + ")'>" + globalSettings.outputNames[outputIndex] + "</button>";
      }
    }
    
    buttonsHTML += "</div><br>"; // End the row and add line break for clarity
  }

  buttonsHTML += "</div>";
  return buttonsHTML;
}


String getHTML_headerHTML() {
  return "<html>";
}

String getHead_headerHTML() {
  return "<head>";
}

String getHead_metaHTML() {
   return "<meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
}

String getStyle_headerHTML() {
  return "<style>";
}

String getStyle_fullScreenDiv() {
  String scriptHTML = "";
  scriptHTML += "function goFullScreen() {";
  scriptHTML += "var elem = document.getElementById('fullScreenDiv');";
  scriptHTML += "if (elem.requestFullscreen) {";
  scriptHTML += "  elem.requestFullscreen();";      
  scriptHTML += "} else if (elem.mozRequestFullScreen) { // Firefox";
  scriptHTML += "  elem.mozRequestFullScreen();";
  scriptHTML += "} else if (elem.webkitRequestFullscreen) { // Chrome, Safari";
  scriptHTML += "  elem.webkitRequestFullscreen();";
  scriptHTML += "} else if (elem.msRequestFullscreen) { // IE/Edge";
  scriptHTML += "elem.msRequestFullscreen();";
  scriptHTML += "}}";

  scriptHTML += "function exitFullScreen() {";
  scriptHTML += "      if (document.exitFullscreen) {";
  scriptHTML += "          document.exitFullscreen();";
  scriptHTML += "      } else if (document.mozCancelFullScreen) { // Firefox";
  scriptHTML += "          document.mozCancelFullScreen();";
  scriptHTML += "      } else if (document.webkitExitFullscreen) { // Chrome, Safari";
  scriptHTML += "          document.webkitExitFullscreen();";
  scriptHTML += "      } else if (document.msExitFullscreen) { // IE/Edge";
  scriptHTML += "          document.msExitFullscreen();";
  scriptHTML += "      }}";
  
  String bodyDivHTML = "<div id=\"fullScreenDiv\">";
  bodyDivHTML += "<button onclick=\"goFullScreen()\">Go Full Screen</button>";
  bodyDivHTML += "</div>";
  
  return "#fullScreenDiv { width: 300px; height: 200px; background-color: lightblue; margin: 20px; }";
}

String getStyle_body_backgroundBlack(bool black) {
  if (!black) {
    return "body { background-color: white; color: black; text-align: center; font-family: Arial, sans-serif; margin: 20px; padding: 10px; }";
  } else {
    return "body { background-color: black; color: white; text-align: center; font-family: Arial, sans-serif; margin: 20px; padding: 10px; }";
  }
}

String getStyle_headings() {
  String style = "h1 { font-size: 36px; margin-bottom: 20px; }";
  style += "h2 { font-size: 28px; font-weight: bold; margin-bottom: 15px; }";
  style += "h3 { font-size: 20px; font-weight: bold; margin-bottom: 15px; }";
  return style;
}

String getStyle_buttons(String backgroundColor, String color) {
   String style = "button { background-color: " + backgroundColor + "; color: " + color + "; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; }";
   style += "button:hover { background-color: #45a049; }";
   return style;
}

String getStyle_footerHTML() {
  return "</style>";
}

String getScript_headerHTML() {
  return "<script>";
}

String getScript_footerHTML () {
  return "</script>";
}

String getHead_footerHTML() {
  return "</head>";
}

String getBody_headerHTML() {
  return "<body>";
}

String getBody_TitleHTML() {
  return "<h2>" + String(TECH_NAME) + " - " + String(MYSTTECH_MODEL) + "</h2>";
}

String getBody_ButtonFullscreenHTML() {
  return "<div id=\'fullScreenDiv\"><button onclick=\"goFullScreen()\">Go Full Screen</button></div>";
}

String getBody_ButtonExitScreenHTML() {
  return "<div id=\"fullScreenDiv\"><button onclick=\"exitFullScreen()\">Exit Full Screen</button></div>";
}

String getBody_footerHTML() {
  return "</body>";
}

String getHTML_footerHTML() {
  return "</html>";
}

String refreshInputs_dataHTML() {

  styleHTML += "#inputsSection, #outputsSection { margin: 20px auto; text-align: center; }";
  styleHTML += ".input, .output { font-size: 28px; margin: 10px; padding: 12px; border: 2px solid #fff; border-radius: 12px; transition: background-color 0.3s ease; }";
  styleHTML += ".input:hover, .output:hover { background-color: #333; }";
  
  onloadHTML += "refreshInputs_data();";
  onloadHTML += "updateInputIndicators();";
  setIntervalHTML += "setInterval(updateInputIndicators, 500);";

  bodyDivHTML += "<div id='inputsSection'>";
  bodyDivHTML += "<h3>" + String(bodyInputTitle) + "</h3>";
  bodyDivHTML += "<div id='inputs'>Waiting for inputs...</div>";
  bodyDivHTML += "</div>";

  String script = "function refreshInputs_data() {";
  script += "fetch('/refreshInputs_dataHTML') ";
  script += ".then(response => response.text()) ";
  script += ".then(data => { document.getElementById('inputs').innerHTML = data; });";
  script += "}";
  return script;
}

// Function to update indicator states (green/red based on input status)
String updateInputIndicatorsFunctionality() {

  styleHTML += ".indicator { display: inline-block; width: 40px; height: 40px; border-radius: 50%; margin: 5px; box-shadow: 0px 2px 4px rgba(0, 0, 0, 0.2); transition: background-color 0.3s ease; }";
  styleHTML += ".indicator.green { background-color: green; }";
  styleHTML += ".indicator.red { background-color: red; }";
  

  String script = "function updateInputIndicators() {";
  script += "  const inputs = document.querySelectorAll('[id^=\"inputIndicator\"]');";  // Select all input indicators
  script += "  inputs.forEach(indicator => {";
  script += "    const inputNumber = indicator.id.replace('inputIndicator', '');";  // Extract input number from indicator ID
  script += "    fetch('/getInputState?input=' + inputNumber)";
  script += "    .then(response => response.text())";
  script += "    .then(stateInput => {";
  script += "      indicator.className = (stateInput === 'LOW') ? 'indicator green' : 'indicator red';";  // Set indicator color based on state
  script += "    });";
  script += "  });";
  script += "}";
  return script;
}

// 6. refreshOutputs_dataHTML() Function
String refreshOutputsA_dataHTML() {

  styleHTML += "#buttonsSectionA { margin-top: 80px; }";
  styleHTML += "button { padding: 10px 20px; font-size: 15px; color: white; background-color: red; border: 2px solid #fff; border-radius: 8px; cursor: pointer; margin: 10px; transition: background-color 0.3s ease, transform 0.2s ease; }";
  styleHTML += "button:hover { transform: scale(1.1); }";
  styleHTML += ".red { background-color: red; }";
  styleHTML += ".green { background-color: green; }";

  onloadHTML += "refreshOutputsA_data();";
  
  setIntervalHTML += "setInterval(refreshOutputsA_data, 250);";
  
  bodyDivHTML += "<div id='outputsSectionA'>";
  bodyDivHTML += "<h2>" + String(bodyOutputATitle) + "</h2>";
  bodyDivHTML += "<div id='outputsA'></div>";
  bodyDivHTML += "</div>";

  bodyDivHTML += generateOutputButtonsAHTML(); // Add buttons section

  String script = "function refreshOutputsA_data() {";
  script += "fetch('/refreshOutputsA_dataHTML') ";
  script += ".then(response => response.html()) ";
  script += ".then(data => { document.getElementById('outputsA').innerHTML = data; })";
  script += ".then(() => { updateButtonColorA(outputNumber); });";
  script += "}";

  script += "function toggleOutputA(outputNumber) {";
  script += "fetch('/toggleOutputAState?output=' + outputNumber) ";
  script += ".then(() => { updateButtonColorA(outputNumber); })";
  script += ".catch(error => console.error('Error toggling output:', error));";
  script += "}";

  script += "function updateButtonColorA(outputNumber) {";
  script += "const button = document.getElementById('outputButtonA' + outputNumber);";
  script += "fetch('/getOutputAState?output=' + outputNumber)";
  script += ".then(response => response.text())";
  script += ".then(state => { button.className = (state === 'LOW') ? 'green' : 'red'; });";
  script += "}";

  printSerialln(script,0);

  return script;
}

// 6. refreshOutputs_dataHTML() Function
String refreshOutputsB_dataHTML() {

  styleHTML += "#buttonsSectionB { margin-top: 80px; }";
  styleHTML += "button { padding: 10px 20px; font-size: 15px; color: white; background-color: red; border: 2px solid #fff; border-radius: 8px; cursor: pointer; margin: 10px; transition: background-color 0.3s ease, transform 0.2s ease; }";
  styleHTML += "button:hover { transform: scale(1.1); }";
  styleHTML += ".red { background-color: red; }";
  styleHTML += ".green { background-color: green; }";

  onloadHTML += "refreshOutputsB_data();";

  setIntervalHTML += "setInterval(refreshOutputsB_data, 250);";
  
  bodyDivHTML += "<div id='outputsSectionB'>";
  //bodyDivHTML += "<h2>" + String(bodyOutputBTitle) + "</h2>";
  bodyDivHTML += "<div id='outputsB'></div>";
  bodyDivHTML += "</div>";

  bodyDivHTML += generateOutputButtonsBHTML(); // Add buttons section

  String script = "function refreshOutputsB_data() {";
  script += "fetch('/refreshOutputsB_dataHTML') ";
  script += ".then(response => response.html()) ";
  script += ".then(data => { document.getElementById('outputsB').innerHTML = data; })";
  script += ".then(() => { updateButtonColorB(outputNumber); });";
  script += "}";

  script += "function toggleOutputB(outputNumber) {";
  script += "fetch('/toggleOutputBState?output=' + outputNumber) ";
  script += ".then(() => { updateButtonColorB(outputNumber); })";
  script += ".catch(error => console.error('Error toggling output:', error));";
  script += "}";

  script += "function updateButtonColorB(outputNumber) {";
  script += "const button = document.getElementById('outputButtonB' + outputNumber);";
  script += "fetch('/getOutputBState?output=' + outputNumber)";
  script += ".then(response => response.text())";
  script += ".then(state => { button.className = (state === 'LOW') ? 'green' : 'red'; });";
  script += "}";

  printSerialln(script,0);

  return script;
}

String refreshDialsLEDs_dataHTML() {

  printSerial(".refresh dials html called");
  
  styleHTML += "#led-matrix-container { padding: 15px; border: 2px solid #888; border-radius: 8px; background-color: #f9f9f9; box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.1); max-width: 400px; margin: auto; }";
  styleHTML += "#led-matrix-container h3 { text-align: center; color: #333; font-family: Arial, sans-serif; }";
  styleHTML += "#led-matrix-container p { margin: 5px 0; color: #555; font-size: 14px; font-family: Arial, sans-serif; }";

  String script = "function refreshInputs() {";
  script += "  fetch('/getAnalogInputs') ";
  script += "    .then(response => response.text()) ";
  script += "    .then(data => { document.getElementById('inputs').innerHTML = data; });";
  script += "}";
  script += "function refreshLEDMatrix() {";
  script += "  fetch('/getLEDMatrix') ";
  script += "    .then(response => response.json()) ";
  script += "    .then(payload => { renderLEDMatrix(payload); });";
  script += "}";
  script += "function renderLEDMatrix(payload) {";
  script += "  document.getElementById('puzzle-name').textContent = payload.puzzleName || 'Unnamed Puzzle';";
  script += "  document.getElementById('group').textContent = payload.group || 'Unknown Group';";
  script += "  document.getElementById('tab').textContent = payload.tab || 'Unknown Tab';";
  script += "  document.getElementById('ip-address').textContent = payload.ipAddress || 'Unknown IP';";
  script += "  const ledMatrix = document.getElementById('led-matrix');";
  script += "  ledMatrix.innerHTML = '';";
  script += "  for (let c = 0; c < 3; c++) {";
  script += "    let ledStrip = payload[`led_strip_${c+1}`];";
  script += "    if (Array.isArray(ledStrip)) {";
  script += "      const rowDiv = document.createElement('div');";
  script += "      for (let i = 0; i < ledStrip.length; i++) {";
  script += "        const indicator = document.createElement('span');";
  script += "        indicator.className = `indicator ${ledStrip[i] === 'on' ? 'green' : 'red'}`;";
  script += "        rowDiv.appendChild(indicator);";
  script += "      }";
  script += "      ledMatrix.appendChild(rowDiv);";
  script += "    }";
  script += "  }";
  script += "}";
  
  onloadHTML += "  refreshInputs();";
  onloadHTML += "  refreshLEDMatrix();";
  
  setIntervalHTML += "  setInterval(refreshInputs, 1000);";
  setIntervalHTML += "  setInterval(refreshLEDMatrix, 5000);";
  
  bodyDivHTML += "<h1>ESP32 Dashboard</h1>";
  bodyDivHTML += "<div id='inputsSection'>";
  bodyDivHTML += "<h2>Analog Inputs</h2>";
  bodyDivHTML += "<div id='inputs'></div>";
  bodyDivHTML += "</div>";
  bodyDivHTML += "<div id='led-matrix-container'>";
  bodyDivHTML += "<h3 id='puzzle-name'>Loading...</h3>";
  bodyDivHTML += "<p><strong>Group:</strong> <span id='group'>N/A</span></p>";
  bodyDivHTML += "<p><strong>Tab:</strong> <span id='tab'>N/A</span></p>";
  bodyDivHTML += "<p><strong>IP Address:</strong> <span id='ip-address'>N/A</span></p>";
  bodyDivHTML += "<div id='led-matrix'></div>";
  bodyDivHTML += "</div>";

  return script;
}


String refreshAdmin_dataHTML() {

    styleHTML += getStyle_body_backgroundBlack(false);
    styleHTML += getStyle_headings();
    styleHTML += getStyle_buttons("#4CAF50", "white");

    styleHTML += "form { display: inline-block; text-align: left; background: white; border: 1px solid #ccc; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }";
    styleHTML += "input[type='text'] { width: 100%; padding: 8px; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; }";
   
    bodyDivHTML += "<h1>Admin Panel</h1>";

    // Module Information
    bodyDivHTML += "<form action='/editConfig' method='POST'>";
    bodyDivHTML += "<h3>Module Information</h3>";
    bodyDivHTML += "<p><strong>Module:</strong> " + String(MYSTTECH_MODEL) + "</p>";
    bodyDivHTML += "<p><strong>Puzzle Name:</strong> " + String(PUZZLE_NAME) + "</p>";
    bodyDivHTML += "<p><strong>Designer:</strong> " + String(DESIGNER_NAME) + "</p>";
    bodyDivHTML += "<p><strong>Technician:</strong> " + String(TECH_NAME) + "</p>";
    bodyDivHTML += "<p><strong>Model:</strong> " + String(MYSTTECH_MODEL) + "</p>";
    bodyDivHTML += "<p><strong>Version:</strong> " + nodeSettings.storedVersion + "</p>";
    bodyDivHTML += "<p><strong>IP Address:</strong> " + wifiSettings.ipaddress + "</p>";
    bodyDivHTML += "<p><strong>Version:</strong> " + nodeSettings.storedVersion + "</p>";
    bodyDivHTML += "<button type='submit' name='action' value='CONFIG'>Edit GPIO Config</button>";
    bodyDivHTML += "<h3>Node-Red Configurations</h3>";
    bodyDivHTML += "<p><strong>MQTT Server:</strong> " + mqttSettings.mqttBroker + "</p>";
    bodyDivHTML += "<p><strong>MQTT Username:</strong> " + mqttSettings.mqttUsername + "</p>";
    bodyDivHTML += "<button type='submit' name='action' value='NODERED'>Edit Node-Red Config</button>";
    bodyDivHTML += "<h3>Network Settings</h3>";
    bodyDivHTML += "<p><strong>WiFi SSID:</strong> " + wifiSettings.ssid + "</p>";
    bodyDivHTML += "<p><strong>WiFi Password:</strong> " + wifiSettings.password + "</p>";
    bodyDivHTML += "<button type='submit' name='action' value='WIFI'>Edit WiFi Config</button>";
    bodyDivHTML += "</form>";
    
    String script = "";
    return script;
}

String refreshConfig_dataHTML() {

    styleHTML += getStyle_body_backgroundBlack(false);
    styleHTML += getStyle_headings();
    styleHTML += getStyle_buttons("#4CAF50", "white");

    styleHTML += "form { display: inline-block; text-align: left; background: white; border: 1px solid #ccc; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }";
    styleHTML += "input[type='text'] { width: 100%; padding: 8px; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; }";
   
    bodyDivHTML += "<h1>GPIO Configuration Panel</h1>";
   
  return "";
}

String refreshNodeRed_dataHTML() {

    styleHTML += getStyle_body_backgroundBlack(false);
    styleHTML += getStyle_headings();
    styleHTML += getStyle_buttons("#4CAF50", "white");

    styleHTML += "form { display: inline-block; text-align: left; background: white; border: 1px solid #ccc; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }";
    styleHTML += "input[type='text'] { width: 100%; padding: 8px; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; }";

    bodyDivHTML += "<h1>Node-Red Configuration Panel</h1>";
    bodyDivHTML += "<form action='/saveRedNode' method='POST'>";
    bodyDivHTML += "<h3>MQTT Network Details</h3>";
    bodyDivHTML += "<label for='nrTab'>UI Dashboard Tab:</label>";
    bodyDivHTML += "<input type='text' id='nrTab' name='nrTab' value='" + String(nodeSettings.nrTab) + "'><br><br>";
    bodyDivHTML += "<label for='nrGroup'>UI Dashboard Group:</label>";
    bodyDivHTML += "<input type='text' id='nrGroup' name='nrGroup' value='" + String(nodeSettings.nrGroup) + "'><br><br>";
    bodyDivHTML += "<label for='mqttBroker'>MQTT Server:</label>";
    bodyDivHTML += "<input type='text' id='mqttBroker' name='mqttBroker' value='" + String(mqttSettings.mqttBroker) + "'><br><br>";
    bodyDivHTML += "<label for='mqttUsername'>MQTT Username:</label>";
    bodyDivHTML += "<input type='text' id='mqttUsername' name='mqttUsername' value='" + String(mqttSettings.mqttUsername) + "'><br><br>";
    bodyDivHTML += "<label for='mqttPassword'>MQTT Password:</label>";
    bodyDivHTML += "<input type='text' id='mqttPassword' name='mqttPassword' value='" + String(mqttSettings.mqttPassword) + "'><br><br>";
    bodyDivHTML += "<button type='submit'>Save Red Node Changes</button>";
    bodyDivHTML += "</form>";
    return "";
}

String refreshWiFi_dataHTML() {

    styleHTML += getStyle_body_backgroundBlack(false);
    styleHTML += getStyle_headings();
    styleHTML += getStyle_buttons("#4CAF50", "white");

    styleHTML += "form { display: inline-block; text-align: left; background: white; border: 1px solid #ccc; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }";
    styleHTML += "input[type='text'] { width: 100%; padding: 8px; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; }";
    
    bodyDivHTML += "<h1>WiFi Configuration Panel</h1>";
    bodyDivHTML += "<form action='/saveWiFi' method='POST'>";
    bodyDivHTML += "<h3>Network Settings</h3>";
    bodyDivHTML += "<p><strong>WiFi SSID:</strong> " + wifiSettings.ssid + "</p>";
    bodyDivHTML += "<p><strong>WiFi Password:</strong> " + wifiSettings.password + "</p>";
    bodyDivHTML += "<label for='ssid'>SSID:</label>";
    bodyDivHTML += "<input type='text' id='ssid' name='ssid' value='" + wifiSettings.ssid + "'><br><br>";
    bodyDivHTML += "<label for='password'>Password:</label>";
    bodyDivHTML += "<input type='text' id='password' name='password' value='" + wifiSettings.password + "'><br><br>";
    bodyDivHTML += "<button type='submit'>Save WiFi Changes</button>";
    bodyDivHTML += "</form>";
    
    return "";
}

String refreshOutputsMatrixSection_dataHTML() {
    // Style for the outputs section and indicators
    String styleHTML = "";
    styleHTML += "#outputsMatrixSection { margin: 30px auto; text-align: center; }";
    styleHTML += ".output { font-size: 18px; padding: 10px; margin: 10px; border: 1px solid #ddd; border-radius: 5px; display: inline-block; }";
    styleHTML += ".indicator { display: inline-block; width: 20px; height: 20px; border-radius: 50%; margin-left: 10px; transition: background-color 0.3s ease; }";
    styleHTML += ".indicator.green { background-color: green; }";
    styleHTML += ".indicator.red { background-color: red; }";

    // Script for fetching and updating the outputs section
    String script = "function refreshIndicators() {";
    script += "  fetch('/getMatrixData')";  // API endpoint to fetch outputs data
    script += "    .then(response => response.json())";
    script += "    .then(data => {";
    script += "      const outputsMatrixSection = document.getElementById('indicatorMatrixGroup');";
    script += "      outputsMatrixSection.innerHTML = '';";  // Clear the current content
    script += "      data.forEach(output => {";
    script += "        const div = document.createElement('div');";
    script += "        div.className = 'output';";
    script += "        div.innerHTML = `Output ${output.id}`;";
    script += "        const indicator = document.createElement('span');";
    script += "        indicator.className = `indicator ${output.state === 'on' ? 'green' : 'red'}`;";
    script += "        div.appendChild(indicator);";
    script += "        outputsSection.appendChild(div);";
    script += "      });";
    script += "    })";
    script += "    .catch(error => console.error('Error fetching outputs:', error));";
    script += "}";
    
    onloadHTML += "  refreshIndicators();";
    setIntervalHTML += "  setInterval(refreshOutputs, 2000);";

    // HTML structure for the outputs section
    String bodyDivHTML = "";
    bodyDivHTML += "<div id='outputsMatrixSection'>";
    bodyDivHTML += "<h2>Outputs Status</h2>";
    bodyDivHTML += "<div id='indicatorMatrixGroup'>Loading...</div>";
    bodyDivHTML += "</div>";

    // Combine all parts and return
    return "<style>" + styleHTML + "</style>" + bodyDivHTML + script;
}


String refreshNavigationButtons_dataHTML(String form) {
  
  // Add navigation styles to styleHTML
  styleHTML += "#navMenu { margin-top: 20px; text-align: center; }";
  styleHTML += "#navMenu a {";
  styleHTML += "  background-color: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; }";
  styleHTML += "  transition: background-color 0.3s ease, transform 0.2s ease;";
  styleHTML += "  text-decoration: none;"; // Remove underline
  styleHTML += "}";
  styleHTML += "#navMenu a:hover {";
  styleHTML += "  transform: scale(1.1);"; // Hover effect: Increase size
  styleHTML += "  hover { background-color: #45a049; }";
  styleHTML += "}";
  
  // Add HTML for navigation buttons
  String navHTML = "<div id='navMenu'>";
    
  if (form != "/home") {
    navHTML += globalHyperlinks.homeButtonHTML;
  } else { navHTML += globalHyperlinks.adminButtonHTML; }

  navHTML += "</div>";

  return navHTML;
}
