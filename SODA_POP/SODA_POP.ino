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
#define  SLED4  9
#define  SLED3  10
#define  SLED2  11
#define  SLED1  12

#define   LED_N_0  0xeb // ;0x44 - common cathode //common anode segments
#define   LED_N_1  0x09 //;0x7e
#define   LED_N_2  0xb3 //;0x25
#define   LED_N_3  0x9b //;0x26
#define   LED_N_4  0x59 //;0x1e
#define   LED_N_5  0xda //;0x86
#define   LED_N_6  0x7a //;0x94
#define   LED_N_7  0x89 //;0x6e
#define   LED_N_8  0xfb //;0x04
#define   LED_N_9  0xd9 //;0x0e
#define   LED_r    0x30 //;0xbd
#define   LED_neg   0x10 //;0xbf
#define   LED_C     0xe2 //;0xc5
#define   LED_n     0x38 //;0xbc
#define   LED_E     0xf2 //;0x85
#define   LED_t     0x72 //;0x95
#define   LED_o     0x3a //;0xb4
#define   LED_d     0x3b //;0x3b
#define   LED_A     0xf9
#define   LED_L     0x62
#define   LED_P     0xf1


// Morse coding
#define MA  0b101              // A
#define MB  0b11000            // B
#define MC  0b11010            // C
#define MD  0b1100             // D
#define ME  0b10               // E
#define MF  0b10010            // F
#define MG  0b1110             // G
#define MH  0b10000            // H
#define MI  0b100              // I
#define MJ  0b10111            // J
#define MK  0b1101             // K
#define ML  0b10100            // L
#define MM  0b111              // M
#define MN  0b110              // N
#define MO  0b1111             // O
#define MP  0b10110            // P
#define MQ  0b11101            // Q
#define MR  0b1010             // R
#define MS  0b1000             // S
#define MT  0b11               // T
#define MU  0b1001             // U
#define MV  0b10001            // V
#define MW  0b1011             // W
#define MX 0b11001            // X
#define MY 0b11011            // Y
#define MZ 0b11100             // Z

#define  M0 0b111111           // 0
#define  M1 0b101111           // 1
#define  M2 0b100111           // 2
#define  M3 0b100011           // 3
#define  M4 0b100001           // 4
#define  M5 0b100000           // 5
#define  M6 0b110000           // 6
#define  M7 0b111000           // 7
#define  M8 0b111100           // 8
#define  M9 0b111110            // 9

//flag definitions

#define     DIT_L     B00000001    //dit latch
#define     DAH_L     B00000001    //dash latch
#define     RIT_ON    B00000001    //rit flag
#define     CSflag    B00000001
#define     bit0set   0X01
#define     bit0cl    0xfe
#define     SK_EN     0x10
#define     MEM_EN    0x80
#define     MEM_EN_CL 0x7f
#define     DIT_LC    0xfe
#define     TimeOutC  0xf7
#define     TimeOut   0x08
#define     UPflag    0x01
#define     DWNflag   0x02

#define     R_sw    6
#define     K_sw    7
#define     E_sw    2
#define     SK_FG   4

// register names
int         ditTime;                    // No. milliseconds per dit
int         dashTime;
byte        keyerControl = 0;
byte        keyerControlH =0;
byte        ritflag = 0;
byte        codespeedflag = 0;
byte        sidtoneflag   = 0;
byte        memoryflag = 0;
int         ktimer = 0;
int         ktimer2 = 0;
byte        letterTime;
byte        wordTime;
byte        code=0x01;
byte        spaceTime;
byte        state;
long        myMdata[64];
unsigned long tcount;
long        duration=0;

byte        LOC = 0;
int         Eadr;

byte      BANDpointer = 1;
byte      EncoderFlag = 0;
byte      wpm =20;
byte      sw_inputs ;
byte      digit1 = 0;
byte      digit2 = 0;
byte      digit3 = 0;
byte      digit4 = 0;
byte      digitX = 0;

byte      d1temp = 0;

volatile int    digit_counter = 1;

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


unsigned long   frequency;
unsigned long   freq_result;
unsigned long   VFOfreq;
unsigned long   TXfreq;
unsigned long   OPfreq;
unsigned long   RITtemp;
unsigned long   RITresult;
unsigned long   time1;
unsigned long   time0;
unsigned long   temp ;
unsigned long   IFfreq;
int             REG = 0;
long            cal_value = 15000;
//**********************************************
unsigned long   IFfreq_default =  491480000;
//***********************************************

// registers for limit testing

unsigned long  low_band_limit;  //low limit, band tuning
unsigned long  high_band_limit; //high limit, band tuning

ISR (TIMER1_COMPA_vect) {TIMER1_SERVICE_ROUTINE();}

void setup() {
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
	loadWPM(20);                 // Fix speed at 20 WPM
	ktimer2 = ditTime;

	delay(100); //let things settle down a bit

	BANDpointer = EEPROM.read(6); // check for operating band
	if (BANDpointer == 0xff) {BANDpointer = 1; calibration();}

	cal_data();    //load calibration data
	si5351.set_correction(cal_value); //correct the clock chip error
	stepK = Fstep50 ; //default tuning rate
	stepSize = 2;
	digit4 = LED_N_6;
	digit3 = LED_n;
	get_band();
	delay(1000);
	displayfreq();
	PLLwrite();
	digitalWrite(MUTE, LOW);
	if (digitalRead(DASHin) == LOW){keyerControl |= SK_EN;} //check for straight key
}

void loop() {
	// test for switch closed
	if (bitRead(keyerControl,SK_FG) == LOW) {iambic();}
	else
		if (digitalRead(DOTin) == LOW)  {Straight_key();}

	state = bitRead(sw_inputs,E_sw);   //read change tuning step switch
	if (state == LOW) {nextFstep();} // debounce, wait for switch release
	while (state == LOW) {
		delay(10) ;
		state = bitRead(sw_inputs,E_sw);
	}

	state = bitRead(sw_inputs,R_sw);   //read RIT switch
	if (state == LOW) {timeRIT();}  //debounce, wait for switch release
	while (state == LOW) {
		delay(10) ;
		state = bitRead(sw_inputs,R_sw);
	}

	if  (bitRead(sw_inputs,K_sw) == LOW){keyer_mode();}

	if (EncoderFlag == 2) {Tune_UP();} //test tune up flag
	if (EncoderFlag == 1) {Tune_DWN();} //test tunr down flag
}
//**************************************************************
//end of switch polling loop
//**************************************************************

void keyer_mode() {
	if (codespeedflag & CSflag == 1) {adjCSoff();}
	if (bitRead(memoryflag,7) != 0) {store_mem();}
	else timebutton();
}

void timeRIT(){
	if  (bitRead(memoryflag,7) !=0) {  //this exits memory entry mode
		memoryflag &= MEM_EN_CL;
		displayfreq();
		digitalWrite(MUTE, LOW);
		do {delay(100);}
		while (bitRead(sw_inputs,R_sw) !=1);
	}

	duration = 0;
	time0 = tcount;
	do {time1 = tcount; duration = time1- time0; //calculate how long the button has been pushed
		//if (duration > 2000)   {digit4 = LED_N_6; digit3 = LED_n; digit2 = 0x00; digit1 = 0x00;}
		if (duration >5000) {digit4 = LED_C; digit3 = LED_A; digit2 = LED_L; digit1 = 0x00;}
		//if (duration >8000) {digit4 = LED_E; digit2 = LED_r; digit2 = LED_A; digit1 = LED_N_5;}
		delay(1); }
	while (bitRead(sw_inputs,R_sw) !=1);
	duration = time1 - time0;

	//  if (duration > 8000) {ee_erase(); duration =0;}
	if (duration >5000) {calibration(); duration = 0;}
	if (duration >2000) {changeBand(); duration = 0;}
	if (duration > 50) {RIT(); duration = 0;}
}

void timebutton() {

	duration = 0;  //clear button duration counter
	time0 = tcount; //set basetime to current counter value

	do {time1 = tcount; duration = time1- time0; //calculate how long the button has been pushed
		if (duration > 50)   {digit4 = LED_N_5; digit3 = LED_E; digit2 = LED_n; digit1 = LED_d;} //short push message
		if (duration > 500) {digit4 = LED_C; digit3 = LED_N_5; digit2 = 0x00; digit1 = 0x00;} //1 second push message
		if (duration > 2000) {;digit4=LED_E; digit3 =LED_n; digit2 = LED_t; digit1 = LED_r;}  //2 second push message
		delay(1); //for some reason a delay call has to be done when doing bit read flag tests or it locks up
		//this doesn't seem to be a problem when doing digital reads of a port pin instead.
	}

	while (bitRead(sw_inputs,K_sw) == 0); // wait until the bit goes high.

	duration = time1- time0; //the duration result isn't saved when exiting the do/while loop, so has to be calculated again
	if (duration >2000) {start_memory(); duration = 0;} //test duration to jump to desired function
	if (duration >500){CodeSpeed(); duration = 0;}
	if (duration >50){int_memory_send(); duration = 0;}
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

	OPfreq  = OPfreq + stepK;  //add frequenc tuning step to frequency word
	if (OPfreq > high_band_limit) {FREQ_decerment();} //band tuning limits
	if (ritflag &= RIT_ON == 1){RITdisplay();} //test for RIT mode
	else  displayfreq();
	PLLwrite();
}

void FREQ_decerment() {

	OPfreq  = OPfreq - stepK;

	if (OPfreq < low_band_limit) {FREQ_incerment();}
	if (ritflag &= RIT_ON == 1){RITdisplay();}
	else  displayfreq();
	PLLwrite();
}


//toggle tuning step rate
void  nextFstep () {

	++ stepSize ;
	if (stepSize == 3) {(stepSize = 1);}

	switch(stepSize) {
		case 1:
			stepK = Fstep200;
			d1temp = digit1;
			digit1 = 0x00;
			delay(100);
			digit1 = d1temp;
			break;
		case 2:
			stepK = Fstep50;
			d1temp = digit1;
			digit1 = 0x00;
			delay(100);
			digit1 = d1temp;
			break;
	}
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
	codespeedflag &= DIT_LC;
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
	if (wpm == 30){return;}
	else wpm = wpm + 1;
	wr_CS();
}

void  CS_dwn() {
	if (wpm == 5) {return;}
	else  wpm = wpm-1;
	wr_CS();
}

void wr_CS() {
	loadWPM(wpm);
	freq_result = wpm;
	freq_result = freq_result%10;
	hex2seg();
	digit1 = digitX;
	freq_result = wpm/10;
	hex2seg();
	digit2 = digitX;
}


/*
 *
 * RIT mode stuff here
 */

void RIT() {

	if (ritflag & RIT_ON == 1){RIText();}
	else RITenable();
}

void RITenable(){
	ritflag |= RIT_ON;
	RITtemp = OPfreq;
	RITdisplay();
}

void RIText() {

	ritflag &= DIT_LC;
	OPfreq = RITtemp;
	PLLwrite();
	displayfreq();
}

void RITdisplay() {
	if (RITtemp >= OPfreq)
	{
		RITresult = RITtemp - OPfreq;
		digit3 = LED_neg;
	}
	else
	{
		RITresult = OPfreq - RITtemp;
		digit3 = 0x00;
	}

	digit4 = LED_r;
	frequency = RITresult;
	frequency = frequency/100;
	freq_result = frequency%10000;
	freq_result = freq_result/1000;
	hex2seg();
	digit2 = digitX;
	freq_result = frequency%1000;
	freq_result = freq_result/100;
	hex2seg();
	digit1 = digitX;
}

////////////////////////////////////////////////
//This breaks up the frequency value into decades and then converts the result
//hex value into the LED 7 seg map.
///////////////////////////////////////////////

void displayfreq(){
	frequency = OPfreq/100;            //first divide by 100 to remove the fractional Hz digits
	freq_result = frequency%1000000;     //get the 100,000 kHz digit by first getting the remainder
	freq_result = freq_result / 100000;  //divide the remainder by 100,000 to get the MSD
	hex2seg();                           //convert the result to the 7 segment code
	digit4 = digitX;                     //load the digit into the display memory for MSD
	freq_result = frequency%100000;      //repeat the process for 10K, 1K and 100 Hz digits
	freq_result = freq_result/10000;
	hex2seg();
	digit3 = digitX;
	freq_result = frequency%10000;
	freq_result = freq_result/1000;
	hex2seg();
	digit2 = digitX;
	freq_result = frequency%1000;
	freq_result = freq_result/100;
	hex2seg();
	digit1 = digitX;
}

void  hex2seg()
{
	if (freq_result == 0) {digitX = LED_N_0;} //this is the conversion table
	if (freq_result == 1) {digitX = LED_N_1;}
	if (freq_result == 2) {digitX = LED_N_2;}
	if (freq_result == 3) {digitX = LED_N_3;}
	if (freq_result == 4) {digitX = LED_N_4;}
	if (freq_result == 5) {digitX = LED_N_5;}
	if (freq_result == 6) {digitX = LED_N_6;}
	if (freq_result == 7) {digitX = LED_N_7;}
	if (freq_result == 8) {digitX = LED_N_8;}
	if (freq_result == 9) {digitX = LED_N_9;}
}

/*
 *
 * timer outside of the normal Ardinu timers
 * does keyer timing and port D mulitplexing for display and
 * switch inputs.
 */

void TIMER1_SERVICE_ROUTINE()
{
	++tcount;

	if (++ktimer > ktimer2)
		keyerControl |= TimeOut;

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
			PORTD = digit1;
			digitalWrite(SLED1, LOW);
			break;

		case 1:
			PORTD = digit2;
			digitalWrite(SLED2, LOW);
			digitalWrite(2, HIGH);
			break;

		case 2:
			PORTD = digit3;
			digitalWrite(SLED3, LOW);
			break;

		case 3:
			if (digit4 == 0xeb){digit4 = 0x00;}//blank MSD if 0
			PORTD = digit4;
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
	if (OPfreq >=IFfreq) {(VFOfreq = OPfreq - IFfreq);} // test if IF is larger then operating freq
	else {(VFOfreq = IFfreq + OPfreq);}
	si5351.set_freq(VFOfreq, 0ULL, SI5351_CLK0);

	if (ritflag == 0) {TXfreq = OPfreq;}
	else {TXfreq = RITtemp;}

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
	if (bitRead(keyerControl, SK_FG) == HIGH) { memoryflag &= MEM_EN_CL; displayfreq();}
}

/*
 * store the entered message
 */
void store_mem(){
	--LOC;
	myMdata[LOC] = 0xff;

	for (int LOC = 0; LOC < 64; LOC++)
	{code = myMdata[LOC];
		if (code == 0x00) {delay(ditTime);}
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
	if (bitRead(keyerControl, SK_FG) == HIGH){sk_mem_send();}
	else

		do {
			if (digitalRead(DASHin) == LOW) { send_memory0();}
			if (digitalRead(DOTin) == LOW) {  send_memory1();}
			if (bitRead(sw_inputs,R_sw) !=1){memoryflag =0; displayfreq(); digitalWrite(MUTE, LOW);}
		}
		while (bitRead(memoryflag, 7) !=0);
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
	if (ritflag & RIT_ON == 1){RITdisplay();}
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
	if (ritflag & RIT_ON == 1){RITdisplay();}
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

	si5351.set_freq(TXfreq, 0ULL, SI5351_CLK1);

	ktimer2 = dashTime*2;
	ktimer = 0;
	keyerControl &= TimeOutC;
	do {
		if (digitalRead(DASHin) == LOW) {dash();} // dash
		if (digitalRead(DOTin) == LOW) {dot();}  //dot
		if (keyerControlH &= DAH_L == 1) {dash();}
		if (keyerControl &= DIT_L == 1) {dot();}
	}
	while (bitRead(keyerControl,3) != 1);

	++LOC ;
	if (LOC == 64) {--LOC;}
	myMdata[LOC] = code;


	ktimer2 = dashTime*2;
	ktimer = 0;
	keyerControl &= TimeOutC;

	do {
		if (digitalRead(DASHin) == LOW) {keyer();} // dash
		if (digitalRead(DOTin) == LOW) {keyer();}  //dot
	}
	while (bitRead(keyerControl,3) != 1);

	++LOC;
	if (LOC == 64) {--LOC;}
	myMdata[LOC] = 0x00;
	Wire.beginTransmission(0x60); //turn off the Tx output, which gets turned on when the chip is updated
	Wire.write(REG+3);
	Wire.write(0xfe);
	Wire.endTransmission();
	if (bitRead(memoryflag,7) != 1) {digitalWrite(MUTE, LOW);}
}

void  dash(){

	if (bitRead(memoryflag,7) != 1) { digitalWrite(TXEN, HIGH);}
	code = code << 1;
	code = code |= bit0set;
	tone(A2, 600);
	keyerControlH &= DIT_LC;
	ktimer2 = dashTime;
	ktimer =0;
	keyerControl &= TimeOutC;
	do  {update_Dot_Latch();}
	while (bitRead(keyerControl,3) != 1);

	digitalWrite(TXEN, LOW);
	noTone(A2);

	ktimer2 = ditTime;
	ktimer = 0;
	keyerControl &= TimeOutC;


	do  {update_Dot_Latch();}
	while (bitRead(keyerControl,3) != 1);
}

void  dot() {

	if (bitRead(memoryflag,7) != 1) { digitalWrite(TXEN, HIGH);}
	code  = code << 1;
	tone(A2, 600);
	ktimer2 = ditTime;
	keyerControl  &= DIT_LC;
	ktimer =0;
	keyerControl &= TimeOutC;
	do {update_Dash_Latch();}
	while (bitRead(keyerControl,3) != 1);

	digitalWrite(TXEN, LOW);
	noTone(A2);
	ktimer2 = ditTime;
	ktimer =0;
	keyerControl &= TimeOutC;

	do {update_Dash_Latch();}
	while (bitRead(keyerControl,3) != 1);

}

void update_Dot_Latch()
{
	if (digitalRead(DOTin) == LOW){keyerControl |= DIT_L;}
}

void update_Dash_Latch()
{
	if (digitalRead(DASHin) == LOW){keyerControlH |= DAH_L;}
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
	si5351.set_freq(TXfreq, 0ULL, SI5351_CLK1);

	memoryflag &= MEM_EN_CL;
	for (int LOC = 0; LOC < 64; LOC++)
	{code = myMdata[LOC];
		if (code == 0x00) {delay(ditTime);}
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
	delay(dashTime);
}

void  dah(){
	if (bitRead(memoryflag,7) != 1) { digitalWrite(TXEN, HIGH);}
	tone(A2, 600);
	delay(dashTime);
	digitalWrite(TXEN, LOW);
	noTone(A2);
	delay(ditTime);
}

void  dit() {
	if (bitRead(memoryflag,7) != 1) { digitalWrite(TXEN, HIGH);}
	tone(A2, 600);
	delay(ditTime);
	digitalWrite(TXEN, LOW);
	noTone(A2);
	delay(ditTime);
}

void loadWPM(int wpm){
	ditTime = 1200/wpm;
	dashTime = ditTime*3;
}


void Straight_key(){
	digitalWrite(MUTE, HIGH);
	Wire.beginTransmission(0x60);
	Wire.write(REG+3);
	Wire.write(0xfd);
	Wire.endTransmission();

	si5351.set_freq(TXfreq, 0ULL, SI5351_CLK1);
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
	digit4 = LED_C;
	digit3 = LED_A;
	digit2 = LED_L;
	digit1 = 0x00;
	calwrite();
	Wire.beginTransmission(0x60); //turn on Tx clock output
	Wire.write(REG+3);
	Wire.write(0xfd);
	Wire.endTransmission();
	delay(500);

	state = bitRead (sw_inputs,K_sw);
	while (state == HIGH) {if  (EncoderFlag == 1) {ADJ_UP();}
		if  (EncoderFlag == 2) {ADJ_DWN();}
		state = bitRead (sw_inputs,K_sw);}


	temp = cal_value;
	EEPROM.write(4, temp);
	temp = cal_value >>8;
	EEPROM.write(5, temp);

	Wire.beginTransmission(0x60); //turn off Tx clock output
	Wire.write(REG+3);
	Wire.write(0xfe);
	Wire.endTransmission();

	while (state == LOW) {delay(100); state = bitRead (sw_inputs,K_sw);}
	digitalWrite(MUTE, LOW);
	OPfreq = IFfreq_default;
	calwrite2();
	displayfreq();
	delay(500);
	state = bitRead (sw_inputs,K_sw);
	while (state == HIGH) {if  (EncoderFlag == 2) {ADJ_UP_f();}
		if  (EncoderFlag == 1) {ADJ_DWN_f();}
		state = bitRead (sw_inputs,K_sw);}

	IFfreq = OPfreq;
	temp = OPfreq;
	EEPROM.write(0, temp);
	temp = OPfreq >>8;
	EEPROM.write(1, temp);
	temp = OPfreq >>16;
	EEPROM.write(2, temp);
	temp = OPfreq >> 24;
	EEPROM.write(3, temp);

	changeBand();

	digit4 = LED_P;
	digit3 = LED_E;
	digit2 = LED_A;
	digit1 = 0x00;
	si5351.set_freq(OPfreq, 0ULL, SI5351_CLK1);
	Wire.beginTransmission(0x60); //turn off Tx clock output
	Wire.write(REG+3);
	Wire.write(0xfc);
	Wire.endTransmission();

	while (state == HIGH) {delay(100); state = bitRead (sw_inputs,K_sw);}
	Wire.beginTransmission(0x60); //turn off Tx clock output
	Wire.write(REG+3);
	Wire.write(0xfe);
	Wire.endTransmission();
	displayfreq();
	while (state == LOW) {delay(100); state = bitRead (sw_inputs,K_sw);}
	delay(500);
}

void ADJ_UP_f(){
	OPfreq = OPfreq + 1000;
	EncoderFlag = 0;
	calwrite2();
}

void ADJ_DWN_f() {
	OPfreq = OPfreq - 1000;
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
	si5351.set_freq(OPfreq, 0ULL, SI5351_CLK0);
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
	digit4 = LED_N_6;
	digit3 = LED_n;
	get_band();

	while (state == LOW) {delay(100); state = bitRead (sw_inputs,K_sw);}

	do {
		if (bitRead(sw_inputs,R_sw) !=1) {nextband();}
	}
	while (bitRead(sw_inputs,K_sw) !=0);
	//displayfreq();
	PLLwrite();
	EEPROM.write(6,BANDpointer);

	do {delay(50);}
	while (bitRead(sw_inputs,K_sw) !=1);

}


void nextband() {
	++BANDpointer;
	if (BANDpointer >= 11)
		BANDpointer = 1;
	get_band();
	do
		delay(50);
	while (bitRead(sw_inputs,R_sw) !=1);
}


void get_band() {
	switch(BANDpointer) {
		case  1: BAND160(); break;
		case  2:  BAND80(); break;
		case  3:  BAND60(); break;
		case  4:  BAND40(); break;
		case  5:  BAND30(); break;
		case  6:  BAND20(); break;
		case  7:  BAND17(); break;
		case  8:  BAND15(); break;
		case  9:  BAND12(); break;
		case 10:  BAND10(); break;
	}

}

void BAND160() {
	digit2 = LED_N_1;
	digit1 = LED_N_6;
	low_band_limit = 180000000;
	high_band_limit = 182000000;
	OPfreq = 180200000;
}

void BAND80() {
	digit2 = LED_N_8;
	digit1 = LED_N_0;
	low_band_limit = 350000000;
	high_band_limit = 400000000;
	OPfreq = 356000000;
}

void BAND60() {
	digit2 = LED_N_6;
	digit1 = LED_N_0;
	low_band_limit = 535150000;
	high_band_limit = 536650000;
	OPfreq = 535150000;
}


void BAND40() {
	digit2 = LED_N_4;
	digit1 = LED_N_0;
	low_band_limit = 700000000;
	high_band_limit = 730000000;
	OPfreq = 703000000;
}

void BAND30() {
	digit2 = LED_N_3;
	digit1 = LED_N_0;
	low_band_limit = 1010000000;
	high_band_limit = 1015000000;
	OPfreq = 1011800000;
}

void BAND20() {
	digit2 = LED_N_2;
	digit1 = LED_N_0;
	low_band_limit = 1400000000;
	high_band_limit = 1450000000;
	OPfreq = 1406000000;
}

void BAND17() {
	digit2 = LED_N_1;
	digit1 = LED_N_7;
	low_band_limit = 1806800000;
	high_band_limit = 1850000000;
	OPfreq = 1807000000;
}

void BAND15() {
	digit2 = LED_N_1;
	digit1 = LED_N_5;
	low_band_limit = 2100000000u;
	high_band_limit = 2150000000u;
	OPfreq = 2106000000u;
}

void BAND12() {
	digit2 = LED_N_1;
	digit1 = LED_N_2;
	low_band_limit = 2489000000u;
	high_band_limit = 2500000000u;
	OPfreq = 2490600000u;
}

void BAND10() {
	digit2 = LED_N_1;
	digit1 = LED_N_0;
	low_band_limit = 2800000000u;
	high_band_limit = 3000000000u;
	OPfreq = 2860000000u;
}

/*
void ee_erase() {
	for (int i=0; i<=7; i++){
	EEPROM.write(i, 0xff);}
	digit4 = LED_d;
	digit3 = LED_N_0;
	digit2 = LED_n;
	digit1 = LED_E;
}
*/
