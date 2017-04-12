/* Copyright (C) 2017 Camil Staps <pd7lol@camilstaps.nl> */

#include "bands.h"

/**
 * Switch to another band. The display and frequencies are updated.
 *
 * @param up: 0 to go down (lower frequency), anything else to go up
 */
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
}

/**
 * Update frequencies after a band change.
 */
void setup_band()
{
  state.op_freq = BAND_OP_FREQS[state.band];
  invalidate_frequencies();
}

/**
 * Store the current band in the EEPROM.
 */
void store_band()
{
  EEPROM.write(EEPROM_BAND, (byte) state.band);
}

// vim: tabstop=2 shiftwidth=2 expandtab:
