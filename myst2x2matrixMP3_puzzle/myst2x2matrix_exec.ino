
void exec_2x2matrix_puzzle() {

  if (lastMatrixButtonPressed == 13) {
    restartFlag = true;
    gameOverFlag = false;
    gameOnFlag = false;
    gameOffFlag = false;
  }

  if (gameOnFlag || restartFlag) {

    if (restartFlag && !gameOnFlag) {
      //generateFUNCRandomSolution(NUM_OF_SOLUTIONS, (NUM_DIGITAL_IOMATRIXPAIRS * NUM_DIGITAL_IOMATRIXPAIRS));
      gameOnFlag = true;
      restartFlag = false;
      gameOverFlag = false;
      gameOffFlag = false;

      //resetFLED();
      loopI2C(0);

      Serial.println("Game is set!");
      sendMessageMQTTPayload("Game is set!", NR_GROUP);
    }

    // check buttons against solution

    if (gameOnFlag) {

      if (lastMatrixButtonPressed == solutionWin[0] && solutionSeq < 0 ) {
        setFLEDPattern(SCANNER, 25);
        solutionSeq = 1;

        String msg = ("Solution started at position: " + String(solutionSeq));
        printSerialln(msg, 0);
        sendMessageMQTTPayload(msg, NR_GROUP);

        playMP3(lastMatrixButtonPressed);
        lastMatrixButtonPressed = -lastMatrixButtonPressed;
        setFLEDPattern(RAINBOW, 25);


      } else if (solutionWin[solutionSeq] == lastMatrixButtonPressed && lastMatrixButtonPressed > 0) {

        solutionSeq++;
        String msg = "Solution position: " + String(solutionSeq);
        printSerialln(msg, 0);
        sendMessageMQTTPayload(msg, NR_GROUP);

        playMP3(lastMatrixButtonPressed);
        lastMatrixButtonPressed = -lastMatrixButtonPressed;

        if (solutionSeq == NUM_OF_SOLUTIONS) {
          solutionFound = true;
          playMP3(MP3_WIN_FILENO);
          for (int d=0; d<5000; d++) {
          setFLEDPattern(STROBE, 25);
          }

        }

      } else if (lastMatrixButtonPressed != solutionWin[0] && lastMatrixButtonPressed > 0 && solutionSeq > 1 ) {

        String msg = "Wrong button pressed! Restart sequence.";
        printSerialln(msg, 0);
        sendMessageMQTTPayload(msg, NR_GROUP);
        solutionSeq = -1;
        lastMatrixButtonPressed = -lastMatrixButtonPressed;
        playMP3(MP3_FAIL_FILENO);
        setFLEDPattern(SCANNER, 25);


      }  else if (lastMatrixButtonPressed == solutionWin[0]) {

        solutionSeq = 1;

        String msg = "Solution re-started at position: " + String(solutionSeq);
        printSerialln(msg, 0);
        sendMessageMQTTPayload(msg, NR_GROUP);

        playMP3(lastMatrixButtonPressed);
        lastMatrixButtonPressed = -lastMatrixButtonPressed;
        setFLEDPattern(RAINBOW_CYCLE, 50);

      } else if (lastMatrixButtonPressed > 0) {
        String msg = "Wrong button pressed! Restart sequence.";
        printSerialln(msg, 0);
        sendMessageMQTTPayload(msg, NR_GROUP);
        solutionSeq = -1;
        lastMatrixButtonPressed = -lastMatrixButtonPressed;
        playMP3(MP3_FAIL_FILENO);
        setFLEDPattern(RANDOM_FILL, 25);


      }

      checkForWin();
    }

    // Update LED states only if there's a change

  } else if (!restartFlag && !gameOverFlag) {
    gameOverFlag = true;
    setFLEDPattern(RAINBOW_GLITTER, 50);

    // Set final LED counts to solution

    String msg = "Game is over.";
    printSerialln(msg, 0);
    sendMessageMQTTPayload(msg, NR_GROUP);
  } else if (!gameOffFlag) {

    gameOffFlag = true;
    Serial.println("Game if off!");
    String msg = "Game is off.";
    printSerialln(msg, 0);
    sendMessageMQTTPayload(msg, NR_GROUP);
    setFLEDPattern(NONE, 50);
    updateFLED_clearAll();

    // No action
  }
}
