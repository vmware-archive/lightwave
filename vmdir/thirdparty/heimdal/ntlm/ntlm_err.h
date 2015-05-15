/* Generated from ntlm_err.et */

#ifndef __ntlm_err_h__
#define __ntlm_err_h__

struct et_list;

void initialize_ntlm_error_table_r(struct et_list **);

void initialize_ntlm_error_table(void);
#define init_ntlm_err_tbl initialize_ntlm_error_table

typedef enum ntlm_error_number{
	HNTLM_ERR_DECODE = -1561745664,
	HNTLM_ERR_INVALID_LENGTH = -1561745663,
	HNTLM_ERR_CRYPTO = -1561745662,
	HNTLM_ERR_RAND = -1561745661,
	HNTLM_ERR_AUTH = -1561745660,
	HNTLM_ERR_TIME_SKEW = -1561745659,
	HNTLM_ERR_OEM = -1561745658,
	HNTLM_ERR_MISSING_NAME_SEPARATOR = -1561745657,
	HNTLM_ERR_MISSING_BUFFER = -1561745656,
	HNTLM_ERR_INVALID_APOP = -1561745655,
	HNTLM_ERR_INVALID_CRAM_MD5 = -1561745654,
	HNTLM_ERR_INVALID_DIGEST_MD5 = -1561745653,
	HNTLM_ERR_INVALID_DIGEST_MD5_RSPAUTH = -1561745652
} ntlm_error_number;

#define ERROR_TABLE_BASE_ntlm -1561745664

#define COM_ERR_BINDDOMAIN_ntlm "heim_com_err-1561745664"

#endif /* __ntlm_err_h__ */
