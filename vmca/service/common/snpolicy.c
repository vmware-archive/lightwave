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

/*
 * Example SNPolicy config:
 *
 * {
 *     "SNValidate": {
 *         "match": {
 *             "data": "req.upn.dn",
 *             "condition": "begins",
 *             "with": "ou=Test,ou=Computers,dc=example,dc=com"
 *         },
 *         "validate": [
 *             {
 *                 "data": "req.csr.subj.o",
 *                 "with": "req.upn.dn.rdn"
 *             }
 *         ]
 *     }
 * }
 *
 * This config will ensure that the CSR requestor's DN is comprised of the
 * organizationName entries in the subjectName field of the CSR.
 * If the requestor's DN does not begin with the match->with rule, then the
 * policy will allow the request to be fulfilled.
 */


static
DWORD
VMCAPolicySNMatchOperationParse(
    json_t                          *pJsonRule,
    PVMCA_SNPOLICY_OPERATION        *ppOperation
    );

static
DWORD
VMCAPolicySNValidateOperationParse(
    json_t                          *pJsonRule,
    PDWORD                          pdwOperationsLen,
    PVMCA_SNPOLICY_OPERATION        **pppOperations
    );

static
VOID
VMCAPolicySNOperationFree(
    PVMCA_SNPOLICY_OPERATION        pOperation
    );

static
VOID
VMCAPolicySNOperationArrayFree(
    PVMCA_SNPOLICY_OPERATION        *ppOperations,
    DWORD                           dwArrayLen
    );

static
DWORD
VMCAPolicySNGetOrgNamesFromCSR(
    PSTR                            pszPKCS10Request,
    PDWORD                          pdwOrgNamesLen,
    PSTR                            **pppszOrgNames
    );

static
DWORD
VMCAPolicySNMatchAuthOUWithCSR(
    PCSTR                           pcszAuthDN,
    PCSTR                           pcszAuthBaseDN,
    DWORD                           dwOrgNamesLen,
    PSTR* const                     ppcszOrgNames
    );

static
int
VMCAPolicySNStrCmpWrapper(
    const void *a,
    const void *b
    );


DWORD
VMCAPolicySNLoad(
    json_t                          *pJsonRules,
    PVMCA_POLICY                    pPolicy
    )
{
    DWORD                           dwError = 0;
    json_t                          *pJsonRulesMatch = NULL;
    json_t                          *pJsonRulesValidate = NULL;
    VMCA_POLICY_RULES               rules = { 0 };

    if (!pJsonRules || !pPolicy)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!(pJsonRulesMatch = json_object_get(pJsonRules, "match")))
    {
        VMCA_LOG_ERROR(
            "[%s,%d] Failed to get SN policy match rule from config (%s)",
            __FUNCTION__,
            __LINE__,
            VMCA_POLICY_FILE_PATH);
        dwError = VMCA_JSON_PARSE_ERROR;
        BAIL_ON_JSON_PARSE_ERROR(dwError);
    }

    if (!(pJsonRulesValidate  = json_object_get(pJsonRules, "validate")))
    {
        VMCA_LOG_ERROR(
            "[%s,%d] SN policy must have either validate rule or action rule in config (%s)",
            __FUNCTION__,
            __LINE__,
            VMCA_POLICY_FILE_PATH);
        dwError = VMCA_JSON_PARSE_ERROR;
        BAIL_ON_JSON_PARSE_ERROR(dwError);
    }

    dwError = VMCAPolicySNMatchOperationParse(
                            pJsonRulesMatch,
                            &rules.SN.pMatch);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAPolicySNValidateOperationParse(
                            pJsonRulesValidate,
                            &rules.SN.dwValidateLen,
                            &rules.SN.ppValidate);
    BAIL_ON_VMCA_ERROR(dwError);

    rules.SN.bEnabled = TRUE;


    pPolicy->Rules = rules;


cleanup:

    return dwError;

error:

    VMCAPolicySNFree(&rules);
    if (pPolicy)
    {
        pPolicy->Rules.SN.bEnabled = FALSE;
        VMCAPolicySNOperationFree(pPolicy->Rules.SN.pMatch);
        VMCAPolicySNOperationArrayFree(pPolicy->Rules.SN.ppValidate, pPolicy->Rules.SN.dwValidateLen);
        pPolicy->Rules.SN.dwValidateLen = 0;
    }

    goto cleanup;
}

DWORD
VMCAPolicySNValidate(
    PVMCA_POLICY                    pPolicy,
    PSTR                            pszPKCS10Request,
    PVMCA_REQ_CONTEXT               pReqContext,
    PBOOLEAN                        pbIsValid
    )
{
    DWORD                           dwError = 0;
    DWORD                           dwOrgNamesLen = 0;
    DWORD                           dwIdx = 0;
    int                             nCount = 0;
    PSTR                            *ppszOrgNames = NULL;
    PSTR                            pszAuthBaseDN = NULL;
    PSTR                            pszAuthDN = NULL;
    PVMCA_LDAP_CONTEXT              pLd = NULL;
    BOOLEAN                         bIsValid = FALSE;

    if (!pPolicy ||
        IsNullOrEmptyString(pszPKCS10Request) ||
        !pReqContext ||
        !pbIsValid)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAOpenLocalLdapServer(&pLd);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAConvertUPNToDN(
                        pLd,
                        pReqContext->pszAuthPrincipal,
                        &pszAuthDN);
    BAIL_ON_VMCA_ERROR(dwError);

    if (VMCAStringCompareA(pPolicy->Rules.SN.pMatch->pszData, VMCA_POLICY_REQ_UPN_DN, TRUE) == 0 &&
        VMCAStringCompareA(pPolicy->Rules.SN.pMatch->pszCondition, VMCA_POLICY_COND_BEGINS, TRUE) == 0)
    {
        dwError = VMCAStringCountSubstring(
                            pszAuthDN,
                            pPolicy->Rules.SN.pMatch->pszWith,
                            &nCount);
        BAIL_ON_VMCA_ERROR(dwError);

        if (nCount == 0)
        {
            VMCA_LOG_INFO(
                    "[%s,%d] CSR requestor (%s) is not a member of policy baseDN...bypassing enforcement",
                    __FUNCTION__,
                    __LINE__,
                    pszAuthDN);

            bIsValid = TRUE;
            goto ret;
        }
    }
    else
    {
        VMCA_LOG_INFO(
                "[%s,%d] Unknown SN policy match rules",
                __FUNCTION__,
                __LINE__);
        dwError = VMCA_POLICY_CONFIG_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    for (dwIdx = 0; dwIdx < pPolicy->Rules.SN.dwValidateLen; ++dwIdx)
    {
        if (!VMCAStringCompareA(
                    pPolicy->Rules.SN.ppValidate[dwIdx]->pszData,
                    VMCA_POLICY_REQ_CSR_SUBJ_ORGS,
                    TRUE))
        {
            dwError = VMCAPolicySNGetOrgNamesFromCSR(
                                    pszPKCS10Request,
                                    &dwOrgNamesLen,
                                    &ppszOrgNames);
            BAIL_ON_VMCA_ERROR(dwError);

            if (!VMCAStringCompareA(
                        pPolicy->Rules.SN.ppValidate[dwIdx]->pszWith,
                        VMCA_POLICY_REQ_UPN_RDN,
                        TRUE))
            {
                dwError = VMCAPolicySNMatchAuthOUWithCSR(
                                        pszAuthDN,
                                        pPolicy->Rules.SN.pMatch->pszWith,
                                        dwOrgNamesLen,
                                        ppszOrgNames);
                BAIL_ON_VMCA_ERROR(dwError);
            }
            else
            {
                VMCA_LOG_INFO(
                    "[%s,%d] Unknown SN policy validate rules",
                    __FUNCTION__,
                    __LINE__);
                dwError = VMCA_POLICY_CONFIG_ERROR;
                BAIL_ON_VMCA_ERROR(dwError);
            }

            VMCAFreeStringArrayA(ppszOrgNames, dwOrgNamesLen);
            ppszOrgNames = NULL;
            dwOrgNamesLen = 0;
        }
        else
        {
            VMCA_LOG_INFO(
                "[%s,%d] Unknown SN policy validate rules",
                __FUNCTION__,
                __LINE__);
            dwError = VMCA_POLICY_CONFIG_ERROR;
            BAIL_ON_VMCA_ERROR(dwError);
        }
    }

    bIsValid = TRUE;


ret:

    *pbIsValid = bIsValid;

cleanup:

    VMCA_SAFE_FREE_STRINGA(pszAuthDN);
    VMCA_SAFE_FREE_STRINGA(pszAuthBaseDN);
    VMCAFreeStringArrayA(ppszOrgNames, dwOrgNamesLen);
    if (pLd)
    {
        VMCALdapClose(pLd);
        pLd = NULL;
    }

    return dwError;

error:

    if (pbIsValid)
    {
        *pbIsValid = FALSE;
    }

    goto cleanup;
}

VOID
VMCAPolicySNFree(
    PVMCA_POLICY_RULES      pPolicyRules
    )
{
    if (pPolicyRules)
    {
        pPolicyRules->SN.bEnabled = FALSE;
        VMCAPolicySNOperationFree(pPolicyRules->SN.pMatch);
        VMCAPolicySNOperationArrayFree(pPolicyRules->SN.ppValidate, pPolicyRules->SN.dwValidateLen);
        pPolicyRules->SN.dwValidateLen = 0;

        pPolicyRules = NULL;
    }
}


static
DWORD
VMCAPolicySNMatchOperationParse(
    json_t                          *pJsonRule,
    PVMCA_SNPOLICY_OPERATION        *ppOperation
    )
{
    DWORD                           dwError = 0;
    PCSTR                           pcszDataTemp = NULL;
    PCSTR                           pcszConditionTemp = NULL;
    PCSTR                           pcszWithTemp = NULL;
    json_t                          *pJsonDataTemp = NULL;
    json_t                          *pJsonWithTemp = NULL;
    json_t                          *pJsonConditionTemp = NULL;
    PVMCA_SNPOLICY_OPERATION        pOperation = NULL;

    if (!pJsonRule || !ppOperation)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!(pJsonDataTemp = json_object_get(pJsonRule, "data")))
    {
        VMCA_LOG_ERROR(
            "[%s,%d] SN policy match rule in config (%s) must have \"data\" clause",
            __FUNCTION__,
            __LINE__,
            VMCA_POLICY_FILE_PATH);
        dwError = VMCA_JSON_PARSE_ERROR;
        BAIL_ON_JSON_PARSE_ERROR(dwError);
    }

    if (!(pJsonConditionTemp = json_object_get(pJsonRule, "condition")))
    {
        VMCA_LOG_ERROR(
            "[%s,%d] SN policy match rule in config (%s) must have \"condition\" clause",
            __FUNCTION__,
            __LINE__,
            VMCA_POLICY_FILE_PATH);
        dwError = VMCA_JSON_PARSE_ERROR;
        BAIL_ON_JSON_PARSE_ERROR(dwError);
    }

    if (!(pJsonWithTemp = json_object_get(pJsonRule, "with")))
    {
        VMCA_LOG_ERROR(
            "[%s,%d] SN policy match rule in config (%s) must have \"with\" clause",
            __FUNCTION__,
            __LINE__,
            VMCA_POLICY_FILE_PATH);
        dwError = VMCA_JSON_PARSE_ERROR;
        BAIL_ON_JSON_PARSE_ERROR(dwError);
    }

    pcszDataTemp = json_string_value(pJsonDataTemp);
    if (IsNullOrEmptyString(pcszDataTemp))
    {
        VMCA_LOG_ERROR(
            "[%s,%d] SN policy match rule in config (%s) cannot have empty \"data\" clause",
            __FUNCTION__,
            __LINE__,
            VMCA_POLICY_FILE_PATH);
        dwError = VMCA_POLICY_CONFIG_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }
    if (VMCAStringCompareA(pcszDataTemp, VMCA_POLICY_REQ_UPN_DN, TRUE))
    {
        VMCA_LOG_ERROR(
            "[%s,%d] Unknown SN policy match rule \"data\" value. (%s)",
            __FUNCTION__,
            __LINE__,
            pcszDataTemp);
        dwError = VMCA_POLICY_CONFIG_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pcszConditionTemp = json_string_value(pJsonConditionTemp);
    if (IsNullOrEmptyString(pcszConditionTemp))
    {
        VMCA_LOG_ERROR(
            "[%s,%d] SN policy match rule in config (%s) cannot have empty \"condition\" clause",
            __FUNCTION__,
            __LINE__,
            VMCA_POLICY_FILE_PATH);
        dwError = VMCA_POLICY_CONFIG_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }
    if (VMCAStringCompareA(pcszConditionTemp, VMCA_POLICY_COND_BEGINS, TRUE))
    {
        VMCA_LOG_ERROR(
            "[%s,%d] Unknown SN policy match rule \"condition\" value. (%s)",
            __FUNCTION__,
            __LINE__,
            pcszConditionTemp);
        dwError = VMCA_POLICY_CONFIG_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pcszWithTemp = json_string_value(pJsonWithTemp);
    if (IsNullOrEmptyString(pcszWithTemp))
    {
        VMCA_LOG_ERROR(
            "[%s,%d] SN policy match rule in config (%s) cannot have empty \"with\" clause",
            __FUNCTION__,
            __LINE__,
            VMCA_POLICY_FILE_PATH);
        dwError = VMCA_POLICY_CONFIG_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
                        sizeof(VMCA_SNPOLICY_OPERATION),
                        (PVOID *)&pOperation);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringA(
                        pcszWithTemp,
                        &pOperation->pszWith);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringA(
                        pcszConditionTemp,
                        &pOperation->pszCondition);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringA(
                        pcszDataTemp,
                        &pOperation->pszData);
    BAIL_ON_VMCA_ERROR(dwError);


    *ppOperation = pOperation;


cleanup:

    return dwError;

error:

    VMCAPolicySNOperationFree(pOperation);
    if (ppOperation)
    {
        *ppOperation = NULL;
    }

    goto cleanup;

}

static
DWORD
VMCAPolicySNValidateOperationParse(
    json_t                          *pJsonRule,
    PDWORD                          pdwOperationsLen,
    PVMCA_SNPOLICY_OPERATION        **pppOperations
    )
{
    DWORD                           dwError = 0;
    DWORD                           dwIdx = 0;
    PCSTR                           pcszDataTemp = NULL;
    PCSTR                           pcszWithTemp = NULL;
    size_t                          szArrayLen = 0;
    json_t                          *pJsonTemp = NULL;
    json_t                          *pJsonDataTemp = NULL;
    json_t                          *pJsonWithTemp = NULL;
    PVMCA_SNPOLICY_OPERATION        *ppOperations = NULL;

    if (!pJsonRule || !pppOperations)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!json_is_array(pJsonRule))
    {
        VMCA_LOG_ERROR(
            "[%s,%d] SN policy validate rule in config (%s) must be an array",
            __FUNCTION__,
            __LINE__,
            VMCA_POLICY_FILE_PATH);
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    szArrayLen = json_array_size(pJsonRule);
    if (szArrayLen < 1)
    {
        VMCA_LOG_ERROR(
            "[%s,%d] SN policy rule validate in config (%s) must be an array with at least 1 object",
            __FUNCTION__,
            __LINE__,
            VMCA_POLICY_FILE_PATH);
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
                        sizeof(PVMCA_SNPOLICY_OPERATION) * (DWORD)szArrayLen,
                        (PVOID *)&ppOperations);
    BAIL_ON_VMCA_ERROR(dwError);

    json_array_foreach(pJsonRule, dwIdx, pJsonTemp)
    {
        if (!(pJsonDataTemp = json_object_get(pJsonTemp, "data")))
        {
            VMCA_LOG_ERROR(
                "[%s,%d] SN policy rule validate in config (%s) must have \"data\" clause",
                __FUNCTION__,
                __LINE__,
                VMCA_POLICY_FILE_PATH);
            dwError = VMCA_JSON_PARSE_ERROR;
            BAIL_ON_JSON_PARSE_ERROR(dwError);
        }

        if (!(pJsonWithTemp = json_object_get(pJsonTemp, "with")))
        {
            VMCA_LOG_ERROR(
                "[%s,%d] SN policy validate rule in config (%s) must have \"with\" clause",
                __FUNCTION__,
                __LINE__,
                VMCA_POLICY_FILE_PATH);
            dwError = VMCA_JSON_PARSE_ERROR;
            BAIL_ON_JSON_PARSE_ERROR(dwError);
        }

        pcszDataTemp = json_string_value(pJsonDataTemp);
        if (IsNullOrEmptyString(pcszDataTemp))
        {
            VMCA_LOG_ERROR(
                "[%s,%d] SN policy validate rule in config (%s) cannot have empty \"data\" clause",
                __FUNCTION__,
                __LINE__,
                VMCA_POLICY_FILE_PATH);
            dwError = VMCA_POLICY_CONFIG_ERROR;
            BAIL_ON_VMCA_ERROR(dwError);
        }
        if (VMCAStringCompareA(pcszDataTemp, VMCA_POLICY_REQ_CSR_SUBJ_ORGS, TRUE))
        {
            VMCA_LOG_ERROR(
                "[%s,%d] Unknown SN policy validate rule \"data\" value. (%s)",
                __FUNCTION__,
                __LINE__,
                pcszDataTemp);
            dwError = VMCA_POLICY_CONFIG_ERROR;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        pcszWithTemp = json_string_value(pJsonWithTemp);
        if (IsNullOrEmptyString(pcszWithTemp))
        {
            VMCA_LOG_ERROR(
                "[%s,%d] SN policy validate rule in config (%s) cannot have empty \"with\" clause",
                __FUNCTION__,
                __LINE__,
                VMCA_POLICY_FILE_PATH);
            dwError = VMCA_POLICY_CONFIG_ERROR;
            BAIL_ON_VMCA_ERROR(dwError);
        }
        if (VMCAStringCompareA(pcszWithTemp, VMCA_POLICY_REQ_UPN_RDN, TRUE))
        {
            VMCA_LOG_ERROR(
                "[%s,%d] Unknown SN policy validate rule \"with\" value. (%s)",
                __FUNCTION__,
                __LINE__,
                pcszWithTemp);
            dwError = VMCA_POLICY_CONFIG_ERROR;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        dwError = VMCAAllocateMemory(
                            sizeof(VMCA_SNPOLICY_OPERATION),
                            (PVOID *)&ppOperations[dwIdx]);
        BAIL_ON_VMCA_ERROR(dwError);

        dwError = VMCAAllocateStringA(
                            pcszWithTemp,
                            &ppOperations[dwIdx]->pszWith);
        BAIL_ON_VMCA_ERROR(dwError);

        dwError = VMCAAllocateStringA(
                            pcszDataTemp,
                            &ppOperations[dwIdx]->pszData);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    *pdwOperationsLen   = (DWORD)szArrayLen;
    *pppOperations      = ppOperations;

cleanup:

    return dwError;

error:

    VMCAPolicySNOperationArrayFree(ppOperations, (DWORD)szArrayLen);
    if (pppOperations)
    {
        *pppOperations = NULL;
    }

    goto cleanup;
}

static
VOID
VMCAPolicySNOperationFree(
    PVMCA_SNPOLICY_OPERATION        pOperation
    )
{
    if (pOperation)
    {
        VMCA_SAFE_FREE_STRINGA(pOperation->pszData);
        VMCA_SAFE_FREE_STRINGA(pOperation->pszCondition);
        VMCA_SAFE_FREE_STRINGA(pOperation->pszWith);
        VMCA_SAFE_FREE_MEMORY(pOperation);
    }
}

static
VOID
VMCAPolicySNOperationArrayFree(
    PVMCA_SNPOLICY_OPERATION        *ppOperations,
    DWORD                           dwArrayLen
    )
{
    DWORD       dwIdx = 0;

    if (ppOperations)
    {
        if (dwArrayLen > 0)
        {
            for (; dwIdx < dwArrayLen; ++dwIdx)
            {
                VMCAPolicySNOperationFree(ppOperations[dwIdx]);
            }
        }

        VMCA_SAFE_FREE_MEMORY(ppOperations);
    }
}

static
DWORD
VMCAPolicySNGetOrgNamesFromCSR(
    PSTR                            pszPKCS10Request,
    PDWORD                          pdwOrgNamesLen,
    PSTR                            **pppszOrgNames
    )
{
    DWORD                           dwError = 0;
    DWORD                           dwIdx = 0;
    DWORD                           dwNumOEntries = 0;
    PSTR                            *ppszOrgNames = NULL;
    PSTR                            *ppszOrgNamesTemp = NULL;
    PSTR                            pszOString = NULL;
    X509_REQ                        *pCSR = NULL;
    X509_NAME                       *pszSubjName = NULL;
    X509_NAME_ENTRY                 *pOEntry = NULL;
    ASN1_STRING                     *pOAsn1 = NULL;
    int                             iOPos = 0;
    size_t                          szNumDNs = 0;
    size_t                          szEntryLength = 0;

    if (IsNullOrEmptyString(pszPKCS10Request) ||
        !pdwOrgNamesLen ||
        !pppszOrgNames)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAPEMToCSR(
                    pszPKCS10Request,
                    &pCSR);
    BAIL_ON_VMCA_ERROR(dwError);

    pszSubjName = X509_REQ_get_subject_name(pCSR);
    if(pszSubjName == NULL)
    {
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    szNumDNs = X509_NAME_entry_count(pszSubjName);
    if (szNumDNs == 0)
    {
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
                        sizeof(PSTR) * (DWORD)szNumDNs,
                        (PVOID *)&ppszOrgNamesTemp);
    BAIL_ON_VMCA_ERROR(dwError);

    for (;;)
    {
        iOPos = X509_NAME_get_index_by_NID(
                        pszSubjName,
                        NID_organizationName,
                        iOPos);
        if (iOPos == -1)
        {
            break;
        }

        pOEntry = X509_NAME_get_entry(
                        pszSubjName,
                        iOPos);
        if (pOEntry == NULL)
        {
            dwError = VMCA_CERT_DECODE_FAILURE;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        pOAsn1 = X509_NAME_ENTRY_get_data(pOEntry);
        if (pOAsn1 == NULL)
        {
            dwError = VMCA_CERT_DECODE_FAILURE;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        szEntryLength = ASN1_STRING_to_UTF8(
                            (unsigned char **)&pszOString,
                            pOAsn1);
        if (!pszOString || szEntryLength != strlen(pszOString))
        {
            dwError = VMCA_CERT_DECODE_FAILURE;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        dwError = VMCAAllocateStringA(
                            pszOString,
                            &ppszOrgNamesTemp[dwIdx]);
        BAIL_ON_VMCA_ERROR(dwError);
        ++dwIdx;
        ++dwNumOEntries;

        if (pszOString)
        {
            OPENSSL_free(pszOString);
            pszOString = NULL;
        }
    }

    dwError = VMCACopyStringArrayA(
                        &ppszOrgNames,
                        dwNumOEntries,
                        ppszOrgNamesTemp,
                        (DWORD)szNumDNs);
    BAIL_ON_VMCA_ERROR(dwError);

    *pppszOrgNames = ppszOrgNames;
    *pdwOrgNamesLen = dwNumOEntries;


cleanup:

    if (pCSR)
    {
        X509_REQ_free(pCSR);
    }
    if (pszOString)
    {
        OPENSSL_free(pszOString);
    }
    VMCAFreeStringArrayA(ppszOrgNamesTemp, szNumDNs);

    return dwError;

error:

    VMCAFreeStringArrayA(ppszOrgNames, dwNumOEntries);
    if (pppszOrgNames)
    {
        *pppszOrgNames = NULL;
    }
    if (pdwOrgNamesLen)
    {
        *pdwOrgNamesLen = 0;
    }

    goto cleanup;
}

/*
 * VMCAPolicySNMatchAuthOUWithCSR is the heart of SN policy validation.
 *
 * It verifies if the CSR organizationName entries in the subjectName field comprise the DN of the requestor.
 *
 * Examples:
 *      * Pass:
 *          * Parameters:
 *              * Machine account DN: cn=user@example.com,ou=ex2,ou=ex1,ou=computers,dc=example,dc=com
 *              * organizationName entries: {ex1, ex2}
 *              * Config file match rule baseDN: ou=ex1,ou=computers,dc=example,dc=com
 *          * Result:
 *              * This will pass because the machine account lives under ou=ex2,ou=ex1 which is
 *              under the baseDN from the config file, and the CSR organizationName entries contain
 *              the OUs that the machine account lives under.
 *      * Pass:
 *          * Parameters:
 *              * Machine account DN: cn=user@example.com,ou=ex2,ou=ex1,ou=computers,dc=example,dc=com
 *              * organizationName entries: {ex2, ex1}
 *              * Config file match rule baseDN: ou=ex1,ou=computers,dc=example,dc=com
 *          * Result:
 *              * This will pass because the machine account lives under ou=ex2,ou=ex1 which is
 *              under the baseDN from the config file, and the CSR organizationName entries contain
 *              the OUs that the machine account lives under (order does not matter).
 *      * Fail:
 *          * Parameters:
 *              * Machine account DN: cn=user@example.com,ou=ex2,ou=ex1,ou=computers,dc=example,dc=com
 *              * organizationName entries: {ex2}
 *              * Config file match rule baseDN: ou=ex1,ou=computers,dc=example,dc=com
 *          * Result:
 *              * This will fail because the machine account lives under ou=ex2,ou=ex1 which is
 *              under the baseDN from the config file, but the CSR organizationName entries do not
 *              contain all of the OUs that the machine account lives under.
 *      * Fail:
 *          * Parameters:
 *              * Machine account DN: cn=user@example.com,ou=ex2,ou=ex1,ou=computers,dc=example,dc=com
 *              * organizationName entries: {ex2, ex2}
 *              * Config file match rule baseDN: ou=ex1,ou=computers,dc=example,dc=com
 *          * Result:
 *              * This will fail because the machine account lives under ou=ex2,ou=ex1 which is
 *              under the baseDN from the config file, but the CSR organizationName entries do not
 *              contain all of the OUs that the machine account lives under, even though the amount
 *              is the same.
 *      * Fail:
 *          * Parameters:
 *              * Machine account DN: cn=user@example.com,ou=ex2,ou=ex1,ou=computers,dc=example,dc=com
 *              * organizationName entries: {ex3, ex4}
 *              * Config file match rule baseDN: ou=ex1,ou=computers,dc=example,dc=com
 *          * Result:
 *              * This will fail because the machine account lives under ou=ex2,ou=ex1 which is
 *              under the baseDN from the config file, but the CSR organizationName entries do not
 *              contain any of the OUs that the machine account lives under.
 *
 * @param pcszAuthDN:               The DN of the CSR requestor
 * @param pcszAuthBaseDN:           The baes DN under which the requestor must live
 * @param dwOrgNamesLen:            The number of organizationName entries in the CSR subjectName field
 * @param ppcszOrgNames:            An array of the organizationName entries
 *
 * @return                          A status code indicating valid (0) or invalid (not 0) request
 */
static
DWORD
VMCAPolicySNMatchAuthOUWithCSR(
    PCSTR                           pcszAuthDN,
    PCSTR                           pcszAuthBaseDN,
    DWORD                           dwOrgNamesLen,
    PSTR* const                     ppcszOrgNames
    )
{
    DWORD                           dwError = 0;
    DWORD                           dwIdx = 0;
    DWORD                           dwNumRDNsInAuthDN = 0;
    DWORD                           dwNumAuthRDNsToValidate = 0;
    DWORD                           dwNumRDNsInBaseDn = 0;
    PSTR                            pszFormattedOrgName = NULL;
    PSTR                            *ppszAuthRDNs = NULL;
    PSTR                            *ppszAuthBaseRDNs = NULL;
    PSTR                            *ppszAuthRDNsNoUserObj = NULL;
    PSTR                            *ppszOrgNamesLocal = NULL;

    if (IsNullOrEmptyString(pcszAuthDN) ||
        IsNullOrEmptyString(pcszAuthBaseDN))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (dwOrgNamesLen < 1 || !ppcszOrgNames)
    {
        VMCA_LOG_INFO(
                "[%s,%d] SNPolicy violation: CSR subjectName did not contain organizationName entries. User (%s)",
                __FUNCTION__,
                __LINE__,
                pcszAuthDN);
        dwError = VMCA_POLICY_VALIDATION_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCADNToRDNArray(
                        pcszAuthBaseDN,
                        FALSE,
                        &dwNumRDNsInBaseDn,
                        &ppszAuthBaseRDNs);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCADNToRDNArray(
                    pcszAuthDN,
                    FALSE,
                    &dwNumRDNsInAuthDN,
                    &ppszAuthRDNs);
    BAIL_ON_VMCA_ERROR(dwError);

    dwNumAuthRDNsToValidate = dwNumRDNsInAuthDN - dwNumRDNsInBaseDn;

    if (dwOrgNamesLen != dwNumAuthRDNsToValidate)
    {
        VMCA_LOG_INFO(
                "[%s,%d] SNPolicy violation: CSR subjectName does not have same number of organizationName entries as auth principal RDNs. User (%s)",
                __FUNCTION__,
                __LINE__,
                pcszAuthDN);
        dwError = VMCA_POLICY_VALIDATION_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCACopyStringArrayA(
                        &ppszOrgNamesLocal,
                        dwOrgNamesLen,
                        ppcszOrgNames,
                        dwOrgNamesLen);
    BAIL_ON_VMCA_ERROR(dwError);

    // Increment AuthRDN ptr array by 1 as first RDN will be machine account obj
    ppszAuthRDNsNoUserObj = ppszAuthRDNs + 1;
    qsort(ppszAuthRDNsNoUserObj, dwNumAuthRDNsToValidate, sizeof(PSTR), &VMCAPolicySNStrCmpWrapper);
    qsort(ppszOrgNamesLocal, dwOrgNamesLen, sizeof(PSTR), &VMCAPolicySNStrCmpWrapper);

    for (; dwIdx < dwNumAuthRDNsToValidate; ++dwIdx)
    {
        dwError = VMCAAllocateStringPrintfA(
                            &pszFormattedOrgName,
                            "ou=%s",
                            ppszOrgNamesLocal[dwIdx]);
        BAIL_ON_VMCA_ERROR(dwError);

        if (VMCAStringCompareA(ppszAuthRDNsNoUserObj[dwIdx], pszFormattedOrgName, TRUE))
        {
            VMCA_LOG_INFO(
                "[%s,%d] SNPolicy violation: CSR subjectName organizationName does not match auth principal DN. User (%s)",
                __FUNCTION__,
                __LINE__,
                pcszAuthDN);
            dwError = VMCA_POLICY_VALIDATION_ERROR;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        VMCA_SAFE_FREE_STRINGA(pszFormattedOrgName);
    }


cleanup:

    VMCA_SAFE_FREE_STRINGA(pszFormattedOrgName);
    VMCAFreeStringArrayA(ppszAuthBaseRDNs, dwNumRDNsInBaseDn);
    VMCAFreeStringArrayA(ppszAuthRDNs, dwNumRDNsInAuthDN);
    VMCAFreeStringArrayA(ppszOrgNamesLocal, dwOrgNamesLen);

    return dwError;

error:

    goto cleanup;
}

static
int
VMCAPolicySNStrCmpWrapper(
    const void *a,
    const void *b
    )
{
    return VMCAStringCompareA(*(PSTR *)a, *(PSTR *)b, TRUE);
}
