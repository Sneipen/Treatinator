#include <Arduino.h>

const int btStatePin = 2; // HC-05 STATE pin
const int BluetoothConnectedPin = 3; // light up blue LED when connected
const int onPin = 4; // light up red LED when arduino has power
const int finishedLED = 5; // light up green LED when finished

const int redButton = 6;
const int greenButton = 7;
const int yellowButton = 8;
const int blueButton = 9;

// Flags and states
bool finished = false;
bool buttonClicked  = false;
 
String color;

void setup() {
  pinMode(btStatePin,    INPUT);
  pinMode(BluetoothConnectedPin,         OUTPUT);
  pinMode(onPin,         OUTPUT);
  pinMode(finishedLED,   OUTPUT);

  pinMode(greenButton,   INPUT_PULLUP);
  pinMode(blueButton,    INPUT_PULLUP);
  pinMode(redButton,     INPUT_PULLUP);
  pinMode(yellowButton,  INPUT_PULLUP);

  digitalWrite(BluetoothConnectedPin, LOW);
  digitalWrite(onPin, HIGH);
  digitalWrite(finishedLED, LOW);

  // Start Bluetooth serial
  connectBluetooth();
}

void loop() {
  // Check current connection status
  bool isConnected = (digitalRead(btStatePin) == HIGH);

  // If disconnected, block here until reconnected
  while (!isConnected) {
    digitalWrite(BluetoothConnectedPin, LOW);  // LED or transistor off
    delay(30);
    isConnected = (digitalRead(btStatePin) == HIGH);
  }

  // Now the master and slave are connected
  digitalWrite(BluetoothConnectedPin, HIGH); 

  // Read buttons
  bool currentRedState    = (digitalRead(redButton)    == LOW);
  bool currentGreenState  = (digitalRead(greenButton)  == LOW);
  bool currentYellowState = (digitalRead(yellowButton) == LOW);
  bool currentBlueState   = (digitalRead(blueButton)   == LOW);

  // If no button has been clicked yet, check for new click
  if (!buttonClicked) {
    if (currentRedState) {
      color = "red";
      buttonClicked = true;
    }
    else if (currentGreenState) {
      color = "green";
      buttonClicked = true;
    }
    else if (currentYellowState) {
      color = "yellow";
      buttonClicked = true;
    }
    else if (currentBlueState) {
      color = "blue";
      buttonClicked = true;
    }
  }

  if (buttonClicked && !finished) {
      Serial1.println(color);
      waitForFinished(); 
  }
}

void connectBluetooth() {
  // Begin the hardware serial to HC-05
  Serial1.begin(38400);
}

void waitForFinished() {
  while (true) {
    if (Serial1.available() > 0) {
      String resp = Serial1.readStringUntil('\n');
      resp.trim();
      if (resp == "finished") {
        digitalWrite(finishedLED, HIGH);
        Serial1.end();  // Stop talking to HC-05
        finished = true;
        break;
      }
    }
  }
}