/* Copyright (C) 2017 Camil Staps <pd7lol@camilstaps.nl> */

#include "display.h"

volatile byte digit_counter = 0;
#define BLINKED_ON ((((byte) tcount) >> 7) & 0x01)

/**
 * The ISR for the display. Should be called regularly (ideally from a timer
 * ISR). Multiplexes the display: every time the function is called, one digit
 * is shown.
 */
void display_isr()
{
#ifdef OPT_DISABLE_DISPLAY
  if ((state.state == S_DEFAULT || state.state == S_KEYING)
      && state.idle_for >= DISABLE_DISPLAY_AFTER
      && !state.inputs.port) {
    digit_counter++;
    if (!(digit_counter & 0x07) && (digit_counter & 0x80)) {
      PORTD = 0x04;
      digitalWrite(SLED1, LOW);
    }
    return;
  }
#endif
  digit_counter = (digit_counter + 1) % 4;

  PORTD = 0x00;

  switch(digit_counter) {
    case 0:
      if (state.display.blinking != BLINK_0 || BLINKED_ON)
        PORTD = state.display.digits[0];
      digitalWrite(SLED1, LOW);
      break;

    case 1:
      if (state.display.blinking != BLINK_1 || BLINKED_ON)
        PORTD = state.display.digits[1];
      digitalWrite(SLED2, LOW);
      break;

    case 2:
      if (state.display.blinking != BLINK_2 || BLINKED_ON)
        PORTD = state.display.digits[2];
      digitalWrite(SLED3, LOW);
      break;

    case 3:
      if (state.display.blinking != BLINK_3 || BLINKED_ON)
        PORTD = state.display.digits[3];
      digitalWrite(SLED4, LOW);
      break;
  }

  if (state.display.dots & (0x01 << digit_counter))
    digitalWrite(2, HIGH);
}

/**
 * Disable the display by setting the common cathodes high.
 */
void disable_display()
{
  digitalWrite(SLED1, HIGH);
  digitalWrite(SLED2, HIGH);
  digitalWrite(SLED3, HIGH);
  digitalWrite(SLED4, HIGH);
}

/**
 * Recompute the display according to the current state.
 */
void invalidate_display()
{
  state.display.blinking = BLINK_NONE;
  switch (state.state) {
    case S_DEFAULT:
    case S_KEYING:
      if (state.rit)
        display_rit();
      else
        display_freq();
      break;
    case S_ADJUST_CS:
      display_cs();
      break;
    case S_STARTUP:
#ifdef OPT_BAND_SELECT
    case S_CHANGE_BAND:
#endif
    case S_CALIBRATION_CHANGE_BAND:
      display_band();
      break;
#ifdef OPT_DFE
    case S_DFE:
      display_dfe();
      break;
#endif
    case S_MEM_ENTER_WAIT:
    case S_MEM_ENTER:
      state.display.digits[3] = LED_E;
      state.display.digits[2] = LED_n;
      state.display.digits[1] = LED_t;
      state.display.digits[0] = LED_r;
      state.display.dots = 0x1;
      break;
    case S_MEM_ENTER_REVIEW:
      state.display.digits[3] = LED_S;
      state.display.digits[2] = LED_t;
      state.display.digits[1] = LED_question;
#ifdef OPT_MORE_MEMORIES
      state.display.digits[0] = LED_DIGITS[memory_index];
#else
      state.display.digits[0] = 0x00;
#endif
      state.display.dots = 0x4;
      break;
    case S_MEM_SEND_WAIT:
    case S_MEM_SEND_TX:
      state.display.digits[3] = LED_S;
      state.display.digits[2] = LED_n;
      state.display.digits[1] = LED_d;
#ifdef OPT_MORE_MEMORIES
      state.display.digits[0] = LED_DIGITS[memory_index];
#else
      state.display.digits[0] = 0x00;
#endif
      state.display.dots = 0x2;
      break;
    case S_CALIBRATION_CORRECTION:
      state.display.digits[3] = LED_c;
      state.display.digits[2] = LED_o;
      state.display.digits[1] = LED_r;
      state.display.digits[0] = LED_r;
      state.display.dots = 0x1;
      break;
    case S_CALIBRATION_PEAK_IF:
      state.display.digits[3] = LED_P;
      state.display.digits[2] = 0x00;
      state.display.digits[1] = LED_I;
      state.display.digits[0] = LED_F;
      state.display.dots = 0x9;
      break;
    case S_CALIBRATION_PEAK_RX:
      state.display.digits[3] = LED_P;
      state.display.digits[2] = 0x00;
      state.display.digits[1] = LED_r;
      state.display.digits[0] = LED_X;
      state.display.dots = 0x9;
      break;
    case S_ERROR:
      state.display.digits[3] = LED_E;
      state.display.digits[2] = LED_r;
      state.display.digits[1] = LED_r;
      state.display.digits[0] = 0x00;
      state.display.dots = 0x2;
      break;
  }
}

/**
 * Toggle one digit for 100ms.
 * Also see toggle_display().
 *
 * @param i the digit to toggle (3 is leftmost).
 */
void toggle_digit(byte i)
{
  byte d1temp = state.display.digits[i];
  state.display.digits[i] = 0x00;
  delay(100);
  state.display.digits[i] = d1temp;
}

/**
 * Turn the display off for 100ms, then turn it back on.
 * Also see toggle_digit().
 */
void toggle_display()
{
  byte temp[5];
  temp[0] = state.display.digits[0];
  temp[1] = state.display.digits[1];
  temp[2] = state.display.digits[2];
  temp[3] = state.display.digits[3];
  temp[4] = state.display.dots;
  state.display.digits[0] = 0x00;
  state.display.digits[1] = 0x00;
  state.display.digits[2] = 0x00;
  state.display.digits[3] = 0x00;
  state.display.dots = 0x0;
  delay(100);
  state.display.digits[0] = temp[0];
  state.display.digits[1] = temp[1];
  state.display.digits[2] = temp[2];
  state.display.digits[3] = temp[3];
  state.display.dots = temp[4];
}

/**
 * Displays the code speed in WPM on the last two digits and a dot before these
 * digits. The other digits are unchanged.
 */
void display_cs()
{
  state.display.digits[0] = LED_DIGITS[state.key.speed % 10];
  state.display.digits[1] = LED_DIGITS[state.key.speed / 10];
  state.display.dots = 0x4;
}

/**
 * Displays `r` and the RIT offset in kHz.
 * Above 9.9 and below -9.9, the display overflows.
 */
void display_rit()
{
  unsigned long offset;

  if (state.rit_tx_freq > state.op_freq) {
    offset = state.rit_tx_freq - state.op_freq;
    state.display.digits[2] = LED_neg;
  } else {
    offset = state.op_freq - state.rit_tx_freq;
    state.display.digits[2] = 0x00;
  }

  state.display.digits[3] = LED_r;
  offset /= 100;
  state.display.digits[1] = LED_DIGITS[(offset % 10000) / 1000];
  state.display.digits[0] = LED_DIGITS[(offset % 1000) / 100];

  state.display.dots = 0x2;
  state.display.blinking = tuning_blinks[state.tuning_step];
}

/**
 * Displays the current frequency in kHz.
 * Blinks a digit when the tuning step is set large.
 */
void display_freq()
{
  // First divide by 100 to remove the fractional Hz digits
  unsigned long frequency = state.op_freq/100;
  // Then display the digits one by one
  state.display.digits[3] = LED_DIGITS[(frequency % 1000000) / 100000];
  if (state.display.digits[3] == LED_N_0)
    state.display.digits[3] = 0x00; //blank MSD if 0
  state.display.digits[2] = LED_DIGITS[(frequency % 100000) / 10000];
  state.display.digits[1] = LED_DIGITS[(frequency % 10000) / 1000];
  state.display.digits[0] = LED_DIGITS[(frequency % 1000) / 100];
  state.display.dots = 0x2;
  state.display.blinking = tuning_blinks[state.tuning_step];
}

#ifdef OPT_DFE
/**
 * Displays `dFE.` when no digit has been keyed in yet in DFE mode.
 * After this, displays the frequency that is being keyed in, blinking the
 * current digit.
 */
void display_dfe()
{
  if (dfe_position == 3) {
    state.display.digits[3] = LED_d;
    state.display.digits[2] = LED_F;
    state.display.digits[1] = LED_E;
    state.display.digits[0] = 0x00;
    state.display.dots = 0x0;
  } else {
    state.display.digits[3] = LED_DIGITS[(dfe_freq % 10000) / 1000];
    if (state.display.digits[3] == LED_N_0)
      state.display.digits[3] = 0x00; //blank MSD if 0
    state.display.digits[2] = LED_DIGITS[(dfe_freq % 1000) / 100];
    state.display.digits[1] = LED_DIGITS[(dfe_freq % 100) / 10];
    state.display.digits[0] = LED_DIGITS[dfe_freq % 10];
    state.display.dots = 0x2;
    state.display.blinking = dfe_position + 1;
  }
}
#endif

/**
 * Displays `bn.` and two digits for the current band (see bands.h)
 */
void display_band()
{
  state.display.digits[3] = LED_N_6;
  state.display.digits[2] = LED_n;
  state.display.digits[1] = LED_DIGITS[BAND_DIGITS_2[state.band]];
  state.display.digits[0] = LED_DIGITS[BAND_DIGITS_1[state.band]];
  state.display.dots = 0x4;
}

// vim: tabstop=2 shiftwidth=2 expandtab:
