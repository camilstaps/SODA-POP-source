# SODA POP source

This repository contains the source for the [SODA POP][sodapop] by [Steve Weber
KD1JV][kd1jv], a single band QRP CW transceiver on a 3.6" square board.

The original code is copyright &copy; [Steven Weber KD1JV][kd1jv], additions
and changes are copyright &copy; [Camil Staps PD7LOL][cs]. This only goes for
the `SODA_POP` directory. Everything is licensed under GPL v3, see the LICENSE
file.

---

- [Operation](#operation)
- [Settings](#settings)
- [Optional features](#optional-features)
- [How to flash the firmware](#how-to-flash-the-firmware)
- [Changelog](#changelog)

---

## Operation

To a large extent, operation is as described in the manual.
It can be described by the state machine below.
A detailed description is given under the image.
Dashed states and edges are [optional features](#optional-features).

![State machine](README/states.png)

### Startup
On startup, the band will be shown (e.g. `bn.20`). After a 1s delay, the rig
will turn on.

### Tuning
Use the rotary encoder to tune. Tuning can be done in steps of 50Hz, 200Hz,
1kHz and 10kHz. Rotate through these steps by pressing the rotary encoder. For
all steps except 50Hz, the corresponding digit on the display will blink.

Direct Frequency Entry (DFE) can be used by holding the encoder button for 1s.
It is only available when a paddle is connected. The display will read `DFE`.
Key in the desired frequency. The current digit blinks. Save with the keyer
switch or cancel with the RIT switch. When all four digits are entered, the
new frequency is saved automatically. Abbreviations can be used for 0 (T) and 9
(N) and for all numbers when enabled (see under
[Optional features](#optional-features)).

### RIT
Pressing the RIT button turns RIT on. The display will show the RIT offset.

### Message memory
Pressing the keyer switch allows you to send a message from memory. There are
two messages, use either side of the paddle to send one. Use the RIT switch
to cancel.

When a message is being transmitted, you can still use the RIT switch to cancel
it. You can also enter beacon mode by pressing the keyer switch. In beacon
mode, the message is repeated continuously with an adjustable delay in between
(see `BEACON_INTERVAL` under [Settings](#settings)). During transmission, the
keys are only checked *between* the transmitted characters, so you'll have to
hold the buttons longer than normally.

To update the memory, hold the keyer switch for 5s. The display will say
`Entr.`. Enter the message using the paddle. This is not possible with a
straight key. The display will flash once after a character space is detected,
and once again after a word space has been detected. To finish, press the keyer
switch again. The message will be played back while the display reads `St.?`
(from 'store'). After this, store the message with either side of the paddle.
Pressing the RIT switch allows you to key in a message again; pressing the RIT
switch once more returns to the default state.

The maximum message length is 64 by default (can be changed in `settings.h`).
If you try to enter more characters, the error routine is enabled (see below).

Using `OPT_MORE_MEMORIES`, it is possible to use up to 10 messages, that can be
selected using the rotary encoder. See under
[Optional features](#optional-features).

### Preferences
Change the code speed by holding the keyer switch for 2s. The display will read
`CS.20`, where `20` is the speed in WPM. Use the paddle or the rotary encoder
to change, and save with the keyer switch.

Change the band by holding the RIT switch for 2s. The display will read
`bn.20`, where `20` is the band. Save with the keyer switch. This is only
possible when compiled with `OPT_BAND_SELECT` (see
[Optional features](#optional-features) below).

### Calibration
The calibration routine is explained in the manual. Hold the RIT switch for 5s
to enter the calibration routine.

The display will read `corr.`. You can correct the Si5351 by connecting a
frequency counter to TP3 and fixing it to 10MHz using the rotary encoder.

Move on with the keyer switch (the display will read `P. IF`). Adjust CT3 to
peak the signal on TP2. Peak the IF LO frequency using the rotary encoder,
peaking the signal on TP1 *with a scope*. If you changed this, go back to
changing CT3 and repeat.

Move on with the keyer switch (the display will read `bn.16`) and set the band
using the rotary encoder.

Move on with the keyer switch (the display will read `P. rX`) and peak the
signal using CT1 and CT2. The adjustment can be sharp, especially on higher
frequency bands.

Pressing the keyer switch will return to the default state.

### Errors
When an error is detected, the display will read `Err.` and a alarm will be
sounded. The only way to recover is to restart the rig.

## Settings
There are several compile-time settings in `settings.h`. Change them before
uploading the code to the chip.

- `WPM_DEFAULT`: the default key speed in WPM (20).
- `KEY_MIN_SPEED`: the minimum key speed in WPM (5). Lower speeds than 5 may
  damage the rig, because the on-time for dashes will be rather long.
- `KEY_MAX_SPEED`: the maximum key speed in WPM (30). Higher speeds than 30 are
  untested and may present timing issues.
- `SIDETONE_FREQ`: the frequency of the sidetone in Hz (600).
- `MEMORY_LENGTH`: the maximum length of messages in memory, including word
  spaces (64). Higher values than 255 are unsupported.
- `MEMORY_EEPROM_START`: the start address of the memory in EEPROM (16). Don't
  change this unless you know what you're doing.
- `BEACON_INTERVAL`: the number of dot times between two transmissions of a
  message in beacon mode. A dot time is 1.2s / WPM (e.g. a beacon interval of
  15 means 1.5s on 10WPM).
- `TUNING_STEPS`: an array of tuning steps in mHz. These steps are rotated
  through with the encoder button.
- `TUNING_STEP_DIGITS`: which digit to blink when in a tuning step. An array of
  the same length as `TUNING_STEPS`. Values should be taken from `BLINK_NONE`,
  `BLINK_0`, `_1`, `_2` and `_3` (`0` is the rightmost digit).
- There are several band plans. Define one of `PLAN_IARU1`, `_IARU2`, `_IARU3`,
  `_VK`.
  The exact boundary definitions are in `bands.h`.
- Change the default operating frequency of a band by defining e.g.

      #define DEFAULT_OP_FREQ_20 1405500000

## Optional features
There are several features that can be added to the rig if you want to. This is
done by adding and removing `#define` lines to `settings.h`.

- `OPT_BAND_SELECT`: change the band by pressing RIT for 2s.
- `OPT_ERASE_EEPROM`: erase the EEPROM by holding RIT for 8s.
- `OPT_STORE_CW_SPEED`: store the CW speed in EEPROM.
- `OPT_DFE`: direct frequency entry by holding the encoder button for 1s.
- `OPT_DFE_OBSCURE_ABBREVIATIONS`: adds number abbreviations to DFE according
  to the table below. Abbreviations for 0 (T) and 9 (N) are always enabled.

  | Letter | Number
  ---|---
  | A | 1
  | U | 2
  | W | 3
  | V | 4
  | S | 5
  | B | 6
  | G | 7
  | D | 8
- `OPT_DISABLE_DISPLAY`: disables the display when in default state after a
  preset time (default: 2.5s). The buttons still work and when something
  happens the display turns on again. The encoder button enables the display
  without doing anything else. The display will blink for 0.5s approximately
  every 30s to prevent you from leaving the rig on by accident.
  This saves about 2.5mA (on 59mA total in RX mode).
- `OPT_MORE_MEMORIES`: allows for up to ten message memories (0 through 9),
  that can be selected using the rotary encoder instead of with the paddle.
  Turning the encoder changes the index, pressing the keyer switch selects that
  memory.

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

- 2017-05-12:
  - VK band plans (PR [#21](/../../pull/21) by VK3IL)
- 2017-05-04:
  - Beacon mode (issue [#6](/../../issues/6))
  - More logical UX for sending and storing memories, using RIT to cancel
  - More memories can be selected by entering a number with the paddle (issue
    [#16](/../../issues/16))
  - `OPT_DFE_OBSCURE_ABBREVIATIONS` is now `OPT_OBSCURE_MORSE_ABBREVIATIONS`
- 2017-05-02:
  - Power saving mode: the last dot now blinks when the display is off (issue
    [#18](/../../issues/18))
  - The old IF frequency is used in the calibration routine
  - Added band plans for IARU-1,2,3 (issue [#13](/../../issues/13))
- 2017-04-23 and -25:
  - Fix timing issues when building with new versions of the Arduino IDE
- 2017-04-21:
  - Added up to 10 message memories (issue [#11](/../../issues/11))
  - Allow for true QSK (issue [#12](/../../issues/12))
  - Allow for user-defined default frequencies (issue [#15](/../../issues/15))
  - Minor fixes
- 2017-04-13:
  - Made tuning steps an option in `settings.h`
- 2017-04-12:
  - Added `OPT_DISABLE_DISPLAY` (issue [#5](/../../issues/5)) (saves ~2.5mA)
  - More power saving by entering sleep mode at the end of `loop()` (~6mA)
  - Added comments to clarify the code
- 2017-04-11:
  - Rewrote all code to a state machine
  - Fixed a bug with entering memory
  - Minor changes to the display
  - Made several things settings (see `settings.h`)
  - Big steps tuning (issue [#3](/../../issues/3))
  - Direct frequency entry (issue [#2](/../../issues/2))
  - Store CW speed in EEPROM (issue [#1](/../../issues/1))
- 2017-04-04:
  - Added bands up to 10m and enabled run-time band switching
  - Fixed rotary encoder issues

[cs]: https://camilstaps.nl
[kd1jv]: http://kd1jv.qrpradio.com/
[sodapop]: https://groups.yahoo.com/neo/groups/AT_Sprint/files/SODA%20POP/
[uno]: https://www.arduino.cc/en/Main/ArduinoBoardUno
[uno-prog]: https://www.arduino.cc/en/Tutorial/ArduinoToBreadboard
