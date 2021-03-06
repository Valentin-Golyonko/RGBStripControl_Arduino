#include <stdint.h>
#include <SoftwareSerial.h>
// HC-06 blt 2.0: not connected: 5 - 50 mA | connected: 4.3 - idle, 20 - receive
// HM-10 ble 4.0: not connected: 8.6-9.3 mA | connected: 9.3 - idle, 9.3 - receive
#include <Wire.h>
#include "RTClib.h"               // 1.781 mA (china) | 6.15 mA (robotdyn)

struct input {
  const uint8_t RX = 7;           // BLT
  const uint8_t TX = 6;

  const uint8_t SDA = A4;         // I2C
  const uint8_t SCL = A5;

  const uint8_t BlueLedPin = 8;   // power indicator
  const uint8_t relayPin = 12;
  const uint8_t pirInputPin = 4;  // choose the input pin for PIR sensor
  const uint8_t pinBuzzer = 3;
  const uint8_t pinPhoto = A0;    // photo resistor

  const uint8_t REDPIN = 10;      // RGB Strip pins
  const uint8_t GREENPIN = 11;
  const uint8_t BLUEPIN = 9;

  uint16_t photo = 0;             // data from it
  bool buzzer_play = false;       // play/stop sound

  bool alarm_on = false;
  uint8_t alarm_duration = 5;     // minutes
  uint8_t alarm_day = 0;          // code: 4001 - 4017
  uint8_t a_day[7] = {1, 1, 1, 1, 1, 1, 1}; // {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  uint16_t alarm_time[7] = {930, 630, 630, 630, 630, 630, 930};
  uint16_t alarm_time_t = 0630;   // code - 40630
  bool alarm = false;
  bool alarm_day_set = true;
  bool repeat = true;             // repeat alarm

  bool autoBrightness = true;     // autoBrightness on/off
  uint8_t autoB = 1;
  uint16_t period = 5000;

  bool light_always = false;
  uint8_t r_in = 100;             // RGB default before receive
  uint8_t g_in = 100;
  uint8_t b_in = 100;
  const uint8_t r_in_def = 125;   // RGB default
  const uint8_t g_in_def = 125;
  const uint8_t b_in_def = 125;
  const uint8_t r_in_alarm = 35;  // RGB Alarm Color
  const uint8_t g_in_alarm = 250;
  const uint8_t b_in_alarm = 5;
  bool done = false;          // flag for receive all 3 colors

  uint8_t relayStatus = 1;    // pin status (def-t = OFF)
  uint8_t pirStatus = 0;      // we start, assuming no motion detected (def-t = OFF)
  bool blt = false;           // BLT transmission ON/OFF
  unsigned char ch_data[4];   // store incoming date from BLT
};

struct input sP;
struct input *ptr = &sP;

SoftwareSerial SerialBLE(sP.RX, sP.TX); // RX,TX on arduino board

RTC_DS3231 rtc;

uint8_t brightness() {
  if (ptr->autoBrightness) {
    sP.autoB = 1;
  } else {
    sP.autoB = 0;
  }
  return ptr->autoB;
}

void Transmit() {
  SerialBLE.println(
    "p" + String(ptr->pirStatus) + "u" +
    "l" + String(ptr->relayStatus) + "w" +
    "f" + String(ptr->photo) + "r" +
    "a" + String(brightness()) + "b" +
    "E"); // end of the line
}

void RGBStrip(uint8_t r, uint8_t g, uint8_t b) {
  float multiplier = 1.0f;
  if (ptr->autoBrightness) {
    multiplier = 1.0f - (float)((sP.photo + 1.0f) / 1025.0f);   // Max outer light -> min RGBStrip brightness
  }

  analogWrite(ptr->REDPIN , r * multiplier);
  analogWrite(ptr->GREENPIN , g * multiplier);
  analogWrite(ptr->BLUEPIN , b * multiplier);
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
        if (rtc_minute >= alarm_time_m) {                             // start min
          if (rtc_minute <= alarm_time_m + sP.alarm_duration - 1) {   // stop min

            //Serial.println("ALARM ON " + String(alarm_time_h) + ":" + String(alarm_time_m));
            digitalWrite(sP.relayPin, 0);   // turn LED ON
            RGBStrip(ptr->r_in_alarm, ptr->g_in_alarm, ptr->b_in_alarm);
            sP.light_always = true;

            if (!sP.alarm_on) {
              sP.buzzer_play = true;        // 1 time music play
              sP.alarm_on = true;
            }
          } else if (sP.alarm_on) {
            digitalWrite(sP.relayPin, 1);   // turn LED OFF
            RGBStrip(0, 0, 0);              // rgb off
            sP.alarm_on = false;
            sP.light_always = false;

            if (ptr->repeat) {
              sP.alarm_time[rtc_day] += 10; // 0630 + 10 = 0640
            }
          }
        }
      }
    }
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
      break;
    case 2: // stop date transmit
      sP.blt = false;
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
        sP.alarm_day_set = true;      // allow alarm
      } else {
        sP.alarm_day_set = false;
      }
      break;
    case 5:                           // autoBrightness on/off
      if ((in % 1000) == 1) {
        sP.autoBrightness = true;
      } else if ((in % 1000) == 0) {
        sP.autoBrightness = false;
      }
      break;
    case 6:
      sP.a_day[in % 1000] = 0;
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
      sP.done = true;  // allow to fire the RGBStrip
      break;
  }

  if ((in / 10000) == 4) {
    uint8_t d = ptr->alarm_day - 1;
    sP.a_day[d] = 1;
    sP.alarm_time[d] = in % 10000;
  }
}

void ListenBlt() {
  if (SerialBLE.available() > 0) {
    uint8_t count = 0;
    for (uint8_t i = 0; i < 4; i++) {
      // read input bytes
      sP.ch_data[i] = SerialBLE.read();
      delay(10);          // magic! for stable receiving
      //Serial.println("-----------");
      //Serial.println(sP.ch_data[i], DEC);
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
      sP.pirStatus = 1;
    }
  } else if (ptr->pirStatus == 1) {
    sP.pirStatus = 0;
  }
}

void Buzzer() {
  tone(ptr->pinBuzzer, 1000);   // Send 1KHz sound signal...
  delay(100);
  noTone(ptr->pinBuzzer);       // Stop sound...
}
