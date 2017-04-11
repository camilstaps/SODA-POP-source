#ifndef _H_DISPLAY
#define _H_DISPLAY

#include "SODA_POP.h"
#include "segments.h"

#define BLINK_NONE 0
#define BLINK_0 1
#define BLINK_1 2
#define BLINK_2 3
#define BLINK_3 4

struct display {
  byte digits[4];
  byte dots:4;
  byte blinking:3;
};

void display_isr(void);
void invalidate_display(void);
void toggle_digit(byte i);

#endif
