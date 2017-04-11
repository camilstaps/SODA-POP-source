#include "key.h"

void adjust_cs(byte adjustment)
{
  state.key.speed += adjustment;
  if (state.key.speed <= KEY_MIN_SPEED)
    state.key.speed = KEY_MIN_SPEED;
  else if (state.key.speed >= KEY_MAX_SPEED)
    state.key.speed = KEY_MAX_SPEED;
  invalidate_display();
}

void load_wpm(byte wpm)
{
  state.key.dot_time = 1200u / ((unsigned int) wpm);
  state.key.dash_time = state.key.dot_time * 3;
}

byte key_active()
{
  return (state.key.mode == KEY_STRAIGHT && digitalRead(DOTin) == LOW)
      || (state.key.mode == KEY_IAMBIC &&
        (digitalRead(DOTin) == LOW || digitalRead(DASHin) == LOW));
}

void straight_key()
{
  if (digitalRead(DOTin) == HIGH)
    return;

  straight_key_handle_enable();

  while (digitalRead(DOTin) == LOW)
    delay(2);

  straight_key_handle_disable();
}

void key_isr()
{
  if (state.key.timer > 0)
    if (--state.key.timer == 0)
      state.key.timeout = 1;
}

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

void wait_check_dot()
{
  do {
    if (digitalRead(DOTin) == LOW)
      state.key.dot = 1;
    delay(1);
  } while (!state.key.timeout);
}

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

void wait_check_dash()
{
  do {
    if (digitalRead(DASHin) == LOW)
      state.key.dash = 1;
    delay(1);
  } while (!state.key.timeout);
}
