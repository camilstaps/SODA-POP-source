/* Copyright (C) 2017 Camil Staps <pd7lol@camilstaps.nl> */

#ifndef _H_MEMORY
#define _H_MEMORY

#include "SODA_POP.h"

void store_memory(byte);
void transmit_memory(byte);
void playback_buffer();
void prepare_buffer_for_tx();
void empty_buffer(void);

#endif

// vim: tabstop=2 shiftwidth=2 expandtab:
