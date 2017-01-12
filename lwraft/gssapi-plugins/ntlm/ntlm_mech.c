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
 * Copyright (C) 2006,2008 by the Massachusetts Institute of Technology.
 * All rights reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 */

/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */
/*
 * Copyright (c) 2006-2008, Novell, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *   * The copyright holder's name is not used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (C) 2014, VMware Inc. All rights reserved.
 *
 * Module: ntlm_mech.c
 * Abstract:
 *     VMware GSSAPI NTLM Authentication Plugin
 *     GSSAPI NTLM Plugin mechanism function table
 *
 * Author: Jonathan Brown (brownj@vmware.com)
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>

#include	<krb5.h>
#include "ntlm_util.h"

/* Copy of GSSAPI plugin struct gss_config structure */
#include "ntlm_mglueP.h"
#include "gssapiP_ntlm.h"



#undef g_token_size

#define HARD_ERROR(v) ((v) != GSS_S_COMPLETE && (v) != GSS_S_CONTINUE_NEEDED)
typedef const gss_OID_desc *gss_OID_const;

static ntlm_token_t make_ntlm_token(char *);
static gss_buffer_desc make_err_msg(char *);



/* NTLM oid structure */
static const gss_OID_desc ntlm_gss_oid_array[] = {
    {NTLM_OID_LENGTH, NTLM_OID},

    /* 2.1.1. Kerberos Principal Name Form:  (rfc 1964)
     * This name form shall be represented by the Object Identifier {iso(1)
     * member-body(2) United States(840) mit(113554) infosys(1) gssapi(2)
     * krb5(2) krb5_name(1)}.  The recommended symbolic name for this type
     * is "GSS_KRB5_NT_PRINCIPAL_NAME". */
    {10, "\052\206\110\206\367\022\001\002\002\001"},

    /* 1.3.6.1.4.1.27433.3.1 */
    {10, "\x2b\x06\x01\x04\x01\x81\xd6\x29\x03\x01"},
};

const gss_OID_desc * const gss_mech_ntlm_oid           = ntlm_gss_oid_array+0;
const gss_OID_desc * const gss_nt_ntlm_name_oid        = ntlm_gss_oid_array+1;
const gss_OID_desc * const gss_ntlm_password_oid       = ntlm_gss_oid_array+2;
/*const gss_OID_desc * const GSS_KRB5_NT_PRINCIPAL_NAME = ntlm_gss_oid_array+1; */

int gss_ntlmint_lib_init(void)
{
#ifdef _GSS_STATIC_LINK
	return gss_ntlmmechglue_init();
#else
	return 0;
#endif
}

void gss_ntlmint_lib_fini(void)
{
}



/*
 * NegHints ::= SEQUENCE {
 *    hintName       [0]  GeneralString      OPTIONAL,
 *    hintAddress    [1]  OCTET STRING       OPTIONAL
 * }
 */

#define HOST_PREFIX	"host@"
#define HOST_PREFIX_LEN	(sizeof(HOST_PREFIX) - 1)


/*ARGSUSED*/
OM_uint32
ntlm_gss_display_status(
		OM_uint32 *minor_status,
		OM_uint32 status_value,
		int status_type,
		gss_OID mech_type,
		OM_uint32 *message_context,
		gss_buffer_t status_string)
{
	dsyslog("Entering display_status\n");

	*message_context = 0;
	switch (status_value) {
	    case ERR_NTLM_NO_MECHS_AVAILABLE:
		/* CSTYLED */
		*status_string = make_err_msg("NTLM cannot find mechanisms to negotiate");
		break;
	    case ERR_NTLM_NO_CREDS_ACQUIRED:
		/* CSTYLED */
		*status_string = make_err_msg("NTLM failed to acquire creds");
		break;
	    case ERR_NTLM_NO_MECH_FROM_ACCEPTOR:
		/* CSTYLED */
		*status_string = make_err_msg("NTLM acceptor did not select a mechanism");
		break;
	    case ERR_NTLM_NEGOTIATION_FAILED:
		/* CSTYLED */
		*status_string = make_err_msg("NTLM failed to negotiate a mechanism");
		break;
	    case ERR_NTLM_NO_TOKEN_FROM_ACCEPTOR:
		/* CSTYLED */
		*status_string = make_err_msg("NTLM acceptor did not return a valid token");
		break;
	    default:
		status_string->length = 0;
		status_string->value = "";
		break;
	}

	dsyslog("Leaving display_status\n");
	return (GSS_S_COMPLETE);
}


/*ARGSUSED*/
OM_uint32
ntlm_gss_import_name(
		    OM_uint32 *minor_status,
		    gss_buffer_t input_name_buffer,
		    gss_OID input_name_type,
		    gss_name_t *output_name)
{
	OM_uint32 status = 0;

	dsyslog("Entering import_name\n");

	status = gss_import_name(minor_status, input_name_buffer,
			input_name_type, output_name);

	dsyslog("Leaving import_name\n");
	return (status);
}

/*ARGSUSED*/
OM_uint32
ntlm_gss_release_name(
			OM_uint32 *minor_status,
			gss_name_t *input_name)
{
	OM_uint32 status = 0;

	dsyslog("Entering release_name\n");

	status = gss_release_name(minor_status, input_name);

	dsyslog("Leaving release_name\n");
	return (status);
}

OM_uint32
ntlm_gss_inquire_cred(
			OM_uint32 *minor_status,
			gss_cred_id_t cred_handle,
			gss_name_t *name,
			OM_uint32 *lifetime,
			int *cred_usage,
			gss_OID_set *mechanisms)
{
	OM_uint32 status = 0;
        ntlm_gss_cred_id_t ntlm_cred_handle = NULL;
        gss_name_t ret_name = NULL;

	dsyslog("Entering inquire_cred\n");

        ntlm_cred_handle = (ntlm_gss_cred_id_t) cred_handle;
        if (ntlm_cred_handle && ntlm_cred_handle->name && name)
        {
            status = gss_duplicate_name(
                         minor_status,
                         ntlm_cred_handle->name,
                         &ret_name);
            if (status == 0)
            {
                *name = ret_name;
            }
        }

	dsyslog("Leaving inquire_cred\n");

	return (status);
}

/*ARGSUSED*/
OM_uint32
ntlm_gss_compare_name(
			OM_uint32 *minor_status,
			const gss_name_t name1,
			const gss_name_t name2,
			int *name_equal)
{
	OM_uint32 status = GSS_S_COMPLETE;
	dsyslog("Entering compare_name\n");

	status = gss_compare_name(minor_status, name1, name2, name_equal);

	dsyslog("Leaving compare_name\n");
	return (status);
}

/*ARGSUSED*/
OM_uint32
ntlm_gss_inquire_names_for_mech(
				OM_uint32	*minor_status,
				gss_OID		mechanism,
				gss_OID_set	*name_types)
{
	OM_uint32   major = 0;
	OM_uint32   minor = 0;

	dsyslog("Entering inquire_names_for_mech\n");
        if (major)
        {
            goto error;
        }

	dsyslog("Leaving inquire_names_for_mech\n");
error:
        if (major)
        {
            *minor_status = minor;
        }
	return (major);
}

OM_uint32
ntlm_gss_unwrap(
		OM_uint32 *minor_status,
		gss_ctx_id_t context_handle,
		gss_buffer_t input_message_buffer,
		gss_buffer_t output_message_buffer,
		int *conf_state,
		gss_qop_t *qop_state)
{
	OM_uint32 ret;
	ret = gss_unwrap(minor_status,
			context_handle,
			input_message_buffer,
			output_message_buffer,
			conf_state,
			qop_state);

	return (ret);
}

OM_uint32
ntlm_gss_wrap(
		OM_uint32 *minor_status,
		gss_ctx_id_t context_handle,
		int conf_req_flag,
		gss_qop_t qop_req,
		gss_buffer_t input_message_buffer,
		int *conf_state,
		gss_buffer_t output_message_buffer)
{
	OM_uint32 ret;
	ret = gss_wrap(minor_status,
		    context_handle,
		    conf_req_flag,
		    qop_req,
		    input_message_buffer,
		    conf_state,
		    output_message_buffer);

	return (ret);
}

OM_uint32
ntlm_gss_process_context_token(
				OM_uint32	*minor_status,
				const gss_ctx_id_t context_handle,
				const gss_buffer_t token_buffer)
{
	OM_uint32 ret;
	ret = gss_process_context_token(minor_status,
					context_handle,
					token_buffer);

	return (ret);
}

OM_uint32
ntlm_gss_context_time(
			OM_uint32	*minor_status,
			const gss_ctx_id_t context_handle,
			OM_uint32	*time_rec)
{
	OM_uint32 ret;
	ret = gss_context_time(minor_status,
			    context_handle,
			    time_rec);
	return (ret);
}
#ifndef LEAN_CLIENT
OM_uint32
ntlm_gss_export_sec_context(
			    OM_uint32	  *minor_status,
			    gss_ctx_id_t *context_handle,
			    gss_buffer_t interprocess_token)
{
	OM_uint32 ret;
	ret = gss_export_sec_context(minor_status,
				    context_handle,
				    interprocess_token);
	return (ret);
}

OM_uint32
ntlm_gss_import_sec_context(
	OM_uint32		*minor_status,
	const gss_buffer_t	interprocess_token,
	gss_ctx_id_t		*context_handle)
{
	OM_uint32 ret;
	ret = gss_import_sec_context(minor_status,
				    interprocess_token,
				    context_handle);
	return (ret);
}
#endif /* LEAN_CLIENT */

OM_uint32
ntlm_gss_inquire_context(
			OM_uint32	*minor_status,
			const gss_ctx_id_t context_handle,
			gss_name_t	*src_name,
			gss_name_t	*targ_name,
			OM_uint32	*lifetime_rec,
			gss_OID		*mech_type,
			OM_uint32	*ctx_flags,
			int		*locally_initiated,
			int		*opened)
{
	OM_uint32 ret = GSS_S_COMPLETE;

	ret = gss_inquire_context(minor_status,
				context_handle,
				src_name,
				targ_name,
				lifetime_rec,
				NULL,
				ctx_flags,
				locally_initiated,
				opened);

    if (mech_type)
        *mech_type = context_handle->mech_type;

	return (ret);
}

OM_uint32
ntlm_gss_wrap_size_limit(
	OM_uint32	*minor_status,
	const gss_ctx_id_t context_handle,
	int		conf_req_flag,
	gss_qop_t	qop_req,
	OM_uint32	req_output_size,
	OM_uint32	*max_input_size)
{
	OM_uint32 ret;
	ret = gss_wrap_size_limit(minor_status,
				context_handle,
				conf_req_flag,
				qop_req,
				req_output_size,
				max_input_size);
	return (ret);
}

OM_uint32
ntlm_gss_get_mic(
		OM_uint32 *minor_status,
		const gss_ctx_id_t context_handle,
		gss_qop_t  qop_req,
		const gss_buffer_t message_buffer,
		gss_buffer_t message_token)
{
	OM_uint32 ret;
	ret = gss_get_mic(minor_status,
		    context_handle,
		    qop_req,
		    message_buffer,
		    message_token);
	return (ret);
}

OM_uint32
ntlm_gss_verify_mic(
		OM_uint32 *minor_status,
		const gss_ctx_id_t context_handle,
		const gss_buffer_t msg_buffer,
		const gss_buffer_t token_buffer,
		gss_qop_t *qop_state)
{
	OM_uint32 ret;
	ret = gss_verify_mic(minor_status,
			    context_handle,
			    msg_buffer,
			    token_buffer,
			    qop_state);
	return (ret);
}

OM_uint32
ntlm_gss_inquire_sec_context_by_oid(
		OM_uint32 *minor_status,
		const gss_ctx_id_t context_handle,
		const gss_OID desired_object,
		gss_buffer_set_t *data_set)
{
	OM_uint32 ret;
	ret = gss_inquire_sec_context_by_oid(minor_status,
			    context_handle,
			    desired_object,
			    data_set);
	return (ret);
}

OM_uint32
ntlm_gss_inquire_cred_by_oid(
		OM_uint32 *minor_status,
		const gss_cred_id_t cred_handle,
		const gss_OID desired_object,
		gss_buffer_set_t *data_set)
{
	OM_uint32 ret;
	ret = gss_inquire_cred_by_oid(minor_status,
				cred_handle,
				desired_object,
				data_set);
	return (ret);
}

OM_uint32
ntlm_gss_set_sec_context_option(
		OM_uint32 *minor_status,
		gss_ctx_id_t *context_handle,
		const gss_OID desired_object,
		const gss_buffer_t value)
{
	OM_uint32 ret;
	ret = gss_set_sec_context_option(minor_status,
			    context_handle,
			    desired_object,
			    value);
	return (ret);
}

OM_uint32
ntlm_gssspi_set_cred_option(OM_uint32 *minor_status,
                       gss_cred_id_t cred_handle,
                       const gss_OID desired_object,
                       const gss_buffer_t value)
{
    OM_uint32 ret = 0;
    ntlm_gss_cred_id_t ntlm_cred = NULL;
    gss_buffer_t value_buf = NULL;


#ifdef _MIT_KRB5_1_11
    ntlm_cred = (ntlm_gss_cred_id_t) *((gss_cred_id_t *) cred_handle);
#else
    ntlm_cred = (ntlm_gss_cred_id_t) cred_handle;
#endif
    if (desired_object->length == GSS_NTLM_PASSWORD_LEN &&
        memcmp(desired_object->elements,
               GSS_NTLM_PASSWORD_OID,
               GSS_NTLM_PASSWORD_LEN) == 0)
    {
        value_buf = calloc(value->length+1, sizeof(gss_buffer_desc));
        value_buf->value = calloc(value->length+1, sizeof(unsigned char));
        if (!value_buf->value)
        {
            return (GSS_S_FAILURE);
        }
    
        memcpy(value_buf->value, value->value, value->length);
        value_buf->length = value->length;
        ntlm_cred->password = value_buf;
    }

    return (ret);
}

OM_uint32
ntlm_gss_wrap_aead(OM_uint32 *minor_status,
		     gss_ctx_id_t context_handle,
		     int conf_req_flag,
		     gss_qop_t qop_req,
		     gss_buffer_t input_assoc_buffer,
		     gss_buffer_t input_payload_buffer,
		     int *conf_state,
		     gss_buffer_t output_message_buffer)
{
    OM_uint32 ret;
	ret = gss_wrap_aead(minor_status,
			    context_handle,
			    conf_req_flag,
			    qop_req,
			    input_assoc_buffer,
			    input_payload_buffer,
			    conf_state,
			    output_message_buffer);

	return (ret);
}

OM_uint32
ntlm_gss_unwrap_aead(OM_uint32 *minor_status,
		       gss_ctx_id_t context_handle,
		       gss_buffer_t input_message_buffer,
		       gss_buffer_t input_assoc_buffer,
		       gss_buffer_t output_payload_buffer,
		       int *conf_state,
		       gss_qop_t *qop_state)
{
	OM_uint32 ret;
	ret = gss_unwrap_aead(minor_status,
			      context_handle,
			      input_message_buffer,
			      input_assoc_buffer,
			      output_payload_buffer,
			      conf_state,
			      qop_state);
	return (ret);
}


OM_uint32
ntlm_gss_wrap_iov_length(OM_uint32 *minor_status,
			   gss_ctx_id_t context_handle,
			   int conf_req_flag,
			   gss_qop_t qop_req,
			   int *conf_state,
			   gss_iov_buffer_desc *iov,
			   int iov_count)
{
	OM_uint32 ret;
	ret = gss_wrap_iov_length(minor_status,
				  context_handle,
				  conf_req_flag,
				  qop_req,
				  conf_state,
				  iov,
				  iov_count);
	return (ret);
}


OM_uint32
ntlm_gss_complete_auth_token(
		OM_uint32 *minor_status,
		const gss_ctx_id_t context_handle,
		gss_buffer_t input_message_buffer)
{
	OM_uint32 ret;
	ret = gss_complete_auth_token(minor_status,
				      context_handle,
				      input_message_buffer);
	return (ret);
}

OM_uint32
ntlm_gss_acquire_cred_impersonate_name(OM_uint32 *minor_status,
					 const gss_cred_id_t impersonator_cred_handle,
					 const gss_name_t desired_name,
					 OM_uint32 time_req,
					 const gss_OID_set desired_mechs,
					 gss_cred_usage_t cred_usage,
					 gss_cred_id_t *output_cred_handle,
					 gss_OID_set *actual_mechs,
					 OM_uint32 *time_rec)
{
	OM_uint32 status = 0;

	dsyslog("Entering ntlm_gss_acquire_cred_impersonate_name\n");


	dsyslog("Leaving ntlm_gss_acquire_cred_impersonate_name\n");
	return (status);
}

OM_uint32
ntlm_gss_display_name_ext(OM_uint32 *minor_status,
			    gss_name_t name,
			    gss_OID display_as_name_type,
			    gss_buffer_t display_name)
{
	OM_uint32 ret = 0;
	ret = gss_display_name_ext(minor_status,
				   name,
				   display_as_name_type,
				   display_name);
	return (ret);
}


OM_uint32
ntlm_gss_inquire_name(OM_uint32 *minor_status,
			gss_name_t name,
			int *name_is_MN,
			gss_OID *MN_mech,
			gss_buffer_set_t *attrs)
{
	OM_uint32 ret;
	ret = gss_inquire_name(minor_status,
			       name,
			       name_is_MN,
			       MN_mech,
			       attrs);
	return (ret);
}

OM_uint32
ntlm_gss_get_name_attribute(OM_uint32 *minor_status,
			      gss_name_t name,
			      gss_buffer_t attr,
			      int *authenticated,
			      int *complete,
			      gss_buffer_t value,
			      gss_buffer_t display_value,
			      int *more)
{
	OM_uint32 ret;
	ret = gss_get_name_attribute(minor_status,
				     name,
				     attr,
				     authenticated,
				     complete,
				     value,
				     display_value,
				     more);
	return (ret);
}

OM_uint32
ntlm_gss_set_name_attribute(OM_uint32 *minor_status,
			      gss_name_t name,
			      int complete,
			      gss_buffer_t attr,
			      gss_buffer_t value)
{
	OM_uint32 ret;
	ret = gss_set_name_attribute(minor_status,
				     name,
				     complete,
				     attr,
				     value);
	return (ret);
}

OM_uint32
ntlm_gss_delete_name_attribute(OM_uint32 *minor_status,
				 gss_name_t name,
				 gss_buffer_t attr)
{
	OM_uint32 ret;
	ret = gss_delete_name_attribute(minor_status,
					name,
					attr);
	return (ret);
}

OM_uint32
ntlm_gss_export_name_composite(OM_uint32 *minor_status,
				 gss_name_t name,
				 gss_buffer_t exp_composite_name)
{
	OM_uint32 ret;
	ret = gss_export_name_composite(minor_status,
					name,
					exp_composite_name);
	return (ret);
}

OM_uint32
ntlm_gss_map_name_to_any(OM_uint32 *minor_status,
			   gss_name_t name,
			   int authenticated,
			   gss_buffer_t type_id,
			   gss_any_t *output)
{
	OM_uint32 ret;
	ret = gss_map_name_to_any(minor_status,
				  name,
				  authenticated,
				  type_id,
				  output);
	return (ret);
}

OM_uint32
ntlm_gss_release_any_name_mapping(OM_uint32 *minor_status,
				    gss_name_t name,
				    gss_buffer_t type_id,
				    gss_any_t *input)
{
	OM_uint32 ret;
	ret = gss_release_any_name_mapping(minor_status,
					   name,
					   type_id,
					   input);
	return (ret);
}



/* following are token creation and reading routines */

/*
 * This routine compares the recieved mechset to the mechset that
 * this server can support. It looks sequentially through the mechset
 * and the first one that matches what the server can support is
 * chosen as the negotiated mechanism. If one is found, negResult
 * is set to ACCEPT_INCOMPLETE if it's the first mech, REQUEST_MIC if
 * it's not the first mech, otherwise we return NULL and negResult
 * is set to REJECT.
 *
 * NOTE: There is currently no way to specify a preference order of
 * mechanisms supported by the acceptor.
 */

/*
 * the next two routines make a token buffer suitable for
 * ntlm_gss_display_status. These currently take the string
 * in name and place it in the token. Eventually, if
 * ntlm_gss_display_status returns valid error messages,
 * these routines will be changes to return the error string.
 */
static ntlm_token_t
make_ntlm_token(char *name)
{
	return (ntlm_token_t)strdup(name);
}

static gss_buffer_desc
make_err_msg(char *name)
{
	gss_buffer_desc buffer;

	if (name == NULL) {
		buffer.length = 0;
		buffer.value = NULL;
	} else {
		buffer.length = strlen(name)+1;
		buffer.value = make_ntlm_token(name);
	}

	return (buffer);
}

/*
 * The Mech OID:
 *      iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2) ntlm(10)
 *        = 1.2.840.113554.1.2.10
 */
static struct _GSS_MECH_PLUGIN_CONFIG ntlm_mechanism =
{
	{NTLM_OID_LENGTH, NTLM_OID},
	NULL,
	ntlm_gss_acquire_cred,
	ntlm_gss_release_cred,
	ntlm_gss_init_sec_context,
#ifndef LEAN_CLIENT
	ntlm_gss_accept_sec_context,
#else
	NULL,
#endif  /* LEAN_CLIENT */
	NULL,				/* gss_process_context_token */
	ntlm_gss_delete_sec_context,	/* gss_delete_sec_context */
	ntlm_gss_context_time,	/* gss_context_time */
	ntlm_gss_get_mic,		/* gss_get_mic */
	ntlm_gss_verify_mic,		/* gss_verify_mic */
	ntlm_gss_wrap,		/* gss_wrap */
	ntlm_gss_unwrap,		/* gss_unwrap */
	ntlm_gss_display_status,
	NULL,				/* gss_indicate_mechs */
	ntlm_gss_compare_name,
	ntlm_gss_display_name,
	ntlm_gss_import_name,
	ntlm_gss_release_name,
	ntlm_gss_inquire_cred,	/* gss_inquire_cred */
	NULL,				/* gss_add_cred */
#ifndef LEAN_CLIENT
	ntlm_gss_export_sec_context,		/* gss_export_sec_context */
	ntlm_gss_import_sec_context,		/* gss_import_sec_context */
#else
	NULL,				/* gss_export_sec_context */
	NULL,				/* gss_import_sec_context */
#endif /* LEAN_CLIENT */
	NULL,				/* gss_inquire_cred_by_mech */
	ntlm_gss_inquire_names_for_mech,
	ntlm_gss_inquire_context,	/* gss_inquire_context */
	NULL,				/* gss_internal_release_oid */
	ntlm_gss_wrap_size_limit,	/* gss_wrap_size_limit */
#ifdef _MIT_KRB5_1_11
	NULL,				/* gss_localname */
	NULL,				/* gssspi_authorize_localname */
#endif
	NULL,				/* gss_export_name */

#ifdef _MIT_KRB5_1_11
	NULL,				/* gss_duplicate_name */
#endif

	NULL,				/* gss_store_cred */
	ntlm_gss_inquire_sec_context_by_oid, /* gss_inquire_sec_context_by_oid */
	ntlm_gss_inquire_cred_by_oid,	/* gss_inquire_cred_by_oid */
	ntlm_gss_set_sec_context_option, /* gss_set_sec_context_option */
	ntlm_gssspi_set_cred_option,	/* gssspi_set_cred_option */
	NULL,				/* gssspi_mech_invoke */
	ntlm_gss_wrap_aead,
	ntlm_gss_unwrap_aead,
	ntlm_gss_wrap_iov,
	ntlm_gss_unwrap_iov,
	ntlm_gss_wrap_iov_length,
	ntlm_gss_complete_auth_token,
	ntlm_gss_acquire_cred_impersonate_name,
	NULL,				/* gss_add_cred_impersonate_name */
	ntlm_gss_display_name_ext,
	ntlm_gss_inquire_name,
	ntlm_gss_get_name_attribute,
	ntlm_gss_set_name_attribute,
	ntlm_gss_delete_name_attribute,
	ntlm_gss_export_name_composite,
	ntlm_gss_map_name_to_any,
	ntlm_gss_release_any_name_mapping,
#ifdef _MIT_KRB5_1_11
	NULL,				/* gss_pseudo_random */
	NULL,				/* gss_set_neg_mechs */
	NULL,				/* gss_inquire_saslname_for_mech */
	NULL,				/* gss_inquire_mech_for_saslname */
	NULL,				/* gss_inquire_attrs_for_mech */
	NULL,				/* gss_acquire_cred_from */
	NULL,				/* gss_store_cred_into */
	NULL,				/* gssspi_acquire_cred_with_password */
	NULL,				/* gss_export_cred */
	NULL,				/* gss_import_cred */
	NULL,				/* gssspi_import_sec_context_by_mech */
	NULL,				/* gssspi_import_name_by_mech */
	NULL,				/* gssspi_import_cred_by_mech */
#endif
};

#ifdef _GSS_STATIC_LINK
#include "mglueP.h"

static int gss_ntlmmechglue_init(void)
{
	struct gss_mech_config mech_ntlm;

	memset(&mech_ntlm, 0, sizeof(mech_ntlm));
	mech_ntlm.mech = &ntlm_mechanism;
	mech_ntlm.mechNameStr = "ntlm";
	mech_ntlm.mech_type = (const gss_OID_desc * const) gss_mech_ntlm;

	return gssint_register_mechinfo(&mech_ntlm);
}
#else
GSS_MECH_PLUGIN_CONFIG gss_mech_initialize(void)
{
	return (&ntlm_mechanism);
}

#endif /* _GSS_STATIC_LINK */
