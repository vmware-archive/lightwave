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
    X509_REQ                    *pRequest
    )
{
    DWORD dwError = 0;
    PSTR pszCN = NULL;
    BOOLEAN bIsIP = FALSE;
    BOOLEAN bIsFQDN = FALSE;
    BOOLEAN bIsValid = FALSE;

    if (!pSNsAllowed || !pReqContext || !pRequest)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    dwError = LwCAX509ReqGetCommonName(pRequest, &pszCN);
    BAIL_ON_LWCA_ERROR(dwError);

    if(LwCAUtilDoesValueHaveWildcards(pszCN))
    {
        LWCA_LOG_ERROR("Policy Violation: CN %s contains wildcards", pszCN);
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_SN_POLICY_VIOLATION);
    }

    bIsIP = LwCAUtilIsValueIPAddr(pszCN);

    dwError = LwCAUtilIsValueFQDN(pszCN, &bIsFQDN);
    BAIL_ON_LWCA_ERROR(dwError);

    if (bIsIP)
    {
        bIsValid = _LwCAPolicyValidateEntry(pszCN, pReqContext, LWCA_POLICY_CFG_TYPE_IP, pSNsAllowed);
    }
    else if (bIsFQDN)
    {
        bIsValid = _LwCAPolicyValidateEntry(pszCN, pReqContext, LWCA_POLICY_CFG_TYPE_FQDN, pSNsAllowed);
    }
    else
    {
        bIsValid = _LwCAPolicyValidateEntry(pszCN, pReqContext, LWCA_POLICY_CFG_TYPE_NAME, pSNsAllowed);
    }

    if (!bIsValid)
    {
        LWCA_LOG_ERROR("Policy Violation: CN %s does not match allowed CN values", pszCN);
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_SN_POLICY_VIOLATION);
    }

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszCN);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCAPolicyValidateSANPolicy(
    PLWCA_POLICY_CFG_OBJ_ARRAY  pSANsAllowed,
    BOOLEAN                     bMultiSANEnabled,
    PLWCA_REQ_CONTEXT           pReqContext,
    X509_REQ                    *pRequest
    )
{
    DWORD dwError = 0;
    DWORD dwIdx = 0;
    PLWCA_STRING_ARRAY pSANArray = NULL;
    BOOLEAN bIsIP = FALSE;
    BOOLEAN bIsFQDN = FALSE;
    BOOLEAN bIsValid = FALSE;

    if (!pSANsAllowed || !pReqContext || !pRequest)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    dwError = LwCAX509ReqGetSubjectAltNames(pRequest, &pSANArray);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!pSANArray)
    {
        // CSR does not contain any SANs. Passing validation
        goto ret;
    }

    if (!bMultiSANEnabled && pSANArray->dwCount > 1)
    {
        LWCA_LOG_ERROR("Policy Violation: Multiple SANs are not allowed");
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_SAN_POLICY_VIOLATION);
    }

    for ( ; dwIdx < pSANArray->dwCount ; ++dwIdx )
    {
        if(LwCAUtilDoesValueHaveWildcards(pSANArray->ppData[dwIdx]))
        {
            LWCA_LOG_ERROR("Policy Violation: SAN %s contains wildcards", pSANArray->ppData[dwIdx]);
            BAIL_WITH_LWCA_ERROR(dwError, LWCA_SAN_POLICY_VIOLATION);
        }

        bIsIP = LwCAUtilIsValueIPAddr(pSANArray->ppData[dwIdx]);

        dwError = LwCAUtilIsValueFQDN(pSANArray->ppData[dwIdx], &bIsFQDN);
        BAIL_ON_LWCA_ERROR(dwError);

        if (bIsIP)
        {
            bIsValid = _LwCAPolicyValidateEntry(
                        pSANArray->ppData[dwIdx],
                        pReqContext,
                        LWCA_POLICY_CFG_TYPE_IP,
                        pSANsAllowed);
        }
        else if (bIsFQDN)
        {
            bIsValid = _LwCAPolicyValidateEntry(
                        pSANArray->ppData[dwIdx],
                        pReqContext,
                        LWCA_POLICY_CFG_TYPE_FQDN,
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
            BAIL_WITH_LWCA_ERROR(dwError, LWCA_SAN_POLICY_VIOLATION);
        }
    }

ret:
cleanup:
    LwCAFreeStringArray(pSANArray);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCAPolicyValidateKeyUsagePolicy(
    DWORD                       dwAllowedKeys,
    X509_REQ                    *pRequest
    )
{
    DWORD dwError = 0;
    DWORD dwKeyUsage = 0;

    if (!pRequest)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    dwError = LwCAX509ReqGetKeyUsage(pRequest, &dwKeyUsage);
    BAIL_ON_LWCA_ERROR(dwError);

    if ( (dwKeyUsage & dwAllowedKeys) != dwKeyUsage )
    {
        LWCA_LOG_ERROR("Policy Violation. Some key usages in CSR are not allowed.");
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_KEY_USAGE_POLICY_VIOLATION);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCAPolicyValidateCertDurationPolicy(
    DWORD                       dwAllowedDuration,
    PLWCA_CERT_VALIDITY         pValidity
    )
{
    DWORD dwError = 0;
    double duration = 0;
    DWORD dwDuration = 0;

    if (!pValidity)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    duration = difftime(pValidity->tmNotAfter, pValidity->tmNotBefore);
    if (duration < 0)
    {
        LWCA_LOG_ERROR("Policy Violation: Invalid cert duration. tmNotBefore greater than tmNotAfter");
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CERT_DURATION_POLICY_VIOLATION);
    }

    dwDuration = (DWORD) duration / LWCA_TIME_SECS_PER_DAY;

    if (dwDuration > dwAllowedDuration)
    {
        LWCA_LOG_ERROR(
            "Policy Violation: Given cert duration %d is higher than allowed duration %d",
            dwDuration,
            dwAllowedDuration);
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CERT_DURATION_POLICY_VIOLATION);
    }

cleanup:
    return dwError;

error:
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
        // TODO: Do Reverse DNS lookup
        LWCA_LOG_INFO("Inzone policy validation currently not implemented. Passing");
        bIsValid = TRUE;
        break;

    case LWCA_POLICY_CFG_TYPE_FQDN:
        // TODO: Do Forward DNS lookup
        LWCA_LOG_INFO("Inzone policy validation currently not implemented. Passing");
        bIsValid = TRUE;
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
        bIsValid = FALSE;
        BAIL_ON_LWCA_POLICY_VIOLATION(bIsValid);
    }

    if (bIsFQDN)
    {
        bIsValid = _LwCAPolicyValidateWithValue(pcszEntry, pszHostFQDN, pcszPrefix, pcszSuffix);
        goto cleanup;
    }

    pszHostname = LwCAStringTokA(pszHostFQDN, ".", &pszNextTok);
    if (!pszHostname)
    {
        bIsValid = FALSE;
        BAIL_ON_LWCA_POLICY_VIOLATION(bIsValid);
    }

    bIsValid = _LwCAPolicyValidateWithValue(pcszEntry, pszHostname, pcszPrefix, pcszSuffix);
    BAIL_ON_LWCA_POLICY_VIOLATION(bIsValid);

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
                pcszValue,
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
