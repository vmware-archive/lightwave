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
 * Copyright (C) 2014, 2015 VMware Inc. All rights reserved.
 *
 * Module: srp_mech_desc_srp10.c
 * Abstract:
 *     VMware GSSAPI SRP Authentication Plugin
 *     GSSAPI SRP Plugin mechanism function table  (OID=1.2.840.113554.1.2.10)
 *
 * Author: Adam Bernstein (abernstein@vmware.com)
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>


#include	<krb5.h>
#include        <vmdirdefines.h>
#include "srp_util.h"
#include "gssapi_alloc.h"


/* Copy of GSSAPI plugin struct gss_config structure */
#include "srp_mglueP.h"
#include "gssapiP_srp.h"
#include "srp_mech.h"

OM_uint32
srp_gss_indicate_mechs(
    OM_uint32 *minor_status,
    gss_OID_set *mech_set)
{
    gss_OID_set_desc *ret_mech_set = NULL;
    gss_OID new_oid = NULL;
    OM_uint32 major = 0;

    if (minor_status)
    {
        *minor_status = 0;
    }

    ret_mech_set = (gss_OID_set_desc *)
        gssalloc_calloc(1, sizeof(*ret_mech_set));
    if (!ret_mech_set)
    {
        major = GSS_S_FAILURE;
        goto error;
    }

    /* Returning only the 2 SRP mech oids */
    ret_mech_set->elements = (gss_OID_desc *)
        gssalloc_calloc(2, sizeof(*ret_mech_set->elements));
    if (!ret_mech_set->elements)
    {
        major = GSS_S_FAILURE;
        goto error;
    }

    major = srp_gss_duplicate_oid(minor_status,
         (gss_OID) gss_mech_srp_oid,
         &new_oid);
    if (major)
    {
        goto error;
    }
    ret_mech_set->elements[0] = *new_oid, new_oid = NULL;

    major = srp_gss_duplicate_oid(minor_status,
         (gss_OID) gss_mech_gssapi_srp_oid,
         &new_oid);
    if (major)
    {
        goto error;
    }
    ret_mech_set->elements[1] = *new_oid, new_oid = NULL;
    ret_mech_set->count = 2;
    *mech_set = ret_mech_set;
    ret_mech_set = NULL;

error:
    if (major)
    {
        /* Free stuff */
        if (ret_mech_set)
        {
            if (ret_mech_set->elements)
            {
                gssalloc_free(ret_mech_set->elements);
            }
            gssalloc_free(ret_mech_set);
        }
        ret_mech_set = NULL;
    }
    return major;
}

static
OM_uint32 KRB5_CALLCONV
srp_gss_inquire_attrs_for_mech(OM_uint32 *minor_status,
                               gss_const_OID mech,
                               gss_OID_set *mech_attrs,
                               gss_OID_set *known_mech_attrs)
{
    OM_uint32 major, tmpMinor;

    /* known_mech_attrs is handled by mechglue */
    *minor_status = 0;

    if (mech_attrs == NULL)
        return (GSS_S_COMPLETE);

    major = gss_create_empty_oid_set(minor_status, mech_attrs);
    if (GSS_ERROR(major))
        goto cleanup;

#define MA_SUPPORTED(ma)    do {                                        \
        major = gss_add_oid_set_member(minor_status, (gss_OID)ma,       \
                                       mech_attrs);                     \
        if (GSS_ERROR(major))                                           \
            goto cleanup;                                               \
    } while (0)

    MA_SUPPORTED(gss_mech_srp_oid);

cleanup:
    if (GSS_ERROR(major))
        gss_release_oid_set(&tmpMinor, mech_attrs);

    return (major);
}

/*
 * The Mech OID:
 *      iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2) srp(10)
 *        = 1.2.840.113554.1.2.10
 */
static struct _GSS_MECH_PLUGIN_CONFIG srp_mechanism =
{
        {GSS_SRP_MECH_OID_LENGTH, GSS_SRP_MECH_OID},
	NULL,
	srp_gss_acquire_cred,
	srp_gss_release_cred,
	srp_gss_init_sec_context,
#ifndef LEAN_CLIENT
	srp_gss_accept_sec_context,
#else
	NULL,
#endif  /* LEAN_CLIENT */
	NULL,				/* gss_process_context_token */
	srp_gss_delete_sec_context,	/* gss_delete_sec_context */
	srp_gss_context_time,	/* gss_context_time */
	srp_gss_get_mic,		/* gss_get_mic */
	srp_gss_verify_mic,		/* gss_verify_mic */
	srp_gss_wrap,		/* gss_wrap */
	srp_gss_unwrap,		/* gss_unwrap */
	srp_gss_display_status,
        srp_gss_indicate_mechs, /* gss_indicate_mechs */
	srp_gss_compare_name,
	srp_gss_display_name,
	srp_gss_import_name,
	srp_gss_release_name,
	srp_gss_inquire_cred,	/* gss_inquire_cred */
	NULL,				/* gss_add_cred */
#ifndef LEAN_CLIENT
	srp_gss_export_sec_context,		/* gss_export_sec_context */
	srp_gss_import_sec_context,		/* gss_import_sec_context */
#else
	NULL,				/* gss_export_sec_context */
	NULL,				/* gss_import_sec_context */
#endif /* LEAN_CLIENT */
	NULL,				/* gss_inquire_cred_by_mech */
	srp_gss_inquire_names_for_mech,
	srp_gss_inquire_context,	/* gss_inquire_context */
        srp_gss_internal_release_oid,
	srp_gss_wrap_size_limit,	/* gss_wrap_size_limit */
#ifdef _MIT_KRB5_1_11
	NULL,				/* gss_localname */
	NULL,				/* gssspi_authorize_localname */
#endif
	NULL,				/* gss_export_name */

#ifdef _MIT_KRB5_1_11
	NULL,				/* gss_duplicate_name */
#endif

	NULL,				/* gss_store_cred */
	srp_gss_inquire_sec_context_by_oid, /* gss_inquire_sec_context_by_oid */
	srp_gss_inquire_cred_by_oid,	/* gss_inquire_cred_by_oid */
	srp_gss_set_sec_context_option, /* gss_set_sec_context_option */
	srp_gssspi_set_cred_option,	/* gssspi_set_cred_option */
	NULL,				/* gssspi_mech_invoke */
	srp_gss_wrap_aead,
	srp_gss_unwrap_aead,
	srp_gss_wrap_iov,
	srp_gss_unwrap_iov,
	srp_gss_wrap_iov_length,
	srp_gss_complete_auth_token,
	srp_gss_acquire_cred_impersonate_name,
	NULL,				/* gss_add_cred_impersonate_name */
	srp_gss_display_name_ext,
	srp_gss_inquire_name,
	srp_gss_get_name_attribute,
	srp_gss_set_name_attribute,
	srp_gss_delete_name_attribute,
	srp_gss_export_name_composite,
	srp_gss_map_name_to_any,
	srp_gss_release_any_name_mapping,
#ifdef _MIT_KRB5_1_11
	NULL,				/* gss_pseudo_random */
	NULL,				/* gss_set_neg_mechs */
	NULL,				/* gss_inquire_saslname_for_mech */
	NULL,				/* gss_inquire_mech_for_saslname */
        srp_gss_inquire_attrs_for_mech,
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

static int gss_srpmechglue_init(void)
{
	struct gss_mech_config mech_srp;

	memset(&mech_srp, 0, sizeof(mech_srp));
	mech_srp.mech = &srp_mechanism;
	mech_srp.mechNameStr = "srp";
	mech_srp.mech_type = (const gss_OID_desc * const) gss_mech_srp;

	return gssint_register_mechinfo(&mech_srp);
}
#else
GSS_MECH_PLUGIN_CONFIG gss_mech_initialize(void)
{
	return (&srp_mechanism);
}

#endif /* _GSS_STATIC_LINK */
