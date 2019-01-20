#include <stdint.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "RTClib.h"               // 1.781 mA

struct pins {
  const uint8_t BlueLedPin = 8;   // power indicator

  const uint8_t REDPIN = 10;      // RGB Strip pins
  const uint8_t GREENPIN = 11;
  const uint8_t BLUEPIN = 9;

  const uint8_t pinBuzzer = 3;
  bool alarm_on = false;
  uint8_t alarm_duration = 2;

  const uint8_t pinPhoto = A0;    // photoresistor and
  uint16_t photo = 0;             // data from it

  const uint8_t RX = 7;           // BLT
  const uint8_t TX = 6;

  const uint8_t SDA = A4;         // I2C
  const uint8_t SCL = A5;
  uint8_t alarm_day = 17;         // alarm days default (1-5 = work days), code - 4015, 4017
  uint8_t a_day[7] = {1, 1, 1, 1, 1, 1, 1}; // {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  uint16_t alarm_time[7] = {930, 630, 630, 630, 630, 630, 930};
  uint16_t alarm_time_t = 0630;   // code - 40630
  bool alarm = false;
  bool alarm_day_set = true;
  bool autoBrightness = false;    // autoBrightness on/off

  uint16_t period = 5000;

  const uint8_t relayPin = 12;
  const uint8_t pirInputPin = 4;  // choose the input pin for PIR sensor
  bool light_always = false;
  uint8_t r_in = 100;             // RGB default
  uint8_t g_in = 100;
  uint8_t b_in = 100;
  const uint8_t r_in_def = 125;   // RGB default
  const uint8_t g_in_def = 125;
  const uint8_t b_in_def = 125;
  bool done = false;          // flag for receive all 3 colors
  uint8_t relayStatus = 1;    // pin status (def-t = OFF)
  uint8_t pirStatus = 0;      // we start, assuming no motion detected (def-t = OFF)
  bool blt = false;           // BLT transmission ON/OFF
  unsigned char ch_data[4];   // store incoming date from BLT
};

struct pins sP;
struct pins *ptr = &sP;

SoftwareSerial SerialBLE(sP.RX, sP.TX); // RX,TX on arduino board

RTC_DS3231 rtc;

void Transmit() {
  SerialBLE.println(
    "p" + (String)ptr->pirStatus + "u" +
    "l" + (String)ptr->relayStatus + "w" +
    "E"); // end of the line
}

void RGBStrip(uint8_t r, uint8_t g, uint8_t b) {
  float multiplaer;
  if (ptr->autoBrightness) {
    multiplaer = 1 - ((ptr->photo + 1) / 1025);     // Max outer light -> min RGBStrip brightness
  } else {
    multiplaer = 1;
  }
  analogWrite(sP.REDPIN , r * multiplaer);
  analogWrite(sP.GREENPIN , g * multiplaer);
  analogWrite(sP.BLUEPIN , b * multiplaer);
}

void Buzzer() {
  tone(ptr->pinBuzzer, 1000);   // Send 1KHz sound signal...
  delay(100);
  noTone(ptr->pinBuzzer);       // Stop sound...
}

void RTC() {
  DateTime now = rtc.now();
  int rtc_day = now.dayOfTheWeek();   // {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  int rtc_hour = now.hour();
  int rtc_minute = now.minute();
  //Serial.println("\tTime: " + String(rtc_day) + ":" + String(rtc_hour) + ":" + String(rtc_minute));

  if (ptr->alarm_day_set) {
    if (sP.a_day[rtc_day] == 1) {           // Alarm #1
      int alarm_time_h = ptr->alarm_time[rtc_day] / 100;
      int alarm_time_m = ptr->alarm_time[rtc_day] % 100;
      if (rtc_hour == alarm_time_h) {
        if (rtc_minute >= alarm_time_m) {
          if (rtc_minute <= alarm_time_m + sP.alarm_duration - 1) {   // min
            if (!sP.alarm_on) {
              //Serial.println("ALARM ON " + String(alarm_time_h) + ":" + String(alarm_time_m));
              digitalWrite(sP.relayPin, 0); // turn LED ON
              RGBStrip(50, 190, 50);        // rgb on, green
              sP.alarm_on = true;
            }
          } else if (sP.alarm_on) {
            //Serial.println("ALARM OFF");
            digitalWrite(sP.relayPin, 1);   // turn LED OFF
            RGBStrip(0, 0, 0);              // rgb off
            sP.alarm_on = false;
          }
        }
      }
    }
  }
}

void Alarm_Days(int d, int _time) {    // {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  switch (d) {
    case 15:
      for (int i = 1; i <= 5; i++) {
        sP.a_day[i] = 1;
        sP.alarm_time[i] = _time;
      }
      break;
    case 17:
      for (int i = 0; i <= 6; i++) {
        sP.a_day[i] = 1;
        sP.alarm_time[i] = _time;
      }
      break;
    case 55:    // "Sun" and "Sat"
      sP.a_day[0] = 1;
      sP.a_day[6] = 1;
      sP.alarm_time[0] = _time;
      sP.alarm_time[6] = _time;
      break;
  }
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
    case 4:
      sP.alarm_day = in % 1000;
      if (ptr->alarm_day > 0) {
        sP.alarm_day_set = true;      // set alarm #1 of 2
      } else {
        sP.alarm_day_set = false;
      }
    case 5:                           // autoBrightness on/off
      if ((in % 1000) == 1) {
        sP.autoBrightness = true;
      } else if ((in % 1000) == 0) {
        sP.autoBrightness = false;
      }
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

  if ((in / 10000) == 4) {
    sP.alarm_time_t = in % 10000;

    Alarm_Days(ptr->alarm_day, ptr->alarm_time_t);  // set alarm #2 of 2
    //Serial.println(ptr->alarm_time);
  }
}

void ListenBlt() {
  if (SerialBLE.available() > 0) {
    uint8_t count = 0;
    for (uint8_t i = 0; i < 4; i++) {
      // read input bytes
      sP.ch_data[i] = SerialBLE.read();
      delay(10);          // magic! for stable receiving
      // Serial.println("-----------");
      // Serial.println(ch_data[i], DEC);
      count++;
      // i use only 2 bytes (2^16-1), ex-pl:  0010 0011 1101 1100  = 9180 (int)
      // 2 bytes MAX = 65535
      if (count == 4) {
        // convert byte to int ;  may be,  int i = atoi(intBuffer) ?
        long int a = (long)(unsigned char)(sP.ch_data[0]) << 24 |
                     (long)(unsigned char)(sP.ch_data[1]) << 16 |
                     (int)(unsigned char)(sP.ch_data[2]) << 8 |
                     (int)(unsigned char)(sP.ch_data[3]);

        //Serial.println(a, DEC);
        GetCommand(a);
        count = 0;
      }
    }
  }
}

void PinStatus() {
  sP.relayStatus = digitalRead(ptr->relayPin);  // relayState
  sP.pirStatus = digitalRead(ptr->pirInputPin); // pirState
  sP.photo = analogRead(ptr->pinPhoto);         // photoResistor

  //  Serial.println(
  //    "p" + (String)ptr->pirInputPin + "u" +
  //    "l" + (String)ptr->relayPin + "w" +
  //    "E");
}

void PIR(uint8_t val) {
  if (val == 1) {  // check if the input is 1
    digitalWrite(sP.relayPin, 0);  // turn LED ON
    RGBStrip(ptr->r_in_def, ptr->g_in_def, ptr->b_in_def);
    if (ptr->pirStatus == 0) {
      //Serial.println("Motion detected!");
      sP.pirStatus = 1;
    }
  } else if (ptr->pirStatus == 1) {
    //Serial.println("Motion ended!");
    sP.pirStatus = 0;
  }
}
