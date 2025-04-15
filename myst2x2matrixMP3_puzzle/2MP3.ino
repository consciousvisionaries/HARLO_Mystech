#include <DFRobotDFPlayerMini.h>  // Make sure you have this library installed

// Software serial for MP3 module
HardwareSerial mp3Serial(2); // RX, TX uart2 rx 16 tx 17
DFRobotDFPlayerMini mp3Player;

unsigned long lastCheckTime = 0;  // To track the time of the last check
const unsigned long checkInterval = 5000;  // 5 seconds interval for rechecking initialization

// files 1-8 individual clips
// files 9 complete clip
// file 10 success sound

int lastVolumeSet;

void setupMP3Player() {
  if (MP3_PLAYER) {
    mp3Serial.end();  // Ensure any previous serial connection is closed

    // Initialize MP3 player
    mp3Serial.begin(9600, SERIAL_8N1, RXTX_Pins[0][1], RXTX_Pins[0][2]); 
    
    if (!mp3Player.begin(mp3Serial)) {
        Serial.printf("MP3 Player not detected.\n");
        mp3PlayerInitialized = false;
    } else {
        mp3PlayerInitialized = true;
        Serial.printf("MP3 Player ready.\n");
        mp3Player.volume(30);
        mp3Player.play(1); // Play the first MP3 file (Full Song)
    }
  }
}
void playMP3(int songNumber) {

  if (MP3_PLAYER) {
    if (mp3PlayerInitialized) {
        mp3Player.play(songNumber);
        Serial.printf("Playing MP3 file number: %d\n", songNumber);
    } 
    delay(50);
  } 
}

void checkMP3Initialization() {
  
    // Check if it's been 5 seconds since the last check
    if (millis() - lastCheckTime >= checkInterval) {
        lastCheckTime = millis();  // Update the time of the last check
        
        // Try to reinitialize the MP3 player if it's not initialized
        if (!mp3PlayerInitialized) {
            printSerialln("Re-checking MP3 Player initialization...",0);
            setupMP3Player();
        }
    }
}

void mp3Loop() {
  
  if (MP3_PLAYER) {
    // Perform other tasks here...

    // Periodically check if MP3 player is initialized
    checkMP3Initialization();

    // Play MP3 file (example)
    playMP3(2); // For example, play the second MP3 file

   
  }
}
