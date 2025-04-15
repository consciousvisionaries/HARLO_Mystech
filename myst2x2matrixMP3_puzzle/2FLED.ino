#include <FastLED.h>

#define FASTLED_VERSION V1.1

CRGB leds[NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2];

void setupFASTLED_GPIO() {
  
    if (NUM_FLED_OUTPUTS >= 1) {
        FastLED.addLeds<WS2812, FLED_PIN1, RGB>(leds, NUM_FLED_ADDLEDS);
        FastLED.clear();
        FastLED.show();
        Serial.println("<end> FAST LED Initialized!");
    }
    if (NUM_FLED_OUTPUTS >=2) {
        FastLED.addLeds<WS2812, FLED_PIN2, RGB>(leds, NUM_FLED_ADDLEDS2);
        FastLED.clear();
        FastLED.show();
        Serial.println("<end> FAST LED 2 Initialized!");
    }
}

void updateFLED_address(int address, String color) {
  if (color == "red") {
    leds[address] = CRGB::Red; 
  } else if (color == "blue") {
    leds[address] = CRGB::Blue; 
  } else if (color == "green") {
    leds[address] = CRGB::Green;
  } else if (color == "orange") {
    leds[address] = CRGB::Orange;
  }
}

void resetFLED() {
  // Reset last LED counts
            for (int i = 0; i < NUM_FLED_CHANNELS; i++) {
                lastLedCount[i] = -1;
                ledCount[i] = -1;
            }
}


void updateFLED_clearAll() {
  fill_solid(leds, NUM_FLED_ADDLEDS, CRGB::Black);
    FastLED.show();

}

void updateFLED_show() {
  FastLED.show();
}






PatternType currentPattern = SPARKLE;

unsigned long lastUpdate = 0;
int patternIndex = 0;
int patternSpeed = 50; // ms between frames

// Set pattern
void setFLEDPattern(PatternType pattern, int speed = 50) {
  currentPattern = pattern;
  patternSpeed = speed;
  patternIndex = 0;
}

// Pattern: Chase
void patternChase() {
  fill_solid(leds, NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2, CRGB::Black);
  leds[patternIndex % (NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2)] = CRGB::White;
  patternIndex++;
  FastLED.show();
}

// Pattern: Color Wipe
void patternColorWipe(CRGB color = CRGB::Blue) {
  if (patternIndex < NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2) {
    leds[patternIndex] = color;
    patternIndex++;
    FastLED.show();
  } else {
    // Reset or loop?
    patternIndex = 0;
    fill_solid(leds, NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2, CRGB::Black);
  }
}

// Pattern: Rainbow
void patternRainbow() {
  fill_rainbow(leds, NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2, patternIndex, 7);
  FastLED.show();
  patternIndex++;
}

void patternTheaterChase() {
  fill_solid(leds, NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2, CRGB::Black);
  for (int i = patternIndex % 3; i < NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2; i += 3) {
    leds[i] = CRGB::Purple;
  }
  FastLED.show();
  patternIndex++;
}

void patternFlash() {
  CRGB color = (patternIndex % 2 == 0) ? CRGB::White : CRGB::Black;
  fill_solid(leds, NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2, color);
  FastLED.show();
  patternIndex++;
}

void patternStrobe() {
  fill_solid(leds, NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2, CRGB::White);
  FastLED.show();
  delay(50);
  fill_solid(leds, NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2, CRGB::Black);
  FastLED.show();
  delay(150);
}

void patternBounce() {
  fill_solid(leds, NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2, CRGB::Black);
  int pos = abs(patternIndex % ((NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2) * 2) - (NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2));
  leds[pos] = CRGB::Cyan;
  FastLED.show();
  patternIndex++;
}

void patternBreath() {
  float brightness = (sin(patternIndex * 0.1) + 1.0) * 127.5;
  fill_solid(leds, NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2, CRGB(brightness, 0, brightness));
  FastLED.show();
  patternIndex++;
}

void patternFire() {
  for (int i = 0; i < NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2; i++) {
    byte heat = random(160, 255);
    leds[i] = CRGB(heat, heat / 2, 0);
  }
  FastLED.show();
}

void patternConfetti() {
  fadeToBlackBy(leds, NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2, 10);
  int pos = random(NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2);
  leds[pos] += CHSV(random8(), 200, 255);
  FastLED.show();
}

void patternCylon() {
  static int direction = 1;
  fill_solid(leds, NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2, CRGB::Black);
  leds[patternIndex] = CRGB::Red;
  FastLED.show();
  patternIndex += direction;
  if (patternIndex == 0 || patternIndex == (NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2 - 1)) direction *= -1;
}

void patternScanner() {
  fill_solid(leds, NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2, CRGB::Black);
  leds[patternIndex] = CRGB::Green;
  FastLED.show();
  patternIndex = (patternIndex + 1) % (NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2);
}

void patternSparkle() {
  int pos = random(NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2);
  leds[pos] = CRGB::White;
  FastLED.show();
  delay(10);
  leds[pos] = CRGB::Black;
}

void patternPulse() {
  uint8_t level = beatsin8(2, 0, 255);
  fill_solid(leds, NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2, CRGB(level, 0, level));
  FastLED.show();
}

void patternRainbowCycle() {
  for (int i = 0; i < NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2; i++) {
    leds[i] = CHSV((i * 256 / (NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2) + patternIndex) & 255, 255, 255);
  }
  FastLED.show();
  patternIndex++;
}

void patternRainbowGlitter() {
  fill_rainbow(leds, NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2, patternIndex, 7);
  if (random8() < 80) leds[random(NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2)] += CRGB::White;
  FastLED.show();
  patternIndex++;
}

void patternTwinkle() {
  for (int i = 0; i < NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2; i++) {
    leds[i] = (random8() < 30) ? CRGB::White : CRGB::Black;
  }
  FastLED.show();
}

void patternRandomFill() {
  if (patternIndex < NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2) {
    leds[patternIndex] = CHSV(random8(), 255, 255);
    FastLED.show();
    patternIndex++;
  } else {
    patternIndex = 0;
    fill_solid(leds, NUM_FLED_ADDLEDS + NUM_FLED_ADDLEDS2, CRGB::Black);
  }
}


void loopFASTLED() {
  if (millis() - lastUpdate < patternSpeed) return;
  lastUpdate = millis();

  switch (currentPattern) {
    case CHASE: patternChase(); break;
    case COLOR_WIPE: patternColorWipe(); break;
    case RAINBOW: patternRainbow(); break;
    case THEATER_CHASE: patternTheaterChase(); break;
    case FLASH: patternFlash(); break;
    case STROBE: patternStrobe(); break;
    case BOUNCE: patternBounce(); break;
    case BREATH: patternBreath(); break;
    case FIRE: patternFire(); break;
    case CONFETTI: patternConfetti(); break;
    case CYLON: patternCylon(); break;
    case SCANNER: patternScanner(); break;
    case SPARKLE: patternSparkle(); break;
    case PULSE: patternPulse(); break;
    case RAINBOW_CYCLE: patternRainbowCycle(); break;
    case RAINBOW_GLITTER: patternRainbowGlitter(); break;
    case TWINKLE: patternTwinkle(); break;
    case RANDOM_FILL: patternRandomFill(); break;
    default: break;
  }
}
