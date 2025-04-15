#define myst2x2matrixMP3_puzzle_UPDATED "April 15th, 2025"

// access point
#define AP_SSID "HARLO Matrix 2x2 Puzzle"
#define AP_PASSWORD "Mysttech" // 'localhost:1880/admin'
#define NUM_OF_SSID 4 

#define PUZZLE_SOLUTION { 2,10,9,7,11,5 }

#define NUM_OF_PWM 1

int PWM[NUM_OF_PWM][16] = {
  //{1, 2, 3, 4}
  { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 }
};

#define PWM_MATRIXIO_DISPLAY_ARRAY_NO 0 // 0 for first pwm

#define PUZZLE_DISPLAY_SORTORDER { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 } 
  
#define NUM_OF_SOLUTIONS 6
#define SOLUTION_TYPE "RELAY" // or MQTT for remote trigger
#define SOLUTION_MQTT_MESSAGE ""
#define RECORD_LOG true
#define SHOW_LOG true
#define SIZE_LOG 250
#define LOG_DELAY 0 // 1 for yes, 0 for no
#define RESET_PREF false

#define MP3_PLAYER false
#define MP3_FAIL_FILENO 17
#define MP3_WIN_FILENO 18

#define EXEC_BATCH1_PIN 5
#define EXEC_BATCH2_PIN 18
#define FLED_PIN1 12
#define FLED_PIN2 16

#define PUZZLE_NAME "HARLO Matrix 2x2 Puzzle"
#define DESIGNER_NAME "Conscious Visionaries"
#define TECH_NAME "MYSTTECH Creations"
#define MYSTTECH_MODEL "MYST2X2MATRIX_PUZZLE"
#define DNSLOCAL "HARLO1.LOCAL"

#define NR_TAB "HARLO"
#define NR_GROUP "Stage 1"
#define NR_TYPE "2X2MATRIX"
#define PULSE_MAX_RANGE 20 // NR_TYPE "3D_ROTARY_PULSE"
#define NUM_DIGITAL_INPUTSA 0
#define NUM_DIGITAL_INPUTSB 0
#define NUM_ANALOG_INPUTPAIRS 0
#define NUM_DIGITAL_IOMATRIXPAIRS 4
#define NUM_OF_MATRIXIO 16
#define NUM_DIGITAL_OUTPUTSA 0
#define NUM_DIGITAL_OUTPUT_ROWSA 0
#define NUM_DIGITAL_OUTPUTSB 2
#define NUM_DIGITAL_OUTPUT_ROWSB 1
#define NUM_FLED_OUTPUTS 1
#define NUM_FLED_CHANNELS 1  // 30/3
#define NUM_FLED_ADDLEDS 50 // Total number of LEDs (10 per dial)
#define NUM_FLED_ADDLEDS2 0 // total for around the room.
#define NUM_RXTX_PORTS 0
#define NUM_I2C_PORTS 1

String bodyInputTitle = PUZZLE_NAME;
String bodyOutputATitle = "Override";
String bodyOutputBTitle = "";
String buttonsHTMLTitle = "";

// github
const char* GITHUB_USER = "consciousvisionaries";
const char* GITHUB_REPO = "HARLO_Mysttech";
const char* GITHUB_BRANCH = "main/myst2x2matrix_puzzle";
const char* GITHUB_BIN = "myst2x2matrix_puzzle.ino.esp32da.bin"; // esp32 wroom


String MQTT_CLIENT_ID = "";           // Replace with MAC Address

struct NodeSettings {
  String puzzleName = PUZZLE_NAME;
  String designer = DESIGNER_NAME;
  String tech = TECH_NAME;
  String mDNS = DNSLOCAL;
  String nrTab = NR_TAB;
  String nrGroup = NR_GROUP;
  String nrType = NR_TYPE;
  String storedVersion = "";
  String ssid = "";
  String password = "";
  String ipaddress = "";
  String mac = "";
};

NodeSettings nodeSettings;

struct WiFiSettings {
  String ssid = "";
  String password = "";
  String ipaddress = "";
  String mac = "";
  String bup_ssid[4] = { "SHAW-6943","Paul's iPhone", "Beyond Entertainment", "TELUS5752" };
  String bup_password[4] = { "eager8184effect","pauladgj", "Gary2019", "c2ks9zcvby" };
};

WiFiSettings wifiSettings;

struct MQTTSettings {
  String mqttUsername = "9grsvy8373";
  String mqttPassword = "8bdehprsuz";
  String mqttBroker = "b37.mqtt.one";
  const char* mqttTopic = "9grsvy8373/harlo";
  String bup_mqttBroker[3] = {"b37.mqtt.one", "192.168.0.129", "broker.emqx.io"};
  String bup_mqttUser[3] = {"9grsvy8373", "pro1polaris",""};
  String bup_mqttPassword[3] = {"8bdehprsuz", "CVr819P*!",""};

};

MQTTSettings mqttSettings;

struct GlobalSettings {

  String inputNames[8] = { "Dial 1a", "Dial 1b", "Dial 2a", "Dial 2b", "Dial 3a", "Dial 3b", "not used", "not used" };
  String outputNames[8] = { "Dial 1a", "Dial 1b", "Dial 2a", "Dial 2b", "Dial 3a", "Dial 3b", "not used", "not used" };
  String colorsFLEDChannels[3] = {"red", "green", "blue"};

};

GlobalSettings globalSettings;

struct GlobalHyperlinks {
  String adminButtonHTML = "";
  String homeButtonHTML = "";
};

GlobalHyperlinks globalHyperlinks;
