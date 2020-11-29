# RGBStripControl_Arduino
Project for managing rgb backlighting with arduino and android applications.

# How it works:
- PIR motion sensor (HC-SR501) to turn On or Off RGB led strip through solid state relay (G3MB-202P);
- Bluetooth (HC-06 v2.0) to communicate between led strip and android application - set color, brightness,  turn on/off;
- Arduino PWM pins through MOSFET transistors (IRL540PBF) to set color.

<p>You can find schematic in the pdf - <a href="https://github.com/Valentin-Golyonko/RGBStripControl_Arduino/blob/master/RGBStripControl_Arduino/RGBStripControl_Arduino.pdf">RGBStripControl_Arduino.pdf</a>, or inside "RGBStripControl_Arduino" holder</p>

<p>Arduino firmware file:
<a href="https://github.com/Valentin-Golyonko/RGBStripControl_Arduino/blob/master/RGBStripControl_Arduino/RGBStripControl_Arduino.ino">RGBStripControl_Arduino.ino</a></p>

# Board view (v.2):
<p><img src="https://github.com/Valentin-Golyonko/RGBStripControl_Arduino/blob/master/RGBStripControl_Arduino/board_view_1(v.2).jpg" alt="board view"></p>
<p><img src="https://github.com/Valentin-Golyonko/RGBStripControl_Arduino/blob/master/RGBStripControl_Arduino/board_view_2(v.2).jpg" alt="board view"></p>


# Android app:
<img src="https://github.com/Valentin-Golyonko/RGBStripControl_Arduino/blob/master/RGBStripControl_Arduino/android_app_view.png" alt="Android app">
