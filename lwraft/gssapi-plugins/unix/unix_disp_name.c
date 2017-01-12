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
