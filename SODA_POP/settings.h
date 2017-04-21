/* Copyright (C) 2017 Camil Staps <pd7lol@camilstaps.nl> */

/* Customisable settings. For more details, see README.md. */
#define WPM_DEFAULT          20 /* Initial paddle speed in WPM */

#define KEY_MIN_SPEED         5 /* Minimal speed in WPM */
#define KEY_MAX_SPEED        30 /* Maximal speed in WPM */

#define SIDETONE_FREQ       600 /* Frequency of the sidetone, in Hz */

#define MEMORY_LENGTH        64 /* Length of memories, including spacing */
#define MEMORY_EEPROM_START  16 /* Start of memory block in EEPROM */

/* The tuning steps (rotated through with the encoder button) in mHz (max 8) */
#define TUNING_STEPS       {5000,       20000,   100000,  1000000}
/* Which digit to blink for each of these tuning steps */
#define TUNING_STEP_DIGITS {BLINK_NONE, BLINK_0, BLINK_1, BLINK_2}

/* Custom features. For more details, see README.md. */

/* Band selection by pressing RIT for 2s */
#define OPT_BAND_SELECT

/* Erase EEPROM by pressing RIT for 8s */
#define OPT_ERASE_EEPROM

/* Store CW speed in EEPROM */
#define OPT_STORE_CW_SPEED

/* Direct frequency entry by holding encoder for 2s */
#define OPT_DFE
/* Obscure CW number abbrevations in DFE mode */
//#define OPT_DFE_OBSCURE_ABBREVIATIONS

/* Disable display when idle */
#define OPT_DISABLE_DISPLAY
#define DISABLE_DISPLAY_AFTER 2500 /* Time to disable display after, in ms */

/* More message memories. Select using the rotary encoder. */
#define OPT_MORE_MEMORIES

// vim: tabstop=2 shiftwidth=2 expandtab:
