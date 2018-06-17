// Made by https://github.com/Valentin-Golyonko. Apache License 2.0.
// DIY: Arduino (nano v3) RGB Strip controller with bluetooth connection to android app.

// Sketch uses 5594 bytes (18%) of program storage space. Maximum is 30720 bytes.
// Global variables use 183 bytes (8%) of dynamic memory, leaving 1865 bytes for local variables. Maximum is 2048 bytes.

#include "functions.h"

#define BlueLedPin 8  // power indicator
#define REDPIN 10     // RGB Strip pins
#define GREENPIN 11
#define BLUEPIN 9

long previousMillis_1 = 0; // will store last time when status was updated
long previousMillis_2 = 0;
long previousMillis_3 = 0;

void setup() {

  // declare digital pins
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin , 1); // turn OFF relay !!!
  // it depends on connection to relay - green led mast be OFF
  // in that case relay is OFF -> no power consuming
  pinMode(pirInputPin, INPUT);  // declare rip-sensor as input
  digitalWrite(pirInputPin , 0);
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(BlueLedPin, OUTPUT); // Blue Led
  digitalWrite(BlueLedPin, 1);

  // UART speed
  // Serial.begin(9600);
  SerialBLE.begin(9600);
}

void loop() {

  // if the data came from SerialBLE
  ListenBlt();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis_1 >= period) { // period from android app
    previousMillis_1 = currentMillis;

    if (blt) {
      // transmite date to android app over bluetooth
      Transmit(p, r);
    }
  }

  if (currentMillis - previousMillis_2 >= 1000) { // timer = 1sec
    previousMillis_2 = currentMillis;

    // update sensors status
    PinStatus();

    // set the color in Rgb Strip
    if (light_always) {
      if (done) {
        RGBStrip(r_in, g_in, b_in);
        done = false;
      }
    } else if (!light_always) {
      // update PIR sensor status
      PIR(p);
    }
  }

  // if no motion detected in last 30s -> turn LED OFF
  if (currentMillis - previousMillis_3 >= 30000) { // timer = 30sec
    previousMillis_3 = currentMillis;
    if (!light_always) {
      if (pir == 0) {
        RGBStrip(0, 0, 0);    // rgb off
        digitalWrite(relayPin, 1);  // turn LED OFF
      }
    }
  }
}

