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
_LwCAPolicyValidateEntry(
    PCSTR                       pcszEntry,
    PLWCA_REQ_CONTEXT           pReqContext,
    LWCA_POLICY_CFG_TYPE        type,
    PLWCA_POLICY_CFG_OBJ_ARRAY  pAllowedObjects
    );

static
BOOLEAN
_LwCAPolicyValidateInzone(
    PCSTR                       pcszEntry,
    LWCA_POLICY_CFG_TYPE        type
    );

static
BOOLEAN
_LwCAPolicyValidateWithReqUPN(
    PCSTR                       pcszEntry,
    PCSTR                       pcszReqUPN,
    PCSTR                       pcszPrefix,
    PCSTR                       pcszSuffix,
    BOOLEAN                     bIsFQDN
    );

static
BOOLEAN
_LwCAPolicyValidateWithValue(
    PCSTR                       pcszEntry,
    PCSTR                       pcszValue,
    PCSTR                       pcszPrefix,
    PCSTR                       pcszSuffix
    );

DWORD
LwCAPolicyValidateSNPolicy(
    PLWCA_POLICY_CFG_OBJ_ARRAY  pSNsAllowed,
    PLWCA_REQ_CONTEXT           pReqContext,
    X509_REQ                    *pRequest,
    BOOLEAN                     *pbIsValid
    )
{
    DWORD dwError = 0;
    PSTR pszCN = NULL;
    BOOLEAN bIsFQDN = FALSE;
    BOOLEAN bIsValid = FALSE;

    if (!pSNsAllowed || !pReqContext || !pRequest || !pbIsValid)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAX509ReqGetCommonName(pRequest, &pszCN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAUtilIsValueFQDN(pszCN, &bIsFQDN);
    BAIL_ON_LWCA_ERROR(dwError);

    if (bIsFQDN)
    {
        bIsValid = _LwCAPolicyValidateEntry(pszCN, pReqContext, LWCA_POLICY_CFG_TYPE_FQDN, pSNsAllowed);
    }
    else if (LwCAUtilIsValueIPAddr(pszCN))
    {
        bIsValid = _LwCAPolicyValidateEntry(pszCN, pReqContext, LWCA_POLICY_CFG_TYPE_IP, pSNsAllowed);
    }
    else
    {
        bIsValid = _LwCAPolicyValidateEntry(pszCN, pReqContext, LWCA_POLICY_CFG_TYPE_NAME, pSNsAllowed);
    }

    if (!bIsValid)
    {
        LWCA_LOG_ERROR("Policy Violation: CN %s does not match allowed CN values", pszCN);
    }

    *pbIsValid = bIsValid;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszCN);
    return dwError;

error:
    if (pbIsValid)
    {
        *pbIsValid = FALSE;
    }

    goto cleanup;
}

DWORD
LwCAPolicyValidateSANPolicy(
    PLWCA_POLICY_CFG_OBJ_ARRAY  pSANsAllowed,
    BOOLEAN                     bMultiSANEnabled,
    PLWCA_REQ_CONTEXT           pReqContext,
    X509_REQ                    *pRequest,
    BOOLEAN                     *pbIsValid
    )
{
    DWORD dwError = 0;
    DWORD dwIdx = 0;
    PLWCA_STRING_ARRAY pSANArray = NULL;
    BOOLEAN bIsFQDN = FALSE;
    BOOLEAN bIsValid = FALSE;

    if (!pSANsAllowed || !pReqContext || !pRequest || !pbIsValid)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAX509ReqGetSubjectAltNames(pRequest, &pSANArray);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bMultiSANEnabled && pSANArray->dwCount > 1)
    {
        LWCA_LOG_ERROR("Policy Violation: Multiple SANs are not allowed");
        goto error;
    }

    for ( ; dwIdx < pSANArray->dwCount ; ++dwIdx )
    {
        dwError = LwCAUtilIsValueFQDN(pSANArray->ppData[dwIdx], &bIsFQDN);
        BAIL_ON_LWCA_ERROR(dwError);

        if (bIsFQDN)
        {
            bIsValid = _LwCAPolicyValidateEntry(
                        pSANArray->ppData[dwIdx],
                        pReqContext,
                        LWCA_POLICY_CFG_TYPE_FQDN,
                        pSANsAllowed);
        }
        else if (LwCAUtilIsValueIPAddr(pSANArray->ppData[dwIdx]))
        {
            bIsValid = _LwCAPolicyValidateEntry(
                        pSANArray->ppData[dwIdx],
                        pReqContext,
                        LWCA_POLICY_CFG_TYPE_IP,
                        pSANsAllowed);
        }
        else
        {
            bIsValid = _LwCAPolicyValidateEntry(
                        pSANArray->ppData[dwIdx],
                        pReqContext,
                        LWCA_POLICY_CFG_TYPE_NAME,
                        pSANsAllowed);
        }

        if (!bIsValid)
        {
            LWCA_LOG_ERROR(
                "Policy Violation: SAN %s does not match allowed SAN values",
                pSANArray->ppData[dwIdx]);
            break;
        }
    }

    *pbIsValid = bIsValid;

cleanup:
    LwCAFreeStringArray(pSANArray);
    return dwError;

error:
    if (pbIsValid)
    {
        *pbIsValid = FALSE;
    }

    goto cleanup;
}

static
BOOLEAN
_LwCAPolicyValidateEntry(
    PCSTR                       pcszEntry,
    PLWCA_REQ_CONTEXT           pReqContext,
    LWCA_POLICY_CFG_TYPE        type,
    PLWCA_POLICY_CFG_OBJ_ARRAY  pAllowedObjects
    )
{
    DWORD dwIdx = 0;
    BOOLEAN bIsValid = FALSE;

    for ( ; dwIdx < pAllowedObjects->dwCount ; ++dwIdx )
    {
        if (pAllowedObjects->ppObj[dwIdx]->type != type)
        {
            continue;
        }

        switch (pAllowedObjects->ppObj[dwIdx]->match)
        {
        case LWCA_POLICY_CFG_MATCH_CONSTANT:
            if (LwCAStringCompareA(pcszEntry, pAllowedObjects->ppObj[dwIdx]->pszValue, TRUE) == 0)
            {
                return TRUE;
            }
            break;

        case LWCA_POLICY_CFG_MATCH_ANY:
            return TRUE;
            break;

        case LWCA_POLICY_CFG_MATCH_REGEX:
            LwCARegexValidate(pcszEntry, pAllowedObjects->ppObj[dwIdx]->pRegex, &bIsValid);
            if (bIsValid)
            {
                return TRUE;
            }
            break;

        case LWCA_POLICY_CFG_MATCH_PRIVATE:
            if ( LwCAUtilIsValuePrivateOrLocalIPAddr(pcszEntry) )
            {
                return TRUE;
            }
            break;

        case LWCA_POLICY_CFG_MATCH_PUBLIC:
            if ( !LwCAUtilIsValuePrivateOrLocalIPAddr(pcszEntry) )
            {
                return TRUE;
            }
            break;

        case LWCA_POLICY_CFG_MATCH_INZONE:
            if ( _LwCAPolicyValidateInzone(pcszEntry, type) )
            {
                return TRUE;
            }
            break;

        case LWCA_POLICY_CFG_MATCH_REQ_HOSTNAME:
            if ( _LwCAPolicyValidateWithReqUPN(
                    pcszEntry,
                    pReqContext->pszBindUPN,
                    pAllowedObjects->ppObj[dwIdx]->pszPrefix,
                    pAllowedObjects->ppObj[dwIdx]->pszSuffix,
                    FALSE) )
            {
                return TRUE;
            }
            break;

        case LWCA_POLICY_CFG_MATCH_REQ_FQDN:
            if ( _LwCAPolicyValidateWithReqUPN(
                    pcszEntry,
                    pReqContext->pszBindUPN,
                    pAllowedObjects->ppObj[dwIdx]->pszPrefix,
                    pAllowedObjects->ppObj[dwIdx]->pszSuffix,
                    TRUE) )
            {
                return TRUE;
            }
            break;

        default:
            break;
        }
    }

    return FALSE;
}

static
BOOLEAN
_LwCAPolicyValidateInzone(
    PCSTR                       pcszEntry,
    LWCA_POLICY_CFG_TYPE        type
    )
{
    BOOLEAN bIsValid = FALSE;

    switch (type)
    {
    case LWCA_POLICY_CFG_TYPE_IP:
        // TODO: Do Reverse DNS lookup (in subsequent commits)
        break;

    case LWCA_POLICY_CFG_TYPE_FQDN:
        // TODO: Do Forward DNS lookup (in subsequent commits)
        break;

    default:
        bIsValid = FALSE;
        break;
    }

    return bIsValid;
}

static
BOOLEAN
_LwCAPolicyValidateWithReqUPN(
    PCSTR                       pcszEntry,
    PCSTR                       pcszReqUPN,
    PCSTR                       pcszPrefix,
    PCSTR                       pcszSuffix,
    BOOLEAN                     bIsFQDN
    )
{
    DWORD dwError = 0;
    PSTR pszTemp = NULL;
    PSTR pszHostFQDN = NULL;
    PSTR pszHostname = NULL;
    PSTR pszNextTok = NULL;
    BOOLEAN bIsValid = FALSE;

    dwError = LwCAAllocateStringA(pcszReqUPN, &pszTemp);
    BAIL_ON_LWCA_ERROR(dwError);

    pszHostFQDN = LwCAStringTokA(pszTemp, "@", &pszNextTok);
    if (!pszHostFQDN)
    {
        goto error;
    }

    if (bIsFQDN)
    {
        bIsValid = _LwCAPolicyValidateWithValue(pcszEntry, pszHostFQDN, pcszPrefix, pcszSuffix);
        goto cleanup;
    }

    pszHostname = LwCAStringTokA(pszHostFQDN, ".", &pszNextTok);
    if (!pszHostname)
    {
        goto error;
    }

    bIsValid = _LwCAPolicyValidateWithValue(pcszEntry, pszHostFQDN, pcszPrefix, pcszSuffix);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszTemp);
    return bIsValid;

error:
    bIsValid = FALSE;
    goto cleanup;
}

static
BOOLEAN
_LwCAPolicyValidateWithValue(
    PCSTR                       pcszEntry,
    PCSTR                       pcszValue,
    PCSTR                       pcszPrefix,
    PCSTR                       pcszSuffix
    )
{
    DWORD dwError = 0;
    PSTR pszExpectedEntry = NULL;
    BOOLEAN bIsValid = FALSE;

    dwError = LwCAAllocateStringPrintfA(
                &pszExpectedEntry,
                "%s%s%s",
                LWCA_SAFE_STRING(pcszPrefix),
                pcszEntry,
                LWCA_SAFE_STRING(pcszSuffix));
    BAIL_ON_LWCA_ERROR(dwError);

    if (LwCAStringCompareA(pcszEntry, pszExpectedEntry, FALSE) == 0)
    {
        bIsValid = TRUE;
    }

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszExpectedEntry);
    return bIsValid;

error:
    bIsValid = FALSE;
    goto cleanup;
}
