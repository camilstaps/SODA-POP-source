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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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
 * A2   tone
 * A3   mute/qsk
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

Si5351 si5351;
#define SLED4 9
#define SLED3 10
#define SLED2 11
#define SLED1 12

#define LED_N_0 0xeb
#define LED_N_1 0x09
#define LED_N_2 0xb3
#define LED_N_3 0x9b
#define LED_N_4 0x59
#define LED_N_5 0xda
#define LED_N_6 0x7a
#define LED_N_7 0x89
#define LED_N_8 0xfb
#define LED_N_9 0xd9
#define LED_r   0x30
#define LED_neg 0x10
#define LED_C   0xe2
#define LED_n   0x38
#define LED_E   0xf2
#define LED_t   0x72
#define LED_o   0x3a
#define LED_d   0x3b
#define LED_A   0xf9
#define LED_L   0x62
#define LED_P   0xf1

const byte LED_DIGITS[] =
  { LED_N_0, LED_N_1, LED_N_2, LED_N_3, LED_N_4
  , LED_N_5, LED_N_6, LED_N_7, LED_N_8, LED_N_9};

// Morse coding
#define MA 0b101    // A
#define MB 0b11000  // B
#define MC 0b11010  // C
#define MD 0b1100   // D
#define ME 0b10     // E
#define MF 0b10010  // F
#define MG 0b1110   // G
#define MH 0b10000  // H
#define MI 0b100    // I
#define MJ 0b10111  // J
#define MK 0b1101   // K
#define ML 0b10100  // L
#define MM 0b111    // M
#define MN 0b110    // N
#define MO 0b1111   // O
#define MP 0b10110  // P
#define MQ 0b11101  // Q
#define MR 0b1010   // R
#define MS 0b1000   // S
#define MT 0b11     // T
#define MU 0b1001   // U
#define MV 0b10001  // V
#define MW 0b1011   // W
#define MX 0b11001  // X
#define MY 0b11011  // Y
#define MZ 0b11100  // Z

#define M0 0b111111 // 0
#define M1 0b101111 // 1
#define M2 0b100111 // 2
#define M3 0b100011 // 3
#define M4 0b100001 // 4
#define M5 0b100000 // 5
#define M6 0b110000 // 6
#define M7 0b111000 // 7
#define M8 0b111100 // 8
#define M9 0b111110 // 9

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

  byte display[4];
};

struct state state;

#define TX_FREQ (state.rit ? state.rit_tx_freq : state.op_freq)

// register names
byte        codespeedflag = 0;
byte        sidtoneflag   = 0;
byte        memoryflag = 0;
byte        code=0x01;
long        myMdata[64];
unsigned long tcount;

byte        LOC = 0;
int         Eadr;

byte EncoderFlag = 0;
byte sw_inputs ;

const int   MUTE =  A3;
const int   TXEN =  13; // production A0
const int   DASHin = A0;
const int   DOTin = A1;

//frequency tuning
int         stepSize;   // tuning rate pointer
long int    stepK;      // temp storage of freq tuning step

long int    Fstep50  = 5000 ;                // 10 Hz step
long int    Fstep200 = 20000;

//encoder

volatile int c = HIGH;        // init state of pin A
volatile int cLast = HIGH;    // init last val the same, low
volatile int d = HIGH;        // make data val low

unsigned long IFfreq;
int           REG = 0;
long          cal_value = 15000;

ISR (TIMER1_COMPA_vect) {TIMER1_SERVICE_ROUTINE();}

void setup() {
  state.key.mode = KEY_IAMBIC;
  state.key.speed = 20;
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
  Wire.beginTransmission(0x60); //turn off the Tx output, which gets turned on when the chip is updated
  Wire.write(REG+3);
  Wire.write(0xfe);
  Wire.endTransmission();

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  OCR1A = 238;
  TCCR1B = 0x0b;
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
  loadWPM(state.key.speed);                 // Fix speed at 20 WPM

  delay(100); //let things settle down a bit

  state.band = (enum band) EEPROM.read(6); // check for operating band
  if (state.band == BAND_UNKNOWN) {
    state.band = (enum band) 0;
    calibration();
  }

  cal_data();    //load calibration data
  si5351.set_correction(cal_value); //correct the clock chip error
  stepK = Fstep50 ; //default tuning rate
  stepSize = 0;
  state.display[3] = LED_N_6;
  state.display[2] = LED_n;
  setup_band();
  delay(1000);
  displayfreq();
  PLLwrite();
  digitalWrite(MUTE, LOW);

  if (digitalRead(DASHin) == LOW)
    state.key.mode = KEY_STRAIGHT;
}

void loop() {
  // test for switch closed
  if (state.key.mode == KEY_IAMBIC)
    iambic();
  else if (digitalRead(DOTin) == LOW)
    Straight_key();

  // Encoder switch
  if (bitRead(sw_inputs, E_sw) == LOW)
    nextFstep(); // debounce, wait for switch release
  while (bitRead(sw_inputs, E_sw) == LOW)
    delay(10);

  // RIT switch
  if (bitRead(sw_inputs, R_sw) == LOW)
     timeRIT(); //debounce, wait for switch release
  while (bitRead(sw_inputs, R_sw) == LOW)
    delay(10);

  // Keyer switch
  if (bitRead(sw_inputs,K_sw) == LOW)
    keyer_mode();

  if (EncoderFlag == 2)
    Tune_UP(); //test tune up flag
  if (EncoderFlag == 1)
    Tune_DWN(); //test tunr down flag
}
//**************************************************************
//end of switch polling loop
//**************************************************************

void keyer_mode() {
  if ((codespeedflag & CSflag) == 1) {adjCSoff();}
  if (bitRead(memoryflag,7) != 0) {store_mem();}
  else timebutton();
}

void timeRIT(){
  unsigned int duration = 0;
  unsigned long start_time = tcount;

  if (bitRead(memoryflag,7) !=0) {  //this exits memory entry mode
    memoryflag &= MEM_EN_CL;
    displayfreq();
    digitalWrite(MUTE, LOW);
    do {delay(100);}
    while (bitRead(sw_inputs,R_sw) !=1);
  }

  do {
    duration = tcount - start_time;
    /*if (duration >8000) {
      state.display[3] = LED_E;
      state.display[1] = LED_r;
      state.display[1] = LED_A;
      state.display[0] = LED_N_5;
    } else*/ if (duration > 5000) {
      state.display[3] = LED_C;
      state.display[2] = LED_A;
      state.display[1] = LED_L;
      state.display[0] = 0x00;
    } else if (duration > 2000) {
      state.display[3] = LED_N_6;
      state.display[2] = LED_n;
      state.display[1] = 0x00;
      state.display[0] = 0x00;
    }
    delay(1);
  } while (!bitRead(sw_inputs,R_sw));

  /*if (duration > 8000)
    ee_erase();
  else*/ if (duration > 5000)
    calibration();
  else if (duration > 2000)
    changeBand();
  else if (duration > 50)
    RIT();
}

void timebutton() {
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
  } while (bitRead(sw_inputs,K_sw) == 0); // wait until the bit goes high.

  if (duration > 2000)
    start_memory();
  else if (duration > 500)
    CodeSpeed();
  else if (duration > 50)
    int_memory_send();
}

//test for keyer mode, send message or store message
void mode_test(){
  if (bitRead(memoryflag,7) != 0) {store_mem();}
  int_memory_send();
}


void iambic(){
  if (digitalRead(DASHin) == LOW) {Ptest();}
  if (digitalRead(DOTin) == LOW)  {Ptest();}
}

//test if paddle used for keying or code speed adj
void Ptest() {
  if (codespeedflag &= CSflag == 1) {CS_Pinput();}
  else keyer();
}

//frequency or code speed adjust test

void  Tune_UP() {
  EncoderFlag = 0;
  if (codespeedflag &= CSflag == 1) {CS_up();}
  else FREQ_incerment();
}

void Tune_DWN() {
  EncoderFlag = 0;
  if (codespeedflag &= CSflag == 1) {CS_dwn();}
  else FREQ_decerment();
}


// adjust the operating frequency
void FREQ_incerment() {
  state.op_freq += stepK;  //add frequenc tuning step to frequency word
  if (state.op_freq > BAND_LIMITS_HIGH[state.band])
    FREQ_decerment(); //band tuning limits
  if (state.rit)
    RITdisplay(); //test for RIT mode
  else
    displayfreq();
  PLLwrite();
}

void FREQ_decerment() {
  state.op_freq -= stepK;
  if (state.op_freq < BAND_LIMITS_LOW[state.band])
    FREQ_incerment();
  if (state.rit)
    RITdisplay();
  else
    displayfreq();
  PLLwrite();
}


//toggle tuning step rate
void  nextFstep () {
  byte d1temp;

  stepSize = (stepSize + 1) % 2;

  d1temp = state.display[0];
  state.display[0] = 0x00;
  stepK = stepSize == 1 ? Fstep200 : Fstep50;
  delay(100);
  state.display[0] = d1temp;
}


/*
 *change keyer code speed stuff here
 */

void CodeSpeed(){
  codespeedflag |= CSflag;
  wr_CS();
}


//clear code speed adjust mode
void adjCSoff() {
  codespeedflag = 0x00;
  displayfreq();

  do {delay(100);}
  while (bitRead(sw_inputs,K_sw)== LOW);
}

//change code speed with paddle
void CS_Pinput() {
  if (digitalRead(DASHin) == LOW) {CS_up();}
  if (digitalRead(DOTin) == LOW)  {CS_dwn();}
  delay(200);
}


void CS_up() {
  if (state.key.speed >= 30)
    return;
  else
    state.key.speed++;
  wr_CS();
}

void  CS_dwn() {
  if (state.key.speed <= 5)
    return;
  else
    state.key.speed--;
  wr_CS();
}

void wr_CS() {
  loadWPM(state.key.speed);
  state.display[0] = LED_DIGITS[state.key.speed % 10];
  state.display[1] = LED_DIGITS[state.key.speed / 10];
}


/*
 *
 * RIT mode stuff here
 */

void RIT() {
  if (state.rit){RIText();}
  else RITenable();
}

void RITenable(){
  state.rit = 1;
  state.rit_tx_freq = state.op_freq;
  RITdisplay();
}

void RIText() {
  state.rit = 0;
  state.op_freq = state.rit_tx_freq;
  PLLwrite();
  displayfreq();
}

void RITdisplay()
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

void displayfreq() {
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

  if (state.key.timer > 0 && --state.key.timer == 0)
      state.key.timeout = 1;

  digitalWrite(SLED1, HIGH);
  digitalWrite(SLED2, HIGH);
  digitalWrite(SLED3, HIGH);
  digitalWrite(SLED4, HIGH);

  DDRD = 0x00;
  PORTD= 0Xff;
  digitalWrite(8, LOW);
  for (int i = 0; i < 10; i++) sw_inputs = PIND; // debounce
  digitalWrite(8, HIGH);
  c = bitRead(sw_inputs,1); //read encoder clock bit
  if (c != cLast)
    encoder(); //call if changed
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
 * encoder, test for direction only on 0 to 1 clock state change
 */
void encoder() {
  if (cLast == 0){
    d = bitRead(sw_inputs, 0);
    if (d == LOW)
      EncoderFlag = 2; //if low
    else
      EncoderFlag = 1; //if high
  }
  cLast = c; //store new state of clock
}


/*
 * output the frequency data to the clock chip.
 */

void PLLwrite() {
  si5351.set_freq(
      state.op_freq >= IFfreq ? state.op_freq - IFfreq: state.op_freq + IFfreq,
      0ULL, SI5351_CLK0);
}

/*
 * set up morse memory input
 */

void start_memory() {
  memoryflag |= MEM_EN ;
  digitalWrite(MUTE, HIGH);
  code  = ME;
  morseOut();
  code = MM;
  morseOut();
  clear_arry();
  if (state.key.mode == KEY_STRAIGHT) {
    memoryflag &= MEM_EN_CL;
    displayfreq();
  }
}

/*
 * store the entered message
 */
void store_mem(){
  --LOC;
  myMdata[LOC] = 0xff;

  for (int LOC = 0; LOC < 64; LOC++)
  {code = myMdata[LOC];
    if (code == 0x00) {delay(state.key.dit_time);}
    if (code != 0xff) {morseOut();}
  }
  select_loc();
}

void select_loc() {

  do {
    if (digitalRead(DASHin) == LOW) {store_memory0();}
    if (digitalRead(DOTin) == LOW) {store_memory1();}
    if  (bitRead(sw_inputs, K_sw) == LOW) {start_memory(); return;}
  }
  while (bitRead(memoryflag,7) !=0);
}

void store_memory0(){
  LOC=0;
  int Eadr = 16;
  for (int LOC = 0; LOC < 64; LOC++) {
    code = myMdata[LOC];
    EEPROM.write(Eadr, code);
    ++Eadr;
  }
  code = MM;
  morseOut();
  code = M2;
  morseOut();

  memoryflag &= MEM_EN_CL;
  displayfreq();
  digitalWrite(MUTE, LOW);
}

void store_memory1(){
  LOC =0;
  int Eadr = 81;
  for (int LOC = 0; LOC < 64; LOC++) {
    code = myMdata[LOC];
    EEPROM.write(Eadr, code);
    ++Eadr;
  }
  code  = MM;
  morseOut();
  code = M1;
  morseOut();
  memoryflag &= MEM_EN_CL;
  displayfreq();
  digitalWrite(MUTE, LOW);
}

void int_memory_send() {
  memoryflag |= MEM_EN;
  if (state.key.mode == KEY_STRAIGHT) {
    sk_mem_send();
  } else {
    do {
      if (digitalRead(DASHin) == LOW) { send_memory0();}
      if (digitalRead(DOTin) == LOW) {  send_memory1();}
      if (bitRead(sw_inputs,R_sw) !=1){memoryflag =0; displayfreq(); digitalWrite(MUTE, LOW);}
    } while (bitRead(memoryflag, 7) !=0);
  }
  do delay(20);
  while (bitRead(sw_inputs, R_sw) == LOW);
}

void sk_mem_send() {
  do {
    if (digitalRead(DOTin) == LOW) {  send_memory1();}
    if (bitRead(sw_inputs,R_sw) !=1){memoryflag =0; displayfreq();}
  }
  while (bitRead(memoryflag, 7) !=0);
}

void send_memory0() {
  int Eadr = 16;
  LOC =0;
  for (int LOC = 0; LOC < 64; LOC++) {
    code =  EEPROM.read(Eadr);
    myMdata[LOC] = code;
    ++Eadr;
  }
  int_morseOut();
  if (state.rit){RITdisplay();}
  else displayfreq();
}

void send_memory1() {
  int Eadr = 81;
  LOC =0;
  for (int LOC = 0; LOC < 64; LOC++) {
    code =  EEPROM.read(Eadr);
    myMdata[LOC] = code;
    ++Eadr;
  }
  int_morseOut();
  if (state.rit){RITdisplay();}
  else displayfreq();
}

/*
 * start of keyer routine
 */


void keyer(){
  code = 0x01;
  inkeyer();
}

void inkeyer() {
  digitalWrite(MUTE, HIGH);

  Wire.beginTransmission(0x60);
  Wire.write(REG+3);
  Wire.write(0xfd);
  Wire.endTransmission();

  si5351.set_freq(TX_FREQ, 0ULL, SI5351_CLK1);

  state.key.timer = state.key.dash_time * 2;
  state.key.timeout = 0;
  do {
    if (digitalRead(DASHin) == LOW) {dash();} // dash
    if (digitalRead(DOTin) == LOW) {dot();}  //dot
    if (state.key.dash)
      dash();
    if (state.key.dit)
      dot();
  } while (!state.key.timeout);

  ++LOC ;
  if (LOC == 64) {--LOC;}
  myMdata[LOC] = code;


  state.key.timer = state.key.dash_time * 2;
  state.key.timeout = 0;

  do {
    if (digitalRead(DASHin) == LOW) {keyer();} // dash
    if (digitalRead(DOTin) == LOW) {keyer();}  //dot
  } while (!state.key.timeout);

  ++LOC;
  if (LOC == 64) {--LOC;}
  myMdata[LOC] = 0x00;
  Wire.beginTransmission(0x60); //turn off the Tx output, which gets turned on when the chip is updated
  Wire.write(REG+3);
  Wire.write(0xfe);
  Wire.endTransmission();
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

void clear_arry() {
  for (int LOC = 0; LOC < 64; LOC++) {myMdata[LOC] = 0xff;}
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
  Wire.beginTransmission(0x60);
  Wire.write(REG+3);
  Wire.write(0xfd);
  Wire.endTransmission();
  si5351.set_freq(TX_FREQ, 0ULL, SI5351_CLK1);

  memoryflag &= MEM_EN_CL;
  for (int LOC = 0; LOC < 64; LOC++)
  {code = myMdata[LOC];
    if (code == 0x00) {delay(state.key.dit_time);}
    if (code != 0xff) {morseOut();}
  }

  Wire.beginTransmission(0x60);
  Wire.write(REG+3);
  Wire.write(0xfe);
  Wire.endTransmission();

  digitalWrite(MUTE, LOW);
}

void morseOut() {
  int i;

  for (i = 7; i >= 0; i--)
    if (code & (1 << i))
      break;

  for (i--; i >= 0; i--) {
    if (code & (1 << i))
      dah();
    else
      dit();
  }
  delay(state.key.dash_time);
}

void dah() {
  if (bitRead(memoryflag,7) != 1) { digitalWrite(TXEN, HIGH);}
  tone(A2, 600);
  delay(state.key.dash_time);
  digitalWrite(TXEN, LOW);
  noTone(A2);
  delay(state.key.dit_time);
}

void dit() {
  if (bitRead(memoryflag,7) != 1) { digitalWrite(TXEN, HIGH);}
  tone(A2, 600);
  delay(state.key.dit_time);
  digitalWrite(TXEN, LOW);
  noTone(A2);
  delay(state.key.dit_time);
}

void loadWPM (byte wpm) {
  state.key.dit_time = 1200u / ((unsigned int) wpm);
  state.key.dash_time = state.key.dit_time*3;
}


void Straight_key(){
  digitalWrite(MUTE, HIGH);
  Wire.beginTransmission(0x60);
  Wire.write(REG+3);
  Wire.write(0xfd);
  Wire.endTransmission();

  si5351.set_freq(TX_FREQ, 0ULL, SI5351_CLK1);
  tone(A2, 600);

  digitalWrite(TXEN, HIGH);
  do delay(2);
  while (digitalRead(DOTin)== LOW);

  digitalWrite(TXEN, LOW);
  noTone(A2);
  delay(5);
  Wire.beginTransmission(0x60);
  Wire.write(REG+3);
  Wire.write(0xfe);
  Wire.endTransmission();
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


void calibration(){
  long temp = cal_value;
  state.display[3] = LED_C;
  state.display[2] = LED_A;
  state.display[1] = LED_L;
  state.display[0] = 0x00;
  calwrite();
  Wire.beginTransmission(0x60); //turn on Tx clock output
  Wire.write(REG+3);
  Wire.write(0xfd);
  Wire.endTransmission();
  delay(500);

  while (bitRead(sw_inputs, K_sw) == HIGH) {
    if (EncoderFlag == 1)
      ADJ_UP();
    else if (EncoderFlag == 2)
      ADJ_DWN();
  }


  temp = cal_value;
  EEPROM.write(4, temp);
  temp = cal_value >>8;
  EEPROM.write(5, temp);

  Wire.beginTransmission(0x60); //turn off Tx clock output
  Wire.write(REG+3);
  Wire.write(0xfe);
  Wire.endTransmission();

  while (bitRead(sw_inputs, K_sw) == LOW)
    delay(100);
  digitalWrite(MUTE, LOW);
  state.op_freq = IF_DEFAULT;
  calwrite2();
  displayfreq();
  delay(500);
  while (bitRead(sw_inputs, K_sw) == HIGH) {
    if  (EncoderFlag == 2)
      ADJ_UP_f();
    if  (EncoderFlag == 1)
      ADJ_DWN_f();
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

  changeBand();

  state.display[3] = LED_P;
  state.display[2] = LED_E;
  state.display[1] = LED_A;
  state.display[0] = 0x00;
  si5351.set_freq(state.op_freq, 0ULL, SI5351_CLK1);
  Wire.beginTransmission(0x60); //turn off Tx clock output
  Wire.write(REG+3);
  Wire.write(0xfc);
  Wire.endTransmission();

  while (bitRead(sw_inputs, K_sw) == HIGH)
    delay(100);
  Wire.beginTransmission(0x60); //turn off Tx clock output
  Wire.write(REG+3);
  Wire.write(0xfe);
  Wire.endTransmission();
  displayfreq();
  while (bitRead(sw_inputs, K_sw) == LOW)
    delay(100);
  delay(500);
}

void ADJ_UP_f(){
  state.op_freq += 1000;
  EncoderFlag = 0;
  calwrite2();
}

void ADJ_DWN_f() {
  state.op_freq -= 1000;
  EncoderFlag = 0;
  calwrite2();
}

void ADJ_UP(){
  cal_value = cal_value + 100;
  EncoderFlag = 0;
  calwrite();
}

void ADJ_DWN() {
  cal_value = cal_value - 100;
  EncoderFlag = 0;
  calwrite();
}

void calwrite() {
  si5351.set_correction(cal_value);
  si5351.set_freq(1000000000, 0ULL, SI5351_CLK1);
}

void calwrite2() {
  si5351.set_freq(state.op_freq, 0ULL, SI5351_CLK0);
  displayfreq();
}

void cal_data(){
  unsigned long temp = 0;

  temp = EEPROM.read(3);
  IFfreq = IFfreq+temp;
  IFfreq = IFfreq << 8;
  temp = EEPROM.read(2);
  IFfreq = IFfreq + temp;
  IFfreq = IFfreq << 8;
  temp = EEPROM.read(1);
  IFfreq = IFfreq + temp;
  IFfreq = IFfreq <<8;
  temp =  EEPROM.read(0);
  IFfreq = IFfreq + temp;

  temp =0;
  temp = EEPROM.read(5);
  cal_value = temp;
  cal_value = cal_value <<8;
  temp = EEPROM.read(4);
  cal_value = cal_value + temp;
}

void changeBand(){
  state.display[3] = LED_N_6;
  state.display[2] = LED_n;
  setup_band();

  while (bitRead(sw_inputs, K_sw) == LOW)
    delay(100);

  do {
    if (bitRead(sw_inputs,R_sw) !=1) {nextband();}
  }
  while (bitRead(sw_inputs,K_sw) !=0);
  displayfreq();
  PLLwrite();
  EEPROM.write(6, (byte) state.band);

  do {delay(50);}
  while (bitRead(sw_inputs,K_sw) !=1);
}


void nextband() {
  state.band = (enum band) (((byte) state.band) + 1);
  if (state.band >= LAST_BAND)
    state.band = (enum band) 0;
  setup_band();
  do
    delay(50);
  while (bitRead(sw_inputs,R_sw) !=1);
}


void setup_band() {
  state.op_freq = BAND_OP_FREQS[state.band];
  state.display[0] = LED_DIGITS[BAND_DIGITS_1[state.band]];
  state.display[1] = LED_DIGITS[BAND_DIGITS_2[state.band]];
}

/*
void ee_erase() {
  for (int i=0; i<=7; i++){
  EEPROM.write(i, 0xff);}
  state.display[3] = LED_d;
  state.display[2] = LED_N_0;
  state.display[1] = LED_n;
  state.display[0] = LED_E;
}
*/

// vim: tabstop=2 shiftwidth=2 expandtab:
