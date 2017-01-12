/*
 * Copyright © 2014 VMware, Inc.  All Rights Reserved.
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
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*
 * Module: gssapiP_unix.h
 * Abstract:
 *     VMware GSSAPI UNIX Authentication Plugin
 *     GSSAPI UNIX private types declaration header file
 *
 * Author: Adam Bernstein (abernstein@vmware.com)
 */


#ifndef	_GSSAPIP_SRP_H_
#define	_GSSAPIP_SRP_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include <gssapi/gssapi.h>
#include <gssapi/gssapi_ext.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <pthread.h>
#include "unix_mglueP.h"
#include "gssapi_unix.h"

#define xmalloc(m) calloc(1, (m))
#define	SEC_CONTEXT_TOKEN 1

#define	ACCEPT_COMPLETE 0
#define	ACCEPT_INCOMPLETE 1
#define	REJECT 2
#define REQUEST_MIC 3
#define	ACCEPT_DEFECTIVE_TOKEN 0xffffffffUL

#define UNIX_MECH_PROTOCOL_MAJ_VERSION 1
#define UNIX_MECH_PROTOCOL_MIN_VERSION 0

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
 * SRP specific error codes (minor status codes)
 */
#define	ERR_SRP_NO_MECHS_AVAILABLE	0x20000001
#define	ERR_SRP_NO_CREDS_ACQUIRED	0x20000002
#define	ERR_SRP_NO_MECH_FROM_ACCEPTOR	0x20000003
#define	ERR_SRP_NEGOTIATION_FAILED	0x20000004
#define	ERR_SRP_NO_TOKEN_FROM_ACCEPTOR	0x20000005

/*
 * send_token_flag is used to indicate in later steps what type
 * of token, if any should be sent or processed.
 * NO_TOKEN_SEND = no token should be sent
 * INIT_TOKEN_SEND = initial token will be sent
 * CONT_TOKEN_SEND = continuing tokens to be sent
 * CHECK_MIC = no token to be sent, but have a MIC to check.
 * ERROR_TOKEN_SEND = error token from peer needs to be sent.
 */

#define SRP_AUTH_STATE_VALUE(e) ((int)(e & 0x7f))
typedef	enum {NO_TOKEN_SEND, INIT_TOKEN_SEND, CONT_TOKEN_SEND,
		CHECK_MIC, ERROR_TOKEN_SEND} send_token_flag;

/* SRP message tags. This range provides 62 usable values */
typedef enum {
    SRP_AUTH_INIT = 0x61,
    SRP_UNIX_SALT_RESPONSE,
    SRP_AUTH_SALT_RESP,
    SRP_AUTH_CLIENT_VALIDATE,
    SRP_AUTH_SERVER_VALIDATE,
    SRP_AUTH_COMPLETE,
    SRP_AUTH_FAILED,
} srp_auth_state;

typedef void *srp_token_t;

/* srp name structure for internal representation. */
typedef struct {
	gss_OID type;
	gss_buffer_t buffer;
	gss_OID	mech_type;
	gss_name_t	mech_name;
} srp_name_desc, *srp_name_t;


typedef struct _srp_gss_cred_id_rec {
    /* protect against simultaneous accesses */
    pthread_mutex_t lock;

    /* OID of this mechanism: SRP */
    gss_OID srp_mech_oid;

    /*
     * This is really a UPN (name@DOMAIN.COM); Leverage k5
     * import/export name to get a UPN string. "I" value where the
     * SRP salt/validator parameters are stored in vmdir.
     */
    gss_name_t name;

    /* Set with gssspi_set_cred_option(..., gss_cred_opt_password_oid_desc, ...) */
    gss_buffer_t password;
} srp_gss_cred_id_rec, *srp_gss_cred_id_t;

/* Structure for context handle */
typedef struct {
	OM_uint32	  magic_num;
	OM_uint32	  state;         /* state of authentication */
	srp_gss_cred_id_t cred;          /* alias cred from acquire_cred */
	int               mic_reqd;
	int               mic_sent;
	int               mic_rcvd;
	int               firstpass;
	OM_uint32         ctx_flags;
	gss_name_t        internal_name; /* alias cred->name */
	gss_OID           mech;          /* SRP mech OID */
        struct SRPUser    *srp_usr;      /* Client SRP context handle */
        struct SRPVerifier *srp_ver;     /* Server SRP context handle */
        krb5_context      krb5_ctx;
        krb5_keyblock     *keyblock;
        AES_KEY           aes_encrypt_key;
        AES_KEY           aes_decrypt_key;
        unsigned char     aes_encrypt_iv[AES_BLOCK_SIZE];
        unsigned char     aes_decrypt_iv[AES_BLOCK_SIZE];
        HMAC_CTX          hmac_ctx;
        char              *unix_username; /* UNIX username */
        char              *username_hash; /* user shadow pwd file hash */
        char              *upn_name;     /* Kerberos UPN Name */
        unsigned char     *srp_session_key;
        int               srp_session_key_len;
} srp_gss_ctx_id_rec, *srp_gss_ctx_id_t;


/*
 * The magic number must be less than a standard pagesize
 * to avoid a possible collision with a real address.
 * 0xa76 = 1010 0101 0110 (binary)
 */
#define	SRP_MAGIC_ID  0x00000a76

#ifdef DEBUG
#define	dsyslog(a)
#else
#define	dsyslog(a)
#define	SRP_STATIC
#endif	/* DEBUG */

/*
 * declarations of internal name mechanism functions
 */

/*
 * Would like to use official SRP mech OID. However, this will break backward
 * compatibility with existing SRP plugin. Continue to use the "made up" MIT
 * SRP mech OID for now.
 */
OM_uint32 srp_gss_acquire_cred
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



OM_uint32 srp_gss_release_cred
(
	OM_uint32 *,		/* minor_status */
	/* CSTYLED */
	gss_cred_id_t	*	/* cred_handle */
);

OM_uint32 unix_gss_init_sec_context
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
OM_uint32 srp_gss_accept_sec_context
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

OM_uint32 srp_gss_compare_name
(
	OM_uint32 *,		/* minor_status */
	const gss_name_t,	/* name1 */
	const gss_name_t,	/* name2 */
	int *			/* name_equal */
);

OM_uint32 srp_gss_display_name
(
	OM_uint32 *,		/* minor_status */
	gss_name_t,		/*  input_name */
	gss_buffer_t,		/*  output_name_buffer */
	gss_OID *		/* output_name_type */
);

OM_uint32 srp_gss_display_status
(
	OM_uint32 *,		/* minor_status */
	OM_uint32,		/* status_value */
	int,			/* status_type */
	gss_OID,		/* mech_type */
	OM_uint32 *,		/* message_context */
	gss_buffer_t		/* status_string */
);

OM_uint32 srp_gss_import_name
(
	OM_uint32 *,		/* minor_status */
	gss_buffer_t,		/* input_name_buffer */
	gss_OID,		/* input_name_type */
	/* CSTYLED */
	gss_name_t *		/* output_name */
);

OM_uint32
srp_gss_export_name(
	OM_uint32 *minor_status,
	const gss_name_t input_name,
	gss_buffer_t exported_name
);

OM_uint32 srp_gss_release_name
(
	OM_uint32 *,		/* minor_status */
	/* CSTYLED */
	gss_name_t *		/* input_name */
);

OM_uint32 srp_gss_inquire_cred
(
	OM_uint32 *,		/* minor_status */
	gss_cred_id_t,		/* cred_handle */
	gss_name_t *,		/* name */
	OM_uint32 *,		/* lifetime */
	int *,			/* cred_usage */
	gss_OID_set *		/* mechanisms */
);

OM_uint32 srp_gss_inquire_names_for_mech
(
	OM_uint32 *,		/* minor_status */
	gss_OID,		/* mechanism */
	gss_OID_set *		/* name_types */
);

OM_uint32 srp_gss_unwrap
(
	OM_uint32 *minor_status,
	gss_ctx_id_t context_handle,
	gss_buffer_t input_message_buffer,
	gss_buffer_t output_message_buffer,
	int *conf_state,
	gss_qop_t *qop_state
);

OM_uint32 srp_gss_wrap
(
	OM_uint32 *minor_status,
	gss_ctx_id_t context_handle,
	int conf_req_flag,
	gss_qop_t qop_req,
	gss_buffer_t input_message_buffer,
	int *conf_state,
	gss_buffer_t output_message_buffer
);

OM_uint32 srp_gss_process_context_token
(
	OM_uint32	*minor_status,
	const gss_ctx_id_t context_handle,
	const gss_buffer_t token_buffer
);

OM_uint32 srp_gss_delete_sec_context
(
	OM_uint32 *minor_status,
	gss_ctx_id_t *context_handle,
	gss_buffer_t output_token
);

OM_uint32 srp_gss_context_time
(
	OM_uint32	*minor_status,
	const gss_ctx_id_t context_handle,
	OM_uint32	*time_rec
);
#ifndef LEAN_CLIENT
OM_uint32 srp_gss_export_sec_context
(
	OM_uint32	*minor_status,
	gss_ctx_id_t	*context_handle,
	gss_buffer_t	interprocess_token
);

OM_uint32 srp_gss_import_sec_context
(
	OM_uint32		*minor_status,
	const gss_buffer_t	interprocess_token,
	gss_ctx_id_t		*context_handle
);
#endif /* LEAN_CLIENT */

OM_uint32 srp_gss_inquire_context
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

OM_uint32 srp_gss_wrap_size_limit
(
	OM_uint32	*minor_status,
	const gss_ctx_id_t context_handle,
	int		conf_req_flag,
	gss_qop_t	qop_req,
	OM_uint32	req_output_size,
	OM_uint32	*max_input_size
);

OM_uint32 srp_gss_get_mic
(
	OM_uint32 *minor_status,
	const gss_ctx_id_t context_handle,
	gss_qop_t qop_req,
	const gss_buffer_t message_buffer,
	gss_buffer_t message_token
);

OM_uint32 srp_gss_verify_mic
(
	OM_uint32 *minor_status,
	const gss_ctx_id_t context_handle,
	const gss_buffer_t msg_buffer,
	const gss_buffer_t token_buffer,
	gss_qop_t *qop_state
);

OM_uint32
srp_gss_inquire_sec_context_by_oid
(
	OM_uint32 *minor_status,
	const gss_ctx_id_t context_handle,
	const gss_OID desired_object,
	gss_buffer_set_t *data_set
);

OM_uint32
srp_gss_inquire_cred_by_oid
(
	OM_uint32 *minor_status,
	const gss_cred_id_t cred_handle,
	const gss_OID desired_object,
	gss_buffer_set_t *data_set
);

OM_uint32
srp_gss_set_sec_context_option
(
	OM_uint32 *minor_status,
	gss_ctx_id_t *context_handle,
	const gss_OID desired_object,
	const gss_buffer_t value
);

OM_uint32
unix_gssspi_set_cred_option
(
	OM_uint32 *minor_status,
        gss_cred_id_t cred_handle,
        const gss_OID desired_object,
        const gss_buffer_t value
);

#ifdef _GSS_STATIC_LINK
int gss_srpint_lib_init(void);
void gss_srpint_lib_fini(void);
#else
GSS_MECH_PLUGIN_CONFIG gss_mech_initialize(void);
#endif /* _GSS_STATIC_LINK */

OM_uint32 srp_gss_wrap_aead
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

OM_uint32 srp_gss_unwrap_aead
(
	OM_uint32 *minor_status,
	gss_ctx_id_t context_handle,
	gss_buffer_t input_message_buffer,
	gss_buffer_t input_assoc_buffer,
	gss_buffer_t output_payload_buffer,
	int *conf_state,
	gss_qop_t *qop_state
);

OM_uint32 srp_gss_wrap_iov
(
	OM_uint32 *minor_status,
	gss_ctx_id_t context_handle,
	int conf_req_flag,
	gss_qop_t qop_req,
	int *conf_state,
	gss_iov_buffer_desc *iov,
	int iov_count
);

OM_uint32 srp_gss_unwrap_iov
(
	OM_uint32 *minor_status,
	gss_ctx_id_t context_handle,
	int *conf_state,
	gss_qop_t *qop_state,
	gss_iov_buffer_desc *iov,
	int iov_count
);

OM_uint32 srp_gss_wrap_iov_length
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
srp_gss_complete_auth_token
(
	OM_uint32 *minor_status,
	const gss_ctx_id_t context_handle,
	gss_buffer_t input_message_buffer
);

OM_uint32
srp_gss_acquire_cred_impersonate_name(
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
srp_gss_display_name_ext
(
	OM_uint32 *minor_status,
	gss_name_t name,
	gss_OID display_as_name_type,
	gss_buffer_t display_name
);

OM_uint32
srp_gss_inquire_name
(
	OM_uint32 *minor_status,
	gss_name_t name,
	int *name_is_MN,
	gss_OID *MN_mech,
	gss_buffer_set_t *attrs
);

OM_uint32
srp_gss_get_name_attribute
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
srp_gss_set_name_attribute
(
	OM_uint32 *minor_status,
	gss_name_t name,
	int complete,
	gss_buffer_t attr,
	gss_buffer_t value
);

OM_uint32
srp_gss_delete_name_attribute
(
	OM_uint32 *minor_status,
	gss_name_t name,
	gss_buffer_t attr
);

OM_uint32
srp_gss_export_name_composite
(
	OM_uint32 *minor_status,
	gss_name_t name,
	gss_buffer_t exp_composite_name
);

OM_uint32
srp_gss_map_name_to_any
(
	OM_uint32 *minor_status,
	gss_name_t name,
	int authenticated,
	gss_buffer_t type_id,
	gss_any_t *output
);

OM_uint32
srp_gss_release_any_name_mapping
(
	OM_uint32 *minor_status,
	gss_name_t name,
	gss_buffer_t type_id,
	gss_any_t *input
);

#ifdef	__cplusplus
}
#endif

#endif /* _GSSAPIP_SRP_H_ */
