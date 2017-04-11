/* Copyright (C) 2017 Camil Staps <pd7lol@camilstaps.nl> */

#include "morse.h"

void morse(byte character)
{
  char i;

  for (i = 7; i >= 0; i--)
    if (character & (1 << i))
      break;

  for (i--; i >= 0; i--) {
    if (character & (1 << i)) {
      key_handle_dash();
      delay(state.key.dash_time);
    } else {
      key_handle_dot();
      delay(state.key.dot_time);
    }
    key_handle_dashdot_end();
    delay(state.key.dot_time);
  }

  delay(state.key.dash_time);
}

// vim: tabstop=2 shiftwidth=2 expandtab:
