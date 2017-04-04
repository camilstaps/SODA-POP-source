# SODA POP source

This repository contains the source for the [SODA POP][sodapop] by [Steve Weber
KD1JV][kd1jv], a single band QRP CW transceiver on a 3.6" square board.

The original code is copyright &copy; [Steven Weber KD1JV][kd1jv], additions
and changes are copyright &copy; [Camil Staps PD7LOL][cs]. This only goes for
the `SODA_POP` directory. Everything is licensed under GPL v3, see the LICENSE
file.

---

- [How to flash the firmware](#how-to-flash-the-firmware)
- [Changelog](#changelog)

---

## How to flash the firmware

By far the easiest way to reprogram the controller is using an
[Arduino Uno][uno].

If you are only planning on flashing new firmware once or twice, just take the
controller from the SODA POP, plug it in the Arduino, flash it, and put it
back.

If you are planning on working on the firmware yourself, you will need to flash
much more often. Not only is moving the chip back and forth annoying and
time-consuming, it will also break the pins eventually. It is possible to
program the controller using in-circuit serial programming (ICSP). This is very
similar to the [method described on the Arduino wiki][uno-prog].

Remove the Atmega chip from the Arduino and connect these four lines:

| Arduino | SODA POP
---|---
| RESET | 1
| RX (0) | 2
| TX (1) | 3
| GND | 8

**Do not connect Vdd**: to program the board, you will connect a USB cable.
The Arduino will be powered from the USB port, the SODA POP from its internal
power supply. Connecting the power lines may cause problems if the voltages
are not exactly the same.

Programming the board can now be done using the Arduino IDE.

Some images of the connections:

![SODA POP ICSP connector](README/icsp-connector-sodapop.jpg)
![Arduino ICSP connector](README/icsp-connector-arduino.jpg)

## Changelog

- 2017-04-04:
	- Added bands up to 10m and enabled run-time band switching
	- Fixed rotary encoder issues

[cs]: https://camilstaps.nl
[kd1jv]: http://kd1jv.qrpradio.com/
[sodapop]: https://groups.yahoo.com/neo/groups/AT_Sprint/files/SODA%20POP/
[uno]: https://www.arduino.cc/en/Main/ArduinoBoardUno
[uno-prog]: https://www.arduino.cc/en/Tutorial/ArduinoToBreadboard
