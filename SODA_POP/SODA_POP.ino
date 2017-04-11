/*
 * Copyright (C) 2017 Steven Weber KD1JV <steve.kd1jv@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 *In order to compile <Si5351Arduino-master> must be in the Arduino libary. This probram is
 *available from http://www.etherkit.com
 *
 *MEGA328P processor 16 MHz
 *Si5351A 3 output clock MOSP10
 *Gray code tuning encoder
 *160 to 17 meter Amatuer radio frequency bands preloaded.

 * Pin function:

 * A0 - dot paddle
 * A1 - dash paddle
 * A2 - tone
 * A3 - mute/qsk
 * A4 - SDA TWI
 * A5 - SCK TWI
 * 13 - TX enable
 *
 *switch input bits locations for sw_input register
 * 0 encoder a
 * 1 encoder b
 * 2 encoder PB
 * 3 unused
 * 4 unused
 * 5 unused
 * 6 Keyer mode switch
 * 7 RIT

 */

#include <EEPROM.h>
#include <Wire.h>
#include <si5351.h>

#include "SODA_POP.h"

Si5351 si5351;
#define SLED4 9
#define SLED3 10
#define SLED2 11
#define SLED1 12

#define SI5351_CLK_RX SI5351_CLK0
#define SI5351_CLK_TX SI5351_CLK1

#define RX_ON_TX_ON   0xff
#define RX_ON_TX_OFF  0xfe
#define RX_OFF_TX_ON  0xfd
#define RX_OFF_TX_OFF 0xfc

#define IF_DEFAULT 491480000

const byte LED_DIGITS[] =
  { LED_N_0, LED_N_1, LED_N_2, LED_N_3, LED_N_4
  , LED_N_5, LED_N_6, LED_N_7, LED_N_8, LED_N_9};

// register names
byte memory_pointer;

const int   MUTE = A3;
const int   TXEN = 13; // production A0
const int   DASHin = A0;
const int   DOTin = A1;

unsigned long IFfreq;
long          cal_value = 15000;

ISR (TIMER1_COMPA_vect)
{
  TIMER1_SERVICE_ROUTINE();
}

void setup()
{
  state.state = S_STARTUP;

  state.key.mode = KEY_IAMBIC;
#ifdef OPT_STORE_CW_SPEED
  state.key.speed = EEPROM.read(EEPROM_CW_SPEED);
  adjust_cs(0);
#else
  state.key.speed = WPM_DEFAULT;
#endif
  state.key.timeout = 1;
  state.key.dash = 0;
  state.key.dot = 0;
  load_cw_speed();

  //switch inputs
  DDRB = 0x3f;
  DDRD = 0Xff;

  pinMode(A0, INPUT_PULLUP);
  pinMode(A2, OUTPUT);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A3, OUTPUT);

  digitalWrite(MUTE, HIGH);
  digitalWrite(TXEN, LOW);
  si5351.init(SI5351_CRYSTAL_LOAD_6PF, 0); //set PLL xtal load
  enable_rx_tx(RX_ON_TX_OFF);

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  OCR1A = 238;
  TCCR1B = 0x0b;
  TIMSK1 |= 1 << OCIE1A;
  interrupts();

  state.band = (enum band) EEPROM.read(6); // check for operating band
  if (state.band == BAND_UNKNOWN) {
    state.band = (enum band) 0;
    state.state = S_CALIBRATION_CORRECTION;
  }

  cal_data(); //load calibration data
  si5351.set_correction(cal_value); //correct the clock chip error
  state.tuning_step = 0;
  setup_band();
  invalidate_display();
  delay(1000);

  state.state = S_DEFAULT;
  invalidate_display();
  digitalWrite(MUTE, LOW);

  if (digitalRead(DASHin) == LOW)
    state.key.mode = KEY_STRAIGHT;
}

void loop()
{
  switch (state.state) {
    case S_DEFAULT:                 loop_default(); break;
    case S_KEYING:                  loop_keying(); break;
    case S_ADJUST_CS:               loop_adjust_cs(); break;
#ifdef OPT_BAND_SELECT
    case S_CHANGE_BAND:             loop_change_band(); break;
#endif
#ifdef OPT_DFE
    case S_DFE:                     loop_dfe(); break;
#endif
    case S_MEM_ENTER_WAIT:          loop_mem_enter_wait(); break;
    case S_MEM_ENTER:               loop_mem_enter(); break;
    case S_MEM_ENTER_REVIEW:        loop_mem_enter_review(); break;
    case S_MEM_SEND_WAIT:           loop_mem_send_wait(); break;
    case S_MEM_SEND_TX:             break;
    case S_CALIBRATION_CORRECTION:  loop_calibration_correction(); break;
    case S_CALIBRATION_PEAK_IF:     loop_calibration_peak_if(); break;
    case S_CALIBRATION_CHANGE_BAND: loop_change_band(); break;
    case S_CALIBRATION_PEAK_RX:     loop_calibration_peak_rx(); break;
    case S_ERROR:                   loop_error(); break;
    default:
      error();
      break;
  }
}

void loop_default()
{
  unsigned int duration;
  if (key_active()) {
    state.state = S_KEYING;
  // Tuning with the rotary encoder
  } else if (rotated_up()) {
    freq_adjust(tuning_steps[state.tuning_step]);
  } else if (rotated_down()) {
    freq_adjust(-tuning_steps[state.tuning_step]);
  } else if (state.inputs.encoder_button) {
    duration = time_encoder_button();
#ifdef OPT_DFE
    if (duration > 1000) {
      if (state.key.mode == KEY_IAMBIC) {
        state.state = S_DFE;
        dfe_character = 0xff;
        dfe_position = 3;
        dfe_freq = 0;
        invalidate_display();
      } else {
        morse(MX);
      }
    } else
#endif
    if (duration > 50) {
      rotate_tuning_steps();
    }
  // Keyer switch for memory and code speed
  } else if (state.inputs.keyer) {
    duration = time_keyer();
    if (duration > 2000) {
      state.state = S_MEM_ENTER_WAIT;
      invalidate_display();
    } else if (duration > 500) {
      state.state = S_ADJUST_CS;
      invalidate_display();
    } else if (duration > 50) {
      state.state = S_MEM_SEND_WAIT;
      invalidate_display();
    }
  // RIT switch for RIT, changing band, calibration and erasing EEPROM
  } else if (state.inputs.rit) {
    duration = time_rit();
#ifdef OPT_ERASE_EEPROM
    if (duration > 8000) {
      ee_erase();
    } else
#endif
    if (duration > 5000) {
      state.state = S_CALIBRATION_CORRECTION;
      invalidate_display();
      calibration_set_correction();
      enable_rx_tx(RX_OFF_TX_ON);
    }
#ifdef OPT_BAND_SELECT
    else if (duration > 2000) {
      state.state = S_CHANGE_BAND;
      invalidate_display();
    }
#endif
    else if (duration > 50) {
      if (state.rit) {
        state.rit = 0;
        state.op_freq = state.rit_tx_freq;
        invalidate_frequencies();
      } else {
        state.rit = 1;
        state.rit_tx_freq = state.op_freq;
      }
      invalidate_display();
    }
  }
}

void loop_keying()
{
  if (state.key.mode == KEY_IAMBIC) {
    iambic_key();
  } else if (digitalRead(DOTin) == LOW) {
    straight_key();
  }
}

void loop_adjust_cs()
{
  // Rotary encoder
  if (rotated_up()) {
    adjust_cs(1);
  } else if (rotated_down()) {
    adjust_cs(-1);
  // Paddle
  } else if (!digitalRead(DASHin)) {
    adjust_cs(1);
    delay(200);
  } else if (!digitalRead(DOTin)) {
    adjust_cs(-1);
    delay(200);
  // Exiting
  } else if (state.inputs.keyer) {
    state.state = S_DEFAULT;
    load_cw_speed();
    store_cw_speed();
    invalidate_display();
    debounce_keyer();
  }
}

void loop_change_band()
{
  // Change with rotary encoder
  if (rotated_up()) {
    nextband(1);
  } else if (rotated_down()) {
    nextband(0);
  // Save with keyer
  } else if (state.inputs.keyer) {
    store_band();

    if (state.state == S_CALIBRATION_CHANGE_BAND) {
      state.state = S_CALIBRATION_PEAK_RX;
      invalidate_frequencies();
      enable_rx_tx(RX_OFF_TX_OFF);
    } else {
      state.state = S_DEFAULT;
    }

    invalidate_display();
    debounce_keyer();
  }
}

#ifdef OPT_DFE
void loop_dfe()
{
  if (dfe_character != 0xff) {
    unsigned int add;
    switch (dfe_character) {
      case M0: case MT: add = 0; break;
#ifdef OPT_DFE_OBSCURE_ABBREVIATIONS
      case M1: case MA: add = 1; break;
      case M2: case MU: add = 2; break;
      case M3: case MW: add = 3; break;
      case M4: case MV: add = 4; break;
      case M5: case MS: add = 5; break;
      case M6: case MB: add = 6; break;
      case M7: case MG: add = 7; break;
      case M8: case MD: add = 8; break;
#else
      case M1: add = 1; break;
      case M2: add = 2; break;
      case M3: add = 3; break;
      case M4: add = 4; break;
      case M5: add = 5; break;
      case M6: add = 6; break;
      case M7: add = 7; break;
      case M8: add = 8; break;
#endif
      case M9: case MN: add = 9; break;
      default:
        morse(Mquestion);
        dfe_character = 0xff;
        return;
    }
    dfe_character = 0xff;

    for (byte i = 0; i < dfe_position; i++)
      add *= 10;
    dfe_freq += add;

    if (dfe_position-- == 0) {
      set_dfe();
      morse(MR);
    }

    invalidate_display();
  } else if (state.inputs.keyer) {
    set_dfe();
    invalidate_display();
    morse(MR);
    debounce_keyer();
  } else if (state.inputs.rit) {
    state.state = S_DEFAULT;
    invalidate_display();
    morse(MX);
    debounce_rit();
  } else if (key_active()) {
    iambic_key();
  }
}

void set_dfe()
{
  state.op_freq = (BAND_LIMITS_LOW[state.band] / 10000000) * 10000000;
  state.op_freq += ((unsigned long) dfe_freq) * 10000;
  fix_op_freq();
  state.state = S_DEFAULT;
}
#endif

void loop_mem_enter_wait()
{
  memory_pointer = 0;

  if (state.inputs.keyer || state.key.mode != KEY_IAMBIC) {
    state.state = S_DEFAULT;
    invalidate_display();
    morse(MX);
    debounce_keyer();
  } else if (key_active()) {
    state.state = S_MEM_ENTER;
  }
}

unsigned long quiet_since;
void loop_mem_enter()
{
  if (state.inputs.keyer) {
    state.state = S_MEM_ENTER_REVIEW;
    invalidate_display();
    playback_buffer();
  } else if (key_active()) {
    iambic_key();
    quiet_since = tcount;
  } else if (quiet_since != 0
      && tcount - quiet_since > 3 * state.key.dash_time
      && buffer[memory_pointer-1] != 0x00) {
    buffer[memory_pointer++] = 0x00;
    buffer[memory_pointer] = 0xff;
    toggle_display();
  }
}

void loop_mem_enter_review()
{
  if (digitalRead(DASHin) == LOW) {
    store_memory(0);
    morse(MM);
    morse(M1);
    state.state = S_DEFAULT;
    invalidate_display();
    while (digitalRead(DASHin) == LOW)
      delay(50);
  } else if (digitalRead(DOTin) == LOW) {
    store_memory(1);
    morse(MM);
    morse(M2);
    state.state = S_DEFAULT;
    invalidate_display();
    while (digitalRead(DOTin) == LOW)
      delay(50);
  } else if (state.inputs.keyer) {
    state.state = S_MEM_ENTER_WAIT;
    memory_pointer = 0;
    invalidate_display();
    debounce_keyer();
  }
}

void loop_mem_send_wait()
{
  // Paddle chooses a memory
  if (!digitalRead(DASHin)) {
    state.state = S_MEM_SEND_TX;
    transmit_memory(0);
    state.state = S_DEFAULT;
    invalidate_display();
  } else if (!digitalRead(DOTin)) {
    state.state = S_MEM_SEND_TX;
    transmit_memory(1);
    state.state = S_DEFAULT;
    invalidate_display();
  // Keyer exits
  } else if (state.inputs.keyer) {
    state.state = S_DEFAULT;
    invalidate_display();
    debounce_keyer();
  }
}

void loop_error()
{
  for (unsigned int f = 400; f < 1000; f += 10) {
    tone(A2, f);
    delay(10);
  }
  noTone(A2);
  delay(450);
}

void loop_calibration_correction()
{
  if (state.inputs.keyer) {
    EEPROM.write(EEPROM_CAL_VALUE, cal_value);
    EEPROM.write(EEPROM_CAL_VALUE + 1, cal_value >> 8);

    state.state = S_CALIBRATION_PEAK_IF;
    state.op_freq = IF_DEFAULT;
    invalidate_frequencies();
    invalidate_display();
    enable_rx_tx(RX_ON_TX_OFF);

    debounce_keyer();
  } else if (rotated_up()) {
    cal_value -= 100;
    calibration_set_correction();
  } else if (rotated_down()) {
    cal_value += 100;
    calibration_set_correction();
  }
}

void loop_calibration_peak_if()
{
  if (state.inputs.keyer) {
    IFfreq = state.op_freq;
    EEPROM.write(EEPROM_IF_FREQ, state.op_freq);
    EEPROM.write(EEPROM_IF_FREQ + 1, state.op_freq >> 8);
    EEPROM.write(EEPROM_IF_FREQ + 2, state.op_freq >> 16);
    EEPROM.write(EEPROM_IF_FREQ + 3, state.op_freq >> 24);

    state.state = S_CALIBRATION_CHANGE_BAND;
    invalidate_frequencies();
    invalidate_display();

    debounce_keyer();
  } else if (rotated_up()) {
    state.op_freq += 1000;
    invalidate_frequencies();
  } else if (rotated_down()) {
    state.op_freq -= 1000;
    invalidate_frequencies();
  }
}

void loop_calibration_peak_rx()
{
  if (state.inputs.keyer) {
    state.state = S_DEFAULT;
    setup_band();
    enable_rx_tx(RX_ON_TX_OFF);
    invalidate_display();
    debounce_keyer();
  }
}

void freq_adjust(long step)
{
  state.op_freq += step;
  fix_op_freq();
  invalidate_display();
  invalidate_frequencies();
}

void fix_op_freq()
{
  if (state.op_freq > BAND_LIMITS_HIGH[state.band])
    state.op_freq = BAND_LIMITS_HIGH[state.band];
  if (state.op_freq < BAND_LIMITS_LOW[state.band])
    state.op_freq = BAND_LIMITS_LOW[state.band];
}

void rotate_tuning_steps()
{
  state.tuning_step++;
  if (state.tuning_step >= sizeof(tuning_steps))
    state.tuning_step = 0;
  invalidate_display();
}

/*
 * timer outside of the normal Ardinu timers
 * does keyer timing and port D mulitplexing for display and
 * switch inputs.
 */
void TIMER1_SERVICE_ROUTINE()
{
  ++tcount;

  key_isr();

  disable_display();
  buttons_isr();
  display_isr();
}

byte morse_char;

void key_handle_start()
{
  morse_char = 0x01;
  digitalWrite(MUTE, HIGH);

  if (state.state == S_KEYING || state.state == S_MEM_SEND_TX)
    enable_rx_tx(RX_OFF_TX_ON);
}

void key_handle_end()
{
  digitalWrite(MUTE, LOW);
  enable_rx_tx(RX_ON_TX_OFF);

  if (state.state == S_KEYING) {
    state.state = S_DEFAULT;
  } else if (state.state == S_MEM_ENTER) {
    if (memory_pointer == MEMORY_LENGTH) {
      error();
      return;
    }
    buffer[memory_pointer++] = morse_char;
    buffer[memory_pointer] = 0xff;
    toggle_display();
  }
#ifdef OPT_DFE
  else if (state.state == S_DFE) {
    dfe_character = morse_char;
  }
#endif
}

void key_handle_dash()
{
  SIDETONE_ENABLE();
  if (state.state == S_KEYING || state.state == S_MEM_SEND_TX)
    digitalWrite(TXEN, HIGH);
  else
    morse_char = (morse_char << 1) | 0x01;
}

void key_handle_dot()
{
  SIDETONE_ENABLE();
  if (state.state == S_KEYING || state.state == S_MEM_SEND_TX)
    digitalWrite(TXEN, HIGH);
  else
    morse_char <<= 1;
}

void key_handle_dashdot_end()
{
  SIDETONE_DISABLE();
  digitalWrite(TXEN, LOW);
}

void straight_key_handle_enable()
{
  SIDETONE_ENABLE();
  digitalWrite(MUTE, HIGH);
  enable_rx_tx(RX_OFF_TX_ON);
  digitalWrite(TXEN, HIGH);
}

void straight_key_handle_disable()
{
  digitalWrite(TXEN, LOW);
  delay(5);
  enable_rx_tx(RX_ON_TX_OFF);
  digitalWrite(MUTE, LOW);
  SIDETONE_DISABLE();
  state.state = S_DEFAULT;
}

void calibration_set_correction()
{
  si5351.set_correction(cal_value);
  si5351.set_freq(1000000000, 0ull, SI5351_CLK1);
}

void cal_data()
{
  unsigned long temp = 0;

  temp = EEPROM.read(3);
  IFfreq = IFfreq+temp;
  IFfreq = IFfreq << 8;
  temp = EEPROM.read(2);
  IFfreq = IFfreq + temp;
  IFfreq = IFfreq << 8;
  temp = EEPROM.read(1);
  IFfreq = IFfreq + temp;
  IFfreq = IFfreq << 8;
  temp = EEPROM.read(0);
  IFfreq = IFfreq + temp;

  temp = 0;
  temp = EEPROM.read(5);
  cal_value = temp;
  cal_value = cal_value <<8;
  temp = EEPROM.read(4);
  cal_value = cal_value + temp;
}

void enable_rx_tx(byte option)
{
  Wire.beginTransmission(0x60);
  Wire.write(3);
  Wire.write(option);
  Wire.endTransmission();
}

void invalidate_frequencies()
{
  unsigned long freq;

  if (state.state == S_CALIBRATION_PEAK_IF)
    freq = state.op_freq;
  else if (state.op_freq >= IFfreq)
    freq = state.op_freq - IFfreq;
  else
    freq = state.op_freq + IFfreq;

  si5351.set_freq(freq, 0ull, SI5351_CLK_RX);
  si5351.set_freq(TX_FREQ(state), 0ull, SI5351_CLK_TX);
}

#ifdef OPT_STORE_CW_SPEED
void store_cw_speed()
{
  EEPROM.write(EEPROM_CW_SPEED, state.key.speed);
}
#endif

#ifdef OPT_ERASE_EEPROM
void ee_erase()
{
  for (byte i=0; i<=7; i++)
    EEPROM.write(i, 0xff);
  state.display.digits[3] = LED_d;
  state.display.digits[2] = LED_N_0;
  state.display.digits[1] = LED_n;
  state.display.digits[0] = LED_E;
}
#endif

void error()
{
  state.state = S_ERROR;
  invalidate_display();
}

// vim: tabstop=2 shiftwidth=2 expandtab:
