// Made by https://github.com/Valentin-Golyonko. Apache License 2.0.
// DIY: Arduino (nano v3) RGB Strip controller with bluetooth connection to android app.

// Sketch uses 5628 bytes (18%) of program storage space. Maximum is 30720 bytes.
// Global variables use 187 bytes (9%) of dynamic memory, leaving 1861 bytes for local variables. Maximum is 2048 bytes.

#include "functions.h"

unsigned long previousMillis_1 = 0; // will store last time when status was updated
unsigned long previousMillis_2 = 0;
unsigned long previousMillis_3 = 0;

uint16_t sensorTimer = 1000; // 1 sec
uint16_t pirTimer = 30000; // 30 sec

void setup() {

    // declare digital pins
    pinMode(ptr->relayPin, OUTPUT);
    digitalWrite(ptr->relayPin , ptr->relayStatus); // turn OFF relay !!!
    // it depends on connection to relay - green led mast be OFF
    // in that case relay is OFF -> no power consuming
    pinMode(ptr->pirInputPin, INPUT);  // declare rip-sensor as input
    digitalWrite(ptr->pirInputPin , ptr->pirStatus);
    pinMode(ptr->REDPIN, OUTPUT);
    pinMode(ptr->GREENPIN, OUTPUT);
    pinMode(ptr->BLUEPIN, OUTPUT);
    pinMode(ptr->BlueLedPin, OUTPUT); // Blue Led
    digitalWrite(ptr->BlueLedPin, 1);

    // UART speed
    // Serial.begin(9600);
    SerialBLE.begin(9600);
}

void loop() {

    // if the data came from SerialBLE
    ListenBlt();

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis_1 >= ptr->period) { // period from android app
        previousMillis_1 = currentMillis;

        if (ptr->blt) {
            // transmite date to android app over bluetooth
            Transmit();
        }
    }

    if (currentMillis - previousMillis_2 >= sensorTimer) { // timer = 1sec
        previousMillis_2 = currentMillis;

        // update sensors status
        PinStatus();

        // set the color in Rgb Strip
        if (ptr->light_always) {
            if (ptr->done) {
                RGBStrip(ptr->r_in, ptr->g_in, ptr->b_in);
                sP.done = false;
            }
        } else if (!ptr->light_always) {
            // update PIR sensor status
            PIR(ptr->pirStatus);
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

