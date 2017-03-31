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



#include "ntlm_util.h"
#include <gssapi/gssapi_krb5.h>
#include <errno.h>
#include <string.h>

OM_uint32
ntlm_gss_acquire_cred(
    OM_uint32 *minor_status,
    gss_name_t desired_name,
    OM_uint32 time_req,
    gss_OID_set desired_mechs,
    gss_cred_usage_t cred_usage,
    gss_cred_id_t *output_cred_handle,
    gss_OID_set *actual_mechs,
    OM_uint32 *time_rec)
{
    OM_uint32 major = 0;
    OM_uint32 minor = 0;
    ntlm_gss_cred_id_t ntlm_cred = NULL;
    gss_name_t gss_krb5_name_buf = NULL;

    /* Allocate the cred structure */
    ntlm_cred = (ntlm_gss_cred_id_t) xmalloc(sizeof(*ntlm_cred));
    if (!ntlm_cred)
    {
        minor = ENOMEM;
        major = GSS_S_FAILURE;
        goto error;
    }
    memset(ntlm_cred, 0, sizeof(*ntlm_cred));

    /* Allocate/set the mech OID; must be NTLM for this method to be called */
    ntlm_cred->ntlm_mech_oid = (gss_OID) xmalloc(sizeof(*ntlm_cred->ntlm_mech_oid));
    if (!ntlm_cred)
    {
        minor = ENOMEM;
        major = GSS_S_FAILURE;
        goto error;
    }
    memset(ntlm_cred->ntlm_mech_oid, 0, sizeof(*ntlm_cred->ntlm_mech_oid));
    ntlm_cred->ntlm_mech_oid->elements = (void *) xmalloc(GSS_NTLM_MECH_OID_LEN);
    if (!ntlm_cred)
    {
        minor = ENOMEM;
        major = GSS_S_FAILURE;
        goto error;
    }
    ntlm_cred->ntlm_mech_oid->length = GSS_NTLM_MECH_OID_LEN;
    memcpy(ntlm_cred->ntlm_mech_oid->elements, NTLM_OID, GSS_NTLM_MECH_OID_LEN);

    if (desired_name)
    {
        /* Really, use krb5 mech OID for name, as the desired output is a UPN */
        major = gss_canonicalize_name(&minor,
                                      desired_name,
                                      (gss_OID) gss_mech_krb5,
                                      &gss_krb5_name_buf);
        if (major)
        {
            goto error;
        }

        ntlm_cred->name = gss_krb5_name_buf, gss_krb5_name_buf = NULL;
    }
    *output_cred_handle = (gss_cred_id_t) ntlm_cred;

error:
    if (major || minor)
    {
        *minor_status = minor;
        if (ntlm_cred->ntlm_mech_oid->elements)
        {
            free(ntlm_cred->ntlm_mech_oid->elements);
        }
        if (ntlm_cred->ntlm_mech_oid)
        {
            free(ntlm_cred->ntlm_mech_oid);
        }
        if (ntlm_cred)
        {
            free(ntlm_cred);
        }
    }

    return major;
}
