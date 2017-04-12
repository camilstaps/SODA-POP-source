/* Copyright (C) 2017 Camil Staps <pd7lol@camilstaps.nl> */

#include "key.h"

/**
 * Change the paddle speed according to the parameter, withing the
 * KEY_MIN_SPEED .. KEY_MAX_SPEED range (inclusive).
 *
 * @param adjustment the adjustment to the speed in WPM.
 */
void adjust_cs(byte adjustment)
{
  state.key.speed += adjustment;
  if (state.key.speed <= KEY_MIN_SPEED)
    state.key.speed = KEY_MIN_SPEED;
  else if (state.key.speed >= KEY_MAX_SPEED)
    state.key.speed = KEY_MAX_SPEED;
  invalidate_display();
}

/**
 * Set up the CW speed as in state.key.speed. This modifies the dot_time and
 * dash_time accordingly.
 */
void load_cw_speed()
{
  state.key.dot_time = 1200u / ((unsigned int) state.key.speed);
  state.key.dash_time = state.key.dot_time * 3;
}

/**
 * Checks whether the key (either straight or paddle) is active.
 *
 * @return 0 if inactive, something else if active.
 */
byte key_active()
{
  return (state.key.mode == KEY_STRAIGHT && digitalRead(DOTin) == LOW)
      || (state.key.mode == KEY_IAMBIC &&
        (digitalRead(DOTin) == LOW || digitalRead(DASHin) == LOW));
}

/**
 * Handles a straight key. Simply calls straight_key_handle_enable() when it is
 * pressed, and straight_key_handle_disable() when it is released.
 */
void straight_key()
{
  if (digitalRead(DOTin) == HIGH)
    return;

  straight_key_handle_enable();

  while (digitalRead(DOTin) == LOW)
    delay(2);

  straight_key_handle_disable();
}

/**
 * The ISR for the keyer. Should be called every 1ms, to ensure proper timing.
 */
void key_isr()
{
  if (state.key.timer > 0)
    if (--state.key.timer == 0)
      state.key.timeout = 1;
}

/**
 * Keys one character using the iambic keying method. This function only does
 * timing. What actually happens is defined by key_handle_dash(),
 * key_handle_dot() and key_handle_dashdot_end(), depending on the state.
 */
void iambic_key()
{
  byte repeat = 1;

  if (digitalRead(DASHin) == HIGH && digitalRead(DOTin) == HIGH)
    return;

  key_handle_start();

  while (repeat) {
    state.key.timer = state.key.dash_time * 2;
    state.key.timeout = 0;
    do {
      if (digitalRead(DASHin) == LOW)
        dash();
      if (digitalRead(DOTin) == LOW)
        dot();
      if (state.key.dash)
        dash();
      if (state.key.dot)
        dot();
    } while (!state.key.timeout);

    state.key.timer = state.key.dash_time * 2;
    state.key.timeout = 0;

    repeat = 0;
    do {
      if (digitalRead(DASHin) == LOW || digitalRead(DOTin) == LOW) {
        repeat = 1;
        break;
      }
    } while (!state.key.timeout);
  }

  key_handle_end();
}

/**
 * Handle one dash. This function only does timing and iambic keying. What
 * actually happens is defined by key_handle_dash() and
 * key_handle_dashdot_end(), depending on the current state.
 */
void dash()
{
  key_handle_dash();
  state.key.dash = 0;

  state.key.timer = state.key.dash_time;
  state.key.timeout = 0;
  wait_check_dot();

  key_handle_dashdot_end();

  state.key.timer = state.key.dot_time;
  state.key.timeout = 0;
  wait_check_dot();
}

/**
 * Wait for a key timeout and update the dot status.
 */
void wait_check_dot()
{
  do {
    if (digitalRead(DOTin) == LOW)
      state.key.dot = 1;
    delay(1);
  } while (!state.key.timeout);
}

/**
 * Handle one dot. This function only does timing and iambic keying. What
 * actually happens is defined by key_handle_dot() and
 * key_handle_dashdot_end(), depending on the current state.
 */
void dot()
{
  key_handle_dot();
  state.key.dot = 0;

  state.key.timer = state.key.dot_time;
  state.key.timeout = 0;
  wait_check_dash();

  key_handle_dashdot_end();

  state.key.timer = state.key.dot_time;
  state.key.timeout = 0;
  wait_check_dash();
}

/**
 * Wait for a key timeout and update the dash status.
 */
void wait_check_dash()
{
  do {
    if (digitalRead(DASHin) == LOW)
      state.key.dash = 1;
    delay(1);
  } while (!state.key.timeout);
}

// vim: tabstop=2 shiftwidth=2 expandtab:
