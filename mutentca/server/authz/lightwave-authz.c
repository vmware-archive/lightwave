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

static
BOOLEAN
LwCAAuthZLWIsUPNInGroup(
    PCSTR                           pcszGroup,
    PLWCA_STRING_ARRAY              pBindUPNGroups
    );


DWORD
LwCAAuthZLWCheckAccess(
    PLWCA_REQ_CONTEXT               pReqCtx,                // IN
    PCSTR                           pcszCAId,               // IN
    LWCA_AUTHZ_X509_DATA            *pX509Data,             // IN
    LWCA_AUTHZ_API_PERMISSION       apiPermissions,         // IN
    PBOOLEAN                        pbAuthorized            // OUT
    )
{
    DWORD                           dwError = 0;
    BOOLEAN                         bAuthorized = FALSE;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) || !pbAuthorized)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    if (apiPermissions & LWCA_AUTHZ_GET_CA_CERT_PERMISSION)
    {
        dwError = LwCAAuthZLWCheckGetCACert(pReqCtx, pcszCAId, pX509Data, &bAuthorized);
        BAIL_ON_LWCA_ERROR(dwError);
        goto ret;
    }

    if (apiPermissions & LWCA_AUTHZ_GET_CA_CRL_PERMISSION)
    {
        dwError = LwCAAuthZLWCheckGetCACRL(pReqCtx, pcszCAId, pX509Data, &bAuthorized);
        BAIL_ON_LWCA_ERROR(dwError);
        goto ret;
    }

    if (apiPermissions & LWCA_AUTHZ_CA_CREATE_PERMISSION)
    {
        dwError = LwCAAuthZLWCheckCACreate(pReqCtx, pcszCAId, pX509Data, &bAuthorized);
        BAIL_ON_LWCA_ERROR(dwError);
        goto ret;
    }

    if (apiPermissions & LWCA_AUTHZ_CA_REVOKE_PERMISSION)
    {
        dwError = LwCAAuthZLWCheckCARevoke(pReqCtx, pcszCAId, pX509Data, &bAuthorized);
        BAIL_ON_LWCA_ERROR(dwError);
        goto ret;
    }

    if (apiPermissions & LWCA_AUTHZ_CERT_SIGN_PERMISSION)
    {
        dwError = LwCAAuthZLWCheckCertSign(pReqCtx, pcszCAId, pX509Data, &bAuthorized);
        BAIL_ON_LWCA_ERROR(dwError);
        goto ret;
    }

    if (apiPermissions & LWCA_AUTHZ_CERT_REVOKE_PERMISSION)
    {
        dwError = LwCAAuthZLWCheckCertRevoke(pReqCtx, pcszCAId, pX509Data, &bAuthorized);
        BAIL_ON_LWCA_ERROR(dwError);
        goto ret;
    }


ret:

    *pbAuthorized = bAuthorized;

cleanup:

    return dwError;

error:

    if (pbAuthorized)
    {
        *pbAuthorized = FALSE;
    }

    goto cleanup;
}

DWORD
LwCAAuthZLWCheckGetCACert(
    PLWCA_REQ_CONTEXT           pReqCtx,                // IN
    PCSTR                       pcszCAId,               // IN
    LWCA_AUTHZ_X509_DATA        *pX509Data,             // IN OPTIONAL: Only needed if api AuthZ check requires X509 data
    PBOOLEAN                    pbAuthorized            // OUT
    )
{
    DWORD                       dwError = 0;

    if (!pbAuthorized)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    *pbAuthorized = TRUE;

cleanup:

    return dwError;

error:

    if (pbAuthorized)
    {
        *pbAuthorized = FALSE;
    }
    goto cleanup;
}

DWORD
LwCAAuthZLWCheckGetCACRL(
    PLWCA_REQ_CONTEXT           pReqCtx,                // IN
    PCSTR                       pcszCAId,               // IN
    LWCA_AUTHZ_X509_DATA        *pX509Data,             // IN OPTIONAL: Only needed if api AuthZ check requires X509 data
    PBOOLEAN                    pbAuthorized            // OUT
    )
{
    DWORD                       dwError = 0;

    if (!pbAuthorized)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    *pbAuthorized = TRUE;

cleanup:

    return dwError;

error:

    if (pbAuthorized)
    {
        *pbAuthorized = FALSE;
    }
    goto cleanup;
}

DWORD
LwCAAuthZLWCheckCACreate(
    PLWCA_REQ_CONTEXT           pReqCtx,                // IN
    PCSTR                       pcszCAId,               // IN
    LWCA_AUTHZ_X509_DATA        *pX509Data,             // IN OPTIONAL: Only needed if api AuthZ check requires X509 data
    PBOOLEAN                    pbAuthorized            // OUT
    )
{
    DWORD                   dwError = 0;
    BOOLEAN                 bIsCAAdmin = FALSE;
    BOOLEAN                 bIsCAOperator = FALSE;
    BOOLEAN                 bAuthorized = FALSE;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) || !pbAuthorized)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    bIsCAAdmin = LwCAAuthZLWIsUPNInGroup(LWCA_CAADMINS_GROUP, pReqCtx->pBindUPNGroups);
    bIsCAOperator = LwCAAuthZLWIsUPNInGroup(LWCA_CAOPERATORS_GROUP, pReqCtx->pBindUPNGroups);

    bAuthorized = (bIsCAAdmin || bIsCAOperator) ? TRUE : FALSE;

    if (bAuthorized)
    {
        LWCA_LOG_INFO("Authorized (%s) to call CA Create API", pReqCtx->pszBindUPN);
    }
    else
    {
        LWCA_LOG_ALERT("(%s) is unauthorized to call CA Create API", pReqCtx->pszBindUPN);
    }

    *pbAuthorized = bAuthorized;


cleanup:

    return dwError;

error:

    if (pbAuthorized)
    {
        *pbAuthorized = bAuthorized;
    }

    goto cleanup;
}

DWORD
LwCAAuthZLWCheckCARevoke(
    PLWCA_REQ_CONTEXT           pReqCtx,                // IN
    PCSTR                       pcszCAId,               // IN
    LWCA_AUTHZ_X509_DATA        *pX509Data,             // IN OPTIONAL: Only needed if api AuthZ check requires X509 data
    PBOOLEAN                    pbAuthorized            // OUT
    )
{
    DWORD                   dwError = 0;
    BOOLEAN                 bIsCAAdmin = FALSE;
    BOOLEAN                 bAuthorized = FALSE;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) || !pbAuthorized)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    bIsCAAdmin = LwCAAuthZLWIsUPNInGroup(LWCA_CAADMINS_GROUP, pReqCtx->pBindUPNGroups);

    bAuthorized = bIsCAAdmin;

    if (bAuthorized)
    {
        LWCA_LOG_INFO("Authorized (%s) to call CA Revoke API", pReqCtx->pszBindUPN);
    }
    else
    {
        LWCA_LOG_ALERT("(%s) is unauthorized to call CA Revoke API", pReqCtx->pszBindUPN);
    }

    *pbAuthorized = bAuthorized;


cleanup:

    return dwError;

error:

    if (pbAuthorized)
    {
        *pbAuthorized = bAuthorized;
    }

    goto cleanup;
}

DWORD
LwCAAuthZLWCheckCertSign(
    PLWCA_REQ_CONTEXT           pReqCtx,                // IN
    PCSTR                       pcszCAId,               // IN
    LWCA_AUTHZ_X509_DATA        *pX509Data,             // IN OPTIONAL: Only needed if api AuthZ check requires X509 data
    PBOOLEAN                    pbAuthorized            // OUT
    )
{
    DWORD                   dwError = 0;
    BOOLEAN                 bAuthorized = FALSE;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) || !pbAuthorized)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    if (LwCAStringCompareA(pReqCtx->pszBindUPNTenant, pcszCAId, FALSE) == 0)
    {
        bAuthorized = TRUE;
    }

    if (bAuthorized)
    {
        LWCA_LOG_INFO("Authorized (%s) to GetSignedCert from CA (%s)", pReqCtx->pszBindUPN, pcszCAId);
    }
    else
    {
        LWCA_LOG_ALERT("(%s) is unauthorized to GetSignedCert from CA (%s)", pReqCtx->pszBindUPN, pcszCAId);
    }

    *pbAuthorized = bAuthorized;


cleanup:

    return dwError;

error:

    if (pbAuthorized)
    {
        *pbAuthorized = FALSE;
    }

    goto cleanup;
}

DWORD
LwCAAuthZLWCheckCertRevoke(
    PLWCA_REQ_CONTEXT           pReqCtx,                // IN
    PCSTR                       pcszCAId,               // IN
    LWCA_AUTHZ_X509_DATA        *pX509Data,             // IN OPTIONAL: Only needed if api AuthZ check requires X509 data
    PBOOLEAN                    pbAuthorized            // OUT
    )
{
    DWORD                   dwError = 0;
    BOOLEAN                 bIsCAAdmin = FALSE;
    BOOLEAN                 bAuthorized = FALSE;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) || !pbAuthorized)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    bIsCAAdmin = LwCAAuthZLWIsUPNInGroup(LWCA_CAADMINS_GROUP, pReqCtx->pBindUPNGroups);

    bAuthorized = bIsCAAdmin;

    if (bAuthorized)
    {
        LWCA_LOG_INFO("Authorized (%s) to revoke a certificate", pReqCtx->pszBindUPN);
    }
    else
    {
        LWCA_LOG_ALERT("(%s) is unauthorized to revoke a certificate", pReqCtx->pszBindUPN);
    }

    *pbAuthorized = bAuthorized;


cleanup:

    return dwError;

error:

    if (pbAuthorized)
    {
        *pbAuthorized = FALSE;
    }

    goto cleanup;
}


static
BOOLEAN
LwCAAuthZLWIsUPNInGroup(
    PCSTR                   pcszGroup,
    PLWCA_STRING_ARRAY      pBindUPNGroups
    )
{
    DWORD                   dwIdx = 0;

    for (; dwIdx < pBindUPNGroups->dwCount ; ++dwIdx)
    {
        if (LwCAStringCompareA(pcszGroup, pBindUPNGroups->ppData[dwIdx], FALSE) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}
