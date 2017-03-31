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
#include "gssapi/gssapi_krb5.h"
#include <krb5.h>
#include <csrp/srp.h>
#include <stdlib.h>

OM_uint32
ntlm_gss_delete_sec_context(
                OM_uint32 *minor_status,
                gss_ctx_id_t *context_handle,
                gss_buffer_t output_token)
{
    ntlm_gss_ctx_id_t ntlm_ctx = NULL;
    OM_uint32 ret = GSS_S_COMPLETE;
      
    if (context_handle == NULL)
    {
        return (GSS_S_FAILURE);
    }

    ntlm_ctx = (ntlm_gss_ctx_id_t) *context_handle;
    if (ntlm_ctx->upn_name)
    {
        free(ntlm_ctx->upn_name);
    }

    if (ntlm_ctx->ntlm_session_key)
    {
        free(ntlm_ctx->ntlm_session_key);
    }

    if (ntlm_ctx->ntlm_usr)
    {
        srp_user_delete(ntlm_ctx->ntlm_usr);
        ntlm_ctx->ntlm_usr = NULL;
    }
    
    if (ntlm_ctx->ntlm_ver)
    {
        srp_verifier_delete(ntlm_ctx->ntlm_ver);
        ntlm_ctx->ntlm_ver = NULL;
    }

    if (ntlm_ctx->mech)
    {
        OM_uint32 min_tmp = GSS_S_COMPLETE;
        gss_release_oid(&min_tmp, &ntlm_ctx->mech);
    }

    krb5_free_keyblock(ntlm_ctx->krb5_ctx, ntlm_ctx->keyblock);
    ntlm_ctx->keyblock = NULL;

    krb5_free_context(ntlm_ctx->krb5_ctx);
    ntlm_ctx->krb5_ctx = NULL;

    free(*context_handle);
    *context_handle = NULL;
    return (ret);
}
