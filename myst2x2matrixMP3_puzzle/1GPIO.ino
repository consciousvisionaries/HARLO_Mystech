#include <Wire.h>

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void update_pulseGPIOCount(int p, int v) {
  pulseGPIOCount[p] = v;
}

void putDigitalOutputState(int pin, bool state) {
  digitalWrite(pin, state);
}

// Method to get the current state of a digital input pin
bool getDigitalInputStateGPIO(int inputPin) {
  int state = digitalRead(inputPin);
  return (state == HIGH) ? HIGH : LOW;  // Return "HIGH" or "LOW"
}

// Method to toggle the output pin state
void toggleOutputStateGPIOA(int outputNumber) {


  if (outputNumber >= 0 && outputNumber < NUM_DIGITAL_OUTPUTSA) {
    int pin = outputPinsA[outputNumber];
    bool currentState = digitalRead(pin);  // Read the current state before toggling
    bool newState = !currentState;         // Toggle state
    digitalWrite(pin, newState);          // Set the new state

    printSerial("Output ");
    printSerial(String(outputNumber));
    printSerial(": Toggled to ");
    printSerialln(newState ? "HIGH" : "LOW", 0);
    sendMessageMQTTPayload("Toggled Output", "UpdateUI");
  } else {
    printSerialln("Error: Invalid output number.", 0);
  }
}

// Method to toggle the output pin state
void toggleOutputStateGPIOB(int outputNumber) {

  if (outputNumber >= 0 && outputNumber < NUM_DIGITAL_OUTPUTSB) {
    int pin = outputPinsB[outputNumber];
    bool currentState = digitalRead(pin);  // Read the current state before toggling
    bool newState = !currentState;         // Toggle state
    digitalWrite(pin, newState);          // Set the new state

    printSerial("Output ");
    printSerial(String(outputNumber));
    printSerial(": Toggled to ");
    printSerialln(newState ? "HIGH" : "LOW", 0);
  } else {
    printSerialln("Error: Invalid output number.", 0);
  }
}

// Method to handle digital input changes
void handleDigitalInputChangeA(int pinIndex) {

  // Check for debounce and pin state change
  unsigned long currentTime = millis();

  if (currentTime - lastDebounceTimeA[pinIndex] > debounceDelay) {
    lastDebounceTimeA[pinIndex] = currentTime; // Update debounce time
    int currentState = digitalRead(inputDigitalPinsA[pinIndex]);

    if (currentState != lastInputDigitalPinStateA[pinIndex]) {
      lastInputDigitalPinStateA[pinIndex] = currentState;  // Update the last known state

      // Trigger action only if the pin goes LOW (e.g., a button press)
      if (currentState == LOW) {
        if (pinIndex == 0) {
          executeFUNCBatchButton1_PIN(EXEC_BATCH1_PIN);
        } else if (pinIndex == 1) {
          executeFUNCBatchButton2();
        } else if (pinIndex == 2) {
          //executeFUNCBatchGPIOPin3();
        }
      }
    }
  }
}

// Method to handle digital input changes
void handleDigitalInputChangeB(int pinIndex) {

  // Check for debounce and pin state change
  unsigned long currentTime = millis();

  if (currentTime - lastDebounceTimeB[pinIndex] > debounceDelay) {
    lastDebounceTimeB[pinIndex] = currentTime; // Update debounce time
    int currentState = digitalRead(inputDigitalPinsB[pinIndex]);

    if (currentState != lastInputDigitalPinStateB[pinIndex]) {
      lastInputDigitalPinStateB[pinIndex] = currentState;  // Update the last known state

      // Trigger action only if the pin goes LOW (e.g., a button press)
      if (currentState == LOW) {
        if (pinIndex == 0) {
          //executeFUNCBatchGPIOPin1();
        } else if (pinIndex == 1) {
          //executeFUNCBatchGPIOPin2();
        } else if (pinIndex == 2) {
          //executeFUNCBatchGPIOPin3();
        }
      }
    }
  }
}


// Method to handle analog input state changes
void handleAnalogInputPairsChange(int pinIndex) {

  int stateA = digitalRead(analogInputPinsA[pinIndex]);
  int stateB = digitalRead(analogInputPinsB[pinIndex]);

  portENTER_CRITICAL(&mux);

  if (stateA != lastStateAnalogInputs[pinIndex]) {
    pulseGPIOCount[pinIndex] += (stateA != stateB) ? 1 : -1;
    pulseGPIOUpdated[pinIndex] = true;
  }

  lastStateAnalogInputs[pinIndex] = stateA;
  portEXIT_CRITICAL(&mux);
}

void handleAnalogInputPairsChange2(int i) {

  portENTER_CRITICAL(&mux);
  int count = pulseGPIOCount[i];
  bool updated = pulseGPIOUpdated[i];
  pulseGPIOUpdated[i] = false;
  portEXIT_CRITICAL(&mux);

  count = constrain(count, 0, (NUM_FLED_ADDLEDS / NUM_FLED_CHANNELS));

  if (updated) {
    printSerial("Dial ");
    printSerial(String(i + 1));
    printSerial(" Count: ");
    printSerialln(String(count), 0);
  }
}


// Constants
const unsigned long DEBOUNCE_DELAY = 20; // milliseconds

// Globals (you must define these properly in your main sketch)
unsigned long lastDebounceTime[NUM_DIGITAL_IOMATRIXPAIRS][NUM_DIGITAL_IOMATRIXPAIRS];

void handleDigitalMatrixIOPairsChange(int pwma) {
  MATRIXIO_changed = false;

  for (int row = 0; row < NUM_DIGITAL_IOMATRIXPAIRS; row++) {
    // Deactivate all rows
    for (int i = 0; i < NUM_DIGITAL_IOMATRIXPAIRS; i++) {
      digitalWrite(digitalMatrixIOPins[0][i], HIGH);
    }

    // Activate current row
    digitalWrite(digitalMatrixIOPins[0][row], LOW);
    delayMicroseconds(50); // settle time

    for (int col = 0; col < NUM_DIGITAL_IOMATRIXPAIRS; col++) {
      bool rawState = digitalRead(digitalMatrixIOPins[1][col]) == LOW;
      bool prevState = buttonMatrixState[row][col];

      if (rawState != prevState) {
        unsigned long currentTime = millis();

        if ((currentTime - lastDebounceTime[row][col]) > DEBOUNCE_DELAY) {
          buttonMatrixState[row][col] = rawState;
          lastDebounceTime[row][col] = currentTime;

          int buttonNumber = col + 1 + (row * NUM_DIGITAL_IOMATRIXPAIRS);

          if (rawState == true) { // Pressed
            MATRIXIO_changed = true;

            pwmArray[pwma].setPWM(PWM[pwma][-lastMatrixButtonPressed - 1] - 1, 0, 0);
            lastMatrixButtonPressed = buttonNumber;
            pwmArray[pwma].setPWM(PWM[pwma][lastMatrixButtonPressed - 1] - 1, 0, 4095);

            printSerial("Button Pressed: ");
            printSerialln(String(lastMatrixButtonPressed), 50);

            switch (lastMatrixButtonPressed) {
              case 13: restartFlag = true; break;
            }

          } else { // Released
            printSerial("Button Released: ");
            printSerialln(String(buttonNumber), 50);

            switch (buttonNumber) {
              case 13: gameOverFlag = true; gameOnFlag = false; break;
            }

            // Optional: show full matrix state
            // printSerialMatrix();
          }
        }
      }
    }
  }
}




void printMATRIXButtonStates() {

  if (MATRIXIO_changed) {

    // Print the button states (pressed or released)
    for (int row = 0; row < NUM_DIGITAL_IOMATRIXPAIRS; row++) {
      for (int col = 0; col < NUM_DIGITAL_IOMATRIXPAIRS; col++) {
        Serial.print(buttonMatrixState[row][col] ? "1" : "0");  // "1" for pressed, "0" for not pressed
        Serial.print(" ");
      }
      Serial.println();
    }
    Serial.println();
  }
}


// Array to track used pins
bool usedPins[50] = {false}; // Assuming a maximum of 50 GPIO pins on the microcontroller

void usePin(int nPin) {

  // Lambda function to check and track pin usage
  auto usePinX = [&](int pin) {
    if (usedPins[pin]) {
      Serial.print("Error: Pin conflict detected on pin ");
      Serial.println(pin);  // Print the pin number causing the conflict
      return;  // Exit if a conflict is found
    }

    usedPins[pin] = true;
    Serial.print("Pin ");
    Serial.print(pin);
    Serial.println(" is available => ");
  };

  usePinX(nPin);  // Call the lambda function to check and track the pin usage
}

// Method to set up GPIO pins (outputs, inputs, analog)
void setupGPIO() {

  Serial.begin(115200);
  initializeDigitalOutputsA();
  initializeDigitalOutputsB();
  initializeDigitalInputsA();
  initializeDigitalInputsB();
  initializeAnalogInputPairs();
  initializeDigitalIOMatrix();
  initializeFLEDOutputs();
  initializeRXTXOutputs();
  initializeMATRIXIOKeyboard();
}

// Main loop to handle GPIO activities
void loopGPIO() {

  if (gameOnFlag) {

    // Poll and handle digital inputs (instead of relying on interrupts)
    for (int i = 0; i < NUM_DIGITAL_INPUTSA; i++) {
      handleDigitalInputChangeA(i);
    }

    // Poll and handle digital inputs (instead of relying on interrupts)
    for (int i = 0; i < NUM_DIGITAL_INPUTSB; i++) {
      handleDigitalInputChangeB(i);
    }


    // Poll and handle analog input changes
    for (int i = 0; i < NUM_ANALOG_INPUTPAIRS; i++) {
      handleAnalogInputPairsChange(i);
      handleAnalogInputPairsChange2(i);
    }

    // Poll matrix io pins
    if (NUM_DIGITAL_IOMATRIXPAIRS >= 2) {
      handleDigitalMatrixIOPairsChange(PWM_MATRIXIO_DISPLAY_ARRAY_NO);
      printMATRIXButtonStates();
    }
  } else if (gameOverFlag) {
    // Poll matrix io pins
    if (NUM_DIGITAL_IOMATRIXPAIRS >= 2) {
      handleDigitalMatrixIOPairsChange(PWM_MATRIXIO_DISPLAY_ARRAY_NO);
      printMATRIXButtonStates();
    }
  }
}

void initializeMATRIXIOKeyboard() {
  // Set row pins as outputs (to drive the rows)
  for (int i = 0; i < NUM_DIGITAL_IOMATRIXPAIRS; i++) {
    pinMode(digitalMatrixIOPins[0][i], OUTPUT);
    digitalWrite(digitalMatrixIOPins[0][i], HIGH); // Set them high initially
  }

  // Set column pins as inputs (to read the column states)
  for (int i = 0; i < NUM_DIGITAL_IOMATRIXPAIRS; i++) {
    pinMode(digitalMatrixIOPins[1][i], INPUT_PULLUP); // Enable pull-up resistors on column pins
  }
}

void initializeDigitalOutputsA() {
  // Initialize output pins (A pins)
  for (int i = 0; i < NUM_DIGITAL_OUTPUTSA; i++) {
    if (outputPinsA[i] == GPIO17_U2TXD) {
      printSerial(" => Setting up GPIO17_U2TXD as an output...");
      Serial2.end();  // Disable UART2


      printSerialln("GPIO17_U2TXD disabled and set as digital output.", 0);

    } else if (outputPinsA[i] == GPIO16_U2RXD_WS2812_16) {
      printSerial(" => Setting up GPIO16_U2RXD_WS2812_16 as an output...");
      Serial1.end();  // Disable UART1



      printSerialln("GPIO16_U2RXD_WS2812_16 disabled and set as digital output.", 0);

    } else if (outputPinsA[i] == GPIO01_U0TXD_LED_TX) {
      printSerial(" => Setting up GPIO01_U0TXD_LED_TX as an output...");
      Serial1.end();  // Disable UART1


      printSerialln("GPIO01_U0TXD_LED_TX disabled and set as digital output.", 0);

    } else if (outputPinsA[i] == GPIO03_U0RXD_LED_RX) {
      printSerial(" => Setting up GPIO03_U0RXD_LED_RX as an output...");
      Serial1.end();  // Disable UART1



      printSerialln("GPIO03_U0RXD_LED_RX disabled and set as digital output.", 0);
    }

    usePin(outputPinsA[i]);  // Check for conflicts
#define RELAY_PIN outputPinsA[i]
    gpio_reset_pin((gpio_num_t)RELAY_PIN);  // Ensure a clean start
    gpio_set_direction((gpio_num_t)RELAY_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)RELAY_PIN, 1);  // Ensure it's HIGH before enabling
    pinMode(outputPinsA[i], OUTPUT);
    digitalWrite(outputPinsA[i], outputPins_initStateA[i] ? HIGH : LOW); // Initialize with state array
    delay(10);  // Small delay after setting the output state
    printSerial("Output pin ");
    printSerial(String(outputPinsA[i]));
    printSerial(" set to ");
    printSerialln(outputPins_initStateA[i] ? "HIGH" : "LOW", 0);

  }
  printSerialln("<end> ." + String(NUM_DIGITAL_OUTPUTSA) + " A-Outputs Initialized.", 0);
  delay(1000);
}

void initializeDigitalOutputsB() {
  // Initialize output pins (B pins)
  for (int i = 0; i < NUM_DIGITAL_OUTPUTSB; i++) {
    if (outputPinsB[i] == GPIO17_U2TXD) {
      printSerial(" => Setting up GPIO17_U2TXD as an output...");
      Serial2.end();  // Disable UART2



      printSerialln("GPIO17_U2TXD disabled and set as digital output.", 0);

    } else if (outputPinsB[i] == GPIO16_U2RXD_WS2812_16) {
      printSerial(" => Setting up GPIO16_U2RXD_WS2812_16 as an output...");
      Serial1.end();  // Disable UART1


      printSerialln("GPIO16_U2RXD_WS2812_16 disabled and set as digital output.", 0);

    } else if (outputPinsB[i] == GPIO01_U0TXD_LED_TX) {
      printSerial(" => Setting up GPIO01_U0TXD_LED_TX as an output...");
      Serial1.end();  // Disable UART1

      printSerialln("GPIO01_U0TXD_LED_TX disabled and set as digital output.", 0);

    } else if (outputPinsB[i] == GPIO03_U0RXD_LED_RX) {
      printSerial(" => Setting up GPIO03_U0RXD_LED_RX as an output...");
      Serial1.end();  // Disable UART1


      printSerialln("GPIO03_U0RXD_LED_RX disabled and set as digital output.", 0);
    }
    usePin(outputPinsB[i]); // Check for conflicts
#define RELAY_PIN outputPinsA[i]
    gpio_reset_pin((gpio_num_t)RELAY_PIN);  // Ensure a clean start
    gpio_set_direction((gpio_num_t)RELAY_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)RELAY_PIN, 1);  // Ensure it's HIGH before enabling
    pinMode(outputPinsB[i], OUTPUT);
    delay(10);  // Allow time for pinMode to take effect
    digitalWrite(outputPinsB[i], outputPins_initStateB[i] ? HIGH : LOW); // Initialize with state array
    delay(10);  // Small delay after setting the output state
    printSerial("Output pin ");
    printSerial(String(outputPinsB[i]));
    printSerial(" set to ");
    printSerialln(outputPins_initStateB[i] ? "HIGH" : "LOW", 0);
  }

  printSerialln("<end> ." + String(NUM_DIGITAL_OUTPUTSB) + " B-Outputs Initialized.", 0);
  delay(1000);


}

void initializeDigitalInputsA() {
  // Initialize input pins (A pins)
  for (int i = 0; i < NUM_DIGITAL_INPUTSA; i++) {
    if (outputPinsA[i] == GPIO17_U2TXD) {
      printSerial(" => Setting up GPIO17_U2TXD as an output...");
      Serial2.end();  // Disable UART2
      delay(10);  // Allow time for UART2 to fully disable

      pinMode(inputDigitalPinsB[i], OUTPUT); // Set pin as OUTPUT
      delay(10);  // Small delay to ensure pinMode change is stable
      digitalWrite(inputDigitalPinsB[i], LOW); // Initialize to LOW
      delay(10);  // Small delay to stabilize the LOW state

      printSerialln("GPIO17_U2TXD disabled and set as digital output.", 0);

    } else if (inputDigitalPinsB[i] == GPIO16_U2RXD_WS2812_16) {
      printSerial(" => Setting up GPIO16_U2RXD_WS2812_16 as an output...");
      Serial1.end();  // Disable UART1
      delay(10);  // Allow time for UART1 to fully disable

      pinMode(inputDigitalPinsB[i], OUTPUT); // Set pin as OUTPUT
      delay(10);  // Small delay to ensure pinMode change is stable
      digitalWrite(inputDigitalPinsB[i], LOW); // Initialize to LOW
      delay(10);  // Small delay to stabilize the LOW state

      printSerialln("GPIO16_U2RXD_WS2812_16 disabled and set as digital output.", 0);

    } else if (inputDigitalPinsB[i] == GPIO01_U0TXD_LED_TX) {
      printSerial(" => Setting up GPIO01_U0TXD_LED_TX as an output...");
      Serial1.end();  // Disable UART1
      delay(10);  // Allow time for UART1 to fully disable

      pinMode(inputDigitalPinsB[i], OUTPUT); // Set pin as OUTPUT
      delay(10);  // Small delay to ensure pinMode change is stable
      digitalWrite(inputDigitalPinsB[i], LOW); // Initialize to LOW
      delay(10);  // Small delay to stabilize the LOW state

      printSerialln("GPIO01_U0TXD_LED_TX disabled and set as digital output.", 0);

    } else if (inputDigitalPinsB[i] == GPIO03_U0RXD_LED_RX) {
      printSerial(" => Setting up GPIO03_U0RXD_LED_RX as an output...");
      Serial1.end();  // Disable UART1
      delay(10);  // Allow time for UART1 to fully disable

      pinMode(inputDigitalPinsB[i], OUTPUT); // Set pin as OUTPUT
      delay(10);  // Small delay to ensure pinMode change is stable
      digitalWrite(inputDigitalPinsB[i], LOW); // Initialize to LOW
      delay(10);  // Small delay to stabilize the LOW state

      printSerialln("GPIO03_U0RXD_LED_RX disabled and set as digital output.", 0);
    }
    usePin(inputDigitalPinsA[i]); // Check for conflicts
    pinMode(inputDigitalPinsA[i], INPUT_PULLUP); // General configuration for other pins
    delay(10);  // Small delay after setting pin mode
    printSerial("Input pin ");
    printSerial(String(inputDigitalPinsA[i]));
    printSerialln(" set to INPUT_PULLUP.", 0);

  }
  printSerialln("<end> ." + String(NUM_DIGITAL_INPUTSA) + " A-Inputs Initialized.", 1000);
}

void initializeDigitalInputsB() {

  // Initialize input pins (B pins)
  for (int i = 0; i < NUM_DIGITAL_INPUTSB; i++) {
    if (inputDigitalPinsB[i] == GPIO16_U2RXD_WS2812_16) {
      Serial2.end();  // Disable UART2
      delay(10);  // Allow time for UART2 to fully disable

      pinMode(inputDigitalPinsB[i], INPUT_PULLUP); // Set RX pin to INPUT_PULLUP
      delay(10);  // Small delay after setting pin mode
      printSerialln("GPIO16_U2TXD disabled and set as digital input.", 0);

    } else if (inputDigitalPinsB[i] == GPIO17_U2TXD) {
      Serial1.end();  // Disable UART1
      delay(10);  // Allow time for UART1 to fully disable

      pinMode(inputDigitalPinsB[i], INPUT_PULLUP); // Set RX pin to INPUT_PULLUP
      delay(10);  // Small delay after setting pin mode
      printSerialln("RX_UART1 disabled and set as digital input.", 0);

    }  else if (inputDigitalPinsB[i] == GPIO03_U0RXD_LED_RX) {
      Serial1.end();  // Disable UART1
      delay(10);  // Allow time for UART1 to fully disable

      pinMode(inputDigitalPinsB[i], INPUT_PULLUP); // Set RX pin to INPUT_PULLUP
      delay(10);  // Small delay after setting pin mode
      printSerialln("RX_UART1 disabled and set as digital input.", 0);

    } else if (inputDigitalPinsB[i] == GPIO01_U0TXD_LED_TX) {
      Serial1.end();  // Disable UART1
      delay(10);  // Allow time for UART1 to fully disable

      pinMode(inputDigitalPinsB[i], INPUT_PULLUP); // Set RX pin to INPUT_PULLUP
      delay(10);  // Small delay after setting pin mode
      printSerialln("RX_UART1 disabled and set as digital input.", 0);
    }
    usePin(inputDigitalPinsB[i]); // Check for conflicts
    pinMode(inputDigitalPinsB[i], INPUT_PULLUP); // General configuration for other pins
    delay(10);  // Small delay after setting pin mode
    printSerial("Input pin ");
    printSerial(String(inputDigitalPinsB[i]));
    printSerialln(" set to INPUT_PULLUP.", 0);

  }
  printSerialln("<end> ." + String(NUM_DIGITAL_INPUTSB) + " B-Inputs Initialized.", 0);
  delay(1000);

}

void initializeAnalogInputPairs() {

  // Initialize analog input pairs (A and B)
  for (int i = 0; i < NUM_ANALOG_INPUTPAIRS; i++) {
    printSerial("Initializing analog input pair: ");
    printSerial(String(i));
    printSerialln("; ", 0);

    usePin(analogInputPinsA[i]); // Check for conflicts
    usePin(analogInputPinsB[i]); // Check for conflicts

    pinMode(analogInputPinsA[i], INPUT);  // Configure analog pin A as INPUT
    delay(10);  // Small delay after setting pin mode
    pinMode(analogInputPinsB[i], INPUT);  // Configure analog pin B as INPUT
    delay(10);  // Small delay after setting pin mode
    printSerial("Analog input pair ");
    printSerial(String(i));
    printSerialln(" initialized.", 0);
  }
  printSerialln("<end> ." + String(NUM_ANALOG_INPUTPAIRS) + " Analog Input Pairs Initialized!", 0);


}

void initializeFLEDOutputs() {

  // Initialize FLED addressable outputs (if applicable)
  for (int i = 0; i < NUM_FLED_OUTPUTS; i++) {
    usePin(outputFLEDPins[i]);
    printSerialln("Initializing FLED addressable output at Pin " + String(outputFLEDPins[i]), 0);

  }
  printSerialln("<end> ." + String(NUM_FLED_OUTPUTS) + " FLED Outputs Initialized!", 0);
}

void initializeRXTXOutputs() {
  // Initialize RXTX addressable outputs (if applicable)
  for (int i = 0; i < NUM_RXTX_PORTS; i++) {
    for (int t = 0; t < 2; t++) {
      usePin(RXTX_Pins[i][t]);
      printSerialln("Initializing RXTX addressable I/O at Pin " + String(RXTX_Pins[i][t]), 0);
    }
  }
  printSerialln("<end> ." + String(NUM_RXTX_PORTS) + " RXTX Ports Initialized!", 0);

}

void initializeI2CCom() {

  for (int i = 0; i < NUM_I2C_PORTS; i++) {

    const int SDA_PIN = I2C_Pins[i][1];
    const int SCL_PIN = I2C_Pins[i][0];

    //Serial1.end();  // Disable UART1
    //Serial2.end();  // Disable UART2
    delay(10);

    // Print initialization message
    printSerial("I2C initialized on SDA pin ");
    printSerial(String(SDA_PIN));
    printSerial(" and SCL pin ");
    printSerialln(String(SCL_PIN), 0);

    // Check for pin conflicts
    usePin(SDA_PIN);  // Ensure SDA pin is not conflicting
    usePin(SCL_PIN);  // Ensure SCL pin is not conflicting


    // Initialize I2C communication with custom SDA and SCL pins
    Wire.begin(SDA_PIN, SCL_PIN);
    scannerLoop();


  }

  printSerialln("<end> I2C Communication Initialized.", 0);

}


void initializeDigitalIOMatrix() {

  for (int i = 0; i < NUM_DIGITAL_IOMATRIXPAIRS; i++) {
    for (int r = 0; r < 2; r++) {
      if (digitalMatrixIOPins[r][i] == GPIO17_U2TXD) {
        printSerial(" => Setting up GPIO17_U2TXD as an output...");
        Serial2.end();  // Disable UART2
        delay(10);  // Allow time for UART2 to fully disable

        pinMode(digitalMatrixIOPins[r][i], OUTPUT); // Set pin as OUTPUT
        delay(10);  // Small delay to ensure pinMode change is stable
        digitalWrite(digitalMatrixIOPins[r][i], LOW); // Initialize to LOW
        delay(10);  // Small delay to stabilize the LOW state

        printSerialln("GPIO17_U2TXD disabled and set as digital output.", 0);

      } else if (digitalMatrixIOPins[r][i] == GPIO16_U2RXD_WS2812_16) {
        printSerial(" => Setting up GPIO16_U2RXD_WS2812_16 as an output...");
        Serial1.end();  // Disable UART1
        delay(10);  // Allow time for UART1 to fully disable

        pinMode(digitalMatrixIOPins[r][i], OUTPUT); // Set pin as OUTPUT
        delay(10);  // Small delay to ensure pinMode change is stable
        digitalWrite(digitalMatrixIOPins[r][i], LOW); // Initialize to LOW
        delay(10);  // Small delay to stabilize the LOW state

        printSerialln("GPIO16_U2RXD_WS2812_16 disabled and set as digital output.", 0);

      } else if (digitalMatrixIOPins[r][i] == GPIO01_U0TXD_LED_TX) {
        printSerial(" => Setting up GPIO01_U0TXD_LED_TX as an output...");
        Serial1.end();  // Disable UART1
        delay(10);  // Allow time for UART1 to fully disable

        pinMode(digitalMatrixIOPins[r][i], OUTPUT); // Set pin as OUTPUT
        delay(10);  // Small delay to ensure pinMode change is stable
        digitalWrite(digitalMatrixIOPins[r][i], LOW); // Initialize to LOW
        delay(10);  // Small delay to stabilize the LOW state

        printSerialln("GPIO01_U0TXD_LED_TX disabled and set as digital output.", 0);

      } else if (digitalMatrixIOPins[r][i] == GPIO03_U0RXD_LED_RX) {
        printSerial(" => Setting up GPIO03_U0RXD_LED_RX as an output...");
        Serial1.end();  // Disable UART1
        delay(10);  // Allow time for UART1 to fully disable

        pinMode(digitalMatrixIOPins[r][i], OUTPUT); // Set pin as OUTPUT
        delay(10);  // Small delay to ensure pinMode change is stable
        digitalWrite(digitalMatrixIOPins[r][i], LOW); // Initialize to LOW
        delay(10);  // Small delay to stabilize the LOW state

        printSerialln("GPIO03_U0RXD_LED_RX disabled and set as digital output.", 0);
      }
      usePin(digitalMatrixIOPins[r][i]); // Check for conflicts
      pinMode(digitalMatrixIOPins[r][i], INPUT_PULLUP); // General configuration for other pins
      delay(10);  // Small delay after setting pin mode
      printSerial("I/O pin ");
      printSerial(String(digitalMatrixIOPins[r][i]));
      printSerialln(" set to INPUT_PULLUP.", 0);
    }
  }


  // Initialize row and column pins
  for (int i = 0; i < NUM_DIGITAL_IOMATRIXPAIRS; i++) {
    pinMode(digitalMatrixIOPins[0][i], OUTPUT);  // Row pins as outputs
    pinMode(digitalMatrixIOPins[1][i], INPUT_PULLUP);  // Column pins as inputs with pull-ups
    digitalWrite(digitalMatrixIOPins[0][i], HIGH);  // Set all rows to HIGH initially (inactive)
  }
  printSerialln("<end> ." + String(NUM_DIGITAL_IOMATRIXPAIRS) + " Matrix I\O Initialized.", 1000);
}
