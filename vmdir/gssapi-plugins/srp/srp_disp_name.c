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



#include "srp_util.h"
#include <gssapi/gssapi_krb5.h>

OM_uint32
srp_gss_display_name(
    OM_uint32 *minor_status,
    gss_name_t input_name,
    gss_buffer_t output_name_buffer,
    gss_OID *output_name_type)
{
    OM_uint32 status = GSS_S_COMPLETE;
    OM_uint32 minor = GSS_S_COMPLETE;
    gss_name_t gss_krb5_name_buf = NULL;
    dsyslog("Entering display_name\n");


    status = gss_canonicalize_name(&minor,
                                  input_name,
                                  (gss_OID) gss_mech_krb5,
                                  &gss_krb5_name_buf);
    if (status)
    {
        goto error;
    }

    status = gss_display_name(minor_status, gss_krb5_name_buf,
        output_name_buffer, output_name_type);

error:
    if (gss_krb5_name_buf)
    {
        gss_release_name(minor_status, &gss_krb5_name_buf);
    }

    dsyslog("Leaving display_name\n");
    return (status);
}
