/**
 * Copyright (C) 2017, 2022 Camil Staps <pa5et@camilstaps.nl>
 * Copyright (C) 2017, 2019 David Giddy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _H_BANDS
#define _H_BANDS

#include "settings.h"

#ifdef PLAN_IARU1
enum band : unsigned char {
  BAND_630, BAND_160, BAND_80, BAND_60, BAND_40, BAND_30, BAND_20,
  BAND_17, BAND_15, BAND_12, BAND_10,
  LAST_BAND, BAND_UNKNOWN = 0xff
};
const unsigned char BAND_DIGITS_2[] = {6,1,8,6,4,3,2,1,1,1,1};
const unsigned char BAND_DIGITS_1[] = {3,6,0,0,0,0,0,7,5,2,0};
# ifndef DEFAULT_OP_FREQ_630
# define DEFAULT_OP_FREQ_630 47250000
# endif
# ifndef DEFAULT_OP_FREQ_160
# define DEFAULT_OP_FREQ_160 183600000
# endif
# ifndef DEFAULT_OP_FREQ_80
# define DEFAULT_OP_FREQ_80  356000000
# endif
# ifndef DEFAULT_OP_FREQ_60
# define DEFAULT_OP_FREQ_60  535150000
# endif
# ifndef DEFAULT_OP_FREQ_40
# define DEFAULT_OP_FREQ_40  703000000
# endif
# ifndef DEFAULT_OP_FREQ_30
# define DEFAULT_OP_FREQ_30 1011600000
# endif
# ifndef DEFAULT_OP_FREQ_20
# define DEFAULT_OP_FREQ_20 1406000000
# endif
# ifndef DEFAULT_OP_FREQ_17
# define DEFAULT_OP_FREQ_17 1808600000
# endif
# ifndef DEFAULT_OP_FREQ_15
# define DEFAULT_OP_FREQ_15 2106000000u
# endif
# ifndef DEFAULT_OP_FREQ_12
# define DEFAULT_OP_FREQ_12 2490600000u
# endif
# ifndef DEFAULT_OP_FREQ_10
# define DEFAULT_OP_FREQ_10 2806000000u
# endif
const unsigned long BAND_LIMITS_LOW[] =
  {  47200000
  ,  181000000
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
  {  47900000
  ,  199999999
  ,  380000000
  ,  536650000
  ,  720000000
  , 1015000000
  , 1435000000
  , 1816800000
  , 2145000000u
  , 2499000000u
  , 2970000000u
  };
const unsigned long BAND_OP_FREQS[] =
  { DEFAULT_OP_FREQ_630
  , DEFAULT_OP_FREQ_160
  , DEFAULT_OP_FREQ_80
  , DEFAULT_OP_FREQ_60
  , DEFAULT_OP_FREQ_40
  , DEFAULT_OP_FREQ_30
  , DEFAULT_OP_FREQ_20
  , DEFAULT_OP_FREQ_17
  , DEFAULT_OP_FREQ_15
  , DEFAULT_OP_FREQ_12
  , DEFAULT_OP_FREQ_10
  };
#else /* End of PLAN_IARU1 */
#ifdef PLAN_IARU2
enum band : unsigned char {
  BAND_630, BAND_160, BAND_80, BAND_60, BAND_40, BAND_30, BAND_20,
  BAND_17, BAND_15, BAND_12, BAND_10,
  LAST_BAND, BAND_UNKNOWN = 0xff
};
const unsigned char BAND_DIGITS_2[] = {6,1,8,6,4,3,2,1,1,1,1};
const unsigned char BAND_DIGITS_1[] = {3,6,0,0,0,0,0,7,5,2,0};
# ifndef DEFAULT_OP_FREQ_630
# define DEFAULT_OP_FREQ_630 47250000
# endif
# ifndef DEFAULT_OP_FREQ_160
# define DEFAULT_OP_FREQ_160 181200000
# endif
# ifndef DEFAULT_OP_FREQ_80
# define DEFAULT_OP_FREQ_80  356000000
# endif
# ifndef DEFAULT_OP_FREQ_60
# define DEFAULT_OP_FREQ_60  535150000
# endif
# ifndef DEFAULT_OP_FREQ_40
# define DEFAULT_OP_FREQ_40  703000000
# endif
# ifndef DEFAULT_OP_FREQ_30
# define DEFAULT_OP_FREQ_30 1011600000
# endif
# ifndef DEFAULT_OP_FREQ_20
# define DEFAULT_OP_FREQ_20 1406000000
# endif
# ifndef DEFAULT_OP_FREQ_17
# define DEFAULT_OP_FREQ_17 1808600000
# endif
# ifndef DEFAULT_OP_FREQ_15
# define DEFAULT_OP_FREQ_15 2106000000u
# endif
# ifndef DEFAULT_OP_FREQ_12
# define DEFAULT_OP_FREQ_12 2490600000u
# endif
# ifndef DEFAULT_OP_FREQ_10
# define DEFAULT_OP_FREQ_10 2806000000u
# endif
const unsigned long BAND_LIMITS_LOW[] =
  {  47200000
  ,  180000000
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
  {  47900000
  ,  199999999
  ,  399999999
  ,  536650000
  ,  730000000
  , 1015000000
  , 1435000000
  , 1816800000
  , 2145000000u
  , 2499000000u
  , 2970000000u
  };
const unsigned long BAND_OP_FREQS[] =
  { DEFAULT_OP_FREQ_630
  , DEFAULT_OP_FREQ_160
  , DEFAULT_OP_FREQ_80
  , DEFAULT_OP_FREQ_60
  , DEFAULT_OP_FREQ_40
  , DEFAULT_OP_FREQ_30
  , DEFAULT_OP_FREQ_20
  , DEFAULT_OP_FREQ_17
  , DEFAULT_OP_FREQ_15
  , DEFAULT_OP_FREQ_12
  , DEFAULT_OP_FREQ_10
  };
#else /* End of PLAN_IARU2 */
#ifdef PLAN_IARU3
enum band : unsigned char {
  BAND_630, BAND_160, BAND_80, BAND_40, BAND_30, BAND_20,
  BAND_17, BAND_15, BAND_12, BAND_10,
  LAST_BAND, BAND_UNKNOWN = 0xff
};
const unsigned char BAND_DIGITS_2[] = {6,1,8,4,3,2,1,1,1,1};
const unsigned char BAND_DIGITS_1[] = {3,6,0,0,0,0,7,5,2,0};
# ifndef DEFAULT_OP_FREQ_630
# define DEFAULT_OP_FREQ_630 47250000
# endif
# ifndef DEFAULT_OP_FREQ_160
# define DEFAULT_OP_FREQ_160 181200000
# endif
# ifndef DEFAULT_OP_FREQ_80
# define DEFAULT_OP_FREQ_80  356000000
# endif
# ifndef DEFAULT_OP_FREQ_40
# define DEFAULT_OP_FREQ_40  703000000
# endif
# ifndef DEFAULT_OP_FREQ_30
# define DEFAULT_OP_FREQ_30 1011600000
# endif
# ifndef DEFAULT_OP_FREQ_20
# define DEFAULT_OP_FREQ_20 1406000000
# endif
# ifndef DEFAULT_OP_FREQ_17
# define DEFAULT_OP_FREQ_17 1808600000
# endif
# ifndef DEFAULT_OP_FREQ_15
# define DEFAULT_OP_FREQ_15 2106000000u
# endif
# ifndef DEFAULT_OP_FREQ_12
# define DEFAULT_OP_FREQ_12 2490600000u
# endif
# ifndef DEFAULT_OP_FREQ_10
# define DEFAULT_OP_FREQ_10 2806000000u
# endif
const unsigned long BAND_LIMITS_LOW[] =
  {  47200000
  ,  180000000
  ,  350000000
  ,  700000000
  , 1010000000
  , 1400000000
  , 1806800000
  , 2100000000u
  , 2489000000u
  , 2800000000u
  };
const unsigned long BAND_LIMITS_HIGH[] =
  {  47900000
  ,  199999999
  ,  390000000
  ,  730000000
  , 1015000000
  , 1435000000
  , 1816800000
  , 2145000000u
  , 2499000000u
  , 2970000000u
  };
const unsigned long BAND_OP_FREQS[] =
  { DEFAULT_OP_FREQ_630
  , DEFAULT_OP_FREQ_160
  , DEFAULT_OP_FREQ_80
  , DEFAULT_OP_FREQ_40
  , DEFAULT_OP_FREQ_30
  , DEFAULT_OP_FREQ_20
  , DEFAULT_OP_FREQ_17
  , DEFAULT_OP_FREQ_15
  , DEFAULT_OP_FREQ_12
  , DEFAULT_OP_FREQ_10
  };
#else /* End of PLAN_IARU3 */
#ifdef PLAN_VK
enum band : unsigned char {
  BAND_630, BAND_160, BAND_80, BAND_40, BAND_30, BAND_20,
  BAND_17, BAND_15, BAND_12, BAND_10,
  LAST_BAND, BAND_UNKNOWN = 0xff
};
const unsigned char BAND_DIGITS_2[] = {6,1,8,4,3,2,1,1,1,1};
const unsigned char BAND_DIGITS_1[] = {3,6,0,0,0,0,7,5,2,0};
# ifndef DEFAULT_OP_FREQ_630
# define DEFAULT_OP_FREQ_630 47250000
# endif
# ifndef DEFAULT_OP_FREQ_160
# define DEFAULT_OP_FREQ_160 183200000
# endif
# ifndef DEFAULT_OP_FREQ_80
# define DEFAULT_OP_FREQ_80  353200000
# endif
# ifndef DEFAULT_OP_FREQ_40
# define DEFAULT_OP_FREQ_40  703200000
# endif
# ifndef DEFAULT_OP_FREQ_30
# define DEFAULT_OP_FREQ_30 1011600000
# endif
# ifndef DEFAULT_OP_FREQ_20
# define DEFAULT_OP_FREQ_20 1406200000
# endif
# ifndef DEFAULT_OP_FREQ_17
# define DEFAULT_OP_FREQ_17 1808600000
# endif
# ifndef DEFAULT_OP_FREQ_15
# define DEFAULT_OP_FREQ_15 2106200000u
# endif
# ifndef DEFAULT_OP_FREQ_12
# define DEFAULT_OP_FREQ_12 2490800000u
# endif
# ifndef DEFAULT_OP_FREQ_10
# define DEFAULT_OP_FREQ_10 2806200000u
# endif
const unsigned long BAND_LIMITS_LOW[] =
  {  47200000
  ,  180000000
  ,  350000000
  ,  700000000
  , 1010000000
  , 1400000000
  , 1806800000
  , 2100000000u
  , 2489000000u
  , 2800000000u
  };
const unsigned long BAND_LIMITS_HIGH[] =
  {  47900000
  ,  187500000
  ,  380000000
  ,  730000000
  , 1015000000
  , 1435000000
  , 1816800000
  , 2145000000u
  , 2499000000u
  , 2970000000u
  };
// Special definitions to support gapped 80m band in VK
#define BAND_80_LOW_TOP 370000000
#define BAND_80_HIGH_BOTTOM 377600000
#define BAND_80_GAP_MIDDLE (BAND_80_LOW_TOP+BAND_80_HIGH_BOTTOM)/2
const unsigned long BAND_OP_FREQS[] =
  { DEFAULT_OP_FREQ_630
  , DEFAULT_OP_FREQ_160
  , DEFAULT_OP_FREQ_80
  , DEFAULT_OP_FREQ_40
  , DEFAULT_OP_FREQ_30
  , DEFAULT_OP_FREQ_20
  , DEFAULT_OP_FREQ_17
  , DEFAULT_OP_FREQ_15
  , DEFAULT_OP_FREQ_12
  , DEFAULT_OP_FREQ_10
  };
#else /* End of PLAN_VK */
#error Please select a band plan using #define PLAN_...
#endif
#endif
#endif
#endif

#endif
// vim: tabstop=2 shiftwidth=2 expandtab:
