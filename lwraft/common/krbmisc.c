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
 * Create a separate KRB file for MIT API/LIB.  So it is not conflict with krbutil.c/heimdal usage.
 */

#include "includes.h"
#include <krb5/krb5.h>

DWORD
VmDirDestroyDefaultKRB5CC(
    VOID
    )
{
    krb5_error_code             dwError = 0;
    krb5_context                pKrb5Ctx = NULL;
    krb5_ccache                 pDefCredCache = NULL;

    dwError = krb5_init_context(&pKrb5Ctx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = krb5_cc_default(pKrb5Ctx, &pDefCredCache);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = krb5_cc_destroy(pKrb5Ctx, pDefCredCache);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "KRB5 default cred cache destroyed");

cleanup:
    if (pKrb5Ctx)
    {
        krb5_free_context(pKrb5Ctx);
    }

    return dwError;

error:
    if (dwError != KRB5_FCC_NOFILE)
    {
        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "KRB5 default cred cache destroy failed (%d)", dwError);
    }

    goto cleanup;
}
