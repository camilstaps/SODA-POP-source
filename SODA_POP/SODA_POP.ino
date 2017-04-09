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

#include "morse.h"
#include "segments.h"
#include "settings.h"

Si5351 si5351;
#define SLED4 9
#define SLED3 10
#define SLED2 11
#define SLED1 12

const byte LED_DIGITS[] =
  { LED_N_0, LED_N_1, LED_N_2, LED_N_3, LED_N_4
  , LED_N_5, LED_N_6, LED_N_7, LED_N_8, LED_N_9};

//flag definitions

#define CSflag    B00000001
#define SK_EN     0x10
#define MEM_EN    0x80
#define MEM_EN_CL 0x7f
#define UPflag    0x01
#define DWNflag   0x02

#define R_sw  6
#define K_sw  7
#define E_sw  2
#define SK_FG 4

#define SI5351_CLK_RX SI5351_CLK0
#define SI5351_CLK_TX SI5351_CLK1

#define RX_ON_TX_ON   0xff
#define RX_ON_TX_OFF  0xfe
#define RX_OFF_TX_ON  0xfd
#define RX_OFF_TX_OFF 0xfc

#define IF_DEFAULT 491480000

enum band {
  BAND_160,
  BAND_80,
  BAND_60,
  BAND_40,
  BAND_30,
  BAND_20,
  BAND_17,
  BAND_15,
  BAND_12,
  BAND_10,
  LAST_BAND,
  BAND_UNKNOWN = 0xff
};

const byte BAND_DIGITS_2[] = {1,8,6,4,3,2,1,1,1,1};
const byte BAND_DIGITS_1[] = {6,0,0,0,0,0,7,5,2,0};
const unsigned long BAND_LIMITS_LOW[] =
  {  180000000
  ,  350000000
  ,  535150000
  ,  700000000
  , 1010000000
  , 1400000000
  , 1806800000
  , 2100000000u
  , 2489000000u
  , 2800000000u
  };
const unsigned long BAND_LIMITS_HIGH[] =
  {  182000000
  ,  400000000
  ,  536650000
  ,  730000000
  , 1015000000
  , 1450000000
  , 1850000000
  , 2150000000u
  , 2500000000u
  , 3000000000u
  };
const unsigned long BAND_OP_FREQS[] =
  {  180200000
  ,  356000000
  ,  535150000
  ,  703000000
  , 1011800000
  , 1406000000
  , 1807000000
  , 2106000000u
  , 2490600000u
  , 2860000000u
  };

struct inputs {
  union {
    struct {
      byte encoder_data:1;
      byte encoder_clock:1;
      byte encoder:1;
      byte unused:3;
      byte rit:1;
      byte keyer:1;
    } pins;
    byte port;
  } buttons;

  union {
    struct {
      byte up:1;
      byte down:1;
    };
    byte value:2;
  } encoder;
  byte encoder_last_clock:1;
};

#define KEY_IAMBIC 0
#define KEY_STRAIGHT 1

struct key_state {
  byte mode:1;
  byte timeout:1;
  byte dit:1;
  byte dash:1;
  byte speed:5;
  byte dit_time;
  unsigned int dash_time;
  unsigned int timer;
};

struct state {
  struct key_state key;

  enum band band;
  unsigned long op_freq;
  unsigned long rit_tx_freq;

  byte rit:1;
  byte code_speed:1;

  byte display[4];
  struct inputs inputs;
};

struct state state;

#define TX_FREQ(state) (state.rit ? state.rit_tx_freq : state.op_freq)

// register names
byte        memoryflag = 0;
byte        code=0x01;
long        myMdata[64];
unsigned long tcount;

byte        LOC = 0;
int         Eadr;

const int   MUTE = A3;
const int   TXEN = 13; // production A0
const int   DASHin = A0;
const int   DOTin = A1;

//frequency tuning
int         stepSize;   // tuning rate pointer
long int    stepK;      // temp storage of freq tuning step

unsigned long IFfreq;
long          cal_value = 15000;

ISR (TIMER1_COMPA_vect)
{
  TIMER1_SERVICE_ROUTINE();
}

void setup()
{
  state.key.mode = KEY_IAMBIC;
  state.key.speed = WPM_DEFAULT;
  state.key.timeout = 1;
  state.key.dash = 0;
  state.key.dit = 0;

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
  loadWPM(state.key.speed);

  delay(100); //let things settle down a bit

  state.band = (enum band) EEPROM.read(6); // check for operating band
  if (state.band == BAND_UNKNOWN) {
    state.band = (enum band) 0;
    calibration();
  }

  cal_data(); //load calibration data
  si5351.set_correction(cal_value); //correct the clock chip error
  stepK = TUNE_STEP_DEFAULT;
  stepSize = 0;
  state.display[3] = LED_N_6;
  state.display[2] = LED_n;
  setup_band();
  delay(1000);
  display_freq();
  write_pll();
  digitalWrite(MUTE, LOW);

  if (digitalRead(DASHin) == LOW)
    state.key.mode = KEY_STRAIGHT;
}

void loop()
{
  // test for switch closed
  if (state.key.mode == KEY_IAMBIC)
    iambic();
  else if (digitalRead(DOTin) == LOW)
    Straight_key();

  // Encoder switch
  if (state.inputs.buttons.pins.encoder)
    nextFstep();
  while (state.inputs.buttons.pins.encoder)
    delay(10); // debounce, wait for switch release

  // RIT switch
  if (state.inputs.buttons.pins.rit)
     timeRIT();
  while (state.inputs.buttons.pins.rit)
    delay(10); //debounce, wait for switch release

  // Keyer switch
  if (state.inputs.buttons.pins.keyer)
    keyer_mode();

  if (state.code_speed) {
    if (state.inputs.encoder.up)
      cs_adjust(1);
    if (state.inputs.encoder.down)
      cs_adjust(-1);
  } else {
    if (state.inputs.encoder.up)
      freq_adjust(stepK);
    if (state.inputs.encoder.down)
      freq_adjust(-((int) stepK));
  }
  state.inputs.encoder.value = 0;
}

void keyer_mode()
{
  if (state.code_speed)
    adjCSoff();
  if (bitRead(memoryflag,7))
    store_mem();
  else
    timebutton();
}

void timeRIT()
{
  unsigned int duration = 0;
  unsigned long start_time = tcount;

  if (bitRead(memoryflag,7) !=0) { //this exits memory entry mode
    memoryflag &= MEM_EN_CL;
    display_freq();
    digitalWrite(MUTE, LOW);
    do
      delay(100);
    while (state.inputs.buttons.pins.rit);
  }

  do {
    duration = tcount - start_time;
#ifdef OPT_ERASE_EEPROM
    if (duration > 8000) {
      state.display[3] = LED_E;
      state.display[1] = LED_r;
      state.display[1] = LED_A;
      state.display[0] = LED_N_5;
    } else
#endif
    if (duration > 5000) {
      state.display[3] = LED_C;
      state.display[2] = LED_A;
      state.display[1] = LED_L;
      state.display[0] = 0x00;
    }
#ifdef OPT_BAND_SELECT
    else if (duration > 2000) {
      state.display[3] = LED_N_6;
      state.display[2] = LED_n;
      state.display[1] = 0x00;
      state.display[0] = 0x00;
    }
#endif
    delay(1);
  } while (state.inputs.buttons.pins.rit);

#ifdef OPT_ERASE_EEPROM
  if (duration > 8000)
    ee_erase();
  else
#endif
  if (duration > 5000)
    calibration();
#ifdef OPT_BAND_SELECT
  else if (duration > 2000)
    changeBand();
#endif
  else if (duration > 50)
    RIT();
}

void timebutton()
{
  unsigned int duration = 0;
  unsigned long start_time = tcount;

  do {
    duration = tcount - start_time;
    if (duration > 2000) {
      state.display[3] = LED_E;
      state.display[2] = LED_n;
      state.display[1] = LED_t;
      state.display[0] = LED_r;
    } else if (duration > 500) {
      state.display[3] = LED_C;
      state.display[2] = LED_N_5;
      state.display[1] = 0x00;
      state.display[0] = 0x00;
    } else if (duration > 50) {
      state.display[3] = LED_N_5;
      state.display[2] = LED_E;
      state.display[1] = LED_n;
      state.display[0] = LED_d;
    }
    delay(1); //for some reason a delay call has to be done when doing bit read flag tests or it locks up
    //this doesn't seem to be a problem when doing digital reads of a port pin instead.
  } while (state.inputs.buttons.pins.keyer); // wait until the bit goes high.

  if (duration > 2000) {
    start_memory();
  } else if (duration > 500) {
    state.code_speed = 1;
    display_cs();
  } else if (duration > 50) {
    int_memory_send();
  }
}

//test for keyer mode, send message or store message
void mode_test()
{
  if (bitRead(memoryflag,7))
    store_mem();
  int_memory_send();
}


void iambic()
{
  if (digitalRead(DASHin) == LOW)
    Ptest();
  if (digitalRead(DOTin) == LOW)
    Ptest();
}

//test if paddle used for keying or code speed adj
void Ptest()
{
  if (state.code_speed)
    CS_Pinput();
  else
    keyer();
}

// adjust the operating frequency
void freq_adjust(int step)
{
  state.op_freq += step;
  if (state.op_freq > BAND_LIMITS_HIGH[state.band])
    state.op_freq = BAND_LIMITS_HIGH[state.band];
  if (state.op_freq < BAND_LIMITS_LOW[state.band])
    state.op_freq = BAND_LIMITS_LOW[state.band];
  if (state.rit)
    display_rit();
  else
    display_freq();
  write_pll();
}

//toggle tuning step rate
void nextFstep()
{
  byte d1temp = state.display[0];
  stepSize = (stepSize + 1) % 2;
  state.display[0] = 0x00;
  stepK = stepSize ? TUNE_STEP_ALT : TUNE_STEP_DEFAULT;
  delay(100);
  state.display[0] = d1temp;
}

/*
 *change keyer code speed stuff here
 */
//clear code speed adjust mode
void adjCSoff()
{
  state.code_speed = 0;
  display_freq();

  do
    delay(100);
  while (state.inputs.buttons.pins.keyer);
}

//change code speed with paddle
void CS_Pinput() {
  if (digitalRead(DASHin) == LOW)
    cs_adjust(1);
  if (digitalRead(DOTin) == LOW)
    cs_adjust(-1);
  delay(200);
}

void cs_adjust(byte adjustment)
{
  state.key.speed += adjustment;
  if (state.key.speed <= 5)
    state.key.speed = 5;
  else if (state.key.speed >= 30)
    state.key.speed = 30;
  display_cs();
}

void display_cs()
{
  loadWPM(state.key.speed);
  state.display[0] = LED_DIGITS[state.key.speed % 10];
  state.display[1] = LED_DIGITS[state.key.speed / 10];
}


/*
 *
 * RIT mode stuff here
 */
void RIT()
{
  if (state.rit) {
    state.rit = 0;
    state.op_freq = state.rit_tx_freq;
    write_pll();
    display_freq();
  } else {
    state.rit = 1;
    state.rit_tx_freq = state.op_freq;
    display_rit();
  }
}

void display_rit()
{
  unsigned long offset;

  if (state.rit_tx_freq > state.op_freq) {
    offset = state.rit_tx_freq - state.op_freq;
    state.display[2] = LED_neg;
  } else {
    offset = state.op_freq - state.rit_tx_freq;
    state.display[2] = 0x00;
  }

  state.display[3] = LED_r;
  offset /= 100;
  state.display[1] = LED_DIGITS[(offset % 10000) / 1000];
  state.display[0] = LED_DIGITS[(offset % 1000) / 100];
}

////////////////////////////////////////////////
//This breaks up the frequency value into decades and then converts the result
//hex value into the LED 7 seg map.
///////////////////////////////////////////////

void display_freq()
{
  // First divide by 100 to remove the fractional Hz digits
  unsigned long frequency = state.op_freq/100;
  // Then display the digits one by one
  state.display[3] = LED_DIGITS[(frequency % 1000000) / 100000];
  state.display[2] = LED_DIGITS[(frequency % 100000) / 10000];
  state.display[1] = LED_DIGITS[(frequency % 10000) / 1000];
  state.display[0] = LED_DIGITS[(frequency % 1000) / 100];
}

/*
 *
 * timer outside of the normal Ardinu timers
 * does keyer timing and port D mulitplexing for display and
 * switch inputs.
 */
volatile byte digit_counter = 0;
void TIMER1_SERVICE_ROUTINE()
{
  ++tcount;

  if (state.key.timer > 0)
    if (--state.key.timer == 0)
      state.key.timeout = 1;

  digitalWrite(SLED1, HIGH);
  digitalWrite(SLED2, HIGH);
  digitalWrite(SLED3, HIGH);
  digitalWrite(SLED4, HIGH);

  DDRD = 0x00;
  PORTD= 0Xff;
  digitalWrite(8, LOW);
  for (byte i = 0; i < 10; i++)
    state.inputs.buttons.port = ~PIND; // debounce
  digitalWrite(8, HIGH);
  if (state.inputs.buttons.pins.encoder_clock != state.inputs.encoder_last_clock) {
    if (!state.inputs.encoder_last_clock) {
      if (state.inputs.buttons.pins.encoder_data)
        state.inputs.encoder.down = 1;
      else
        state.inputs.encoder.up = 1;
    }
    state.inputs.encoder_last_clock = state.inputs.buttons.pins.encoder_clock;
  }
  DDRD = 0xff;

  digit_counter = (digit_counter + 1) % 4;

  switch(digit_counter) {
    case 0:
      PORTD = state.display[0];
      digitalWrite(SLED1, LOW);
      break;

    case 1:
      PORTD = state.display[1];
      digitalWrite(SLED2, LOW);
      digitalWrite(2, HIGH);
      break;

    case 2:
      PORTD = state.display[2];
      digitalWrite(SLED3, LOW);
      break;

    case 3:
      if (state.display[3] == 0xeb){state.display[3] = 0x00;}//blank MSD if 0
      PORTD = state.display[3];
      digitalWrite(SLED4, LOW);
      break;
  }
}

/*
 * output the frequency data to the clock chip.
 */
void write_pll()
{
  si5351.set_freq(
      state.op_freq >= IFfreq ? state.op_freq - IFfreq: state.op_freq + IFfreq,
      0ull, SI5351_CLK0);
}

/*
 * set up morse memory input
 */
void start_memory()
{
  memoryflag |= MEM_EN ;
  digitalWrite(MUTE, HIGH);
  morseOut(ME);
  morseOut(MM);
  clear_arry();
  if (state.key.mode == KEY_STRAIGHT) {
    memoryflag &= MEM_EN_CL;
    display_freq();
  }
}

/*
 * store the entered message
 */
void store_mem()
{
  --LOC;
  myMdata[LOC] = 0xff;

  for (int LOC = 0; LOC < 64; LOC++) {
    byte code = myMdata[LOC];
    if (code == 0x00)
      delay(state.key.dit_time);
    if (code != 0xff)
      morseOut(code);
  }

  select_loc();
}

void select_loc()
{
  do {
    if (digitalRead(DASHin) == LOW)
      store_memory0();
    if (digitalRead(DOTin) == LOW)
      store_memory1();
    if (state.inputs.buttons.pins.keyer) {
      start_memory();
      return;
    }
  } while (bitRead(memoryflag,7) !=0);
}

void store_memory0()
{
  LOC=0;
  int Eadr = 16;
  for (int LOC = 0; LOC < 64; LOC++) {
    EEPROM.write(Eadr, myMdata[LOC]);
    ++Eadr;
  }
  morseOut(MM);
  morseOut(M2);

  memoryflag &= MEM_EN_CL;
  display_freq();
  digitalWrite(MUTE, LOW);
}

void store_memory1()
{
  LOC =0;
  int Eadr = 81;
  for (int LOC = 0; LOC < 64; LOC++) {
    EEPROM.write(Eadr, myMdata[LOC]);
    ++Eadr;
  }
  morseOut(MM);
  morseOut(M1);
  memoryflag &= MEM_EN_CL;
  display_freq();
  digitalWrite(MUTE, LOW);
}

void int_memory_send()
{
  memoryflag |= MEM_EN;
  if (state.key.mode == KEY_STRAIGHT) {
    sk_mem_send();
  } else {
    do {
      if (digitalRead(DASHin) == LOW)
        send_memory0();
      if (digitalRead(DOTin) == LOW)
        send_memory1();
      if (state.inputs.buttons.pins.rit) {
        memoryflag =0;
        display_freq();
        digitalWrite(MUTE, LOW);
      }
    } while (bitRead(memoryflag, 7) !=0);
  } do delay(20);
  while (state.inputs.buttons.pins.rit);
}

void sk_mem_send()
{
  do {
    if (digitalRead(DOTin) == LOW)
      send_memory1();
    if (state.inputs.buttons.pins.rit) {
      memoryflag =0;
      display_freq();
    }
  } while (bitRead(memoryflag, 7) !=0);
}

void send_memory0()
{
  int Eadr = 16;
  LOC =0;
  for (int LOC = 0; LOC < 64; LOC++) {
    myMdata[LOC] = EEPROM.read(Eadr);
    ++Eadr;
  }
  int_morseOut();
  if (state.rit)
    display_rit();
  else
    display_freq();
}

void send_memory1()
{
  int Eadr = 81;
  LOC =0;
  for (int LOC = 0; LOC < 64; LOC++) {
    myMdata[LOC] = EEPROM.read(Eadr);
    ++Eadr;
  }
  int_morseOut();
  if (state.rit)
    display_rit();
  else
    display_freq();
}

/*
 * start of keyer routine
 */
void keyer()
{
  code = 0x01;
  inkeyer();
}

void inkeyer()
{
  digitalWrite(MUTE, HIGH);

  enable_rx_tx(RX_OFF_TX_ON);
  si5351.set_freq(TX_FREQ(state), 0ull, SI5351_CLK_TX);

  state.key.timer = state.key.dash_time * 2;
  state.key.timeout = 0;
  do {
    if (digitalRead(DASHin) == LOW)
      dash();
    if (digitalRead(DOTin) == LOW)
      dot();
    if (state.key.dash)
      dash();
    if (state.key.dit)
      dot();
  } while (!state.key.timeout);

  if (++LOC == 64)
    --LOC;
  myMdata[LOC] = code;

  state.key.timer = state.key.dash_time * 2;
  state.key.timeout = 0;

  do {
    if (digitalRead(DASHin) == LOW)
      keyer();
    if (digitalRead(DOTin) == LOW)
      keyer();
  } while (!state.key.timeout);

  if (++LOC == 64)
    --LOC;
  myMdata[LOC] = 0x00;

  enable_rx_tx(RX_ON_TX_OFF);
  if (bitRead(memoryflag,7) != 1) {digitalWrite(MUTE, LOW);}
}

void dash()
{
  if (bitRead(memoryflag,7) != 1)
    digitalWrite(TXEN, HIGH);
  code = (code << 1) | 0x01;
  tone(A2, 600);
  state.key.dash = 0;
  state.key.timer = state.key.dash_time;
  state.key.timeout = 0;
  do
    update_Dot_Latch();
  while (!state.key.timeout);

  digitalWrite(TXEN, LOW);
  noTone(A2);

  state.key.timer = state.key.dit_time;
  state.key.timeout = 0;

  do
    update_Dot_Latch();
  while (!state.key.timeout);
}

void dot()
{
  if (bitRead(memoryflag,7) != 1)
    digitalWrite(TXEN, HIGH);
  code <<= 1;
  tone(A2, 600);
  state.key.dit = 0;
  state.key.timer = state.key.dit_time;
  state.key.timeout = 0;
  do
    update_Dash_Latch();
  while (!state.key.timeout);

  digitalWrite(TXEN, LOW);
  noTone(A2);
  state.key.timer = state.key.dit_time;
  state.key.timeout = 0;

  do
    update_Dash_Latch();
  while (!state.key.timeout);
}

void update_Dot_Latch()
{
  if (digitalRead(DOTin) == LOW)
    state.key.dit = 1;
}

void update_Dash_Latch()
{
  if (digitalRead(DASHin) == LOW)
    state.key.dash = 1;
}

// clear the message memory bank by filling with 0xff
void clear_arry()
{
  for (int LOC = 0; LOC < 64; LOC++)
    myMdata[LOC] = 0xff;
}

/*
 * output a morse characer encoded 1 = dash, 0 = dot, 1 bit start bit added to start of string.
 * example, E = 0x02
 * send morse message
 *
 */
void int_morseOut()
{
  digitalWrite(MUTE, HIGH);
  enable_rx_tx(RX_OFF_TX_ON);
  si5351.set_freq(TX_FREQ(state), 0ull, SI5351_CLK1);

  memoryflag &= MEM_EN_CL;
  for (int LOC = 0; LOC < 64; LOC++) {
    byte code = myMdata[LOC];
    if (code == 0x00)
      delay(state.key.dit_time);
    if (code != 0xff)
      morseOut(code);
  }

  enable_rx_tx(RX_ON_TX_OFF);
  digitalWrite(MUTE, LOW);
}

void morseOut(byte code)
{
  char i;

  for (i = 7; i >= 0; i--)
    if (code & (1 << i))
      break;

  for (i--; i >= 0; i--) {
    if (bitRead(memoryflag,7) != 1)
      digitalWrite(TXEN, HIGH);
    tone(A2, 600);
    delay((code & (1 << i)) ? state.key.dash_time : state.key.dit_time);
    digitalWrite(TXEN, LOW);
    noTone(A2);
    delay(state.key.dit_time);
  }

  delay(state.key.dash_time);
}

void loadWPM (byte wpm)
{
  state.key.dit_time = 1200u / ((unsigned int) wpm);
  state.key.dash_time = state.key.dit_time*3;
}


void Straight_key()
{
  digitalWrite(MUTE, HIGH);
  enable_rx_tx(RX_OFF_TX_ON);

  si5351.set_freq(TX_FREQ(state), 0ull, SI5351_CLK1);
  tone(A2, 600);

  digitalWrite(TXEN, HIGH);
  do
    delay(2);
  while (digitalRead(DOTin)== LOW);

  digitalWrite(TXEN, LOW);
  noTone(A2);
  delay(5);

  enable_rx_tx(RX_ON_TX_OFF);
  digitalWrite(MUTE, LOW);
}

/*
 * Ref oscillator frequency is calibrated first
 * 10MHz signal outputted on CLOCK 0
 * tune to equal exactly 10.000,000 MHz
 * adjusted in 1 Hz steps.
 * display shows corectional factor in 0.01Hz. 1Hz is 100 on display.
 * Push keyer PB to advance to next IF offset cal
 * Push keyer PB to advance to band select
 * Push keyer PB again to store and exit
 */
void calibration()
{
  long temp = cal_value;
  state.display[3] = LED_C;
  state.display[2] = LED_A;
  state.display[1] = LED_L;
  state.display[0] = 0x00;
  calibration_set_correction();
  enable_rx_tx(RX_OFF_TX_ON);
  delay(500);

  while (!state.inputs.buttons.pins.keyer) {
    if (state.inputs.encoder.up)
      cal_value -= 100;
    else if (state.inputs.encoder.down)
      cal_value += 100;
    state.inputs.encoder.value = 0;
    calibration_set_correction();
  }

  temp = cal_value;
  EEPROM.write(4, temp);
  temp = cal_value >>8;
  EEPROM.write(5, temp);

  enable_rx_tx(RX_ON_TX_OFF);

  while (state.inputs.buttons.pins.keyer)
    delay(100);
  digitalWrite(MUTE, LOW);
  state.op_freq = IF_DEFAULT;
  calibration_set_op_freq();
  display_freq();
  delay(500);
  while (!state.inputs.buttons.pins.keyer) {
    if (state.inputs.encoder.up)
      state.op_freq += 1000;
    if (state.inputs.encoder.down)
      state.op_freq -= 1000;
    state.inputs.encoder.value = 0;
    calibration_set_op_freq();
  }

  IFfreq = state.op_freq;
  temp = state.op_freq;
  EEPROM.write(0, temp);
  temp = state.op_freq >>8;
  EEPROM.write(1, temp);
  temp = state.op_freq >>16;
  EEPROM.write(2, temp);
  temp = state.op_freq >> 24;
  EEPROM.write(3, temp);

  while (state.inputs.buttons.pins.keyer)
    delay(100);
  changeBand();

  state.display[3] = LED_P;
  state.display[2] = LED_E;
  state.display[1] = LED_A;
  state.display[0] = 0x00;
  si5351.set_freq(state.op_freq, 0ull, SI5351_CLK1);
  enable_rx_tx(RX_OFF_TX_OFF);

  while (!state.inputs.buttons.pins.keyer)
    delay(100);
  enable_rx_tx(RX_ON_TX_OFF);
  display_freq();
  while (state.inputs.buttons.pins.keyer)
    delay(100);
  delay(500);
}

void calibration_set_correction()
{
  si5351.set_correction(cal_value);
  si5351.set_freq(1000000000, 0ull, SI5351_CLK1);
}

void calibration_set_op_freq()
{
  si5351.set_freq(state.op_freq, 0ull, SI5351_CLK0);
  display_freq();
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

void changeBand()
{
  state.display[3] = LED_N_6;
  state.display[2] = LED_n;
  setup_band();

  do
    if (state.inputs.buttons.pins.rit)
      nextband();
  while (!state.inputs.buttons.pins.keyer);

  display_freq();
  write_pll();
  EEPROM.write(6, (byte) state.band);

  do
    delay(50);
  while (state.inputs.buttons.pins.keyer);
}

void nextband()
{
  state.band = (enum band) (((byte) state.band) + 1);
  if (state.band >= LAST_BAND)
    state.band = (enum band) 0;
  setup_band();
  do
    delay(50);
  while (state.inputs.buttons.pins.rit);
}

void setup_band()
{
  state.op_freq = BAND_OP_FREQS[state.band];
  state.display[0] = LED_DIGITS[BAND_DIGITS_1[state.band]];
  state.display[1] = LED_DIGITS[BAND_DIGITS_2[state.band]];
}

void enable_rx_tx(byte option)
{
  Wire.beginTransmission(0x60);
  Wire.write(3);
  Wire.write(option);
  Wire.endTransmission();
}

void ee_erase()
{
  for (byte i=0; i<=7; i++)
    EEPROM.write(i, 0xff);
  state.display[3] = LED_d;
  state.display[2] = LED_N_0;
  state.display[1] = LED_n;
  state.display[0] = LED_E;
}

// vim: tabstop=2 shiftwidth=2 expandtab:
