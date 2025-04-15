#include <Preferences.h>
Preferences preferences;

enum PatternType {
  NONE,
  CHASE,
  COLOR_WIPE,
  RAINBOW,
  THEATER_CHASE,
  FLASH,
  STROBE,
  BOUNCE,
  BREATH,
  FIRE,
  CONFETTI,
  CYLON,
  SCANNER,
  SPARKLE,
  PULSE,
  RAINBOW_CYCLE,
  RAINBOW_GLITTER,
  TWINKLE,
  RANDOM_FILL
};



bool gameOnFlag = false;
bool gameOverFlag = true;
bool restartFlag = false;
bool gameOffFlag = false;

bool solutionFound = false;
bool solutionStable = false;
int solutionWin[8] = PUZZLE_SOLUTION;
unsigned long solutionCheckStart = 0;
int solutionSeq = 0;

String styleHTML;
String onloadHTML;
String setIntervalHTML;
String bodyDivHTML;
String scriptHTML;
String navButtonHTML;

const int inputDigitalPinsA[2] = {
  GPIO25_ADC2_CH8_DAC1_I2S_LCK,
  GPIO12_ADC2_CH5_HSPI_MOS0_TOUCH5_MTDI
};

const int inputDigitalPinsB[8] = {
  GPIN36_ADC1_CH0_VP,
  GPIN39_ADC1_CH3_VN,
  GPIN34_ADC1_CH6_CAM_Y8,
  GPIN35_ADC1_CH7_CAM_Y9,
  GPIO32_ADC1_CH4_TOUCH9,
  GPIO33_ADC1_CH5_TOUCH8,
  GPIO25_ADC2_CH8_DAC1_I2S_LCK,
  GPIO26_ADC2_CH9_CAM_SIOD_DAC2_I2S_BCK
};

int inputDigitalPinStateA[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int inputDigitalPinStateB[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

int inputAnalogTouchPins[10] = {
  GPIO04_ADC2_CH0_CAM_Y2_TOUCH0,
  GPIO00_ADC2_CH1_BOOT_TOUCH1,
  GPIO02_ADC2_CH2_BOOT_TOUCH2_LEDIO2,
  GPIO15_ADC2_CH3_MTDO_TOUCH3_HSPI_CS,
  GPIO13_ADC2_CH4_HSPI_MOS1_TOUCH4,       // relay output 1 (exec)
  GPIO12_ADC2_CH5_HSPI_MOS0_TOUCH5_MTDI,
  GPIO14_ADC2_CH6_HSPI_CLK_TOUCH6,
  GPIO27_ADC2_CH7_CAM_SIOC_TOUCH7,        // relay output 2 (exec)
  GPIO33_ADC1_CH5_TOUCH8,
  GPIO32_ADC1_CH4_TOUCH9
};

const int analogInputPinsA[8] = {
  GPIN34_ADC1_CH6_CAM_Y8,
  GPIO33_ADC1_CH5_TOUCH8,
  GPIO32_ADC1_CH4_TOUCH9, -1, -1, -1, -1, -1
};

const int analogInputPinsB[8] = {
  GPIN36_ADC1_CH0_VP,
  GPIO15_ADC2_CH3_MTDO_TOUCH3_HSPI_CS,
  GPIO13_ADC2_CH4_HSPI_MOS1_TOUCH4, -1, -1, -1, -1, -1
};

const int digitalMatrixIOPins[2][4] = {{
    GPIO13_ADC2_CH4_HSPI_MOS1_TOUCH4,
    GPIO14_ADC2_CH6_HSPI_CLK_TOUCH6,
    GPIO27_ADC2_CH7_CAM_SIOC_TOUCH7,
    GPIO26_ADC2_CH9_CAM_SIOD_DAC2_I2S_BCK
  },
  {
    GPIO23_VSPI_MOSI,
    GPIO19_VSPI_MISO_CAM_Y5,
    GPIO25_ADC2_CH8_DAC1_I2S_LCK,
    GPIO32_ADC1_CH4_TOUCH9
  }
};

const int outputPinsA[8] = {
  GPIO27_ADC2_CH7_CAM_SIOC_TOUCH7,
  GPIO14_ADC2_CH6_HSPI_CLK_TOUCH6,
  GPIO12_ADC2_CH5_HSPI_MOS0_TOUCH5_MTDI,
  GPIO13_ADC2_CH4_HSPI_MOS1_TOUCH4,
  GPIO19_VSPI_MISO_CAM_Y5,
  GPIO21_I2C_SDA,
  GPIO22_I2C_SCL,
  GPIO23_VSPI_MOSI
};

const int outputPinsB[8] = {
  GPIO05_VSPI_CS_CAM_Y3_SDIO,
  GPIO18_VSPI_CLK_CAM_Y4,
  GPIO15_ADC2_CH3_MTDO_TOUCH3_HSPI_CS,
  GPIO02_ADC2_CH2_BOOT_TOUCH2_LEDIO2,
  GPIO00_ADC2_CH1_BOOT_TOUCH1,
  GPIO04_ADC2_CH0_CAM_Y2_TOUCH0,
  GPIO16_U2RXD_WS2812_16,
  GPIO17_U2TXD
};

const int outputFLEDPins[2] = {
  GPIO12_ADC2_CH5_HSPI_MOS0_TOUCH5_MTDI,
  GPIO16_U2RXD_WS2812_16
};

const int RXTX_Pins[2][2] = {
  {GPIO16_U2RXD_WS2812_16, GPIO17_U2TXD},
  {GPIO03_U0RXD_LED_RX, GPIO01_U0TXD_LED_TX}
};

const int I2C_Pins[2][2] = {
 

  {GPIO22_I2C_SCL, GPIO21_I2C_SDA},
   {GPIO25_ADC2_CH8_DAC1_I2S_LCK, GPIO33_ADC1_CH5_TOUCH8}
};

int ledCount[3] = {0, 0, 0};
int lastLedCount[3] = { -1, -1, -1}; // Tracks the last LED count for each dial

static unsigned long lastExecutionTime = 0; // Tracks the last execution time

int outputPins_initStateA[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int outputPins_initStateB[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

volatile int lastStateAnalogInputs[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

unsigned long lastDebounceTimeA[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // Store last debounce time for each input
unsigned long lastDebounceTimeB[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // Store last debounce time for each input

unsigned long debounceDelay = 50; // 50ms debounce time
int lastInputDigitalPinStateA[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH}; // Stores the last known state of the pins
int lastInputDigitalPinStateB[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH}; // Stores the last known state of the pins

volatile int pulseGPIOCount[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static int lastpulseGPIOCount[8] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile bool pulseGPIOUpdated[8] = {false, false, false, false, false, false, false, false};

int FLEDCount[8] = {0, 0, 0, 0, 0, 0 , 0 , 0};
int lastFLEDCount[8] = { -1, -1, -1, -1, -1, -1, -1, -1};

bool buttonMatrixState[NUM_DIGITAL_IOMATRIXPAIRS][NUM_DIGITAL_IOMATRIXPAIRS];  // Store button states

bool MATRIXIO_changed = false;
unsigned long lastMatrixDebounceTime[NUM_DIGITAL_IOMATRIXPAIRS][NUM_DIGITAL_IOMATRIXPAIRS];  // Debounce timing
int lastMatrixButtonPressed = 0;

void saveNodeSettings() {
  preferences.begin("node", false);
  preferences.putString("puzzleName", nodeSettings.puzzleName);
  preferences.putString("designer", nodeSettings.designer);
  preferences.putString("tech", nodeSettings.tech);
  preferences.putString("mDNS", nodeSettings.mDNS);
  preferences.putString("nrTab", nodeSettings.nrTab);
  preferences.putString("nrGroup", nodeSettings.nrGroup);
  preferences.putString("nrType", nodeSettings.nrType);
  preferences.putString("storedVersion", nodeSettings.storedVersion);
  preferences.end();
  printSerial("storedVersion saved: ");
  printSerialln(nodeSettings.storedVersion, 1000);
}

void loadNodeSettings() {
  preferences.begin("node", true);
  nodeSettings.puzzleName = preferences.getString("puzzleName", PUZZLE_NAME);
  nodeSettings.designer = preferences.getString("designer", DESIGNER_NAME);
  nodeSettings.tech = preferences.getString("tech", TECH_NAME);
  nodeSettings.mDNS = preferences.getString("mDNS", DNSLOCAL);
  nodeSettings.nrTab = preferences.getString("nrTab", NR_TAB);
  nodeSettings.nrGroup = preferences.getString("nrGroup", NR_GROUP);
  nodeSettings.nrType = preferences.getString("nrType", NR_TYPE);
  nodeSettings.storedVersion = preferences.getString("storedVersion", "");
  preferences.end();
}

void saveWiFiSettings() {
  preferences.begin("wifi", false);
  preferences.putString("ssid", wifiSettings.ssid);
  preferences.putString("password", wifiSettings.password);
  preferences.putString("ipaddress", wifiSettings.ipaddress);
  preferences.putString("mac", wifiSettings.mac);
  for (int i = 0; i < 2; i++) {
    String temp = ("bup_ssid" + String(i));  // Create the key string for bup_ssid
    preferences.putString(temp.c_str(), wifiSettings.bup_ssid[i]);

    temp = ("bup_password" + String(i));  // Create the key string for bup_password
    preferences.putString(temp.c_str(), wifiSettings.bup_password[i]);
  }
  preferences.end();
}

void loadWiFiSettings() {
  preferences.begin("wifi", true);
  wifiSettings.ssid = preferences.getString("ssid", "");
  wifiSettings.password = preferences.getString("password", "");
  wifiSettings.ipaddress = preferences.getString("ipaddress", "");
  wifiSettings.mac = preferences.getString("mac", "");
  for (int i = 0; i < 2; i++) {
    String temp = ("bup_ssid" + String(i));  // Create the key string for bup_ssid
    wifiSettings.bup_ssid[i] = preferences.getString(temp.c_str(), wifiSettings.bup_ssid[i]);

    temp = ("bup_password" + String(i));  // Create the key string for bup_password
    wifiSettings.bup_password[i] = preferences.getString(temp.c_str(), wifiSettings.bup_password[i]);
  }
  preferences.end();
}

void saveMQTTSettings() {
  preferences.begin("mqtt", false);
  preferences.putString("mqttUsername", mqttSettings.mqttUsername);
  preferences.putString("mqttPassword", mqttSettings.mqttPassword);
  preferences.putString("mqttBroker", mqttSettings.mqttBroker);
  for (int i = 0; i < 3; i++) {
    String temp = ("bup_mqttBroker" + String(i));  // Create the key string for bup_mqttBroker
    preferences.putString(temp.c_str(), mqttSettings.bup_mqttBroker[i]);

    temp = ("bup_mqttUser" + String(i));  // Create the key string for bup_mqttUser
    preferences.putString(temp.c_str(), mqttSettings.bup_mqttUser[i]);

    temp = ("bup_mqttPassword" + String(i));  // Create the key string for bup_mqttPassword
    preferences.putString(temp.c_str(), mqttSettings.bup_mqttPassword[i]);
  }
  preferences.end();
}

void loadMQTTSettings() {
  preferences.begin("mqtt", true);
  mqttSettings.mqttUsername = preferences.getString("mqttUsername", mqttSettings.mqttUsername);
  mqttSettings.mqttPassword = preferences.getString("mqttPassword", mqttSettings.mqttPassword);
  mqttSettings.mqttBroker = preferences.getString("mqttBroker", mqttSettings.mqttBroker);
  for (int i = 0; i < 3; i++) {
    String temp = ("bup_mqttBroker" + String(i));  // Create the key string for bup_mqttBroker
    mqttSettings.bup_mqttBroker[i] = preferences.getString(temp.c_str(), mqttSettings.bup_mqttBroker[i]);

    temp = ("bup_mqttUser" + String(i));  // Create the key string for bup_mqttUser
    mqttSettings.bup_mqttUser[i] = preferences.getString(temp.c_str(), mqttSettings.bup_mqttUser[i]);

    temp = ("bup_mqttPassword" + String(i));  // Create the key string for bup_mqttPassword
    mqttSettings.bup_mqttPassword[i] = preferences.getString(temp.c_str(), mqttSettings.bup_mqttPassword[i]);
  }
  preferences.end();
}

void saveGlobalSettings() {
  preferences.begin("global", false);
  preferences.putString("nrTab", nodeSettings.nrTab);
  preferences.putString("nrGroup", nodeSettings.nrGroup);
  for (int i = 0; i < 8; i++) {
    String temp = ("inputName" + String(i));  // Create the key string for inputName
    preferences.putString(temp.c_str(), globalSettings.inputNames[i]);

    temp = ("outputName" + String(i));  // Create the key string for outputName
    preferences.putString(temp.c_str(), globalSettings.outputNames[i]);
  }
  for (int i = 0; i < 3; i++) {
    String temp = ("colorsFLEDChannels" + String(i));  // Create the key string for colorsFLEDChannels
    preferences.putString(temp.c_str(), globalSettings.colorsFLEDChannels[i]);
  }
  preferences.end();
}

void loadGlobalSettings() {
  preferences.begin("global", true);
  nodeSettings.nrTab = preferences.getString("nrTab", NR_TAB);
  nodeSettings.nrGroup = preferences.getString("nrGroup", NR_GROUP);
  for (int i = 0; i < 8; i++) {
    String temp = ("inputName" + String(i));  // Create the key string for inputName
    globalSettings.inputNames[i] = preferences.getString(temp.c_str(), globalSettings.inputNames[i]);

    temp = ("outputName" + String(i));  // Create the key string for outputName
    globalSettings.outputNames[i] = preferences.getString(temp.c_str(), globalSettings.outputNames[i]);
  }
  for (int i = 0; i < 3; i++) {
    String temp = ("colorsFLEDChannels" + String(i));  // Create the key string for colorsFLEDChannels
    globalSettings.colorsFLEDChannels[i] = preferences.getString(temp.c_str(), globalSettings.colorsFLEDChannels[i]);
  }
  preferences.end();
}

void saveGlobalHyperlinks() {
  preferences.begin("hyperlinks", false);
  preferences.putString("homeButtonHTML", globalHyperlinks.homeButtonHTML);
  preferences.putString("adminButtonHTML", globalHyperlinks.adminButtonHTML);
  preferences.end();
}

void loadGlobalHyperlinks() {
  preferences.begin("hyperlinks", true);
  globalHyperlinks.homeButtonHTML = preferences.getString("homeButtonHTML", "");
  globalHyperlinks.adminButtonHTML = preferences.getString("adminButtonHTML", "");
  preferences.end();
}

void loadAllSettings() {
  loadNodeSettings();
  loadWiFiSettings();
  loadMQTTSettings();
  loadGlobalSettings();
  loadGlobalHyperlinks();
}

void saveAllSettings() {
  saveNodeSettings();
  saveWiFiSettings();
  saveMQTTSettings();
  saveGlobalSettings();
  saveGlobalHyperlinks();
}
