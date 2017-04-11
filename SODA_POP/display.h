#ifndef _H_DISPLAY
#define _H_DISPLAY

#include "SODA_POP.h"
#include "segments.h"

struct display {
  byte digits[4];
  byte dots:4;
};

void display_isr(void);
void invalidate_display(void);
void toggle_digit(byte i);

#endif
