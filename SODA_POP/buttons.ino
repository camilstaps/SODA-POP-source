/* Copyright (C) 2017 Camil Staps <pd7lol@camilstaps.nl> */

#include "buttons.h"

/**
 * The ISR for buttons. Should be called regularly (ideally from a timer ISR).
 * Set PORTD to inputs and stores the state of buttons to state.inputs.
 * Checks if the rotary encoder was turned and updates state.inputs.encoders.
 * Restores PORTD to outputs.
 */
void buttons_isr()
{
  DDRD = 0x00;
  PORTD = 0xff;
  digitalWrite(8, LOW);
  for (byte i = 0; i < 10; i++)
    state.inputs.port = ~PIND; // debounce
  digitalWrite(8, HIGH);
  if (state.inputs.encoder_clock != state.inputs.encoder_last_clock) {
    if (!state.inputs.encoder_last_clock) {
      if (state.inputs.encoder_data)
        state.inputs.down = 1;
      else
        state.inputs.up = 1;
    }
    state.inputs.encoder_last_clock = state.inputs.encoder_clock;
  }
  DDRD = 0xff;
}

/**
 * Time how long the RIT button was pressed.
 * After 2s, the display will read `bn.` (if compiled with OPT_BAND_SELECT).
 * After 5s, the display will read `CAL.`.
 * After 8s, the display will read `ErAS` (if compiled with OPT_ERASE_EEPROM).
 */
unsigned int time_rit()
{
  unsigned int duration = 0;
  unsigned long start_time = tcount;

  do {
    duration = tcount - start_time;
#ifdef OPT_ERASE_EEPROM
    if (duration > 8000) {
      state.display.digits[3] = LED_E;
      state.display.digits[1] = LED_r;
      state.display.digits[1] = LED_A;
      state.display.digits[0] = LED_N_5;
      state.display.dots = 0x0;
    } else
#endif
    if (duration > 5000) {
      state.display.digits[3] = LED_C;
      state.display.digits[2] = LED_A;
      state.display.digits[1] = LED_L;
      state.display.digits[0] = 0x00;
      state.display.dots = 0x2;
    }
#ifdef OPT_BAND_SELECT
    else if (duration > 2000) {
      state.display.digits[3] = LED_N_6;
      state.display.digits[2] = LED_n;
      state.display.digits[1] = 0x00;
      state.display.digits[0] = 0x00;
      state.display.dots = 0x4;
    }
#endif
  } while (state.inputs.rit);
  debounce_rit();

  return duration;
}

/**
 * Time how long the keyer button was pressed.
 * After 50ms, the display will read `Send`.
 * After 500ms, the display will read `CS.`.
 * After 2s, the display will read `Entr.`.
 *
 * @return the time the button was pressed in ms.
 */
unsigned int time_keyer()
{
  unsigned int duration = 0;
  unsigned long start_time = tcount;

  do {
    duration = tcount - start_time;
    if (duration > 2000) {
      state.display.digits[3] = LED_E;
      state.display.digits[2] = LED_n;
      state.display.digits[1] = LED_t;
      state.display.digits[0] = LED_r;
      state.display.dots = 0x1;
    } else if (duration > 500) {
      state.display.digits[3] = LED_C;
      state.display.digits[2] = LED_N_5;
      state.display.digits[1] = 0x00;
      state.display.digits[0] = 0x00;
      state.display.dots = 0x4;
    } else if (duration > 50) {
      state.display.digits[3] = LED_S;
      state.display.digits[2] = LED_n;
      state.display.digits[1] = LED_d;
      state.display.digits[0] = 0x00;
      state.display.dots = 0x2;
    }
  } while (state.inputs.keyer); // wait until the bit goes high.
  debounce_keyer();

  return duration;
}

/**
 * Time how long the encoder button was pressed.
 * After 1s, the display is set to read `dFE`.
 *
 * @return the time the button was pressed in ms.
 */
unsigned int time_encoder_button()
{
  unsigned int duration = 0;
  unsigned long start_time = tcount;

  do {
    duration = tcount - start_time;
#ifdef OPT_DFE
    if (duration > 1000) {
      state.display.digits[3] = LED_d;
      state.display.digits[2] = LED_F;
      state.display.digits[1] = LED_E;
      state.display.digits[0] = 0x00;
      state.display.dots = 0x0;
    }
#endif
  } while (state.inputs.encoder_button);
  debounce_encoder_button();

  return duration;
}

/**
 * Debounche the keyer button.
 */
void debounce_keyer()
{
  do
    delay(50);
  while (state.inputs.keyer);
}

/**
 * Debounce the RIT button.
 */
void debounce_rit()
{
  do
    delay(50);
  while (state.inputs.rit);
}

/**
 * Debounce the rotary encoder button.
 */
void debounce_encoder_button()
{
  do
    delay(50);
  while (state.inputs.encoder_button);
}

/**
 * Check if the rotary encoder was turned downwards.
 * After checking, the status will be cleared.
 *
 * @return 1 if it was, 0 if not.
 */
byte rotated_down()
{
  if (!state.inputs.down)
    return 0;
  state.inputs.down = 0;
  return 1;
}

/**
 * Check if the rotary encoder was turned upwards.
 * After checking, the status will be cleared.
 *
 * @return 1 if it was, 0 if not.
 */
byte rotated_up()
{
  if (!state.inputs.up)
    return 0;
  state.inputs.up = 0;
  return 1;
}

// vim: tabstop=2 shiftwidth=2 expandtab:
