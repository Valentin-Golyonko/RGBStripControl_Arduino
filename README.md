# RGBStripControl_Arduino
Project for managing rgb backlighting with arduino and android applications.

<p>Arduino nano processes date from:</p>
<p>&nbsp&nbsp 1) PIR motion sensor (HC-SR501) to turn On or Off RGB led strip through solid state relay (G3MB-202P);</p>
<p>&nbsp&nbsp 2) bluetooth (HC-06 v2.0) to communicate between led strip and android application - set color, brightness,  turn on/off;</p>
<p>&nbsp&nbsp 3) arduino PWM pins through MOSFET transistors (IRL540PBF) to set color.</p>
<p>You can find android app view, images, schematic and PCB in the pdf - <a href="https://github.com/Valentin-Golyonko/RGBStripControl_Arduino/blob/master/RGBStripControl_Arduino/RGBStripControl_Arduino.pdf">RGBStripControl_Arduino.pdf</a>, or inside "RGBStripControl_Arduino" holder</p>

<p>Arduino firmware file:
<a href="https://github.com/Valentin-Golyonko/RGBStripControl_Arduino/blob/master/RGBStripControl_Arduino/RGBStripControl_Arduino.ino">RGBStripControl_Arduino.ino</a></p>

<p>Board view:</p>
<img src="https://github.com/Valentin-Golyonko/RGBStripControl_Arduino/blob/master/RGBStripControl_Arduino/board_view.png" alt="board view">
