#define MQTT_MAX_PACKET_SIZE 225  // Adjust size as needed
#define MQTT_PORT 1883
#define MQTT_VERSION V1.2

#define MAX_PENDING_MESSAGES 250  // Maximum number of sent messages we can store
#define MAX_NODES 10  // Maximum number of nodes to store

NodeSettings nodeArray[MAX_NODES];  // Array to store nodes
int nodeCount = 0;  // To keep track of the number of nodes added

int unvalidatedMessages = 0;

unsigned long lastReconnectAttempt = 0;
unsigned long startupDelay = 0;

// Array to store messages sent
String sentMessages[MAX_PENDING_MESSAGES];
unsigned long sentTimes[MAX_PENDING_MESSAGES];  // Array to store the time messages were sent
bool messageConfirmed[MAX_PENDING_MESSAGES];     // Flags for confirming message reception

void connect_designatedMQTT() {

  if (connected) {

    startupDelay = 15000; // wait 10 seconds before checking validation loop

    client.setServer(mqttSettings.mqttBroker.c_str(), MQTT_PORT);
    client.setCallback(mqttCallback);  // Set MQTT callback function

    const char* mac = WiFi.macAddress().c_str();
    printSerialln("MQTT Callback should be set", 0);

    unsigned long startAttemptTime = millis();
    const unsigned long retryInterval = 5000;  // Retry interval (in milliseconds)
    const unsigned long timeout = 10000;       // Timeout after 10 seconds

    String MQTT_CLIENT_ID = WiFi.macAddress();

    while (!client.connected() && millis() - startAttemptTime < timeout) {
      printSerial("Connecting to MQTT...");
      if (client.connect(MQTT_CLIENT_ID.c_str(), mqttSettings.mqttUsername.c_str(), mqttSettings.mqttPassword.c_str())) {
        printSerialln("Connected to MQTT Broker (Server): " + mqttSettings.mqttBroker, 0);

        if (client.subscribe(mqttSettings.mqttTopic)) {
          printSerialln("Successfully subscribed to topic: " + String(mqttSettings.mqttTopic), 0);
          isSubscribed = true;
        } else {
          printSerialln("Failed to subscribe to topic.", 0);
          isSubscribed = false;
        }
      } else {
        printSerial("Failed (state=");
        printSerial(String(client.state()));
        isSubscribed = false;
        printSerialln("). Retrying in 5 seconds...", 0);
        startupDelay = 10000; // reset delay
        delay(retryInterval);
      }
    }

    if (!client.connected()) {
      printSerialln("Expected MQTT connection failed.", 0);
      //connect_bupMQTTBroker();
    }
  }
}


void connect_bupMQTTBroker() {

  if (connected) {

    /// need to update
    client.setServer(mqttSettings.mqttBroker.c_str(), MQTT_PORT);
    client.setCallback(mqttCallback);  // Set MQTT callback function
    printSerialln("MQTT Callback should be set", 0);

    unsigned long startAttemptTime = millis();
    const unsigned long retryInterval = 5000;  // Retry interval (in milliseconds)
    const unsigned long timeout = 30000;       // Timeout after 30 seconds

    String MQTT_CLIENT_ID = WiFi.macAddress();

    while (!client.connected() && millis() - startAttemptTime < timeout) {
      printSerial("Connecting to MQTT...");
      if (client.connect(MQTT_CLIENT_ID.c_str())) {
        printSerialln("Connected to BROKER", 0);

        if (client.subscribe(mqttSettings.mqttTopic)) {
          printSerialln("Successfully subscribed to topic: " + String(mqttSettings.mqttTopic), 0);
          isSubscribed = true;
        } else {
          printSerialln("Failed to subscribe to topic.", 0);
          isSubscribed = false;
        }

      } else {
        printSerial("Failed (state=");
        printSerial(String(client.state()));
        printSerialln("). Retrying in 5 seconds...", 0);
        startupDelay = 10000; // reset delay
        delay(retryInterval);
      }
    }

    if (!client.connected()) {
      printSerialln("MQTT connection failed. Check credentials and server availability.", 0);
    }
  }
}

void clientMQTTConnected() {

  if (connected) {

    client.loop();  // Ensure MQTT is being handled
    startupDelay = 0; // startup over
    checkMessageTimeout();  // Check for message timeouts
    handleMQTTConnection();
  }
}

void checkMessageTimeout() {
  unsigned long currentMillisMT = millis();

  // Check for unconfirmed messages that have timed out
  for (int i = 0; i < MAX_PENDING_MESSAGES; i++) {
    if (sentMessages[i] != "") {  // Only check if the message is set
      if (!messageConfirmed[i]) { // If message is not confirmed
        if (currentMillisMT - sentTimes[i] >= 10000 + startupDelay) {  // 5 seconds timeout
          printSerialln("Message timeout: " + sentMessages[i], 0);
          isSubscribed = false;
          unsubscribeFromTopic();
          messageConfirmed[i] = false;  // Reset the message as not confirmed
          unvalidatedMessages++;       // Increment unvalidated message count
          sentMessages[i] = "";        // Clear the sent message
        }
      }
    }
  }
}

boolean reconnect() {
  if (client.connect(MQTT_CLIENT_ID.c_str())) {
    printSerialln("Connected to MQTT broker." + String(mqttSettings.mqttBroker), 0);
    if (client.subscribe(mqttSettings.mqttTopic)) {
      printSerialln("Reconnect... successfully subscribed to topic: " + String(mqttSettings.mqttTopic), 0);
      isSubscribed = true;
    }
    return true;
  }
  return false;
}

void unsubscribeFromTopic() {

  printSerialln("Unsubscribing from topic...", 0);

  if (client.unsubscribe(mqttSettings.mqttTopic)) {
    printSerialln("Unsubscribe request sent.", 0);
  } else {
    printSerialln("Failed to send unsubscribe request.", 0);
  }
}

void handleMQTTConnection() {

  // Check if the client is connected, if not attempt to reconnect
  if (!client.connected()) {
    if (millis() - lastReconnectAttempt > 5000) {  // Retry every 5 seconds
      printSerialln("Attempting to reconnect to MQTT broker...", 0);
      if (reconnect()) {
        lastReconnectAttempt = millis();  // Reset reconnect attempt time
      }
    }
  } else {
    // The client is connected, handle the subscription status
    if (!isSubscribed) {
      printSerialln("Client is connected but not subscribed. Resubscribing...", 0);
      if (client.subscribe(mqttSettings.mqttTopic)) {
        printSerialln("Successfully subscribed to topic: " + String(mqttSettings.mqttTopic), 0);
        isSubscribed = true;
      } else {
        printSerialln("Failed to subscribe to topic.", 0);
      }
    }
  }
}

void publishDataMQTTPayload_Doc(String jsonPayload) {

  printSerial("Payload size: ");
  printSerial(String(jsonPayload.length()));
  printSerial(" MQTT Buffer Size: ");
  printSerial(String(MQTT_MAX_PACKET_SIZE));

  if (jsonPayload.length() > MQTT_MAX_PACKET_SIZE) {
    printSerialln(" => Error: Payload size exceeds MQTT buffer size.", 0);
    return;
  }

  // Store the message and timestamp
  for (int i = 0; i < MAX_PENDING_MESSAGES; i++) {
    if (messageConfirmed[i]) {
      sentMessages[i] = "";
    }
    if (sentMessages[i] == "") {
      sentMessages[i] = jsonPayload;
      sentTimes[i] = millis();
      messageConfirmed[i] = false;  // Initially, no message is confirmed
      break;
    }
  }

  // Send the payload via MQTT
  if (client.publish(mqttSettings.mqttTopic, jsonPayload.c_str())) {
    printSerial("Data sent to topic: " + String(mqttSettings.mqttTopic) + "; ");
    printSerialln(jsonPayload, 0);
  } else {
    printSerialln(" => Failed to send data.", 0);
  }
}


void sendMessageMQTTPayload(String message, String activity) {
  DynamicJsonDocument doc(MQTT_MAX_PACKET_SIZE);
  doc["message"] = (message + " ");
  doc["activity"] = activity;
  
  doc["mac"] = WiFi.macAddress();

  if (activity == "FirmwareStatus-->") {

    doc["activity"] = "Firmware Status Update";

    doc["type"] = nodeSettings.nrType;

    if (NUM_DIGITAL_OUTPUTSA > 0) {
      doc["outA"] = NUM_DIGITAL_OUTPUTSA;
      doc["outAR"] = NUM_DIGITAL_OUTPUT_ROWSA;
    }
    if (NUM_DIGITAL_OUTPUTSB > 0) {
      doc["outB"] = NUM_DIGITAL_OUTPUTSB;
      doc["outBR"] = NUM_DIGITAL_OUTPUT_ROWSB;
    }
    if (NUM_DIGITAL_INPUTSA > 0) {
      doc["inpA"] = NUM_DIGITAL_INPUTSA;
    }
    if (NUM_DIGITAL_INPUTSB > 0) {
      doc["inpB"] = NUM_DIGITAL_INPUTSB;
    }
    if (NUM_ANALOG_INPUTPAIRS > 0) {
      doc["anaI"] = NUM_ANALOG_INPUTPAIRS;
    }
    if (NUM_FLED_ADDLEDS > 0) {
      doc["leds"] = NUM_FLED_ADDLEDS;
      doc["channels"] = NUM_FLED_CHANNELS;
    }
  }

  String jsonPayload;
  serializeJson(doc, jsonPayload);
  publishDataMQTTPayload_Doc(jsonPayload);
  delay(50);
}

void send3D_ROTARY_PULSEMQTTData(int count1, int count2, int count3) {

  DynamicJsonDocument doc(MQTT_MAX_PACKET_SIZE);
  doc["activity"] = "UpdateUI";
  doc[" "] = "";

  doc["mac"] = WiFi.macAddress();
  doc["puzzleName"] = PUZZLE_NAME;
  doc[" "] = "";

  doc["ls1"] = count1;
  doc["ls2"] = count2;
  doc["ls3"] = count3;
  doc["s1"] = solutionWin[0];
  doc["s2"] = solutionWin[1];
  doc["s3"] = solutionWin[2];
  doc["version"] = nodeSettings.storedVersion;

  doc["sfound"] = solutionFound;

  String jsonPayload;
  serializeJson(doc, jsonPayload);
  publishDataMQTTPayload_Doc(jsonPayload);
}

void sendADDMEMessageMQTT() {

  DynamicJsonDocument doc(MQTT_MAX_PACKET_SIZE);

  doc["activity"] = "ADD ME1";
  doc[" "] = "";
  doc["mac"] = WiFi.macAddress();
  doc[" "] = "";
  doc["puzzleName"] = PUZZLE_NAME;
  doc["designer"] = DESIGNER_NAME;
  doc["tech"] = TECH_NAME;
  doc["tab"] = nodeSettings.nrTab;

  String jsonPayload;
  serializeJson(doc, jsonPayload);
  publishDataMQTTPayload_Doc(jsonPayload);
  delay(50);

  DynamicJsonDocument doc1(MQTT_MAX_PACKET_SIZE);

  doc1["activity"] = "ADD ME2";
  doc1[" "] = "";

  doc1["mac"] = wifiSettings.mac;
  doc1[" "] = "";

  doc1["mDNS"] = nodeSettings.mDNS;
  doc1["storedVersion"] = nodeSettings.storedVersion;
  doc1["ipaddress"] = wifiSettings.ipaddress;
  doc1["ssid"] = wifiSettings.ssid;
  doc1["password"] = wifiSettings.password;

  serializeJson(doc1, jsonPayload);
  publishDataMQTTPayload_Doc(jsonPayload);

}

void mqttCallback(char* topic, byte* payload, unsigned int length) {

  printSerial("Free heap: ");
  printSerial(String(ESP.getFreeHeap()));

  String receivedMessage = String((char*)payload, length);

  // Check if the received message matches any sent message
  for (int i = 0; i < MAX_PENDING_MESSAGES; i++) {
    if (sentMessages[i] == receivedMessage) {
      messageConfirmed[i] = true;  // Mark message as confirmed

      printSerialln(" | Message server verified: " + receivedMessage, 0);
      sentMessages[i] = "";
      return;  // Exit once confirmed
    }
  }

  DynamicJsonDocument doc(MQTT_MAX_PACKET_SIZE);
  DeserializationError error = deserializeJson(doc, receivedMessage);
  if (error) {
    printSerial("JSON parse error: ");
    printSerialln(error.c_str(), 0);
    return;
  }

  // Further processing if activity is found
  if (doc.containsKey("activity")) {
    String activity = doc["activity"].as<String>();
    printSerial("Activity: ");
    printSerialln(activity, 0);

    // Handle different activities
    if (activity == "getFirmwareUpdate") {
      sendMessageMQTTPayload("Firmware Status Request Confirmed from: " + String(PUZZLE_NAME), "FirmwareStatus-->");
    } else if (activity == (String(NR_GROUP) + " Button Pressed")) {

      executeFUNCBatchButton1_PIN(EXEC_BATCH1_PIN);


    } else if (activity == (String(NR_GROUP) + " Button 2 Pressed")) {
      executeFUNCBatchButton2();

    } else if (activity == (String(NR_GROUP) + " Button 3 Pressed")) {

      executeFUNCBatchButton3_PIN(EXEC_BATCH2_PIN);

    } else if (activity == "ADD ME") {
      if (nodeCount < MAX_NODES) {

        // Extract node settings directly from the root of the JSON payload
        nodeSettings.puzzleName = doc["puzzleName"].as<String>().length() > 0 ? doc["puzzleName"].as<String>() : nodeSettings.puzzleName;
        nodeSettings.designer = doc["designer"].as<String>().length() > 0 ? doc["designer"].as<String>() : nodeSettings.designer;
        nodeSettings.tech = doc["tech"].as<String>().length() > 0 ? doc["tech"].as<String>() : nodeSettings.tech;
        nodeSettings.mDNS = doc["mDNS"].as<String>().length() > 0 ? doc["mDNS"].as<String>() : nodeSettings.mDNS;
        nodeSettings.nrTab = doc["nrTab"].as<String>().length() > 0 ? doc["nrTab"].as<String>() : nodeSettings.nrTab;
        nodeSettings.nrGroup = doc["nrGroup"].as<String>().length() > 0 ? doc["nrGroup"].as<String>() : nodeSettings.nrGroup;
        nodeSettings.nrType = doc["nrType"].as<String>().length() > 0 ? doc["nrType"].as<String>() : nodeSettings.nrType;
        nodeSettings.storedVersion = doc["storedVersion"].as<String>().length() > 0 ? doc["storedVersion"].as<String>() : nodeSettings.storedVersion;

        // Extract WiFi settings directly from the root of the JSON payload
        wifiSettings.mac = doc["mac"].as<String>().length() > 0 ? doc["mac"].as<String>() : wifiSettings.mac;
        wifiSettings.ipaddress = doc["ipAddress"].as<String>().length() > 0 ? doc["ipAddress"].as<String>() : wifiSettings.ipaddress;
        wifiSettings.ssid = doc["ssid"].as<String>().length() > 0 ? doc["ssid"].as<String>() : wifiSettings.ssid;
        wifiSettings.password = doc["password"].as<String>().length() > 0 ? doc["password"].as<String>() : wifiSettings.password;

        bool nodeFound = false;

        // Search if the node already exists based on the MAC address
        for (int i = 0; i < nodeCount; i++) {
          if (nodeArray[i].mac == wifiSettings.mac) {

            nodeFound = true;
            printSerialln("Node exists: " + wifiSettings.mac, 0);
            break;
          }
        }

        // If node does not exist, add a new one
        if (!nodeFound) {
          nodeArray[nodeCount].mac = wifiSettings.mac;
          nodeArray[nodeCount].puzzleName = nodeSettings.puzzleName;
          nodeArray[nodeCount].designer = nodeSettings.designer;
          nodeArray[nodeCount].tech = nodeSettings.tech;
          nodeArray[nodeCount].mDNS = nodeSettings.mDNS;
          nodeArray[nodeCount].nrTab = nodeSettings.nrTab;
          nodeArray[nodeCount].nrGroup = nodeSettings.nrGroup;
          nodeArray[nodeCount].nrType = nodeSettings.nrType;
          nodeArray[nodeCount].storedVersion = nodeSettings.storedVersion;

          // Store WiFi settings in the new node
          nodeArray[nodeCount].mac = wifiSettings.mac;
          nodeArray[nodeCount].ipaddress = wifiSettings.ipaddress;
          nodeArray[nodeCount].ssid = wifiSettings.ssid;
          nodeArray[nodeCount].password = wifiSettings.password;

          nodeCount++;  // Increment the node count

          printSerialln("Added new node: " + wifiSettings.mac, 1000);
        }
        printSerial("Node ##: ");
        printSerialln(String(nodeCount), 1000);
        saveNodeSettings();
        saveWiFiSettings();

        return;
      } else {
        printSerialln("Node array is full. Cannot add more nodes.", 0);
      }
    } else {
      printSerialln("Unexpected activity: " + activity, 0);
    }
  } else {
    printSerialln("Key 'activity' not found in message: " + receivedMessage, 0);
  }
}
