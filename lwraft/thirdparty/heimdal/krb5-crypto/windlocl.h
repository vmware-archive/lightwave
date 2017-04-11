#include <stdint.h>
#include <stdio.h>

typedef enum wind_error_number{
        WIND_ERR_NONE = -969269760,
        WIND_ERR_NO_PROFILE = -969269759,
        WIND_ERR_OVERRUN = -969269758,
        WIND_ERR_UNDERUN = -969269757,
        WIND_ERR_LENGTH_NOT_MOD2 = -969269756,
        WIND_ERR_LENGTH_NOT_MOD4 = -969269755,
        WIND_ERR_INVALID_UTF8 = -969269754,
        WIND_ERR_INVALID_UTF16 = -969269753,
        WIND_ERR_INVALID_UTF32 = -969269752,
        WIND_ERR_NO_BOM = -969269751,
        WIND_ERR_NOT_UTF16 = -969269750
} wind_error_number;


#define WIND_RW_LE      1
#define WIND_RW_BE      2
#define WIND_RW_BOM     4

