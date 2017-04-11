#ifndef _H_SODA_POP
#define _H_SODA_POP

#include "bands.h"
#include "buttons.h"
#include "display.h"
#include "key.h"
#include "memory.h"
#include "morse.h"
#include "settings.h"

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
  unsigned char tuning_step:2;

  struct display display;
  struct inputs inputs;
};

static struct soda_pop state;
static unsigned long tcount;

static char buffer[MEMORY_LENGTH];

const long tuning_steps[] = {5000, 20000, 100000, 1000000};
const byte tuning_blinks[] = {BLINK_NONE, BLINK_0, BLINK_1, BLINK_2};

#define TX_FREQ(state) (state.rit ? state.rit_tx_freq : state.op_freq)

#define SIDETONE_ENABLE() {tone(A2, SIDETONE_FREQ);}
#define SIDETONE_DISABLE() {noTone(A2);}

#ifdef OPT_DFE
byte dfe_character;
byte dfe_position;
unsigned int dfe_freq;
#endif

#endif
