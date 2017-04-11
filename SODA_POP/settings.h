/*
 * For more detailed descriptions of these settings, see README.md.
 */

#define WPM_DEFAULT          20 // Initial paddle speed in WPM

#define KEY_MIN_SPEED         5 // Minimal speed in WPM
#define KEY_MAX_SPEED        30 // Maximal speed in WPM

#define SIDETONE_FREQ       600 // Frequency of the sidetone, in Hz

#define MEMORY_LENGTH        64 // Length of memories in characters , including spacing
#define MEMORY_EEPROM_START  16 // Start of memory block in EEPROM

// Custom features
#define OPT_BAND_SELECT  // Band selection by pressing RIT for 2s
#define OPT_ERASE_EEPROM // Erase EEPROM by pressing RIT for 8s
#define OPT_DFE          // Direct frequency entry by holding encoder for 2s
//#define OPT_DFE_OBSCURE_ABBREVIATIONS // Obscure CW number abbrevations in DFE mode
