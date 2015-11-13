/*
 * Copyright (C) 2014, VMware Inc. All rights reserved.
 *
 * Module: srp_disp_name.c
 * Abstract:
 *     VMware GSSAPI SRP Authentication Plugin
 *     Implements SRP display name; KRB5 canonicalized name format
 *
 * Author: Adam Bernstein (abernstein@vmware.com)
 */

#include "unix_util.h"
#include <gssapi/gssapi_krb5.h>

OM_uint32
srp_gss_display_name(
    OM_uint32 *minor_status,
    gss_name_t input_name,
    gss_buffer_t output_name_buffer,
    gss_OID *output_name_type)
{
    OM_uint32 status = GSS_S_COMPLETE;
    dsyslog("Entering display_name\n");

    status = gss_display_name(minor_status, input_name,
        output_name_buffer, NULL);

    dsyslog("Leaving display_name\n");
    return (status);
}
