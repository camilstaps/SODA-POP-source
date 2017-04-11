#include "memory.h"

void store_memory(byte nr)
{
  int addr = MEMORY_EEPROM_START + nr * MEMORY_LENGTH;
  for (byte i = 0; i < MEMORY_LENGTH; i++)
    EEPROM.write(addr++, buffer[i]);
}

void transmit_memory(byte nr)
{
  int addr = MEMORY_EEPROM_START + nr * MEMORY_LENGTH;
  for (byte i = 0; i < MEMORY_LENGTH; i++)
    buffer[i] = EEPROM.read(addr++);
  playback_buffer();
}

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

void empty_buffer()
{
  for (byte i = 0; i < MEMORY_LENGTH; i++)
    buffer[i] = 0xff;
}
