#include "buttons.h"

void buttons_isr()
{
  DDRD = 0x00;
  PORTD= 0Xff;
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
    delay(1);
  } while (state.inputs.rit);
  debounce_rit();

  return duration;
}

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
      state.display.digits[3] = LED_N_5;
      state.display.digits[2] = LED_E;
      state.display.digits[1] = LED_n;
      state.display.digits[0] = LED_d;
      state.display.dots = 0x0;
    }
    delay(1); //for some reason a delay call has to be done when doing bit read flag tests or it locks up
    //this doesn't seem to be a problem when doing digital reads of a port pin instead.
  } while (state.inputs.keyer); // wait until the bit goes high.
  debounce_keyer();

  return duration;
}

unsigned int time_encoder_button()
{
  unsigned int duration = 0;
  unsigned long start_time = tcount;

  do {
    duration = tcount - start_time;
#ifdef OPT_DFE
    if (duration > 1000) {
      state.display.digits[3] = LED_D;
      state.display.digits[2] = LED_F;
      state.display.digits[1] = LED_E;
      state.display.digits[0] = 0x00;
      state.display.dots = 0x0;
    }
#endif
    delay(1);
  } while (state.inputs.encoder_button);
  debounce_encoder_button();

  return duration;
}

void debounce_keyer()
{
  while (state.inputs.keyer)
    delay(50);
}

void debounce_rit()
{
  while (state.inputs.rit)
    delay(50);
}

void debounce_encoder_button()
{
  while (state.inputs.encoder_button)
    delay(50);
}

byte rotated_down()
{
  if (!state.inputs.down)
    return 0;
  state.inputs.down = 0;
  return 1;
}

byte rotated_up()
{
  if (!state.inputs.up)
    return 0;
  state.inputs.up = 0;
  return 1;
}
