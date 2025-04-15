
String prepareBinaryResult(int dec) {
  
  // Convert the decimal number to a binary string
  String binaryResult = String(dec, BIN);

  // Print or use the binary result
  Serial.println(binaryResult);  // Example: print it to the serial monitor
  return binaryResult;
}

void checkForWin() {
  
    if (solutionFound) {
        if (!solutionStable) {
            solutionCheckStart = millis();
            solutionStable = true;
        }

        if (millis() - solutionCheckStart >= 2000) {
            executeFUNCBatchButton1_PIN(EXEC_BATCH1_PIN);
            solutionStable = false;
        }
    } else {
        solutionStable = false;
    }
}

void checkVersionWinScenario() {
  
  if (solutionFound && solutionStable) {
    if (SOLUTION_TYPE == "MQTT") {
      sendMessageMQTTPayload("Activate remote relay 2 > in " + String(NR_GROUP) + " (" + String(PUZZLE_NAME) + ")", String(SOLUTION_MQTT_MESSAGE));
    } else if (SOLUTION_TYPE == "RELAY") {
      executeFUNCBatchButton1_PIN(outputPinsB[0]);
    }
    delay(50);
    for (int i = 0; i < 3; i++) {
      update_pulseGPIOCount(i, 0);
    }
    gameOnFlag = false;
    restartFlag = false;
    solutionStable = false;
    solutionFound = false;
  }
}

void generateFUNCRandomSolution(int numbersolutions, int rangemax) {

  randomSeed(analogRead(0));
  printSerial("Solution: ");
  bool usedNumbers[numbersolutions] = {false};
  
  for (int i = 0; i < numbersolutions; i++) {
    int randNum;
    do {
      randNum = random(1, rangemax);
    } while (usedNumbers[randNum]); 
    solutionWin[i] = randNum;
    usedNumbers[randNum] = true;  
    printSerial(String(solutionWin[i]));
    printSerial(", ");
  }

  printSerialln("<end>", 0);
}

int getCount(int maxcount, int analoginput) {

    int TOT_RANGE = (PULSE_MAX_RANGE * (NUM_FLED_ADDLEDS / NUM_FLED_CHANNELS));
    int clampedpulseGPIOCount = constrain(pulseGPIOCount[analoginput], 0, TOT_RANGE);
    pulseGPIOCount[analoginput] = clampedpulseGPIOCount;
    return map(clampedpulseGPIOCount, 0, TOT_RANGE, 0, maxcount);
}

void executeFUNCBatchButton1_PIN(int pin) {
  
    Serial.println("Solution stable for 5 seconds. Executing GPIO1 batch 1 operation...");
    digitalWrite(pin, HIGH);
    delayESPTask(5000);
    for (int i = 0; i < 3; i++) {
        pulseGPIOCount[i] = 0;
    }
    gameOnFlag = false;
    restartFlag = false;
    digitalWrite(pin, LOW);
    Serial.println("Batch 1 completed... Relay 1 trigger; forced 'Game Win'");
}

void executeFUNCBatchButton2() {
  
    Serial.print("Batch 2 requested. Executing game start batch 2 operation...");
    delayESPTask(5000);
    gameOnFlag = false;
    restartFlag = true;
    Serial.println("Batch 2 completed... restarted game!");
}

void executeFUNCBatchButton3_PIN(int pin) {
  
    Serial.print("Batch 3 requested. Executing GPIO2 batch 3 operation...");
    digitalWrite(pin, LOW);
    delayESPTask(5000);
    digitalWrite(pin, HIGH);
    Serial.println("Batch 3 completed... Relay two trigger; held for 5 secs.");
}

void executeFUNCBatchMQTT(String activity) {
  
    Serial.println("Key 'activity': " + activity);
}
