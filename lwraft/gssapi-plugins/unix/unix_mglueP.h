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
 * This header contains the private mechglue definitions.
 *
 * Copyright (c) 1995, by Sun Microsystems, Inc.
 * All rights reserved.
 */

/*
 * Module: srp_mglueP.h
 * Abstract:
 *     VMware GSSAPI SRP Authentication Plugin
 *     GSSAPI SRP function table private type definitions
 *
 * Author: Adam Bernstein (abernstein@vmware.com)
 */


#ifndef _SRP_MGLUEP_H_
#define _SRP_MGLUEP_H_

#include <gssapi/gssapi.h>
#include <gssapi/gssapi_ext.h>
/*
 * Exact copy of the mglueP.h "struct gss_config". This is contained
 * in a private header file, so this internal plugin structure cannot
 * be consumed publically.
 */

/*
 * This is the definition of the mechs_array struct, which is used to
 * define the mechs array table. This table is used to indirectly
 * access mechanism specific versions of the gssapi routines through
 * the routines in the glue module (gssd_mech_glue.c)
 *
 * This contants all of the functions defined in gssapi.h except for
 * gss_release_buffer() and gss_release_oid_set(), which I am
 * assuming, for now, to be equal across mechanisms.  
 */
 
typedef struct _GSS_MECH_PLUGIN_CONFIG {
    gss_OID_desc    mech_type;
    void *	    context;
    OM_uint32       (*gss_acquire_cred)
	(
		    OM_uint32*,		/* minor_status */
		    gss_name_t,		/* desired_name */
		    OM_uint32,		/* time_req */
		    gss_OID_set,	/* desired_mechs */
		    int,		/* cred_usage */
		    gss_cred_id_t*,	/* output_cred_handle */
		    gss_OID_set*,	/* actual_mechs */
		    OM_uint32*		/* time_rec */
		    );
    OM_uint32       (*gss_release_cred)
	(
		    OM_uint32*,		/* minor_status */
		    gss_cred_id_t*	/* cred_handle */
		    );
    OM_uint32       (*gss_init_sec_context)
	(
		    OM_uint32*,			/* minor_status */
		    gss_cred_id_t,		/* claimant_cred_handle */
		    gss_ctx_id_t*,		/* context_handle */
		    gss_name_t,			/* target_name */
		    gss_OID,			/* mech_type */
		    OM_uint32,			/* req_flags */
		    OM_uint32,			/* time_req */
		    gss_channel_bindings_t,	/* input_chan_bindings */
		    gss_buffer_t,		/* input_token */
		    gss_OID*,			/* actual_mech_type */
		    gss_buffer_t,		/* output_token */
		    OM_uint32*,			/* ret_flags */
		    OM_uint32*			/* time_rec */
		    );
    OM_uint32       (*gss_accept_sec_context)
	(
		    OM_uint32*,			/* minor_status */
		    gss_ctx_id_t*,		/* context_handle */
		    gss_cred_id_t,		/* verifier_cred_handle */
		    gss_buffer_t,		/* input_token_buffer */
		    gss_channel_bindings_t,	/* input_chan_bindings */
		    gss_name_t*,		/* src_name */
		    gss_OID*,			/* mech_type */
		    gss_buffer_t,		/* output_token */
		    OM_uint32*,			/* ret_flags */
		    OM_uint32*,			/* time_rec */
		    gss_cred_id_t*		/* delegated_cred_handle */
		    );
    OM_uint32       (*gss_process_context_token)
	(
		    OM_uint32*,		/* minor_status */
		    gss_ctx_id_t,	/* context_handle */
		    gss_buffer_t	/* token_buffer */
		    );
    OM_uint32       (*gss_delete_sec_context)
	(
		    OM_uint32*,		/* minor_status */
		    gss_ctx_id_t*,	/* context_handle */
		    gss_buffer_t	/* output_token */
		    );
    OM_uint32       (*gss_context_time)
	(
		    OM_uint32*,		/* minor_status */
		    gss_ctx_id_t,	/* context_handle */
		    OM_uint32*		/* time_rec */
		    );
    OM_uint32       (*gss_get_mic)
	(
		    OM_uint32*,		/* minor_status */
		    gss_ctx_id_t,	/* context_handle */
		    gss_qop_t,		/* qop_req */
		    gss_buffer_t,	/* message_buffer */
		    gss_buffer_t	/* message_token */
		    );
    OM_uint32       (*gss_verify_mic)
	(
		    OM_uint32*,		/* minor_status */
		    gss_ctx_id_t,	/* context_handle */
		    gss_buffer_t,	/* message_buffer */
		    gss_buffer_t,	/* token_buffer */
		    gss_qop_t*		/* qop_state */
		    );
    OM_uint32       (*gss_wrap)
	(
		    OM_uint32*,		/* minor_status */
		    gss_ctx_id_t,	/* context_handle */
		    int,		/* conf_req_flag */
		    gss_qop_t,		/* qop_req */
		    gss_buffer_t,	/* input_message_buffer */
		    int*,		/* conf_state */
		    gss_buffer_t	/* output_message_buffer */
		    );
    OM_uint32       (*gss_unwrap)
	(
		    OM_uint32*,		/* minor_status */
		    gss_ctx_id_t,	/* context_handle */
		    gss_buffer_t,	/* input_message_buffer */
		    gss_buffer_t,	/* output_message_buffer */
		    int*,		/* conf_state */
		    gss_qop_t*		/* qop_state */
		    );
    OM_uint32       (*gss_display_status)
	(
		    OM_uint32*,		/* minor_status */
		    OM_uint32,		/* status_value */
		    int,		/* status_type */
		    gss_OID,		/* mech_type */
		    OM_uint32*,		/* message_context */
		    gss_buffer_t	/* status_string */
		    );
    OM_uint32       (*gss_indicate_mechs)
	(
		    OM_uint32*,		/* minor_status */
		    gss_OID_set*	/* mech_set */
		    );
    OM_uint32       (*gss_compare_name)
	(
		    OM_uint32*,		/* minor_status */
		    gss_name_t,		/* name1 */
		    gss_name_t,		/* name2 */
		    int*		/* name_equal */
		    );
    OM_uint32       (*gss_display_name)
	(
		    OM_uint32*,		/* minor_status */
		    gss_name_t,		/* input_name */
		    gss_buffer_t,	/* output_name_buffer */
		    gss_OID*		/* output_name_type */
		    );
    OM_uint32       (*gss_import_name)
	(
		    OM_uint32*,		/* minor_status */
		    gss_buffer_t,	/* input_name_buffer */
		    gss_OID,		/* input_name_type */
		    gss_name_t*		/* output_name */
		    );
    OM_uint32       (*gss_release_name)
	(
		    OM_uint32*,		/* minor_status */
		    gss_name_t*		/* input_name */
		    );
    OM_uint32       (*gss_inquire_cred)
	(
		    OM_uint32 *,		/* minor_status */
		    gss_cred_id_t,		/* cred_handle */
		    gss_name_t *,		/* name */
		    OM_uint32 *,		/* lifetime */
		    int *,			/* cred_usage */
		    gss_OID_set *		/* mechanisms */
		    );
    OM_uint32	    (*gss_add_cred)
	(
		    OM_uint32 *,	/* minor_status */
		    gss_cred_id_t,	/* input_cred_handle */
		    gss_name_t,		/* desired_name */
		    gss_OID,		/* desired_mech */
		    gss_cred_usage_t,	/* cred_usage */
		    OM_uint32,		/* initiator_time_req */
		    OM_uint32,		/* acceptor_time_req */
		    gss_cred_id_t *,	/* output_cred_handle */
		    gss_OID_set *,	/* actual_mechs */
		    OM_uint32 *,	/* initiator_time_rec */
		    OM_uint32 *		/* acceptor_time_rec */
		    );
    OM_uint32	    (*gss_export_sec_context)
	(
		    OM_uint32 *,	/* minor_status */
		    gss_ctx_id_t *,	/* context_handle */
		    gss_buffer_t	/* interprocess_token */
		    );
    OM_uint32	    (*gss_import_sec_context)
	(
		    OM_uint32 *,	/* minor_status */
		    gss_buffer_t,	/* interprocess_token */
		    gss_ctx_id_t *	/* context_handle */
		    );
    OM_uint32 	    (*gss_inquire_cred_by_mech)
	(
		    OM_uint32 *,	/* minor_status */
		    gss_cred_id_t,	/* cred_handle */
		    gss_OID,		/* mech_type */
		    gss_name_t *,	/* name */
		    OM_uint32 *,	/* initiator_lifetime */
		    OM_uint32 *,	/* acceptor_lifetime */
		    gss_cred_usage_t *	/* cred_usage */
		    );
    OM_uint32	    (*gss_inquire_names_for_mech)
	(
		    OM_uint32 *,	/* minor_status */
		    gss_OID,		/* mechanism */
		    gss_OID_set *	/* name_types */
		    );
    OM_uint32	(*gss_inquire_context)
	(
		    OM_uint32 *,	/* minor_status */
		    gss_ctx_id_t,	/* context_handle */
		    gss_name_t *,	/* src_name */
		    gss_name_t *,	/* targ_name */
		    OM_uint32 *,	/* lifetime_rec */
		    gss_OID *,		/* mech_type */
		    OM_uint32 *,	/* ctx_flags */
		    int *,	   	/* locally_initiated */
		    int *		/* open */
		    );
    OM_uint32	    (*gss_internal_release_oid)
	(
		    OM_uint32 *,	/* minor_status */
		    gss_OID *		/* OID */
	 );
    OM_uint32	     (*gss_wrap_size_limit)
	(
		    OM_uint32 *,	/* minor_status */
		    gss_ctx_id_t,	/* context_handle */
		    int,		/* conf_req_flag */
		    gss_qop_t,		/* qop_req */
		    OM_uint32,		/* req_output_size */
		    OM_uint32 *		/* max_input_size */
	 );
#if 0
    int		     (*pname_to_uid)
	(
		    char *,		/* pname */
		    gss_OID,		/* name type */
		    gss_OID,		/* mech type */
		    uid_t *		/* uid */
		    );
	OM_uint32		(*gssint_userok)
	(
		    OM_uint32 *,	/* minor_status */
		    const gss_name_t,	/* pname */
		    const char *,	/* local user */
		    int *		/* user ok? */
	/* */);
#endif

#ifdef _MIT_KRB5_1_11
        OM_uint32        (KRB5_CALLCONV *gss_localname)
        (
                    OM_uint32 *,        /* minor */
                    const gss_name_t,   /* name */
                    gss_const_OID,      /* mech_type */
                    gss_buffer_t /* localname */
            );
        OM_uint32               (KRB5_CALLCONV *gssspi_authorize_localname)
        (
                    OM_uint32 *,        /* minor_status */
                    const gss_name_t,   /* pname */
                    gss_const_buffer_t, /* local user */
                    gss_const_OID       /* local nametype */
        /* */);

#endif

	OM_uint32		(*gss_export_name)
	(
		OM_uint32 *,		/* minor_status */
		const gss_name_t,	/* input_name */
		gss_buffer_t		/* exported_name */
	/* */);

#ifdef _MIT_KRB5_1_11
        OM_uint32       (KRB5_CALLCONV *gss_duplicate_name)
        (
                    OM_uint32*,         /* minor_status */
                    const gss_name_t,   /* input_name */
                    gss_name_t *        /* output_name */
        /* */);
#endif

	OM_uint32	(*gss_store_cred)
	(
		OM_uint32 *,		/* minor_status */
		const gss_cred_id_t,	/* input_cred */
		gss_cred_usage_t,	/* cred_usage */
		const gss_OID,		/* desired_mech */
		OM_uint32,		/* overwrite_cred */
		OM_uint32,		/* default_cred */
		gss_OID_set *,		/* elements_stored */
		gss_cred_usage_t *	/* cred_usage_stored */
	/* */);


	/* GGF extensions */

	OM_uint32       (*gss_inquire_sec_context_by_oid)
	(
		    OM_uint32 *,	/* minor_status */
		    const gss_ctx_id_t, /* context_handle */
		    const gss_OID,      /* OID */
		    gss_buffer_set_t *  /* data_set */
		    );
	OM_uint32       (*gss_inquire_cred_by_oid)
	(
		    OM_uint32 *,	/* minor_status */
		    const gss_cred_id_t, /* cred_handle */
		    const gss_OID,      /* OID */
		    gss_buffer_set_t *  /* data_set */
		    );
	OM_uint32       (*gss_set_sec_context_option)
	(
		    OM_uint32 *,	/* minor_status */
		    gss_ctx_id_t *,     /* context_handle */
		    const gss_OID,      /* OID */
		    const gss_buffer_t  /* value */
		    );
	OM_uint32       (*gssspi_set_cred_option)
	(
		    OM_uint32 *,	/* minor_status */
		    gss_cred_id_t,      /* cred_handle */
		    const gss_OID,      /* OID */
		    const gss_buffer_t	/* value */
		    );
	OM_uint32       (*gssspi_mech_invoke)
	(
		    OM_uint32*,		/* minor_status */
		    const gss_OID, 	/* mech OID */
		    const gss_OID,      /* OID */
		    gss_buffer_t 	/* value */
		    );

	/* AEAD extensions */
	OM_uint32	(*gss_wrap_aead)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_ctx_id_t,		/* context_handle */
	    int,			/* conf_req_flag */
	    gss_qop_t,			/* qop_req */
	    gss_buffer_t,		/* input_assoc_buffer */
	    gss_buffer_t,		/* input_payload_buffer */
	    int *,			/* conf_state */
	    gss_buffer_t		/* output_message_buffer */
	/* */);

	OM_uint32	(*gss_unwrap_aead)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_ctx_id_t,		/* context_handle */
	    gss_buffer_t,		/* input_message_buffer */
	    gss_buffer_t,		/* input_assoc_buffer */
	    gss_buffer_t,		/* output_payload_buffer */
	    int *,			/* conf_state */
	    gss_qop_t *			/* qop_state */
	/* */);

	/* SSPI extensions */
	OM_uint32	(*gss_wrap_iov)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_ctx_id_t,		/* context_handle */
	    int,			/* conf_req_flag */
	    gss_qop_t,			/* qop_req */
	    int *,			/* conf_state */
	    gss_iov_buffer_desc *,	/* iov */
	    int				/* iov_count */
	/* */);

	OM_uint32	(*gss_unwrap_iov)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_ctx_id_t,		/* context_handle */
	    int *,			/* conf_state */
	    gss_qop_t *,		/* qop_state */
	    gss_iov_buffer_desc *,	/* iov */
	    int				/* iov_count */
	/* */);

	OM_uint32	(*gss_wrap_iov_length)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_ctx_id_t,		/* context_handle */
	    int,			/* conf_req_flag*/
	    gss_qop_t, 			/* qop_req */
	    int *, 			/* conf_state */
	    gss_iov_buffer_desc *,	/* iov */
	    int				/* iov_count */
	/* */);

	OM_uint32       (*gss_complete_auth_token)
	(
		    OM_uint32*,		/* minor_status */
		    const gss_ctx_id_t,	/* context_handle */
		    gss_buffer_t	/* input_message_buffer */
		    );

	/* New for 1.8 */

	OM_uint32	(*gss_acquire_cred_impersonate_name)
	(
	    OM_uint32 *,		/* minor_status */
	    const gss_cred_id_t,	/* impersonator_cred_handle */
	    const gss_name_t,		/* desired_name */
	    OM_uint32,			/* time_req */
	    const gss_OID_set,		/* desired_mechs */
	    gss_cred_usage_t,		/* cred_usage */
	    gss_cred_id_t *,		/* output_cred_handle */
	    gss_OID_set *,		/* actual_mechs */
	    OM_uint32 *			/* time_rec */
	/* */);

	OM_uint32	(*gss_add_cred_impersonate_name)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_cred_id_t,		/* input_cred_handle */
	    const gss_cred_id_t,	/* impersonator_cred_handle */
	    const gss_name_t,		/* desired_name */
	    const gss_OID,		/* desired_mech */
	    gss_cred_usage_t,		/* cred_usage */
	    OM_uint32,			/* initiator_time_req */
	    OM_uint32,			/* acceptor_time_req */
	    gss_cred_id_t *,		/* output_cred_handle */
	    gss_OID_set *,		/* actual_mechs */
	    OM_uint32 *,		/* initiator_time_rec */
	    OM_uint32 *			/* acceptor_time_rec */
	/* */);

	OM_uint32	(*gss_display_name_ext)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_name_t,			/* name */
	    gss_OID,			/* display_as_name_type */
	    gss_buffer_t		/* display_name */
	/* */);

	OM_uint32	(*gss_inquire_name)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_name_t,			/* name */
	    int *,			/* name_is_MN */
	    gss_OID *,			/* MN_mech */
	    gss_buffer_set_t *		/* attrs */
	/* */);

	OM_uint32	(*gss_get_name_attribute)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_name_t,			/* name */
	    gss_buffer_t,		/* attr */
	    int *,			/* authenticated */
	    int *,			/* complete */
	    gss_buffer_t,		/* value */
	    gss_buffer_t,		/* display_value */
	    int *			/* more */
	/* */);

	OM_uint32	(*gss_set_name_attribute)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_name_t,			/* name */
	    int,			/* complete */
	    gss_buffer_t,		/* attr */
	    gss_buffer_t		/* value */
	/* */);

	OM_uint32	(*gss_delete_name_attribute)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_name_t,			/* name */
	    gss_buffer_t		/* attr */
	/* */);

	OM_uint32	(*gss_export_name_composite)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_name_t,			/* name */
	    gss_buffer_t		/* exp_composite_name */
	/* */);

	OM_uint32	(*gss_map_name_to_any)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_name_t,			/* name */
	    int,			/* authenticated */
	    gss_buffer_t,		/* type_id */
	    gss_any_t *			/* output */
	/* */);

	OM_uint32	(*gss_release_any_name_mapping)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_name_t,			/* name */
	    gss_buffer_t,		/* type_id */
	    gss_any_t *			/* input */
	/* */);

#ifdef _MIT_KRB5_1_11
        OM_uint32       (KRB5_CALLCONV *gss_pseudo_random)
        (
            OM_uint32 *,                /* minor_status */
            gss_ctx_id_t,               /* context */
            int,                        /* prf_key */
            const gss_buffer_t,         /* prf_in */
            ssize_t,                    /* desired_output_len */
            gss_buffer_t                /* prf_out */
        /* */);

	OM_uint32	(KRB5_CALLCONV *gss_set_neg_mechs)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_cred_id_t,		/* cred_handle */
	    const gss_OID_set		/* mech_set */
	/* */);

	OM_uint32	(KRB5_CALLCONV *gss_inquire_saslname_for_mech)
	(
	    OM_uint32 *,		/* minor_status */
	    const gss_OID,		/* desired_mech */
	    gss_buffer_t,		/* sasl_mech_name */
	    gss_buffer_t,		/* mech_name */
	    gss_buffer_t		/* mech_description */
	/* */);

	OM_uint32	(KRB5_CALLCONV *gss_inquire_mech_for_saslname)
	(
	    OM_uint32 *,		/* minor_status */
	    const gss_buffer_t,		/* sasl_mech_name */
	    gss_OID *			/* mech_type */
	/* */);

	OM_uint32	(KRB5_CALLCONV *gss_inquire_attrs_for_mech)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_const_OID,		/* mech */
	    gss_OID_set *,		/* mech_attrs */
	    gss_OID_set *		/* known_mech_attrs */
	/* */);

	/* Credential store extensions */

	OM_uint32       (KRB5_CALLCONV *gss_acquire_cred_from)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_name_t,			/* desired_name */
	    OM_uint32,			/* time_req */
	    gss_OID_set,		/* desired_mechs */
	    gss_cred_usage_t,		/* cred_usage */
	    gss_const_key_value_set_t,	/* cred_store */
	    gss_cred_id_t *,		/* output_cred_handle */
	    gss_OID_set *,		/* actual_mechs */
	    OM_uint32 *			/* time_rec */
	/* */);

	OM_uint32       (KRB5_CALLCONV *gss_store_cred_into)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_cred_id_t,		/* input_cred_handle */
	    gss_cred_usage_t,		/* input_usage */
	    gss_OID,			/* desired_mech */
	    OM_uint32,			/* overwrite_cred */
	    OM_uint32,			/* default_cred */
	    gss_const_key_value_set_t,	/* cred_store */
	    gss_OID_set *,		/* elements_stored */
	    gss_cred_usage_t *		/* cred_usage_stored */
	/* */);

	OM_uint32       (KRB5_CALLCONV *gssspi_acquire_cred_with_password)
	(
	    OM_uint32 *,		/* minor_status */
	    const gss_name_t,		/* desired_name */
	    const gss_buffer_t,	 /* password */
	    OM_uint32,			/* time_req */
	    const gss_OID_set,		/* desired_mechs */
	    int,			/* cred_usage */
	    gss_cred_id_t *,		/* output_cred_handle */
	    gss_OID_set *,		/* actual_mechs */
	    OM_uint32 *			/* time_rec */
	/* */);

	OM_uint32       (KRB5_CALLCONV *gss_export_cred)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_cred_id_t,		/* cred_handle */
	    gss_buffer_t		/* token */
	/* */);

	OM_uint32       (KRB5_CALLCONV *gss_import_cred)
	(
		OM_uint32 *,		/* minor_status */
		gss_buffer_t,		/* token */
		gss_cred_id_t *		/* cred_handle */
	/* */);

	OM_uint32       (KRB5_CALLCONV *gssspi_import_sec_context_by_mech)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_OID,			/* desired_mech */
	    gss_buffer_t,		/* interprocess_token */
	    gss_ctx_id_t *		/* context_handle */
	/* */);

	OM_uint32       (KRB5_CALLCONV *gssspi_import_name_by_mech)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_OID,			/* mech_type */
	    gss_buffer_t,		/* input_name_buffer */
	    gss_OID,			/* input_name_type */
	    gss_name_t*			/* output_name */
	/* */);

	OM_uint32       (KRB5_CALLCONV *gssspi_import_cred_by_mech)
	(
	    OM_uint32 *,		/* minor_status */
	    gss_OID,			/* mech_type */
	    gss_buffer_t,		/* token */
	    gss_cred_id_t *		/* cred_handle */
	/* */);

#ifdef _MIT_KRB5_1_12
        /* get_mic_iov extensions, added in 1.12 */

        OM_uint32       (KRB5_CALLCONV *gss_get_mic_iov)
        (
            OM_uint32 *,                /* minor_status */
            gss_ctx_id_t,               /* context_handle */
            gss_qop_t,                  /* qop_req */
            gss_iov_buffer_desc *,      /* iov */
            int                         /* iov_count */
        );

        OM_uint32       (KRB5_CALLCONV *gss_verify_mic_iov)
        (
            OM_uint32 *,                /* minor_status */
            gss_ctx_id_t,               /* context_handle */
            gss_qop_t *,                /* qop_state */
            gss_iov_buffer_desc *,      /* iov */
            int                         /* iov_count */
        );

        OM_uint32       (KRB5_CALLCONV *gss_get_mic_iov_length)
        (
            OM_uint32 *,                /* minor_status */
            gss_ctx_id_t,               /* context_handle */
            gss_qop_t,                  /* qop_req */
            gss_iov_buffer_desc *,      /* iov */
            int                         /* iov_count */
        );
#endif

#endif


} *GSS_MECH_PLUGIN_CONFIG;

typedef struct gss_ctx_id_struct {
 struct gss_ctx_id_struct *loopback;
 gss_OID mech_type;
 gss_ctx_id_t internal_ctx_id;
} gss_union_ctx_id_desc, *gss_union_ctx_id_t;

#endif
