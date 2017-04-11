/* Copyright (C) 2017 Camil Staps <pd7lol@camilstaps.nl> */

#ifndef _H_BANDS
#define _H_BANDS

enum band : unsigned char {
  BAND_160,
  BAND_80,
  BAND_60,
  BAND_40,
  BAND_30,
  BAND_20,
  BAND_17,
  BAND_15,
  BAND_12,
  BAND_10,
  LAST_BAND,
  BAND_UNKNOWN = 0xff
};

const unsigned char BAND_DIGITS_2[] = {1,8,6,4,3,2,1,1,1,1};
const unsigned char BAND_DIGITS_1[] = {6,0,0,0,0,0,7,5,2,0};
const unsigned long BAND_LIMITS_LOW[] =
  {  180000000
  ,  350000000
  ,  535150000
  ,  700000000
  , 1010000000
  , 1400000000
  , 1806800000
  , 2100000000u
  , 2489000000u
  , 2800000000u
  };
const unsigned long BAND_LIMITS_HIGH[] =
  {  182000000
  ,  400000000
  ,  536650000
  ,  730000000
  , 1015000000
  , 1450000000
  , 1850000000
  , 2150000000u
  , 2500000000u
  , 3000000000u
  };
const unsigned long BAND_OP_FREQS[] =
  {  180200000
  ,  356000000
  ,  535150000
  ,  703000000
  , 1011800000
  , 1406000000
  , 1807000000
  , 2106000000u
  , 2490600000u
  , 2860000000u
  };

#endif

// vim: tabstop=2 shiftwidth=2 expandtab:
