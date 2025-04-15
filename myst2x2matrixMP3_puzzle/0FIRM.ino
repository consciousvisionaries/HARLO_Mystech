#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Update.h>
#include <PubSubClient.h>
#include <ESPmDNS.h>
#include <string>
#include <Adafruit_PWMServoDriver.h>;

Adafruit_PWMServoDriver pwmArray[1] = { Adafruit_PWMServoDriver(0x40) };


#define FIRMWARE_VERSION "V1.x"

WiFiClient espClient;
PubSubClient client(espClient);

// Declare the StaticJsonDocument
DynamicJsonDocument doc(MQTT_MAX_PACKET_SIZE);

String comment = "";
unsigned long loggedMessages = 0;
String recordLog[SIZE_LOG] = {};
String tmp = "";

bool mp3PlayerInitialized = false;

bool isSubscribed = false;  // Flag to track subscription status

// Function to print a simple statement (appends to comment)
void printSerial(String statement) {

  comment = comment + statement;
}

// Function to print a formatted statement (like printf)
void printSerialf(const char* format, ...) {
  va_list args;
  va_start(args, format);
  Serial.vprintf(format, args);  // Print formatted output
  va_end(args);
}

// Function to log and print a statement with a delay
void printSerialln(String statement, int MSDELAY) {

  // Only add "[R] " at the start if it's not already present
  if (!comment.startsWith("[R]   ")) {
    comment = "[R]   " + comment + statement;  // Prepend "[R] " only if not already present
  } else {
    comment = comment + statement;  // Just append the statement if "[R]" is already there
  }
  if (loggedMessages <= 9) {
    comment = "  " + comment;
  } else if (loggedMessages <= 99) {
    comment = " " + comment;
  }
  comment = String(loggedMessages) + comment;

  if (SHOW_LOG) {
    Serial.println(comment);
  }

  if (RECORD_LOG) {

    if (loggedMessages < SIZE_LOG) {
      recordLog[loggedMessages++] = comment;  // Store the log entry
      //if (isSubscribed) { sendMessageMQTTPayload(comment,"LogFile-->"); }
    }
  }
  comment = "";
  delay(MSDELAY * LOG_DELAY);  // Delay for the specified milliseconds
}

void setup() {

  Serial.begin(115200);

  delay(1000);

  connectWiFi();

  printSerialln(".wifi call completed", 1000);

  if (!RESET_PREF) {

    loadAllSettings();

    printSerialln(".credentials call completed", 1000);
  } else {
    saveAllSettings();
    printSerialln(">>> Credential call skipped... pull from code constraints... initialize.", 1000);
  }

  checkForUpdates();
  printSerialln(".firmware update call completed", 1000);

  if (!SHOW_LOG && RECORD_LOG) {
    Serial.println(" => display serial log not available...");
  } else if (!RECORD_LOG) {
    Serial.println(" => serial log is not being tracked by developer. Contact +1 604-819-1324 for assistance and leave a voice message.");
  } else {
    Serial.println("");
  }

  connect_designatedMQTT();
  printSerialln(".mqtt call completed", 1000);

  setupGPIO();
  printSerialln(".gpio call completed", 1000);

  setupFASTLED_GPIO();
  printSerialln(".fled call completed", 1000);

  setupMP3Player();
  printSerialln(".mp3 call completed", 1000);

  setupI2C();
  printSerialln(".i2c call completed", 1000);

  setupDashboard();
  printSerialln(".dashboard call completed", 1000);

  mystTechBMP();
  printSerialln("All rights reserved. (c) 2025 Appsations, Inc. dba. MystTech Creations", 1000);
  printSerialln("   for more info email: paul@consciousvisionaries || dial 604-819-1324.", 0);
  printSerialln("                        https://consciousvisionaries.ca", 0);

  printSerialln("             Local DNS: " + String(DNSLOCAL), 0);
  printSerialln("            IP Address: " + wifiSettings.ipaddress + "/", 0);
  printSerialln("                Broker: " + mqttSettings.mqttBroker, 0);
  printSerialln("                 Topic: " + String(mqttSettings.mqttTopic), 0);
  printSerialln("                        ", 0);
  printSerialln("      Firmware Version: " + nodeSettings.storedVersion, 0);
  printSerialln(" MystTECH DEVICE READY: " + String(MYSTTECH_MODEL) + "", 100);


  sendMessageMQTTPayload("New Device updating MystTECH UI ...", "FirmwareStatus-->");

  printSerialln(".device added to dashboard", 1000);

  sendADDMEMessageMQTT();
  printSerialln(".addme has been shared on MQTT", 1000);

}

void loop() {


  loopFIRMWARE();
  checkForWin();
  checkVersionWinScenario();
  clientMQTTConnected();
  loopGPIO();
  loopFASTLED();
  mp3Loop();
  loopI2C(0);
  if  (String(NR_TYPE) == "2X2MATRIX") {
    exec_2x2matrix_puzzle();
  }
}

void logSerialPort(bool record) {




}





void delayESPTask(int d) {
  delay(d);
}

bool connected = false;

void connectWiFi() {

  connected = false;
  for (int i = 0; i < NUM_OF_SSID; i++) {
    printSerialln("Attempting WIFI Loop #: " + String(i), 0);
    if (i != -1) {
      wifiSettings.ssid = wifiSettings.bup_ssid[i];
      wifiSettings.password = wifiSettings.bup_password[i];
      printSerialln("Trying backup credentials...", 0);
    } else {
      printSerialln("Trying stored credentials...", 0);
    }

    printSerialf("Connecting to WiFi: %s", wifiSettings.ssid.c_str());
    printSerialf("WiFi Password: %s", wifiSettings.password.c_str());
    WiFi.begin(wifiSettings.ssid.c_str(), wifiSettings.password.c_str());

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {

      printSerial(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      printSerialln("WiFi connected.", 0);
      printSerial("IP Address: ");
      printSerialln(WiFi.localIP().toString(), 0);
      wifiSettings.ipaddress = WiFi.localIP().toString();
      connected = true;
      break;
    } else {
      printSerialln("WiFi connection failed. Trying next network...", 500);

    }
  }

  if (!connected) {
    printSerialln("No Wi-Fi connection established. Starting Access Point...", 500);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    printSerial("Access Point IP Address: ");
    printSerialln(WiFi.softAPIP().toString(), 0);
  }


  // Set mDNS hostname
  if (MDNS.begin(MYSTTECH_MODEL)) {
    printSerialln("mDNS responder started: " + String(MYSTTECH_MODEL), 500);
  } else {
    printSerialln("Error starting mDNS", 500);
  }
}

unsigned long lastUpdateCheck = 0; // Global variable to track the last update check time
const unsigned long updateInterval = 3600000; // 1 hour in milliseconds

void loopFIRMWARE() {
  // Check if it's time to perform a firmware update check
  if (millis() - lastUpdateCheck >= updateInterval) {
    printSerialln("Checking for firmware updates...", 0);
    checkForUpdates();
    lastUpdateCheck = millis(); // Reset the timer
  }
}

void checkForUpdates() {
  HTTPClient http;
  String versionURL = String("https://raw.githubusercontent.com/") +
                      String(GITHUB_USER) + "/" +
                      String(GITHUB_REPO) + "/" +
                      String(GITHUB_BRANCH) + "/" +
                      "version.txt";

  printSerialln("Fetching version from URL: " + versionURL, 0);
  http.begin(versionURL);
  int httpCode = http.GET();
  if (httpCode == 200) {
    String newVersion = http.getString();
    newVersion.trim();

    if (newVersion != nodeSettings.storedVersion) {
      printSerialln("New version available: " + newVersion, 0);
      if (updateFirmware(newVersion)) {
        nodeSettings.storedVersion = newVersion;
        saveNodeSettings();
        printSerialln("Update complete. Rebooting...", 5000);
        ESP.restart();
      } else {
        printSerialln("Firmware update failed.", 0);
      }
    } else {
      printSerialln("Device firmware is up-to-date.", 0);
    }
  } else {
    printSerialln("Failed to fetch version info. HTTP code: " + String(httpCode), 0);
  }
  http.end();
}

bool updateFirmware(const String& newVersion) {
  HTTPClient http;
  String firmwareURL = String("https://raw.githubusercontent.com/") +
                       String(GITHUB_USER) + "/" +
                       String(GITHUB_REPO) + "/" +
                       String(GITHUB_BRANCH) + "/" +
                       String(GITHUB_BIN);

  printSerialln("Fetching firmware from URL: " + firmwareURL, 0);
  http.begin(firmwareURL);
  int httpCode = http.GET();

  if (httpCode == 200) {
    int contentLength = http.getSize();
    if (Update.begin(contentLength)) {
      WiFiClient* client = http.getStreamPtr();
      size_t written = Update.writeStream(*client);

      if (written == contentLength) {
        printSerialln("Firmware written successfully.", 0);
        if (Update.end() && Update.isFinished()) {
          return true;
        } else {
          printSerialln("Update not finished.", 0);
        }
      } else {
        printSerialln("Firmware write failed.", 0);
      }
    } else {
      printSerialln("Not enough space for update.", 0);
    }
  } else {
    printSerialln("Failed to download firmware. HTTP code: " + String(httpCode), 0);
  }

  http.end();
  return false;
}



// Define the 7x7 bitmap for each character in "MystTech" using * and space
byte mystTechBMPArray[8][7] = {
  // 'M' (7x7) - Needs a diagonal shape
  {0b1000001, 0b1100011, 0b1010101, 0b1001101, 0b1000001, 0b1000001, 0b1000001},  // 'M'

  // 'y' (7x7) - Has a diagonal at the bottom
  {0b1000001, 0b1000001, 0b1000001, 0b0111111, 0b0000001, 0b0000001, 0b0000001},  // 'y'

  // 's' (7x7) - Standard S shape
  {0b0111111, 0b0100000, 0b0111111, 0b0000001, 0b0000001, 0b0111111, 0b0000000},  // 's'

  // 't' (lowercase, improved) (7x7) - Better horizontal bar and vertical stroke
  {0b0010000, 0b0010000, 0b0010000, 0b1111111, 0b0010000, 0b0010000, 0b0010000},  // 't'

  // 'T' (7x7) - Uppercase T with a wider top bar
  {0b1111111, 0b0010000, 0b0010000, 0b0010000, 0b0010000, 0b0010000, 0b0010000},  // 'T'

  // 'e' (7x7) - Standard e shape
  {0b0111111, 0b0100000, 0b0111111, 0b0100000, 0b0100000, 0b0111111, 0b0000000},  // 'e'

  // 'c' (7x7) - Standard c shape
  {0b0111111, 0b0100000, 0b0100000, 0b0100000, 0b0100000, 0b0100000, 0b0111111},  // 'c'

  // 'h' (7x7) - Standard h shape
  {0b1000001, 0b1000001, 0b1111111, 0b1000001, 0b1000001, 0b1000001, 0b1000001}   // 'h'
};


// Function to print the 7x7 bitmap characters for "MystTech" using * for blocks
void mystTechBMP() {

  for (int row = 0; row < 7; row++) {  // Loop over 7 rows for each character
    // Print the rows of all characters one by one
    for (int charIndex = 0; charIndex < 8; charIndex++) {
      for (int col = 0; col < 7; col++) {  // Loop over the 7 columns
        // Print '*' for '1' bit and space ' ' for '0' bit
        delay(3);

        if ((mystTechBMPArray[charIndex][row] >> (6 - col)) & 1) {
          printSerial("█");  // Print '*' for 'on' pixel
        } else {
          printSerial(" ");  // Print space for 'off' pixel
        }
      }
      printSerial("  ");  // Add space between characters
    }
    printSerialln("", 0); // Move to the next line after printing all characters for this row
  }
}


#define GPIO03_U0RXD_LED_RX 3
#define GPIO01_U0TXD_LED_TX 1
#define GPIO16_U2RXD_WS2812_16 16
#define GPIO17_U2TXD 17
#define GPIO23_VSPI_MOSI 23 // digital out 1 - win game
#define GPIO22_I2C_SCL 22
#define GPIO21_I2C_SDA 21
#define GPIO19_VSPI_MISO_CAM_Y5 19 // digital out 2 - other
#define GPIO18_VSPI_CLK_CAM_Y4 18
#define GPIO13_ADC2_CH4_HSPI_MOS1_TOUCH4 13 // fled 2
#define GPIO12_ADC2_CH5_HSPI_MOS0_TOUCH5_MTDI 12 // fled
#define GPIO02_ADC2_CH2_BOOT_TOUCH2_LEDIO2 2
#define GPIO04_ADC2_CH0_CAM_Y2_TOUCH0 4 // dial 1
#define GPIO05_VSPI_CS_CAM_Y3_SDIO 5 // dial 1
#define GPIO26_ADC2_CH9_CAM_SIOD_DAC2_I2S_BCK 26 // digital input 1 /// end game
#define GPIO32_ADC1_CH4_TOUCH9 32 // digital input 2 /// start game.... new sort
#define GPIO27_ADC2_CH7_CAM_SIOC_TOUCH7 27 // digital input 3 /// trigger relay 3
#define GPIO33_ADC1_CH5_TOUCH8 33 // dial 2
#define GPIO14_ADC2_CH6_HSPI_CLK_TOUCH6 14 // dial 2
#define GPIO25_ADC2_CH8_DAC1_I2S_LCK 25
#define GPIN36_ADC1_CH0_VP 36
#define GPIN39_ADC1_CH3_VN 39
#define GPIN34_ADC1_CH6_CAM_Y8 34 // dial 3
#define GPIN35_ADC1_CH7_CAM_Y9 35 // dial 3
#define GPIO15_ADC2_CH3_MTDO_TOUCH3_HSPI_CS 15
#define GPIO00_ADC2_CH1_BOOT_TOUCH1 0
