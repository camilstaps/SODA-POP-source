/* Copyright (C) 2017 Camil Staps <pd7lol@camilstaps.nl> */

#ifndef _H_SODA_POP
#define _H_SODA_POP

#include "settings.h"

#include "bands.h"
#include "buttons.h"
#include "display.h"
#include "key.h"
#include "memory.h"
#include "morse.h"

enum state : unsigned char {
  S_STARTUP,
  S_DEFAULT,
  S_KEYING,
  S_ADJUST_CS,
#ifdef OPT_BAND_SELECT
  S_CHANGE_BAND,
#endif
#ifdef OPT_DFE
  S_DFE,
#endif
  S_MEM_SEND_WAIT,
  S_MEM_SEND_TX,
  S_MEM_ENTER_WAIT,
  S_MEM_ENTER,
  S_MEM_ENTER_REVIEW,
  S_CALIBRATION_CORRECTION,
  S_CALIBRATION_PEAK_IF,
  S_CALIBRATION_CHANGE_BAND,
  S_CALIBRATION_PEAK_RX,
  S_ERROR
};

struct soda_pop {
  enum state state;

  struct key_state key;

  enum band band;
  unsigned long op_freq;
  unsigned long rit_tx_freq;

  unsigned char rit:1;
  unsigned char tuning_step:3;

  unsigned char beacon:1;
  unsigned char mem_tx_index;

  struct display display;
  volatile struct inputs inputs;

  unsigned int idle_for;
};

static struct soda_pop state;
static volatile unsigned long tcount;

static char buffer[MEMORY_LENGTH];

const long tuning_steps[] = TUNING_STEPS;
const byte tuning_blinks[] = TUNING_STEP_DIGITS;

#define TX_FREQ(state) (state.rit ? state.rit_tx_freq : state.op_freq)

#define SIDETONE_ENABLE() {tone(A2, SIDETONE_FREQ);}
#define SIDETONE_DISABLE() {noTone(A2);}

#ifdef OPT_DFE
byte dfe_character;
byte dfe_position;
unsigned int dfe_freq;
#endif

#ifdef OPT_MORE_MEMORIES
byte memory_index;
byte memory_index_character;
#endif

#define EEPROM_IF_FREQ   0 // 4 bytes
#define EEPROM_BAND      6 // 1 byte
#ifdef OPT_STORE_CW_SPEED
#define EEPROM_CW_SPEED  7 // 1 byte
#endif
#define EEPROM_CAL_VALUE 8 // 4 bytes

#ifdef OPT_AUTO_BAND
#define PCA9536_BUS_ADDR 0x41	// I2C address for PCA9536
#undef OPT_BAND_SELECT
#endif

#endif

// vim: tabstop=2 shiftwidth=2 expandtab:
