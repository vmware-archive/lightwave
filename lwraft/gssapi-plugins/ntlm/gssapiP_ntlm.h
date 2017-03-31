/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


/*
 *
 * Module: gssapiP_ntlm.h
 * Abstract:
 *     VMware GSSAPI NTLM Authentication Plugin
 *     GSSAPI NTLM private types declaration header file
 *
 * Author: Jonathan Brown (brownj@vmware.com)
 */


#ifndef	_GSSAPIP_NTLM_H_
#define	_GSSAPIP_NTLM_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include <gssapi/gssapi.h>
#include <gssapi/gssapi_ext.h>
#include <openssl/aes.h>
#include <pthread.h>
#include "ntlm_mglueP.h"
#include "gssapi_ntlm.h"

#define xmalloc(m) calloc(1, (m))
#define	SEC_CONTEXT_TOKEN 1
#define	NTLM_SIZE_OF_INT 4

#define	ACCEPT_COMPLETE 0
#define	ACCEPT_INCOMPLETE 1
#define	REJECT 2
#define REQUEST_MIC 3
#define	ACCEPT_DEFECTIVE_TOKEN 0xffffffffUL

/*
 * constants for der encoding/decoding routines.
 */

#define	MECH_OID		0x10
#define	OCTET_STRING		0x04
#define	CONTEXT			0xa0
#define	SEQUENCE		0x30
#define	SEQUENCE_OF		0x30
#define	BIT_STRING		0x03
#define	BIT_STRING_LENGTH	0x02
#define	BIT_STRING_PADDING	0x01
#define	ENUMERATED		0x0a
#define	ENUMERATION_LENGTH	1
#define	HEADER_ID		0x60
#define GENERAL_STRING		0x1b

/*
 * NTLM specific error codes (minor status codes)
 */
#define	ERR_NTLM_NO_MECHS_AVAILABLE	0x20000001
#define	ERR_NTLM_NO_CREDS_ACQUIRED	0x20000002
#define	ERR_NTLM_NO_MECH_FROM_ACCEPTOR	0x20000003
#define	ERR_NTLM_NEGOTIATION_FAILED	0x20000004
#define	ERR_NTLM_NO_TOKEN_FROM_ACCEPTOR	0x20000005

/*
 * send_token_flag is used to indicate in later steps what type
 * of token, if any should be sent or processed.
 * NO_TOKEN_SEND = no token should be sent
 * INIT_TOKEN_SEND = initial token will be sent
 * CONT_TOKEN_SEND = continuing tokens to be sent
 * CHECK_MIC = no token to be sent, but have a MIC to check.
 * ERROR_TOKEN_SEND = error token from peer needs to be sent.
 */

#define NTLM_AUTH_STATE_VALUE(e) ((int)(e & 0x7f))
typedef	enum {NO_TOKEN_SEND, INIT_TOKEN_SEND, CONT_TOKEN_SEND,
		CHECK_MIC, ERROR_TOKEN_SEND} send_token_flag;

/* NTLM message tags. This range provides 62 usable values */
typedef enum {
    NTLM_AUTH_INIT = 0x61, 
    NTLM_AUTH_SALT_RESP,
    NTLM_AUTH_CLIENT_VALIDATE,
    NTLM_AUTH_SERVER_VALIDATE,
    NTLM_AUTH_COMPLETE,
    NTLM_AUTH_FAILED,
} ntlm_auth_state;

/*
 * The Mech OID:
 * The OID of the standard NTLM mechanism is:
 *      ntlm(10) = 1.3.6.1.4.1.311.2.2.10
 */

#define	NTLM_OID_LENGTH 10
#define	NTLM_OID "\x2b\x06\x01\x04\x01\x82\x37\x02\x02\x0a"

/* 1.3.6.1.4.1.27433.3.1 */
#define GSS_CRED_OPT_PW     "\x2b\x06\x01\x04\x01\x81\xd6\x29\x03\x01"
#define GSS_CRED_OPT_PW_LEN 10

typedef void *ntlm_token_t;

/* ntlm name structure for internal representation. */
typedef struct {
	gss_OID type;
	gss_buffer_t buffer;
	gss_OID	mech_type;
	gss_name_t	mech_name;
} ntlm_name_desc, *ntlm_name_t;

typedef struct _ntlm_gss_cred_id_rec {
    /* protect against simultaneous accesses */
    pthread_mutex_t lock;

    /* OID of this mechanism: NTLM */
    gss_OID ntlm_mech_oid;

    /*
     * This is really a UPN (name@DOMAIN.COM); Leverage k5 
     * import/export name to get a UPN string. "I" value where the 
     * NTLM salt/validator parameters are stored in vmdir.
     */
    gss_name_t name; 

    /* Set with gssspi_set_cred_option(..., gss_cred_opt_password_oid_desc, ...) */
    gss_buffer_t password;

#if 1
/* More stuff as needed here */
#endif
} ntlm_gss_cred_id_rec, *ntlm_gss_cred_id_t;

/* Structure for context handle */
typedef struct {
	OM_uint32	  magic_num;
	OM_uint32	  state;         /* state of authentication */
	ntlm_gss_cred_id_t cred;          /* alias cred from acquire_cred */
	int               mic_reqd;
	int               mic_sent;
	int               mic_rcvd;
	int               firstpass;
	OM_uint32         ctx_flags;
	gss_name_t        internal_name; /* alias cred->name */
	gss_OID           mech;          /* NTLM mech OID */
        struct SRPUser    *ntlm_usr;      /* Client NTLM context handle */
        struct SRPVerifier *ntlm_ver;     /* Server NTLM context handle */
        krb5_context      krb5_ctx;
        krb5_keyblock     *keyblock;
        AES_KEY           aes_encrypt_key;
        AES_KEY           aes_decrypt_key;
        unsigned char     aes_encrypt_iv[AES_BLOCK_SIZE];
        unsigned char     aes_decrypt_iv[AES_BLOCK_SIZE];
        char              *upn_name;     /* Kerberos UPN Name */
        gss_name_t        gss_upn_name;  /* GSS UPN Name */
        unsigned char     *ntlm_session_key;
        int               ntlm_session_key_len;
} ntlm_gss_ctx_id_rec, *ntlm_gss_ctx_id_t;

/*
 * The magic number must be less than a standard pagesize
 * to avoid a possible collision with a real address.
 * 0xa76 = 1010 0101 0110 (binary)
 */
#define	NTLM_MAGIC_ID  0x00000a76

#ifdef DEBUG
#define	dsyslog(a)
#else
#define	dsyslog(a)
#define	NTLM_STATIC
#endif	/* DEBUG */

/*
 * declarations of internal name mechanism functions
 */

OM_uint32 ntlm_gss_acquire_cred
(
	OM_uint32 *,		/* minor_status */
	gss_name_t,		/* desired_name */
	OM_uint32,		/* time_req */
	gss_OID_set,		/* desired_mechs */
	gss_cred_usage_t,	/* cred_usage */
	gss_cred_id_t *,	/* output_cred_handle */
	gss_OID_set *,		/* actual_mechs */
	OM_uint32 *		/* time_rec */
);

OM_uint32 ntlm_gss_release_cred
(
	OM_uint32 *,		/* minor_status */
	/* CSTYLED */
	gss_cred_id_t	*	/* cred_handle */
);

OM_uint32 ntlm_gss_init_sec_context
(
	OM_uint32 *,		/* minor_status */
	gss_cred_id_t,		/* claimant_cred_handle */
	gss_ctx_id_t *,		/* context_handle */
	gss_name_t,		/* target_name */
	gss_OID,		/* mech_type */
	OM_uint32,		/* req_flags */
	OM_uint32,		/* time_req */
	gss_channel_bindings_t, /* input_chan_bindings */
	gss_buffer_t,		/* input_token */
	gss_OID *,		/* actual_mech_type */
	gss_buffer_t,		/* output_token */
	OM_uint32 *,		/* ret_flags */
	OM_uint32 *		/* time_rec */
);

#ifndef LEAN_CLIENT
OM_uint32 ntlm_gss_accept_sec_context
(
	OM_uint32 *,		/* minor_status */
	gss_ctx_id_t *,		/* context_handle */
	gss_cred_id_t,		/* verifier_cred_handle */
	gss_buffer_t,		/* input_token_buffer */
	gss_channel_bindings_t, /* input_chan_bindings */
	gss_name_t *,		/* src_name */
	gss_OID *,		/* mech_type */
	gss_buffer_t,		/* output_token */
	OM_uint32 *,		/* ret_flags */
	OM_uint32 *,		/* time_rec */
	/* CSTYLED */
	gss_cred_id_t *		/* delegated_cred_handle */
);
#endif /* LEAN_CLIENT */

OM_uint32 ntlm_gss_compare_name
(
	OM_uint32 *,		/* minor_status */
	const gss_name_t,	/* name1 */
	const gss_name_t,	/* name2 */
	int *			/* name_equal */
);

OM_uint32 ntlm_gss_display_name
(
	OM_uint32 *,		/* minor_status */
	gss_name_t,		/*  input_name */
	gss_buffer_t,		/*  output_name_buffer */
	gss_OID *		/* output_name_type */
);

OM_uint32 ntlm_gss_display_status
(
	OM_uint32 *,		/* minor_status */
	OM_uint32,		/* status_value */
	int,			/* status_type */
	gss_OID,		/* mech_type */
	OM_uint32 *,		/* message_context */
	gss_buffer_t		/* status_string */
);

OM_uint32 ntlm_gss_import_name
(
	OM_uint32 *,		/* minor_status */
	gss_buffer_t,		/* input_name_buffer */
	gss_OID,		/* input_name_type */
	/* CSTYLED */
	gss_name_t *		/* output_name */
);

OM_uint32 ntlm_gss_release_name
(
	OM_uint32 *,		/* minor_status */
	/* CSTYLED */
	gss_name_t *		/* input_name */
);

OM_uint32 ntlm_gss_inquire_cred
(
	OM_uint32 *,		/* minor_status */
	gss_cred_id_t,		/* cred_handle */
	gss_name_t *,		/* name */
	OM_uint32 *,		/* lifetime */
	int *,			/* cred_usage */
	gss_OID_set *		/* mechanisms */
);

OM_uint32 ntlm_gss_inquire_names_for_mech
(
	OM_uint32 *,		/* minor_status */
	gss_OID,		/* mechanism */
	gss_OID_set *		/* name_types */
);

OM_uint32 ntlm_gss_unwrap
(
	OM_uint32 *minor_status,
	gss_ctx_id_t context_handle,
	gss_buffer_t input_message_buffer,
	gss_buffer_t output_message_buffer,
	int *conf_state,
	gss_qop_t *qop_state
);

OM_uint32 ntlm_gss_wrap
(
	OM_uint32 *minor_status,
	gss_ctx_id_t context_handle,
	int conf_req_flag,
	gss_qop_t qop_req,
	gss_buffer_t input_message_buffer,
	int *conf_state,
	gss_buffer_t output_message_buffer
);

OM_uint32 ntlm_gss_process_context_token
(
	OM_uint32	*minor_status,
	const gss_ctx_id_t context_handle,
	const gss_buffer_t token_buffer
);

OM_uint32 ntlm_gss_delete_sec_context
(
	OM_uint32 *minor_status,
	gss_ctx_id_t *context_handle,
	gss_buffer_t output_token
);

OM_uint32 ntlm_gss_context_time
(
	OM_uint32	*minor_status,
	const gss_ctx_id_t context_handle,
	OM_uint32	*time_rec
);
#ifndef LEAN_CLIENT
OM_uint32 ntlm_gss_export_sec_context
(
	OM_uint32	*minor_status,
	gss_ctx_id_t	*context_handle,
	gss_buffer_t	interprocess_token
);

OM_uint32 ntlm_gss_import_sec_context
(
	OM_uint32		*minor_status,
	const gss_buffer_t	interprocess_token,
	gss_ctx_id_t		*context_handle
);
#endif /* LEAN_CLIENT */

OM_uint32 ntlm_gss_inquire_context
(
	OM_uint32	*minor_status,
	const gss_ctx_id_t context_handle,
	gss_name_t	*src_name,
	gss_name_t	*targ_name,
	OM_uint32	*lifetime_rec,
	gss_OID		*mech_type,
	OM_uint32	*ctx_flags,
	int		*locally_initiated,
	int		*opened
);

OM_uint32 ntlm_gss_wrap_size_limit
(
	OM_uint32	*minor_status,
	const gss_ctx_id_t context_handle,
	int		conf_req_flag,
	gss_qop_t	qop_req,
	OM_uint32	req_output_size,
	OM_uint32	*max_input_size
);

OM_uint32 ntlm_gss_get_mic
(
	OM_uint32 *minor_status,
	const gss_ctx_id_t context_handle,
	gss_qop_t qop_req,
	const gss_buffer_t message_buffer,
	gss_buffer_t message_token
);

OM_uint32 ntlm_gss_verify_mic
(
	OM_uint32 *minor_status,
	const gss_ctx_id_t context_handle,
	const gss_buffer_t msg_buffer,
	const gss_buffer_t token_buffer,
	gss_qop_t *qop_state
);

OM_uint32
ntlm_gss_inquire_sec_context_by_oid
(
	OM_uint32 *minor_status,
	const gss_ctx_id_t context_handle,
	const gss_OID desired_object,
	gss_buffer_set_t *data_set
);

OM_uint32
ntlm_gss_inquire_cred_by_oid
(
	OM_uint32 *minor_status,
	const gss_cred_id_t cred_handle,
	const gss_OID desired_object,
	gss_buffer_set_t *data_set
);

OM_uint32
ntlm_gss_set_sec_context_option
(
	OM_uint32 *minor_status,
	gss_ctx_id_t *context_handle,
	const gss_OID desired_object,
	const gss_buffer_t value
);

OM_uint32
ntlm_gssspi_set_cred_option
(
	OM_uint32 *minor_status,
        gss_cred_id_t cred_handle,
        const gss_OID desired_object,
        const gss_buffer_t value
);

#ifdef _GSS_STATIC_LINK
int gss_ntlmint_lib_init(void);
void gss_ntlmint_lib_fini(void);
#else
GSS_MECH_PLUGIN_CONFIG gss_mech_initialize(void);
#endif /* _GSS_STATIC_LINK */

OM_uint32 ntlm_gss_wrap_aead
(
	OM_uint32 *minor_status,
	gss_ctx_id_t context_handle,
	int conf_req_flag,
	gss_qop_t qop_req,
	gss_buffer_t input_assoc_buffer,
	gss_buffer_t input_payload_buffer,
	int *conf_state,
	gss_buffer_t output_message_buffer
);

OM_uint32 ntlm_gss_unwrap_aead
(
	OM_uint32 *minor_status,
	gss_ctx_id_t context_handle,
	gss_buffer_t input_message_buffer,
	gss_buffer_t input_assoc_buffer,
	gss_buffer_t output_payload_buffer,
	int *conf_state,
	gss_qop_t *qop_state
);

OM_uint32 ntlm_gss_wrap_iov
(
	OM_uint32 *minor_status,
	gss_ctx_id_t context_handle,
	int conf_req_flag,
	gss_qop_t qop_req,
	int *conf_state,
	gss_iov_buffer_desc *iov,
	int iov_count
);

OM_uint32 ntlm_gss_unwrap_iov
(
	OM_uint32 *minor_status,
	gss_ctx_id_t context_handle,
	int *conf_state,
	gss_qop_t *qop_state,
	gss_iov_buffer_desc *iov,
	int iov_count
);

OM_uint32 ntlm_gss_wrap_iov_length
(
	OM_uint32 *minor_status,
	gss_ctx_id_t context_handle,
	int conf_req_flag,
	gss_qop_t qop_req,
	int *conf_state,
	gss_iov_buffer_desc *iov,
	int iov_count
);

OM_uint32
ntlm_gss_complete_auth_token
(
	OM_uint32 *minor_status,
	const gss_ctx_id_t context_handle,
	gss_buffer_t input_message_buffer
);

OM_uint32
ntlm_gss_acquire_cred_impersonate_name(
    OM_uint32 *,	    /* minor_status */
    const gss_cred_id_t,    /* impersonator_cred_handle */
    const gss_name_t,	    /* desired_name */
    OM_uint32,		    /* time_req */
    const gss_OID_set,	    /* desired_mechs */
    gss_cred_usage_t,	    /* cred_usage */
    gss_cred_id_t *,	    /* output_cred_handle */
    gss_OID_set *,	    /* actual_mechs */
    OM_uint32 *);	    /* time_rec */

OM_uint32
ntlm_gss_display_name_ext
(
	OM_uint32 *minor_status,
	gss_name_t name,
	gss_OID display_as_name_type,
	gss_buffer_t display_name
);

OM_uint32
ntlm_gss_inquire_name
(
	OM_uint32 *minor_status,
	gss_name_t name,
	int *name_is_MN,
	gss_OID *MN_mech,
	gss_buffer_set_t *attrs
);

OM_uint32
ntlm_gss_get_name_attribute
(
	OM_uint32 *minor_status,
	gss_name_t name,
	gss_buffer_t attr,
	int *authenticated,
	int *complete,
	gss_buffer_t value,
	gss_buffer_t display_value,
	int *more
);

OM_uint32
ntlm_gss_set_name_attribute
(
	OM_uint32 *minor_status,
	gss_name_t name,
	int complete,
	gss_buffer_t attr,
	gss_buffer_t value
);

OM_uint32
ntlm_gss_delete_name_attribute
(
	OM_uint32 *minor_status,
	gss_name_t name,
	gss_buffer_t attr
);

OM_uint32
ntlm_gss_export_name_composite
(
	OM_uint32 *minor_status,
	gss_name_t name,
	gss_buffer_t exp_composite_name
);

OM_uint32
ntlm_gss_map_name_to_any
(
	OM_uint32 *minor_status,
	gss_name_t name,
	int authenticated,
	gss_buffer_t type_id,
	gss_any_t *output
);

OM_uint32
ntlm_gss_release_any_name_mapping
(
	OM_uint32 *minor_status,
	gss_name_t name,
	gss_buffer_t type_id,
	gss_any_t *input
);

#ifdef	__cplusplus
}
#endif

#endif /* _GSSAPIP_NTLM_H_ */
