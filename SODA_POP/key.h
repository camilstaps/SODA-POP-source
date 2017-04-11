#ifndef _H_KEY
#define _H_KEY

#include "SODA_POP.h"

#define KEY_IAMBIC 0
#define KEY_STRAIGHT 1

struct key_state {
  unsigned char mode:1;
  unsigned char timeout:1;
  unsigned char dot:1;
  unsigned char dash:1;
  unsigned char speed:5;
  unsigned char dot_time;
  unsigned int dash_time;
  unsigned int timer;
};

#endif
