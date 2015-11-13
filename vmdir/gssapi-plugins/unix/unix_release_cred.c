/*
 * Copyright (C) 2014, VMware Inc. All rights reserved.
 *
 * Module: srp_release_cred.c
 * Abstract:
 *     VMware GSSAPI SRP Authentication Plugin
 *     Implements SRP release cred
 *
 * Author: Adam Bernstein (abernstein@vmware.com)
 */

#include "unix_util.h"
#include "gssapi_alloc.h"

OM_uint32
srp_gss_release_cred(OM_uint32 *minor_status,
            gss_cred_id_t *cred_handle)
{
    OM_uint32 status = 0;
    OM_uint32 min = 0;
    srp_gss_cred_id_t srp_cred = NULL;

    dsyslog("Entering srp_gss_release_cred\n");

    if (minor_status == NULL || cred_handle == NULL)
    {
        return (GSS_S_CALL_INACCESSIBLE_WRITE);
    }

    *minor_status = 0;

    if (*cred_handle == GSS_C_NO_CREDENTIAL)
    {
        return (GSS_S_COMPLETE);
    }

    srp_cred = (srp_gss_cred_id_t) *cred_handle;
    if (srp_cred->srp_mech_oid)
    {
        if (srp_cred->srp_mech_oid->elements)
        {
            gssalloc_free(srp_cred->srp_mech_oid->elements);
        }
        gssalloc_free(srp_cred->srp_mech_oid);
    }
    if (srp_cred->name)
    {
        gss_release_name(&min, &srp_cred->name);
    }
    if (srp_cred->password)
    {
        gss_release_buffer(&min, srp_cred->password);
        gssalloc_free(srp_cred->password);
    }

    gssalloc_free(srp_cred);

    *cred_handle = NULL;

    dsyslog("Leaving srp_gss_release_cred\n");
    return (status);
}
