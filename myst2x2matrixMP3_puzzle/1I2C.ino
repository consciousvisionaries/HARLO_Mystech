

void setupI2C() {

  initializeI2CCom();

  setupPWM();
}

void loopI2C(int pwma) {

  loopPWM(pwma);
}

void setupPWM() {

  for (int i = 0; i < NUM_OF_PWM; i++) {

    printSerialln(("Begin PWM..." + i) , 0);

    // Initialize the PCA9685 with default MODE1 and MODE2 settings
    if (!pwmArray[i].begin()) {
      Serial.print("Error initializing pwmArray #");
      Serial.print(i);
      Serial.println(" PCA9685. Check your wiring.");
      scannerLoop();
      //while (1);  // Halt execution if initialization fails
    } else {
      printSerialln("Address: xxxx now occupied", 0);
    }

    delay(1000);

    // Optionally configure mode settings (e.g., enable autoincrement and all-call address)
    // ledArray.configure(PCA9685_MODE1_AUTOINCR | PCA9685_MODE1_ALLCALL, PCA9685_MODE2_TOTEMPOLE);

    // Set the PWM frequency to 1000 Hz (example)
    pwmArray[i].setPWMFreq(1000);
  }

  Serial.println("PCA9685 initialization complete.");
}

void scannerLoop() {

  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(5000);  // Wait 5 seconds before scanning again
}

unsigned long previousMillis = 0;  // will store last time the LED was updated
unsigned long ledDelay = 200;      // delay time for LED pattern
int currentLED[NUM_OF_MATRIXIO] = {0};



void loopPWM(int pwma) {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= ledDelay) {
    previousMillis = currentMillis;

    // Turn off the previous LED
    int previousIndex = (currentLED[pwma] - 1 + NUM_OF_MATRIXIO) % NUM_OF_MATRIXIO;
    pwmArray[pwma].setPWM(PWM[pwma][previousIndex] - 1, 0, 0);

    // Turn on the current LED
    pwmArray[pwma].setPWM(PWM[pwma][currentLED[pwma]] - 1, 0, 4095);

    // Move to the next LED
    currentLED[pwma] = (currentLED[pwma] + 1) % NUM_OF_MATRIXIO;
  }
}
