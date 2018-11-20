/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

#include "includes.h"


DWORD
VmAfdRestGetCACerts(
    PVMAFD_ROOT_FETCH_ARG       pArgs,
    BOOLEAN                     bDetail,
    BOOLEAN                     bInsecure,
    PVMAFD_CA_CERT_ARRAY        *ppCACerts
    )
{
    DWORD                       dwError = 0;
    PSTR                        pszToken = NULL;
    PSTR                        pszCAPath = NULL;
    PWSTR                       pwszCAPath = NULL;
    PVM_HTTP_CLIENT             pHttpClient = NULL;
    PVMAFD_CA_CERT_ARRAY        pCACerts = NULL;

    if (!pArgs || !ppCACerts)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    /* support insecure flag to bypass lightwave server certs check */
    if (bInsecure)
    {
        pszCAPath = NULL;
    }
    else
    {
        dwError = VmAfSrvGetCAPath(&pwszCAPath);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringAFromW(pwszCAPath, &pszCAPath);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    /* acquire token */
    dwError = VmAfdAcquireOIDCToken(
                  pArgs->pszDCName,
                  pArgs->pszDomain,
                  pArgs->pszUserName,
                  pArgs->pszPassword,
                  pszCAPath,
                  (pArgs->bUseLwCA ? VMAFD_OIDC_LWCA_SCOPE : VMAFD_OIDC_VMDIR_SCOPE),
                  &pszToken);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmHttpClientInit(&pHttpClient, pszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmHttpClientSetToken(pHttpClient, VMHTTP_TOKEN_TYPE_BEARER, pszToken);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pArgs->bUseLwCA)
    {
        VmAfdLog(VMAFD_DEBUG_ANY,
                 "[%s:%d] Use MutentCA Params: CA Server (%s) CA Id (%s)",
                 __FUNCTION__,
                 __LINE__,
                 pArgs->pszLwCAServer,
                 pArgs->pszLwCAId);

        dwError = VmAfdRestGetCACertsLwCA(
                            pArgs->pszLwCAServer,
                            pArgs->pszLwCAId,
                            bDetail,
                            pHttpClient,
                            &pCACerts);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdRestGetCACertsVMCA(
                            pArgs->pszDCName,
                            bDetail,
                            pHttpClient,
                            &pCACerts);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppCACerts = pCACerts;


cleanup:

    VmHttpClientFreeHandle(pHttpClient);
    VMAFD_SAFE_FREE_MEMORY(pszCAPath);
    VMAFD_SAFE_FREE_MEMORY(pwszCAPath);
    VMAFD_SAFE_FREE_STRINGA(pszToken);

    return dwError;

error:

    VecsFreeCACertArray(pCACerts);
    VmAfdLog(VMAFD_DEBUG_ANY, "Error: [%s : %d]", __FUNCTION__, dwError);

    goto cleanup;
}
