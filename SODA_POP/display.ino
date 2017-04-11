#include "display.h"

volatile byte digit_counter = 0;
void display_isr()
{
  digit_counter = (digit_counter + 1) % 4;

  switch(digit_counter) {
    case 0:
      PORTD = state.display.digits[0];
      digitalWrite(SLED1, LOW);
      break;

    case 1:
      PORTD = state.display.digits[1];
      digitalWrite(SLED2, LOW);
      break;

    case 2:
      PORTD = state.display.digits[2];
      digitalWrite(SLED3, LOW);
      break;

    case 3:
      if (state.display.digits[3] == LED_N_0)
        state.display.digits[3] = 0x00; //blank MSD if 0
      PORTD = state.display.digits[3];
      digitalWrite(SLED4, LOW);
      break;
  }

  if (state.display.dots & (0x01 << digit_counter))
    digitalWrite(2, HIGH);
}

void disable_display()
{
  digitalWrite(SLED1, HIGH);
  digitalWrite(SLED2, HIGH);
  digitalWrite(SLED3, HIGH);
  digitalWrite(SLED4, HIGH);
}

void invalidate_display()
{
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
    case S_CHANGE_BAND:
    case S_CALIBRATION_CHANGE_BAND:
      display_band();
      break;
    case S_MEM_ENTER_WAIT:
    case S_MEM_ENTER:
      state.display.digits[3] = LED_E;
      state.display.digits[2] = LED_n;
      state.display.digits[1] = LED_t;
      state.display.digits[0] = LED_r;
      state.display.dots = 0x1;
      break;
    case S_MEM_ENTER_REVIEW:
      state.display.digits[3] = LED_E;
      state.display.digits[2] = LED_n;
      state.display.digits[1] = LED_t;
      state.display.digits[0] = LED_question;
      state.display.dots = 0x2;
      break;
    case S_MEM_SEND_WAIT:
    case S_MEM_SEND_TX:
      state.display.digits[3] = LED_N_5;
      state.display.digits[2] = LED_E;
      state.display.digits[1] = LED_n;
      state.display.digits[0] = LED_d;
      state.display.dots = 0x0;
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

void toggle_digit(byte i)
{
  byte d1temp = state.display.digits[i];
  state.display.digits[i] = 0x00;
  delay(100);
  state.display.digits[i] = d1temp;
}

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

void display_cs()
{
  state.display.digits[0] = LED_DIGITS[state.key.speed % 10];
  state.display.digits[1] = LED_DIGITS[state.key.speed / 10];
  state.display.dots = 0x4;
}

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
}

void display_freq()
{
  // First divide by 100 to remove the fractional Hz digits
  unsigned long frequency = state.op_freq/100;
  // Then display the digits one by one
  state.display.digits[3] = LED_DIGITS[(frequency % 1000000) / 100000];
  state.display.digits[2] = LED_DIGITS[(frequency % 100000) / 10000];
  state.display.digits[1] = LED_DIGITS[(frequency % 10000) / 1000];
  state.display.digits[0] = LED_DIGITS[(frequency % 1000) / 100];
  state.display.dots = 0x2;
}

void display_band()
{
  state.display.digits[3] = LED_N_6;
  state.display.digits[2] = LED_n;
  state.display.digits[1] = LED_DIGITS[BAND_DIGITS_2[state.band]];
  state.display.digits[0] = LED_DIGITS[BAND_DIGITS_1[state.band]];
  state.display.dots = 0x4;
}
