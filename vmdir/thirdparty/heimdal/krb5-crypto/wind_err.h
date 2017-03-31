/* Generated from wind_err.et */
/* $Id$ */

#ifndef __wind_err_h__
#define __wind_err_h__

struct et_list;

void initialize_wind_error_table_r(struct et_list **);

void initialize_wind_error_table(void);
#define init_wind_err_tbl initialize_wind_error_table

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

#define ERROR_TABLE_BASE_wind -969269760

#define COM_ERR_BINDDOMAIN_wind "heim_com_err-969269760"

#endif /* __wind_err_h__ */
