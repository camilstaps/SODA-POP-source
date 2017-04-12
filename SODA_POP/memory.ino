/* Copyright (C) 2017 Camil Staps <pd7lol@camilstaps.nl> */

#include "memory.h"

/**
 * Store the buffer in EEPROM.
 *
 * @param nr the index to store the message in.
 */
void store_memory(byte nr)
{
  int addr = MEMORY_EEPROM_START + nr * MEMORY_LENGTH;
  for (byte i = 0; i < MEMORY_LENGTH; i++)
    EEPROM.write(addr++, buffer[i]);
}

/**
 * Transmit a message memory from EEPROM.
 *
 * @param nr the index of the message to send.
 */
void transmit_memory(byte nr)
{
  int addr = MEMORY_EEPROM_START + nr * MEMORY_LENGTH;
  for (byte i = 0; i < MEMORY_LENGTH; i++)
    buffer[i] = EEPROM.read(addr++);
  playback_buffer();
}

/**
 * Plays the message buffer on the sidetone. This function only takes care of
 * timing. What actually happens is defined by key_handle_* functions.
 * See also morse() for a single character.
 */
void playback_buffer()
{
  for (byte i = MEMORY_LENGTH - 1; i; i--) {
    if (buffer[i] == 0x00 || buffer[i] == 0xff)
      buffer[i] = 0xff;
    else
      break;
  }

  key_handle_start();
  for (byte i = 0; i < MEMORY_LENGTH; i++) {
    byte character = buffer[i];
    if (character == 0x00) {
      delay(state.key.dot_time);
    } else if (character == 0xff) {
      key_handle_end();
      return;
    }
    morse(character);
  }
  key_handle_end();
}

/**
 * Clear the message buffer.
 */
void empty_buffer()
{
  for (byte i = 0; i < MEMORY_LENGTH; i++)
    buffer[i] = 0xff;
}

// vim: tabstop=2 shiftwidth=2 expandtab:
