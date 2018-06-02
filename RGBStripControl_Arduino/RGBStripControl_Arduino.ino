// Made by https://github.com/Valentin-Golyonko. Apache License 2.0.
// DIY: Arduino (nano v3) RGB Strip controller with bluetooth connection to android app.

#include <SoftwareSerial.h>

#define BlueLedPin 8  // power indicator
#define REDPIN 10     // RGB Strip pins
#define GREENPIN 11
#define BLUEPIN 9

SoftwareSerial SerialBLE(7, 6); // RX,TX on arduino board

int relayPin = 12;
bool light_always = false;

int pirInputPin = 5;  // choose the input pin (for PIR sensor)
//READ: please change PIR sensor Pin number to 4 (I could not solder it) :(
int pir = LOW;        // we start, assuming no motion detected

int r_in = 100, g_in = 100, b_in = 100; // RGB default
bool done = false; // flag for recieve all 3 colors

long previousMillis_1 = 0; // will store last time when status was updated
long previousMillis_2 = 0;
long previousMillis_3 = 0;
int period = 5000;

int r = -1, p = -1; // pin status
bool blt = false; // BLT trancieve ON/OFF
char ch_data[4]; // store incoming date from BLT

void setup() {

  // declare digital pins
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin , HIGH); // turn OFF relay !!!
  // it depends on connection to relay - green led mast be OFF
  // in that case relay is OFF -> no power consuming
  pinMode(pirInputPin, INPUT);  // declare rip-sensor as input
  digitalWrite(pirInputPin , LOW);
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(BlueLedPin, OUTPUT); // Blue Led
  digitalWrite(BlueLedPin, HIGH);

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
      if (pir == LOW) {
        RGBStrip(0, 0, 0);    // rgb off
        digitalWrite(relayPin, HIGH);  // turn LED OFF
      }
    }
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
          digitalWrite(relayPin , LOW); // coz in LOW Consumed current is less (~66mA)
          light_always = true;
          RGBStrip(r_in, g_in, b_in);
          break;
        case 0:
          RGBStrip(0, 0, 0);
          digitalWrite(relayPin , HIGH); // Consumed current ~130mA
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

void PinStatus() {
  r = digitalRead(relayPin);  // relayState
  p = digitalRead(pirInputPin); // pirState

  //  Serial.println(
  //    "p" + (String)p + "u" +
  //    "l" + (String)r + "w" +
  //    "E");
}

void Transmit(int p, int r) {
  SerialBLE.println(
    "p" + (String)p + "u" +
    "l" + (String)r + "w" +
    "E"); // end of the line
}

void PIR(int val) {
  if (val == HIGH) {  // check if the input is HIGH
    digitalWrite(relayPin, LOW);  // turn LED ON
    RGBStrip(125, 125, 125);
    if (pir == LOW) {
      // Serial.println("Motion detected!");
      pir = HIGH;
    }
  } else {
    if (pir == HIGH) {
      // Serial.println("Motion ended!");
      pir = LOW;
    }
  }
}

void RGBStrip(int r, int g, int b) {
  r_in = r; // save last state
  g_in = g;
  b_in = b;
  analogWrite(REDPIN , r);
  analogWrite(GREENPIN , g);
  analogWrite(BLUEPIN , b);
  // Serial.println("RGB: " + (String)r + "." + (String)g + "." + (String)b);
}
