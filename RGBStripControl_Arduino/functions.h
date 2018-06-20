#include <stdint.h>
#include <SoftwareSerial.h>

struct pins {
    const uint8_t BlueLedPin = 8;   // power indicator

    const uint8_t REDPIN = 10;      // RGB Strip pins
    const uint8_t GREENPIN = 11;
    const uint8_t BLUEPIN = 9;

    const uint8_t RX = 7;           // BLE
    const uint8_t TX = 6;

    uint16_t period = 5000;

    const uint8_t relayPin = 12;
    const uint8_t pirInputPin = 5;  // choose the input pin (for PIR sensor)
    bool light_always = false;
    uint8_t r_in = 100;             // RGB default
    uint8_t g_in = 100;
    uint8_t b_in = 100;
    const uint8_t r_in_def = 125;   // RGB default
    const uint8_t g_in_def = 125;
    const uint8_t b_in_def = 125;
    bool done = false;          // flag for receive all 3 colors
    uint8_t relayStatus = 1;       // pin status (def-t = OFF)
    uint8_t pirStatus = 0;          // we start, assuming no motion detected (def-t = OFF)
    bool blt = false;           // BLT transmission ON/OFF
    unsigned char ch_data[4];   // store incoming date from BLT
};

struct pins sP;
struct pins *ptr = &sP;

SoftwareSerial SerialBLE(sP.RX, sP.TX); // RX,TX on arduino board

void Transmit() {
    SerialBLE.println(
            "p" + (String)ptr->pirStatus + "u" +
            "l" + (String)ptr->relayStatus + "w" +
            "E"); // end of the line
}

void RGBStrip(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(sP.REDPIN , r);
    analogWrite(sP.GREENPIN , g);
    analogWrite(sP.BLUEPIN , b);
}

void GetCommand(uint16_t in) {
    // We obtain the pin number by integer division (we find 1 number == pin)
    // and the action we need by obtaining the remainder of the division by 1000.
    switch (in / 1000) {
        case 1: // start data transfer
            Transmit();
            sP.blt = true;
            sP.period = (in % 1000) * 1000;  // in % 1000 = 5 -> 5sec = 5000ms
            // Serial.println(blt);
            // Serial.println(period);
            break;
        case 2: // stop transmition date
            sP.blt = false;
            // Serial.println(blt);
            break;
        case 3:
            // 3002 <= in <= 3000, relayPin + ON always / OFF
            switch (in % 1000) {
                case 2:
                    digitalWrite(sP.relayPin , 0); // coz in 0 Consumed current is less (~66mA)
                    sP.light_always = true;
                    RGBStrip(ptr->r_in, ptr->g_in, ptr->b_in);
                    break;
                case 0:
                    RGBStrip(0, 0, 0);
                    digitalWrite(sP.relayPin , 1); // Consumed current ~130mA
                    sP.light_always = false;
                    break;
            }
            break;
        case 10:  // 10xxx - red pin 10 to value xxx
            sP.r_in = in % 1000;
            sP.done = false; // continue reading \ read 2 more param.
            break;
        case 11:  // 11xxx - green pin 11 to value xxx
            sP.g_in = in % 1000;
            sP.done = false;
            break;
        case 13:  // 13 = code for blue pin 9, coz 90255 does't get in to 2 bytes :(
            sP.b_in = in % 1000;
            sP.done = true;  // allow to light ON RGBStrip
            break;
    }
}

void ListenBlt() {
    if (SerialBLE.available() > 0) {
        uint8_t count = 0;
        for (int i = 0; i < 4; i++) {
            // read input bytes
            sP.ch_data[i] = SerialBLE.read();
            delay(10);    // magic! for stable receiving
            // Serial.println("-----------");
            // Serial.println(ch_data[i], DEC);
            count++;
            // i use only 2 bytes (2^16-1), ex-pl:  0010 0011 1101 1100  = 9180 (int)
            if (count == 4) {
                // convert byte to int ;  may be,  int i = atoi(intBuffer) ?
                long int a = (long)(unsigned char)(sP.ch_data[0]) << 24 |
                             (long)(unsigned char)(sP.ch_data[1]) << 16 |
                             (int)(unsigned char)(sP.ch_data[2]) << 8 |
                             (int)(unsigned char)(sP.ch_data[3]);

                // Serial.println(a, DEC);
                GetCommand(a);
                count = 0;
            }
        }
    }
}

void PinStatus() {
    sP.relayStatus = digitalRead(ptr->relayPin);  // relayState
    sP.pirStatus = digitalRead(ptr->pirInputPin); // pirState

    //  Serial.println(
    //    "p" + (String)ptr->pirInputPin + "u" +
    //    "l" + (String)ptr->relayPin + "w" +
    //    "E");
}

void PIR(int val) {
    if (val == 1) {  // check if the input is 1
        digitalWrite(sP.relayPin, 0);  // turn LED ON
        RGBStrip(ptr->r_in_def, ptr->g_in_def, ptr->b_in_def);
        if (ptr->pirStatus == 0) {
            // Serial.println("Motion detected!");
            sP.pirStatus = 1;
        }
    } else if (ptr->pirStatus == 1) {
            // Serial.println("Motion ended!");
            sP.pirStatus = 0;
    }
}
