// Made by https://github.com/Valentin-Golyonko. Apache License 2.0.
//
// DIY: Arduino (nano v3) RGB Strip controller with bluetooth connection to android app.
//
// Sketch uses 11862 bytes (38%) of program storage space. Maximum is 30720 bytes.
// Global variables use 489 bytes (23%) of dynamic memory,
// leaving 1559 bytes for local variables. Maximum is 2048 bytes.

#include "functions.h"
#include "music.h"

unsigned long previousMillis_1 = 0; // will store last time when status was updated
unsigned long previousMillis_2 = 0;
unsigned long previousMillis_3 = 0;

uint16_t sensorTimer = 1000;  // 1 sec
uint16_t pirTimer = 30000;    // 30 sec

void setup() {

  // declare digital pins
  pinMode(ptr->relayPin, OUTPUT);
  digitalWrite(ptr->relayPin , ptr->relayStatus); // turn OFF relay !!!
  // it depends on connection to relay - green led mast be OFF
  // in that case relay is OFF -> no power consuming
  pinMode(ptr->pirInputPin, INPUT);   // declare rip-sensor as input
  digitalWrite(ptr->pirInputPin , ptr->pirStatus);
  pinMode(ptr->REDPIN, OUTPUT);
  pinMode(ptr->GREENPIN, OUTPUT);
  pinMode(ptr->BLUEPIN, OUTPUT);
  pinMode(ptr->BlueLedPin, OUTPUT);   // Blue Led
  digitalWrite(ptr->BlueLedPin, 1);
  pinMode(ptr->pinBuzzer, OUTPUT);
  pinMode(ptr->pinPhoto, INPUT);

  rtc.begin();

  //Serial.begin(9600);         // UART speed
  SerialBLE.begin(9600);

  Buzzer();
}

void loop() {

  if (ptr->buzzer_play) {
    if (ptr->alarm_on) {        // if time to Alarm
      play();                   // play music
      sP.buzzer_play = false;   // but 1 time
    }
  }

  ListenBlt();        // if the data came from SerialBLE

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis_1 >= ptr->period) { // period from android app
    previousMillis_1 = currentMillis;

    if (ptr->blt) {
      Transmit();     // transmit date to android app over bluetooth
    }
  }

  if (currentMillis - previousMillis_2 >= sensorTimer) { // timer = 1sec
    previousMillis_2 = currentMillis;

    RTC();

    PinStatus();      // update sensors status

    if (ptr->light_always) {
      if (ptr->done) {          // get date from android App and set the color in Rgb Strip
        RGBStrip(ptr->r_in, ptr->g_in, ptr->b_in);
        sP.done = false;
      }
    } else if (!ptr->light_always) {
      PIR(ptr->pirStatus);      // else - update PIR sensor status
    }
  }

  // if no motion detected in last 30s -> turn LED OFF
  if (currentMillis - previousMillis_3 >= pirTimer) { // timer = 30sec
    previousMillis_3 = currentMillis;
    if (!ptr->light_always) {
      if (ptr->pirStatus == 0) {
        RGBStrip(0, 0, 0);    // rgb off
        digitalWrite(ptr->relayPin, 1);  // turn LED OFF
      }
    }
  }
}
