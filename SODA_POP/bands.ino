/* Copyright (C) 2017 Camil Staps <pd7lol@camilstaps.nl> */

#include "bands.h"

void nextband(byte up)
{
  if (up)
    state.band = (enum band) (((byte) state.band) + 1);
  else
    state.band = (enum band) (((byte) state.band) - 1);
  if (state.band == BAND_UNKNOWN)
    state.band = (enum band) ((byte) LAST_BAND - 1);
  else if (state.band >= LAST_BAND)
    state.band = (enum band) 0;

  setup_band();

  invalidate_display();
  invalidate_frequencies();
}

void setup_band()
{
  state.op_freq = BAND_OP_FREQS[state.band];
  invalidate_frequencies();
}

void store_band()
{
  EEPROM.write(EEPROM_BAND, (byte) state.band);
}

// vim: tabstop=2 shiftwidth=2 expandtab:
