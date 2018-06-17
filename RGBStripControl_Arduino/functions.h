
#include <SoftwareSerial.h>

#define BlueLedPin 8  // power indicator
#define REDPIN 10     // RGB Strip pins
#define GREENPIN 11
#define BLUEPIN 9
#define RX 7
#define TX 6

SoftwareSerial SerialBLE(RX, TX); // RX,TX on arduino board

int period = 5000;
const int relayPin = 12;
const int pirInputPin = 5;  // choose the input pin (for PIR sensor)
bool light_always = false;
int pir = 0;                // we start, assuming no motion detected
int r_in = 100, g_in = 100, b_in = 100; // RGB default
bool done = false;          // flag for recieve all 3 colors
int r = -1, p = -1;         // pin status
bool blt = false;           // BLT trancieve ON/OFF
unsigned char ch_data[4];   // store incoming date from BLT

void Transmit(int p, int r) {
  SerialBLE.println(
    "p" + (String)p + "u" +
    "l" + (String)r + "w" +
    "E"); // end of the line
}

void RGBStrip(int r, int g, int b) {
  r_in = r;   // save last state
  g_in = g;
  b_in = b;
  analogWrite(REDPIN , r);
  analogWrite(GREENPIN , g);
  analogWrite(BLUEPIN , b);
  // Serial.println("RGB: " + (String)r + "." + (String)g + "." + (String)b);
}

void GetCommand(int in) {
  // We obtain the pin number by integer division (we find 1 number == pin)
  // and the action we need by obtaining the remainder of the division by 1000.
  switch (in / 1000) {
    case 1: // start data transfer
      Transmit(p, r);
      blt = true;
      period = (in % 1000) * 1000;  // in % 1000 = 5 -> 5sec = 5000ms
      // Serial.println(blt);
      // Serial.println(period);
      break;
    case 2: // stop transmition date
      blt = false;
      // Serial.println(blt);
      break;
    case 3:
      // 3002 <= in <= 3000, relayPin + ON always / OFF
      switch (in % 1000) {
        case 2:
          digitalWrite(relayPin , 0); // coz in 0 Consumed current is less (~66mA)
          light_always = true;
          RGBStrip(r_in, g_in, b_in);
          break;
        case 0:
          RGBStrip(0, 0, 0);
          digitalWrite(relayPin , 1); // Consumed current ~130mA
          light_always = false;
          break;
      }
      break;
    case 10:  // 10xxx - red pin 10 to value xxx
      r_in = in % 1000;
      done = false; // continue reading \ read 2 more param.
      break;
    case 11:  // 11xxx - green pin 11 to value xxx
      g_in = in % 1000;
      done = false;
      break;
    case 13:  // 13 = code for blue pin 9, coz 90255 does't get in to 2 bytes :(
      b_in = in % 1000;
      done = true;  // allow to light ON RGBStrip
      break;
  }
}

void ListenBlt() {
  if (SerialBLE.available() > 0) {
    int count = 0;
    for (int i = 0; i < 4; i++) {
      // read input bytes
      ch_data[i] = SerialBLE.read();
      delay(10);    // magic! for stable receiving
      // Serial.println("-----------");
      // Serial.println(ch_data[i], DEC);
      count++;
      // i use only 2 bytes (2^16-1), ex-pl:  0010 0011 1101 1100  = 9180 (int)
      if (count == 4) {
        // convert byte to int ;  may be,  int i = atoi(intBuffer) ?
        int a = (long)(unsigned char)(ch_data[0]) << 24 |
                (long)(unsigned char)(ch_data[1]) << 16 |
                (int)(unsigned char)(ch_data[2]) << 8 |
                (int)(unsigned char)(ch_data[3]);

        // Serial.println(a, DEC);
        GetCommand(a);
        count = 0;
      }
    }
  }
}

void PinStatus() {
  r = digitalRead(relayPin);  // relayState
  p = digitalRead(pirInputPin); // pirState

  //  Serial.println(
  //    "p" + (String)p + "u" +
  //    "l" + (String)r + "w" +
  //    "E");
}

void PIR(int val) {
  if (val == 1) {  // check if the input is 1
    digitalWrite(relayPin, 0);  // turn LED ON
    RGBStrip(125, 125, 125);
    if (pir == 0) {
      // Serial.println("Motion detected!");
      pir = 1;
    }
  } else {
    if (pir == 1) {
      // Serial.println("Motion ended!");
      pir = 0;
    }
  }
}
