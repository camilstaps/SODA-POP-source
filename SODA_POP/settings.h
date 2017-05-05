/* Copyright (C) 2017 Camil Staps <pd7lol@camilstaps.nl> */

/* Customisable settings. For more details, see README.md. */
#define WPM_DEFAULT          20 /* Initial paddle speed in WPM */

#define KEY_MIN_SPEED         5 /* Minimal speed in WPM */
#define KEY_MAX_SPEED        30 /* Maximal speed in WPM */

#define SIDETONE_FREQ       600 /* Frequency of the sidetone, in Hz */

#define MEMORY_LENGTH        64 /* Length of memories, including spacing */
#define MEMORY_EEPROM_START  16 /* Start of memory block in EEPROM */

#define BEACON_INTERVAL      15 /* Interval of TXs in number of dot-times */

/* The tuning steps (rotated through with the encoder button) in mHz (max 8) */
#define TUNING_STEPS       {5000,       20000,   100000,  1000000}
/* Which digit to blink for each of these tuning steps */
#define TUNING_STEP_DIGITS {BLINK_NONE, BLINK_0, BLINK_1, BLINK_2}

/* The band plan. Should be one of the following:
 * - PLAN_IARU1
 * - PLAN_IARU2
 * - PLAN_IARU3
 * - PLAN_VK
 * It is possible to override the default operating frequency using:
 * #define DEFAULT_OP_FREQ_20 1405500000
 * ... setting the default frequency to 14.055 MHz on 20m.
 */
#define PLAN_IARU2

/* Custom default operating frequencies, per band, in mHz.
 * See bands.h for defaults */
//#define DEFAULT_OP_FREQ_40  703000000

/* Custom features. For more details, see README.md. */

/* Band selection by pressing RIT for 2s */
#define OPT_BAND_SELECT

/* Erase EEPROM by pressing RIT for 8s */
#define OPT_ERASE_EEPROM

/* Store CW speed in EEPROM */
#define OPT_STORE_CW_SPEED

/* Direct frequency entry by holding encoder for 2s */
#define OPT_DFE

/* Disable display when idle */
#define OPT_DISABLE_DISPLAY
#define DISABLE_DISPLAY_AFTER 2500 /* Time to disable display after, in ms */

/* More message memories. Select using the rotary encoder. */
#define OPT_MORE_MEMORIES

/* Obscure CW number abbrevations in DFE and more memories mode */
//#define OPT_OBSCURE_MORSE_ABBREVIATIONS

// vim: tabstop=2 shiftwidth=2 expandtab:
