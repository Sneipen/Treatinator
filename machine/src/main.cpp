#include <Stepper.h>
#include <FastLED.h>
#include <IRremote.h>
#include <EEPROM.h>
#include "DFRobotDFPlayerMini.h"

// Remote buttons
const uint32_t gardin = 0xDF20FB04;
const uint32_t red = 0x8D72FB04;
const uint32_t green = 0x8E71FB04;
const uint32_t yellow = 0x9C63FB04;
const uint32_t blue = 0x9E61FB04;

const int receiverPin = 36;

// Defines the number of steps per rotation
const int stepsPerRevolution = 2048;

// Sensor pins for detecting aluminum tape
const int sensorPins[4] = {52, 50, 48, 46};

// Stepper motor pins for each conveyor
Stepper steppers[4] = {    // IN1, IN3, IN2, IN4
    Stepper(stepsPerRevolution, 47, 51, 49, 53),  // Stepper 1 - 47,49,51,53
    Stepper(stepsPerRevolution, 23, 27, 25, 29),  // Stepper 2 - 23,25,27,29
    Stepper(stepsPerRevolution, 39, 43, 41, 45), // Stepper 3 - 39,41,43,45
    Stepper(stepsPerRevolution, 31, 35, 33, 37)  // Stepper 4 - 31,33,35,37
};

// LED settings
#define NUM_LEDS 13
#define BRIGHTNESS 75
#define FADE_RATE 180           // Higher = Faster fade (0-255)
CRGB leds[4][NUM_LEDS];
const int ledPins[4] = {44, 42, 40, 38}; // Pins for LED strips
const CRGB conveyorColors[4] = {CRGB::Red, CRGB::Green, CRGB::Gold, CRGB::Blue};
static uint8_t hueBase = 0;              // base hue for our rainbow
static int currentFrame = 0;

CRGBPalette16 currentPalette = RainbowColors_p;
uint8_t previousLedsToLight = 0;
float currentDisplayedLEDs = 0.0;

// Timing variables
unsigned long previousMillis[4] = {0, 0, 0, 0};
const unsigned long ledInterval = 30; // Milliseconds between LED updates

DFRobotDFPlayerMini player;
const int num_updates = 434;
const uint8_t lastTrackNumber = 11;
const int trackNumberAddr = 0;
int trackNumber = 1;

const uint8_t tracks[lastTrackNumber][num_updates] PROGMEM = {
  {1, 3, 8, 4, 5, 7, 7, 6, 6, 6, 6, 6, 6, 6, 5, 4, 4, 4, 3, 4, 4, 4, 6, 11, 7, 6, 5, 5, 4, 4, 4, 5, 6, 6, 6, 4, 3, 3, 3, 5, 6, 5, 5, 4, 9, 5, 5, 5, 4, 3, 7, 6, 6, 6, 4, 3, 4, 4, 6, 7, 5, 6, 7, 4, 8, 4, 4, 8, 7, 6, 7, 5, 3, 3, 3, 3, 6, 7, 5, 9, 6, 7, 6, 5, 6, 8, 6, 5, 5, 5, 5, 3, 4, 4, 6, 7, 6, 5, 5, 4, 8, 4, 9, 7, 5, 5, 6, 6, 5, 4, 5, 6, 6, 6, 6, 5, 7, 9, 4, 5, 10, 13, 12, 13, 13, 11, 8, 7, 8, 10, 10, 9, 10, 9, 10, 13, 10, 10, 11, 12, 12, 13, 12, 10, 7, 4, 6, 8, 7, 6, 6, 5, 5, 9, 5, 6, 11, 13, 13, 13, 12, 9, 6, 4, 5, 9, 10, 10, 10, 9, 10, 13, 10, 11, 12, 12, 13, 12, 12, 9, 6, 4, 6, 8, 8, 8, 7, 4, 7, 3, 3, 8, 13, 13, 13, 13, 11, 8, 5, 6, 10, 10, 10, 9, 10, 8, 10, 4, 4, 10, 12, 12, 13, 12, 11, 8, 5, 4, 7, 11, 10, 11, 9, 10, 13, 9, 10, 11, 13, 12, 13, 12, 11, 7, 4, 7, 9, 10, 10, 10, 9, 13, 10, 10, 10, 12, 12, 12, 12, 13, 9, 7, 5, 5, 10, 10, 10, 11, 10, 13, 10, 9, 11, 12, 13, 13, 13, 12, 10, 7, 7, 10, 10, 10, 10, 10, 10, 13, 10, 10, 10, 12, 12, 13, 13, 11, 9, 7, 6, 8, 7, 6, 8, 6, 5, 10, 5, 6, 9, 13, 12, 13, 12, 11, 8, 5, 4, 9, 11, 10, 10, 9, 9, 13, 10, 10, 11, 12, 12, 12, 13, 11, 9, 7, 6, 6, 5, 8, 7, 7, 10, 6, 7, 6, 11, 13, 12, 13, 12, 11, 9, 8, 9, 10, 10, 10, 10, 10, 12, 5, 6, 7, 11, 12, 12, 13, 12, 10, 8, 7, 7, 10, 10, 10, 10, 9, 13, 10, 10, 10, 12, 13, 13, 12, 12, 9, 7, 6, 7, 11, 10, 10, 10, 10, 13, 9, 10, 10, 12, 12, 13, 13, 12, 10, 6, 7, 7, 7, 6, 6, 5, 4, 8, 4, 5, 10, 13, 13, 13, 13, 11, 9, 7, 9, 10, 10, 9, 10, 10, 13, 9, 10, 10, 11, 12, 12, 13, 12, 11, 8, 6, 7, 8, 8},
  {1, 7, 3, 3, 3, 3, 3, 3, 6, 7, 5, 5, 6, 7, 6, 5, 6, 8, 5, 6, 7, 6, 5, 5, 6, 5, 5, 5, 4, 5, 5, 5, 6, 6, 5, 8, 5, 4, 5, 6, 3, 3, 2, 2, 2, 10, 13, 13, 12, 9, 8, 9, 11, 10, 13, 10, 9, 10, 9, 10, 10, 9, 9, 12, 12, 13, 11, 8, 6, 7, 9, 13, 10, 9, 9, 9, 9, 8, 7, 7, 7, 13, 12, 12, 10, 7, 5, 8, 8, 8, 10, 9, 12, 8, 8, 8, 8, 8, 9, 13, 12, 11, 9, 6, 7, 8, 8, 9, 9, 10, 13, 9, 8, 9, 8, 8, 12, 11, 12, 11, 9, 6, 6, 7, 8, 9, 8, 7, 11, 7, 7, 7, 7, 7, 13, 12, 12, 11, 7, 6, 7, 8, 10, 11, 11, 10, 13, 9, 9, 9, 8, 7, 13, 12, 12, 10, 8, 7, 9, 10, 10, 11, 10, 13, 8, 8, 9, 8, 8, 10, 13, 12, 11, 9, 6, 6, 7, 8, 9, 10, 9, 13, 9, 9, 9, 8, 8, 12, 12, 12, 11, 9, 6, 8, 9, 9, 10, 9, 10, 13, 9, 8, 8, 8, 8, 13, 12, 12, 10, 7, 6, 6, 7, 7, 9, 8, 8, 12, 8, 8, 8, 8, 8, 13, 12, 12, 10, 6, 6, 7, 8, 8, 10, 8, 8, 11, 8, 7, 7, 7, 10, 13, 12, 11, 8, 5, 6, 6, 7, 8, 9, 8, 8, 12, 8, 7, 7, 7, 12, 12, 12, 10, 8, 5, 5, 7, 6, 8, 8, 8, 12, 7, 7, 7, 7, 6, 13, 12, 12, 10, 7, 6, 5, 7, 7, 9, 8, 7, 7, 11, 7, 7, 7, 10, 13, 12, 12, 10, 7, 9, 8, 8, 9, 10, 9, 9, 13, 10, 9, 9, 9, 12, 12, 12, 11, 8, 7, 5, 7, 8, 10, 10, 9, 9, 13, 9, 9, 10, 10, 12, 11, 13, 11, 8, 8, 10, 10, 11, 10, 10, 10, 9, 13, 10, 9, 10, 8, 13, 12, 12, 10, 7, 8, 11, 11, 10, 10, 10, 10, 10, 13, 9, 9, 9, 10, 13, 12, 11, 9, 6, 5, 7, 9, 9, 10, 9, 9, 8, 13, 8, 8, 8, 12, 12, 12, 11, 8, 6, 7, 8, 9, 8, 9, 9, 9, 8, 13, 9, 7, 8, 13, 12, 12, 10, 8, 7, 8, 8, 8, 9, 8, 7, 8, 12, 7, 8, 8, 9, 13, 12, 12, 10, 6, 6, 7, 7, 9, 9, 9, 8, 8, 11, 8, 8},
  {2, 6, 3, 4, 2, 2, 2, 2, 2, 1, 2, 3, 3, 3, 2, 3, 3, 3, 3, 3, 9, 12, 10, 13, 13, 11, 6, 6, 6, 10, 11, 10, 10, 9, 9, 8, 7, 7, 10, 12, 10, 13, 13, 11, 8, 5, 6, 12, 10, 8, 6, 6, 7, 6, 6, 6, 11, 12, 10, 13, 13, 8, 6, 4, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 12, 11, 11, 13, 12, 7, 6, 5, 9, 12, 13, 11, 10, 10, 9, 9, 9, 9, 12, 11, 11, 13, 10, 7, 4, 5, 9, 12, 11, 9, 9, 8, 7, 6, 6, 9, 12, 10, 12, 13, 11, 7, 6, 5, 10, 10, 9, 8, 7, 7, 7, 6, 6, 10, 11, 10, 13, 11, 8, 6, 4, 5, 8, 7, 8, 8, 8, 7, 8, 7, 9, 11, 10, 10, 13, 11, 7, 4, 4, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 12, 11, 10, 13, 10, 8, 5, 5, 9, 12, 11, 11, 10, 9, 8, 7, 7, 9, 12, 10, 12, 13, 10, 7, 5, 5, 8, 10, 10, 9, 7, 6, 5, 5, 5, 9, 12, 9, 12, 13, 9, 5, 3, 4, 6, 6, 6, 6, 8, 10, 10, 9, 9, 11, 11, 10, 13, 13, 8, 5, 5, 7, 10, 9, 8, 7, 7, 7, 8, 8, 9, 11, 12, 10, 13, 12, 8, 5, 5, 8, 10, 9, 8, 8, 7, 7, 6, 6, 8, 12, 10, 11, 13, 11, 9, 5, 5, 8, 10, 8, 8, 7, 6, 6, 5, 6, 10, 12, 11, 11, 13, 9, 6, 4, 6, 8, 8, 8, 8, 8, 8, 9, 8, 8, 11, 11, 10, 12, 13, 9, 6, 4, 7, 9, 9, 8, 8, 8, 8, 8, 7, 7, 11, 11, 10, 13, 13, 8, 5, 3, 6, 10, 10, 8, 7, 6, 6, 5, 5, 7, 12, 11, 11, 13, 13, 9, 5, 5, 7, 9, 9, 9, 8, 7, 8, 7, 7, 9, 12, 10, 11, 13, 10, 6, 5, 6, 9, 9, 9, 9, 8, 8, 8, 8, 8, 10, 11, 9, 12, 13, 10, 5, 4, 5, 9, 9, 9, 9, 8, 7, 7, 6, 5, 11, 12, 10, 12, 13, 8, 5, 3, 4, 9, 9, 7, 7, 6, 5, 6, 5, 7, 12, 11, 11, 12, 13, 9, 7, 6, 8, 10, 9, 9, 9, 9, 8, 8, 8, 9, 12, 9, 10, 11, 13, 7, 6, 6, 9, 9, 9, 9, 9, 9, 9, 8, 8, 10, 12, 9, 11, 2},
  {5, 8, 5, 6, 5, 4, 4, 4, 4, 4, 9, 7, 8, 7, 10, 13, 10, 8, 7, 5, 4, 3, 2, 7, 8, 8, 7, 6, 6, 10, 6, 5, 10, 11, 11, 8, 7, 5, 5, 4, 5, 5, 5, 4, 4, 4, 8, 3, 3, 4, 10, 11, 9, 8, 7, 5, 3, 2, 4, 7, 8, 7, 7, 7, 10, 6, 5, 7, 13, 13, 8, 9, 9, 6, 7, 4, 5, 5, 5, 5, 5, 8, 13, 9, 9, 8, 10, 9, 9, 7, 6, 4, 3, 2, 4, 8, 7, 7, 6, 6, 9, 6, 6, 10, 10, 11, 8, 7, 6, 4, 4, 5, 6, 6, 6, 5, 4, 4, 4, 8, 3, 10, 11, 10, 8, 7, 5, 3, 3, 2, 3, 5, 5, 5, 5, 5, 9, 5, 5, 11, 11, 12, 10, 9, 9, 7, 6, 6, 6, 5, 5, 4, 7, 12, 8, 7, 7, 10, 11, 9, 7, 6, 4, 3, 2, 3, 9, 7, 8, 6, 7, 10, 6, 5, 8, 10, 11, 9, 7, 6, 4, 3, 4, 5, 8, 7, 7, 6, 5, 10, 6, 4, 8, 11, 11, 9, 7, 6, 4, 3, 1, 7, 8, 8, 8, 7, 7, 10, 6, 7, 9, 11, 11, 9, 7, 6, 7, 6, 7, 7, 8, 7, 6, 7, 13, 8, 7, 7, 11, 11, 10, 8, 7, 5, 3, 2, 2, 7, 10, 9, 8, 7, 6, 10, 5, 5, 12, 11, 9, 8, 7, 5, 4, 4, 4, 7, 5, 6, 7, 6, 4, 7, 3, 6, 10, 11, 9, 7, 6, 5, 4, 4, 5, 6, 5, 5, 5, 4, 3, 8, 5, 9, 11, 11, 9, 8, 6, 6, 7, 6, 6, 6, 5, 6, 6, 9, 13, 8, 8, 9, 11, 12, 8, 7, 5, 4, 3, 2, 6, 8, 7, 7, 7, 6, 10, 5, 5, 11, 10, 10, 8, 7, 6, 4, 4, 4, 7, 6, 5, 5, 4, 4, 8, 4, 4, 10, 11, 9, 8, 6, 4, 4, 4, 4, 8, 9, 8, 8, 7, 7, 10, 6, 7, 12, 10, 13, 8, 9, 7, 6, 6, 5, 5, 5, 5, 5, 6, 8, 11, 7, 8, 11, 11, 9, 7, 6, 4, 3, 2, 4, 9, 9, 8, 8, 7, 11, 7, 7, 9, 10, 10, 9, 7, 6, 4, 4, 3, 6, 7, 6, 5, 4, 4, 8, 4, 3, 9, 11, 10, 8, 7, 5, 4, 3, 2, 4, 5, 5, 5, 5, 4, 8, 4, 5, 11, 11, 11, 9, 7, 5, 4, 5, 5, 4},
  {3, 6, 8, 3, 3, 1, 2, 2, 2, 2, 3, 3, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 3, 2, 2, 2, 2, 2, 3, 4, 8, 4, 4, 4, 4, 4, 4, 4, 4, 11, 12, 10, 11, 8, 6, 10, 7, 7, 9, 9, 8, 8, 8, 7, 7, 7, 9, 13, 10, 10, 9, 5, 7, 3, 4, 5, 6, 5, 5, 6, 8, 9, 7, 8, 12, 10, 10, 9, 6, 8, 4, 5, 6, 8, 8, 7, 8, 8, 9, 9, 8, 12, 11, 10, 10, 8, 4, 7, 4, 4, 6, 6, 5, 6, 5, 5, 5, 5, 9, 13, 9, 10, 9, 5, 8, 4, 4, 6, 8, 8, 8, 7, 7, 7, 7, 7, 13, 10, 10, 10, 6, 7, 3, 3, 4, 5, 5, 5, 5, 7, 8, 8, 7, 12, 11, 10, 10, 8, 6, 9, 5, 5, 8, 9, 8, 8, 8, 9, 9, 8, 10, 12, 9, 10, 9, 5, 8, 5, 5, 7, 10, 9, 8, 8, 7, 8, 7, 8, 13, 10, 11, 10, 7, 4, 10, 7, 7, 9, 7, 7, 7, 7, 7, 7, 6, 12, 11, 10, 10, 7, 4, 7, 4, 4, 6, 5, 6, 6, 5, 5, 5, 5, 10, 13, 10, 11, 9, 5, 8, 4, 5, 7, 8, 8, 7, 6, 7, 7, 7, 8, 13, 9, 10, 9, 6, 3, 8, 4, 4, 6, 6, 6, 5, 5, 5, 4, 4, 12, 11, 11, 10, 7, 4, 7, 3, 4, 8, 7, 8, 7, 8, 8, 7, 6, 11, 12, 10, 11, 8, 5, 9, 6, 7, 7, 8, 7, 7, 7, 7, 5, 4, 6, 13, 10, 10, 10, 5, 4, 9, 6, 7, 7, 5, 4, 4, 5, 5, 5, 4, 12, 11, 11, 10, 7, 4, 8, 4, 4, 8, 8, 8, 8, 8, 8, 7, 7, 11, 12, 10, 11, 8, 5, 4, 10, 6, 8, 7, 7, 7, 7, 7, 6, 6, 7, 13, 11, 10, 9, 5, 3, 7, 4, 4, 6, 5, 5, 4, 7, 7, 7, 7, 13, 10, 11, 10, 7, 4, 4, 9, 6, 7, 8, 8, 7, 7, 8, 7, 7, 11, 12, 10, 11, 8, 4, 3, 7, 3, 6, 7, 5, 5, 4, 4, 4, 4, 7, 13, 11, 10, 9, 5, 3, 8, 4, 6, 8, 8, 8, 7, 7, 7, 7, 6, 13, 11, 11, 10, 6, 3, 3, 7, 3, 6, 6, 6, 5, 6, 7, 8, 7, 11, 12, 10, 10, 8, 5, 5, 9, 5, 7, 8, 7, 7, 5},
  {3, 10, 5, 5, 4, 4, 5, 4, 3, 4, 4, 5, 5, 5, 4, 3, 4, 3, 3, 3, 3, 3, 4, 9, 5, 5, 4, 5, 6, 5, 5, 4, 4, 5, 5, 5, 4, 4, 3, 4, 4, 4, 4, 3, 4, 10, 6, 5, 4, 4, 5, 5, 5, 5, 4, 4, 4, 4, 3, 3, 2, 3, 3, 3, 2, 2, 3, 8, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 9, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 1, 5, 13, 13, 11, 9, 6, 5, 4, 5, 5, 5, 5, 7, 6, 6, 6, 6, 7, 8, 7, 6, 6, 6, 13, 10, 10, 8, 6, 5, 5, 4, 5, 6, 6, 8, 7, 6, 5, 5, 5, 7, 6, 6, 6, 6, 10, 13, 9, 8, 6, 5, 3, 4, 4, 5, 5, 8, 8, 7, 7, 6, 7, 7, 8, 8, 6, 5, 10, 13, 9, 8, 6, 5, 4, 4, 5, 5, 5, 6, 7, 6, 4, 4, 4, 6, 6, 6, 6, 6, 9, 13, 8, 8, 7, 5, 4, 4, 5, 6, 6, 7, 7, 7, 6, 6, 7, 7, 7, 7, 6, 6, 8, 11, 13, 8, 6, 5, 5, 5, 5, 5, 5, 7, 7, 6, 5, 5, 5, 5, 6, 6, 5, 6, 7, 11, 12, 8, 7, 5, 4, 3, 4, 5, 5, 7, 7, 7, 7, 7, 7, 6, 7, 6, 6, 6, 8, 11, 12, 8, 7, 6, 4, 4, 4, 5, 5, 7, 7, 7, 7, 6, 6, 6, 5, 5, 5, 5, 6, 11, 13, 8, 7, 5, 4, 4, 4, 4, 5, 5, 7, 6, 6, 6, 6, 7, 7, 7, 7, 6, 7, 11, 13, 8, 7, 5, 5, 5, 5, 5, 6, 6, 8, 7, 6, 5, 5, 6, 6, 6, 6, 6, 6, 10, 9, 12, 7, 6, 3, 4, 4, 5, 5, 5, 8, 8, 7, 6, 6, 6, 8, 7, 6, 6, 5, 10, 9, 12, 7, 6, 5, 5, 5, 5, 5, 5, 6, 6, 5, 5, 5, 5, 6, 5, 5, 5, 5, 9, 9, 12, 7, 6, 5, 4, 3, 4, 4, 4, 7, 6, 6, 6, 5, 7, 7, 6, 7, 6, 6, 9, 10, 12, 7, 6, 5, 5, 5, 5, 6, 6, 8, 7, 7, 6, 6, 6, 7, 6, 7, 6, 7, 9, 10, 8, 12, 7, 5, 5, 5, 5, 5, 5, 7, 8, 8},
  {3, 6, 10, 6, 7, 6, 6, 6, 6, 6, 5, 6, 9, 10, 11, 11, 7, 6, 6, 6, 5, 6, 6, 6, 6, 10, 6, 6, 6, 6, 5, 4, 5, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 6, 4, 4, 4, 5, 4, 4, 4, 3, 3, 2, 2, 2, 2, 3, 3, 3, 7, 4, 4, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 6, 2, 2, 2, 2, 2, 2, 1, 2, 3, 4, 3, 4, 3, 3, 3, 3, 3, 7, 3, 3, 3, 3, 3, 3, 3, 9, 12, 12, 12, 10, 9, 8, 9, 8, 8, 9, 12, 7, 7, 6, 7, 6, 7, 6, 12, 12, 13, 10, 7, 6, 7, 7, 7, 9, 9, 13, 10, 10, 9, 10, 9, 9, 11, 11, 12, 12, 8, 7, 8, 6, 5, 7, 8, 13, 10, 9, 9, 9, 9, 9, 8, 12, 12, 13, 8, 6, 5, 5, 6, 6, 9, 9, 13, 10, 10, 10, 9, 9, 9, 11, 12, 13, 10, 9, 8, 8, 8, 8, 8, 9, 8, 7, 11, 7, 7, 7, 6, 8, 12, 11, 12, 8, 6, 5, 5, 5, 6, 9, 10, 10, 9, 13, 9, 9, 8, 8, 11, 13, 13, 10, 7, 7, 6, 6, 5, 6, 8, 8, 8, 8, 12, 7, 6, 6, 9, 12, 13, 11, 9, 9, 9, 9, 9, 8, 9, 9, 8, 7, 11, 6, 7, 7, 7, 11, 12, 12, 8, 8, 9, 8, 8, 7, 8, 8, 9, 7, 7, 11, 6, 6, 6, 10, 12, 12, 11, 7, 5, 5, 5, 5, 6, 8, 9, 9, 8, 13, 8, 8, 7, 8, 13, 12, 12, 8, 6, 5, 6, 6, 5, 6, 7, 7, 6, 6, 10, 6, 5, 5, 11, 12, 12, 9, 7, 8, 8, 7, 8, 9, 9, 10, 10, 10, 13, 9, 9, 8, 9, 13, 11, 12, 10, 8, 9, 9, 8, 9, 8, 8, 6, 5, 5, 10, 6, 6, 6, 11, 12, 12, 10, 7, 6, 5, 6, 6, 8, 9, 10, 9, 9, 13, 9, 8, 8, 10, 12, 12, 11, 7, 6, 5, 5, 5, 6, 9, 10, 9, 8, 8, 12, 8, 7, 8, 7, 6, 6, 6, 6, 6, 6, 6, 6, 8, 9, 9, 8, 8, 12, 8, 8, 7, 11, 12, 12, 12, 10, 9, 9, 9, 8, 8, 8, 8, 6, 5, 6, 9, 6, 6, 7, 12, 12, 12, 9, 7, 6, 7, 7, 7, 9, 9, 10, 9, 9, 9, 12},
  {2, 9, 4, 4, 4, 4, 4, 3, 4, 4, 4, 4, 4, 4, 4, 6, 7, 7, 10, 6, 7, 6, 6, 6, 6, 4, 4, 4, 4, 5, 6, 6, 5, 6, 6, 6, 6, 5, 6, 9, 5, 5, 5, 5, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 3, 3, 3, 2, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 8, 4, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 6, 2, 1, 2, 2, 3, 9, 11, 13, 12, 9, 7, 8, 8, 8, 8, 8, 7, 7, 7, 7, 13, 11, 10, 10, 10, 11, 13, 12, 10, 6, 7, 7, 7, 7, 11, 11, 10, 10, 8, 10, 12, 8, 7, 9, 12, 13, 12, 9, 7, 7, 8, 8, 8, 8, 8, 6, 6, 8, 12, 13, 11, 10, 10, 12, 12, 12, 10, 6, 7, 6, 6, 8, 10, 10, 10, 9, 10, 9, 13, 8, 8, 10, 13, 13, 10, 9, 7, 8, 8, 8, 7, 8, 7, 7, 9, 8, 13, 10, 9, 9, 10, 12, 13, 11, 9, 6, 6, 6, 7, 8, 13, 10, 10, 10, 9, 8, 8, 7, 7, 9, 13, 12, 10, 8, 9, 10, 10, 11, 10, 9, 13, 8, 8, 8, 7, 7, 7, 8, 10, 13, 12, 11, 7, 7, 7, 8, 8, 10, 13, 9, 9, 9, 9, 8, 8, 7, 9, 10, 13, 13, 11, 8, 8, 8, 8, 8, 8, 8, 11, 6, 6, 12, 11, 10, 10, 11, 9, 13, 12, 9, 7, 7, 7, 7, 7, 10, 13, 13, 10, 10, 9, 9, 7, 8, 10, 10, 13, 13, 11, 8, 8, 8, 8, 8, 8, 8, 10, 6, 6, 11, 12, 11, 10, 11, 10, 13, 12, 11, 7, 7, 8, 6, 6, 12, 11, 13, 10, 11, 9, 9, 8, 8, 10, 11, 13, 11, 11, 6, 7, 9, 8, 8, 11, 6, 6, 6, 7, 12, 11, 11, 10, 11, 11, 13, 12, 10, 7, 7, 7, 7, 8, 11, 11, 13, 9, 9, 8, 8, 9, 8, 10, 12, 13, 12, 8, 8, 8, 8, 9, 9, 9, 8, 11, 7, 7, 6, 6, 5, 5, 10, 12, 13, 11, 8, 5, 7, 7, 7, 8, 11, 10, 13, 10, 10, 9, 9, 9, 8, 9, 13, 13, 10, 8, 7, 9, 9, 8, 8, 8, 8, 11, 7, 9, 11, 10, 10, 10, 11, 13, 12, 12, 9, 7, 8, 7, 7},
  {6, 11, 6, 6, 6, 6, 6, 6, 5, 5, 5, 4, 4, 4, 4, 4, 5, 5, 10, 13, 9, 9, 9, 9, 9, 8, 8, 8, 6, 5, 5, 5, 5, 5, 5, 6, 10, 7, 7, 8, 7, 8, 7, 7, 7, 7, 7, 4, 4, 3, 3, 3, 3, 3, 3, 7, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 6, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 6, 2, 2, 2, 2, 2, 2, 1, 4, 4, 4, 4, 3, 4, 4, 3, 3, 3, 7, 3, 3, 3, 2, 3, 4, 3, 3, 3, 3, 3, 2, 2, 2, 7, 12, 13, 12, 10, 8, 7, 8, 9, 10, 12, 11, 10, 11, 10, 9, 9, 9, 9, 10, 10, 13, 12, 10, 8, 8, 8, 9, 10, 11, 10, 8, 9, 9, 10, 9, 9, 9, 9, 13, 13, 12, 10, 9, 7, 7, 9, 9, 9, 10, 9, 8, 7, 10, 9, 9, 9, 13, 10, 12, 12, 10, 8, 7, 7, 8, 7, 10, 10, 9, 8, 8, 8, 7, 8, 12, 8, 8, 13, 12, 11, 9, 8, 8, 9, 9, 11, 11, 11, 11, 10, 13, 10, 10, 10, 10, 9, 12, 13, 10, 9, 9, 9, 10, 9, 10, 13, 9, 9, 9, 10, 10, 10, 10, 9, 8, 12, 12, 11, 9, 7, 7, 8, 7, 9, 13, 9, 10, 8, 9, 9, 8, 8, 8, 9, 11, 13, 11, 9, 9, 7, 7, 8, 9, 10, 13, 9, 8, 8, 7, 7, 8, 8, 8, 11, 13, 11, 9, 10, 8, 9, 10, 10, 13, 11, 11, 10, 10, 9, 9, 9, 10, 9, 10, 13, 12, 10, 9, 8, 7, 9, 10, 10, 9, 13, 9, 10, 11, 10, 11, 10, 9, 9, 13, 12, 9, 9, 8, 8, 10, 11, 13, 13, 10, 10, 10, 11, 10, 10, 10, 10, 9, 13, 12, 10, 9, 6, 7, 8, 10, 11, 11, 13, 10, 10, 10, 10, 9, 9, 9, 8, 13, 12, 11, 8, 7, 8, 10, 10, 11, 13, 11, 11, 10, 10, 10, 10, 10, 10, 8, 12, 12, 11, 9, 9, 8, 9, 10, 10, 10, 10, 13, 10, 10, 10, 9, 10, 9, 8, 12, 12, 11, 10, 9, 7, 8, 8, 9, 10, 13, 9, 8, 9, 9, 8, 8, 8, 8, 6, 6, 6, 6, 7, 9, 10, 9, 9, 10, 10, 13, 9, 9, 8, 8, 7, 8, 8, 11, 13, 11, 6},
  {1, 4, 8, 5, 8, 7, 6, 5, 6, 6, 6, 4, 4, 2, 2, 2, 3, 3, 8, 4, 5, 5, 7, 8, 6, 6, 6, 7, 7, 7, 5, 5, 5, 5, 6, 10, 6, 7, 7, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 4, 5, 5, 8, 4, 5, 6, 7, 7, 7, 7, 8, 8, 7, 7, 6, 6, 6, 10, 11, 13, 13, 10, 7, 4, 6, 8, 10, 10, 9, 9, 8, 11, 9, 9, 9, 11, 13, 13, 13, 10, 8, 6, 6, 7, 10, 10, 8, 8, 8, 9, 8, 12, 7, 9, 11, 13, 13, 11, 9, 7, 7, 7, 9, 10, 8, 8, 7, 9, 12, 7, 7, 10, 12, 12, 13, 11, 10, 8, 7, 7, 8, 10, 8, 7, 11, 8, 7, 6, 6, 8, 12, 13, 13, 12, 9, 7, 8, 9, 10, 11, 10, 9, 9, 11, 13, 9, 9, 9, 11, 11, 13, 12, 8, 7, 5, 7, 8, 11, 9, 9, 8, 12, 9, 7, 8, 7, 11, 12, 13, 12, 10, 8, 7, 8, 8, 10, 9, 8, 8, 8, 13, 8, 8, 7, 11, 12, 13, 12, 10, 7, 7, 6, 7, 10, 9, 8, 7, 11, 8, 7, 7, 7, 10, 12, 13, 13, 10, 8, 5, 7, 8, 10, 10, 9, 9, 9, 13, 9, 8, 9, 10, 12, 12, 12, 10, 8, 7, 7, 7, 10, 9, 9, 8, 12, 8, 7, 8, 8, 9, 11, 13, 13, 11, 9, 8, 8, 9, 10, 10, 8, 7, 7, 13, 7, 6, 7, 9, 12, 12, 13, 11, 9, 7, 6, 6, 8, 10, 9, 7, 7, 12, 7, 6, 7, 9, 12, 13, 13, 11, 9, 7, 7, 9, 9, 11, 10, 9, 9, 11, 13, 10, 9, 9, 11, 12, 13, 11, 9, 7, 7, 8, 9, 10, 9, 8, 7, 12, 8, 8, 8, 8, 11, 12, 13, 12, 10, 7, 7, 7, 7, 11, 8, 8, 8, 8, 13, 8, 8, 7, 11, 12, 13, 12, 10, 8, 6, 7, 7, 10, 8, 7, 7, 12, 8, 7, 8, 7, 10, 12, 13, 12, 10, 8, 6, 6, 8, 10, 9, 8, 8, 9, 13, 9, 9, 9, 10, 11, 12, 13, 11, 8, 6, 6, 7, 10, 10, 8, 8, 8, 13, 8, 8, 8, 10, 12, 13, 13, 10, 8, 6, 6, 7, 9, 9, 8, 7, 7, 8, 11, 7, 7, 10, 11, 13, 13, 11, 8, 5, 5, 6, 8, 9, 7, 7, 6, 12, 7, 6, 8, 10, 12, 12, 13, 12},
  {1, 6, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 4, 4, 4, 4, 3, 7, 2, 3, 3, 3, 4, 4, 5, 6, 5, 4, 5, 6, 6, 5, 5, 11, 13, 12, 10, 9, 9, 8, 7, 6, 10, 9, 8, 9, 8, 8, 9, 8, 12, 13, 12, 12, 10, 9, 8, 6, 6, 5, 7, 10, 8, 6, 6, 5, 5, 5, 11, 13, 12, 11, 10, 9, 10, 5, 7, 8, 8, 6, 5, 5, 9, 9, 9, 8, 13, 13, 12, 11, 10, 8, 6, 7, 6, 8, 11, 10, 9, 8, 8, 7, 8, 12, 11, 13, 12, 10, 9, 10, 8, 5, 8, 11, 9, 9, 8, 8, 9, 9, 9, 8, 13, 12, 12, 10, 9, 8, 6, 6, 5, 6, 9, 7, 5, 4, 4, 5, 5, 10, 13, 12, 11, 10, 9, 10, 6, 6, 8, 8, 6, 6, 5, 9, 11, 10, 9, 10, 13, 12, 11, 10, 8, 6, 7, 7, 9, 11, 11, 9, 9, 9, 8, 8, 8, 13, 13, 12, 11, 10, 8, 6, 6, 7, 10, 10, 10, 9, 9, 8, 9, 7, 8, 13, 13, 11, 10, 9, 8, 6, 5, 5, 6, 8, 7, 6, 6, 5, 5, 6, 6, 13, 13, 11, 10, 9, 9, 8, 7, 7, 7, 6, 5, 5, 8, 10, 9, 9, 10, 13, 12, 11, 9, 8, 7, 7, 5, 7, 11, 11, 9, 7, 6, 6, 6, 6, 13, 13, 12, 11, 9, 8, 8, 8, 7, 9, 10, 8, 8, 7, 9, 10, 8, 7, 13, 13, 11, 10, 9, 8, 7, 5, 5, 5, 8, 6, 5, 5, 5, 5, 5, 4, 13, 13, 12, 10, 9, 7, 4, 5, 7, 8, 5, 5, 5, 8, 11, 12, 10, 10, 13, 12, 12, 10, 9, 6, 5, 5, 6, 11, 11, 9, 9, 8, 8, 8, 7, 13, 13, 12, 11, 10, 9, 9, 8, 8, 11, 11, 9, 9, 9, 9, 11, 10, 9, 12, 13, 12, 10, 9, 9, 7, 6, 6, 6, 8, 7, 6, 6, 5, 6, 6, 5, 13, 13, 12, 10, 9, 9, 9, 8, 8, 7, 6, 6, 5, 9, 9, 9, 9, 9, 13, 13, 11, 10, 8, 6, 4, 4, 7, 10, 10, 9, 9, 8, 7, 7, 7, 9, 13, 12, 11, 9, 8, 6, 5, 6, 9, 10, 9, 8, 8, 8, 9, 9, 7, 11, 13, 12, 10, 9, 9, 6, 5, 4, 5, 9, 7, 6, 5, 5, 5, 5, 4, 13, 13, 12, 10, 9, 10, 9, 9, 4},
};


// function declerations:
void feed(int conveyor);
void bluetoothFeed(int conveyor);
void playTrack();
void playVoice(int conveyor);
void disableStepper();
void soundLightShow();

void setup() {
  // Initialize the stepper motor speed and sensor pins
  for (int i = 0; i < 4; i++) {
    steppers[i].setSpeed(6);           // Set speed for each stepper
    pinMode(sensorPins[i], INPUT);     // Set sensor pins as input
  }

  IrReceiver.begin(receiverPin, ENABLE_LED_FEEDBACK);

  // Initialize LED strips individually (due to compile-time constant requirement)
  FastLED.addLeds<WS2812, 44, GRB>(leds[0], NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, 42, GRB>(leds[1], NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, 40, GRB>(leds[2], NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812, 38, GRB>(leds[3], NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < NUM_LEDS; j++) {
      leds[i][j] = CRGB::Black;
    }
  }
  FastLED.show();
  
  Serial.begin(9600);
  Serial1.begin(38400); // Bluetooth
  Serial2.begin(9600); // DF Player Mini
  
  player.begin(Serial2);
  

  EEPROM.get(trackNumberAddr, trackNumber);
  
  if (trackNumber < 1 || trackNumber > lastTrackNumber) { // This means track was never set, and we start at track 1
    trackNumber = 1;
    EEPROM.put(trackNumberAddr, trackNumber);
  }

  player.volume(17);
  
  
}

void loop() {

  if (Serial1.available()) {
    int conveyor;
    String message = Serial1.readStringUntil('\n'); // stops at newline

    message.trim(); // remove any trailing \r or \n

    if (message == "red") {
      conveyor = 0;
    } 
    else if (message == "green") {
      conveyor = 1;
    }
    else if (message == "yellow") {
      conveyor = 2;
    }
    else if (message == "blue") {
      conveyor = 3;
    }
    else {
      conveyor = -1;
    }

    if(conveyor != -1) {
      bluetoothFeed(conveyor);
    }
}
  
  if (IrReceiver.decode()) {
    // Print the decoded value
    uint32_t receivedSignal = IrReceiver.decodedIRData.decodedRawData;
    Serial.print("Received IR signal: ");
    Serial.println(receivedSignal, HEX);
    int conveyor;
    // Check if the received signal matches the target signal
    
    if(receivedSignal == red) {
      Serial.println("Red button clicked");
      conveyor = 0;
    }
    else if(receivedSignal == green) {
      Serial.println("Green button clicked");
      conveyor = 1;
    }
    else if(receivedSignal == yellow) {
      Serial.println("Yellow button clicked");
      conveyor = 2;
    }
    else if(receivedSignal == blue) {
      Serial.println("Blue button clicked");
      conveyor = 3;
    }
    else {
      Serial.println("Unknown signal..");
      conveyor = -1;
    }
    Serial.println("**********************************************");
    delay(100);

    if(conveyor != -1) {
      feed(conveyor);
    }
    // Resume receiver for the next signal
    IrReceiver.resume();
  }
}

void feed(int conveyor) {
  unsigned long currentMillis;
  int sensorValue = digitalRead(sensorPins[conveyor]);
  int ledIndex = 0; // Current LED being updated

  // Step motor and animate LEDs while tape is not detected
  while (true) {
    // Step the motor
    if(conveyor % 2) {
      steppers[conveyor].step(1);
    }
    else {
      steppers[conveyor].step(-1);
    }
    

    // Check sensor state
    sensorValue = digitalRead(sensorPins[conveyor]);

    // Exit loop if tape is detected
    if (sensorValue == LOW) {
      Serial.println("Tape detected, stopping conveyor.");
      break;
    }

    // Update LEDs without blocking
    currentMillis = millis();
    if (currentMillis - previousMillis[conveyor] >= ledInterval) {
      previousMillis[conveyor] = currentMillis;

      // Animate LEDs with a fading trail effect
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[conveyor][i].fadeToBlackBy(50); // Gradual fade effect
      }

      leds[conveyor][ledIndex] = conveyorColors[conveyor]; // Light up current LED
      FastLED.show();

      ledIndex = (ledIndex + 1) % NUM_LEDS; // Move to next LED
    }
  }

  // Perform extra steps after tape is detected
  int num_steps_taken = 0;
  while(num_steps_taken < 740) {
    if(conveyor % 2) {
      steppers[conveyor].step(1);
    } else {
      steppers[conveyor].step(-1);
    }
    num_steps_taken++;
    currentMillis = millis();
    if (currentMillis - previousMillis[conveyor] >= ledInterval) {
      previousMillis[conveyor] = currentMillis;

      // Animate LEDs with a fading trail effect
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[conveyor][i].fadeToBlackBy(50); // Gradual fade effect
      }

      leds[conveyor][ledIndex] = conveyorColors[conveyor]; // Light up current LED
      FastLED.show();

      ledIndex = (ledIndex + 1) % NUM_LEDS; // Move to next LED
    }
  }
    
    
  
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[conveyor][i] = CRGB::Black;
  }
  FastLED.show();
  disableStepper();
  Serial.print("Feeding completed. Waiting for the next command.");
}


void disableStepper() {
  // Set all stepper pins to LOW
    digitalWrite(47, LOW);
    digitalWrite(49, LOW);
    digitalWrite(51, LOW);
    digitalWrite(53, LOW);

    digitalWrite(23, LOW);
    digitalWrite(25, LOW);
    digitalWrite(27, LOW);
    digitalWrite(29, LOW);

    digitalWrite(39, LOW);
    digitalWrite(41, LOW);
    digitalWrite(43, LOW);
    digitalWrite(45, LOW);

    digitalWrite(31, LOW);
    digitalWrite(33, LOW);
    digitalWrite(35, LOW);
    digitalWrite(37, LOW);
  }


void playTrack() {
  EEPROM.get(trackNumberAddr, trackNumber);

  player.volume(17);
  player.play(trackNumber);
  trackNumber++;

  if (trackNumber > lastTrackNumber) {
    trackNumber = 1; 
  }
  EEPROM.put(trackNumberAddr, trackNumber);
}


void playVoice(int conveyor) {
  player.volume(17);
  if(conveyor == 0) {
    player.play(12);
  }
  else if(conveyor == 1) {
    player.play(13);
  }
  else if(conveyor == 2) {
    player.play(14);
  }
  else if(conveyor == 3) {
    player.play(15);
  }
  delay(3000);
}


void bluetoothFeed(int conveyor) {
  playVoice(conveyor);
  int playerVolume = 17;
  unsigned long trackStartTime = millis();
  unsigned long lastVolumeDecreaseTime = 0;
  playTrack();
  unsigned long currentMillis;
  while (true) {
  
    // Step the motor
    
    if(conveyor % 2) {
      steppers[conveyor].step(1);
    }
    else {
      steppers[conveyor].step(-1);
    }
    

    // Check sensor state
    int sensorValue = digitalRead(sensorPins[conveyor]);

    // Exit loop if tape is detected
    if (sensorValue == LOW) {
      Serial.println("Tape detected, stopping conveyor.");
      break;
    }
  
    // Update LEDs without blocking
    currentMillis = millis();
    if (currentMillis - previousMillis[conveyor] >= ledInterval) {
      previousMillis[conveyor] = currentMillis;

      soundLightShow();
      
    }
  }

  // Perform extra steps after tape is detected
  int num_steps_taken = 0;
  while(num_steps_taken < 740) {
    if(conveyor % 2) {
      steppers[conveyor].step(1);
    } else {
      steppers[conveyor].step(-1);
    }
    num_steps_taken++;
    currentMillis = millis();
    if (currentMillis - previousMillis[conveyor] >= ledInterval) {
      previousMillis[conveyor] = currentMillis;

      soundLightShow();
    }
  }

  while(currentFrame <= num_updates) {
    currentMillis = millis();
    if (currentMillis - previousMillis[conveyor] >= ledInterval) {
      previousMillis[conveyor] = currentMillis;

      soundLightShow();
    }

  unsigned long currentMillis = millis();
  // Song is 13s long. If 10s played decrement 1 vol every 176 millis in order to get to low volume in a 3 second transition
  if ((currentMillis - trackStartTime > 10000) && 
      (lastVolumeDecreaseTime == 0 || currentMillis - lastVolumeDecreaseTime > 176)) {
    
    if (playerVolume > 3) {
      playerVolume--;
      player.volume(playerVolume);
      lastVolumeDecreaseTime = currentMillis;
    }
  }
  }

   hueBase = 0;
   currentFrame = 0;
   previousMillis[conveyor] = 0;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < NUM_LEDS; j++) {
      leds[i][j] = CRGB::Black;
    }
  }
  FastLED.show();
  Serial1.println("finished");
}





void soundLightShow() {
  // 1) Read how many LEDs to light from the array in PROGMEM
  //    Each element in the track array is assumed to be between 0 and 13.
  if (currentFrame > num_updates) {
    return;
  }
  uint8_t ledsToLight = pgm_read_byte(&tracks[trackNumber - 1][currentFrame]);

  // 2) Increment hueBase to animate the rainbow over time
  hueBase++;

  // 3) Update each of the 4 LED strips
  for (int stripIndex = 0; stripIndex < 4; stripIndex++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      if (i < ledsToLight) {
        // For active LEDs: set a fresh rainbow color without fading
        leds[stripIndex][i] = CHSV(hueBase + i * 10, 255, 255);
      } else {
        // For LEDs not meant to be lit, fade them gradually
        leds[stripIndex][i].fadeToBlackBy(FADE_RATE);
      }
    }
  }

  // 4) Display the updated LED colors
  FastLED.show();

  // 5) Move to the next frame for the next update
  currentFrame++;
}