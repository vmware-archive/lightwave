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
DWORD
_JsonAttrStringCreate(
    PLWCA_JSON_OBJECT   pJson,
    PCSTR               pcszType,
    ...
    );

static
DWORD
_JsonAttrStringArrayCreate(
    PLWCA_JSON_OBJECT       pJson,
    PCSTR                   pcszType,
    PLWCA_STRING_ARRAY      pszArray
    );

static
DWORD
_JsonAttrIntCreate(
    PLWCA_JSON_OBJECT   pJson,
    PCSTR               pcszType,
    int                 iArgc,
    ...
    );

DWORD
LwCASerializeConfigCAToJSON(
    PCSTR       pcszCAId,
    PCSTR       pcszDomain,
    PSTR        *ppszReqBody
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pRoot = NULL;
    PLWCA_JSON_OBJECT   pAttr = NULL;
    PSTR                pszDN = NULL;
    PSTR                pszReqBody = NULL;

    if (IsNullOrEmptyString(pcszCAId) ||
        IsNullOrEmptyString(pcszDomain) ||
        !ppszReqBody
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonObjectCreate(&pRoot);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringPrintfA(&pszDN,
                                        LWCA_POST_CA_CONFIG_DN,
                                        pcszCAId,
                                        pcszDomain
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetStringToObject(pRoot, LWCA_LDAP_DN, pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonArrayCreate(&pAttr);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _JsonAttrStringCreate(pAttr,
                                    LWCA_POST_OBJ_CLASS,
                                    LWCA_POST_CONTAINER,
                                    LWCA_POST_TOP,
                                    NULL
                                    );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _JsonAttrStringCreate(pAttr, LWCA_LDAP_CN, pcszCAId, NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetJsonToObject(pRoot, LWCA_POST_JSON_ATTR, pAttr);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonDumps(pRoot, JSON_DECODE_ANY, &pszReqBody);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszReqBody = pszReqBody;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszDN);
    LWCA_SAFE_JSON_DECREF(pRoot);
    LWCA_SAFE_JSON_DECREF(pAttr);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszReqBody);
    if (ppszReqBody)
    {
        *ppszReqBody = NULL;
    }
    goto cleanup;
}

DWORD
LwCASerializeCAToJSON(
    PCSTR               pcszCAId,
    PLWCA_DB_CA_DATA    pCAData,
    PCSTR               pcszParentCA,
    PCSTR               pcszDomain,
    PSTR                *ppszReqBody
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pRoot = NULL;
    PLWCA_JSON_OBJECT   pAttr = NULL;
    PSTR                pszDN = NULL;
    PSTR                pszReqBody = NULL;
    PBYTE               pszPtr = NULL;


    if (IsNullOrEmptyString(pcszCAId) ||
        IsNullOrEmptyString(pcszDomain) ||
        !pCAData
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonObjectCreate(&pRoot);
    BAIL_ON_LWCA_ERROR(dwError);

    if (IsNullOrEmptyString(pcszParentCA))
    {
        dwError = LwCAAllocateStringPrintfA(&pszDN,
                                            LWCA_POST_ROOT_CA_DN_ENTRY,
                                            pcszCAId,
                                            pcszDomain
                                            );

        BAIL_ON_LWCA_ERROR(dwError);
    }
    else
    {
        dwError = LwCAAllocateStringPrintfA(&pszDN,
                                            LWCA_POST_INTERMEDIATE_CA_DN_ENTRY,
                                            pcszCAId,
                                            pcszParentCA,
                                            pcszDomain
                                            );
        BAIL_ON_LWCA_ERROR(dwError);
    }
    dwError = LwCAJsonSetStringToObject(pRoot, LWCA_LDAP_DN, pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonArrayCreate(&pAttr);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _JsonAttrStringCreate(pAttr,
                                    LWCA_POST_OBJ_CLASS,
                                    LWCA_POST_CA_OBJ_CLASS,
                                    LWCA_POST_PKICA_AUX_CLASS,
                                    NULL
                                    );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _JsonAttrStringCreate(pAttr, LWCA_LDAP_CN, pcszCAId, NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!IsNullOrEmptyString(pCAData->pszSubjectName))
    {
        dwError = _JsonAttrStringCreate(pAttr,
                                        LWCA_POST_CA_SUBJ_NAME,
                                        pCAData->pszSubjectName,
                                        NULL
                                        );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCAData->pEncryptedPrivateKey)
    {
        pszPtr = pCAData->pEncryptedPrivateKey->pData;
        dwError = _JsonAttrStringCreate(pAttr,
                                        LWCA_POST_CA_ENCR_PRIV_KEY,
                                        pszPtr,
                                        NULL
                                        );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCAData->pszCRLNumber))
    {
        dwError = _JsonAttrStringCreate(pAttr,
                                        LWCA_POST_CA_CRL_NUM,
                                        pCAData->pszCRLNumber,
                                        NULL
                                        );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszParentCA))
    {
        dwError = _JsonAttrStringCreate(pAttr,
                                        LWCA_POST_CA_PARENT,
                                        pcszParentCA,
                                        NULL
                                        );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCAData->pCertificates)
    {
        dwError = _JsonAttrStringArrayCreate(pAttr,
                                             LWCA_POST_CA_CERTIFICATES,
                                             (PLWCA_STRING_ARRAY)pCAData->pCertificates
                                             );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _JsonAttrIntCreate(pAttr,
                                 LWCA_POST_CA_STATUS,
                                 1,
                                 pCAData->status
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetJsonToObject(pRoot, LWCA_POST_JSON_ATTR, pAttr);
    BAIL_ON_LWCA_ERROR(dwError);
    LWCA_SAFE_JSON_DECREF(pAttr);

    dwError = LwCAJsonDumps(pRoot, JSON_DECODE_ANY, &pszReqBody);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszReqBody = pszReqBody;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszDN);
    LWCA_SAFE_JSON_DECREF(pRoot);
    LWCA_SAFE_JSON_DECREF(pAttr);
    return dwError;

error:
    if (!ppszReqBody)
    {
        *ppszReqBody = NULL;
    }
    LWCA_SAFE_FREE_STRINGA(pszReqBody);
    goto cleanup;
}

static
DWORD
_JsonAttrStringCreate(
    PLWCA_JSON_OBJECT   pJson,
    PCSTR               pcszType,
    ...
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pEntry = NULL;
    PLWCA_JSON_OBJECT   pObj = NULL;
    va_list             argList = { 0 };
    PSTR                pszStr = NULL;

    dwError = LwCAJsonObjectCreate(&pEntry);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetStringToObject(pEntry, "type", pcszType);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonArrayCreate(&pObj);
    BAIL_ON_LWCA_ERROR(dwError);

    va_start(argList, pcszType);
    pszStr = va_arg(argList, PSTR);
    while (pszStr != NULL)
    {

        dwError = LwCAJsonAppendStringToArray(pObj, pszStr);
        BAIL_ON_LWCA_ERROR(dwError);

        pszStr = va_arg(argList, PSTR);
    }

    dwError = LwCAJsonSetJsonToObject(pEntry, "value", pObj);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonAppendJsonToArray(pJson, pEntry);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    va_end(argList);
    LWCA_SAFE_JSON_DECREF(pEntry);
    LWCA_SAFE_JSON_DECREF(pObj);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_JsonAttrStringArrayCreate(
    PLWCA_JSON_OBJECT       pJson,
    PCSTR                   pcszType,
    PLWCA_STRING_ARRAY      pszArray
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pEntry = NULL;
    PLWCA_JSON_OBJECT   pObj = NULL;
    int                 i = 0;

    if (!pJson || IsNullOrEmptyString(pcszType) || !pszArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonObjectCreate(&pObj);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetStringToObject(pObj, "type", pcszType);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonArrayCreate(&pEntry);
    BAIL_ON_LWCA_ERROR(dwError);

    for (i = 0; i < pszArray->dwCount; ++i)
    {
        dwError = LwCAJsonAppendStringToArray(pEntry, pszArray->ppData[i]);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonSetJsonToObject(pObj, "value", pEntry);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonAppendJsonToArray(pJson, pObj);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_JSON_DECREF(pEntry);
    LWCA_SAFE_JSON_DECREF(pObj);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_JsonAttrIntCreate(
    PLWCA_JSON_OBJECT   pJson,
    PCSTR               pcszType,
    int                 iArgc,
    ...
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pEntry = NULL;
    PLWCA_JSON_OBJECT   pObj = NULL;
    va_list             argList = { 0 };
    PSTR                pszNum = NULL;
    int                 iValue = 0;
    int                 i = 0;

    dwError = LwCAJsonObjectCreate(&pObj);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetStringToObject(pObj, "type", pcszType);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonArrayCreate(&pEntry);
    BAIL_ON_LWCA_ERROR(dwError);

    va_start(argList, iArgc);
    for (i = 0; i < iArgc; ++i)
    {
        iValue = va_arg(argList, int);
        dwError = LwCAAllocateStringPrintfA(&pszNum, "%d", iValue);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAJsonAppendStringToArray(pEntry, pszNum);
        BAIL_ON_LWCA_ERROR(dwError);

        LWCA_SAFE_FREE_STRINGA(pszNum);
    }

    dwError = LwCAJsonSetJsonToObject(pObj, "value", pEntry);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonAppendJsonToArray(pJson, pObj);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    va_end(argList);
    LWCA_SAFE_FREE_STRINGA(pszNum);
    LWCA_SAFE_JSON_DECREF(pEntry);
    LWCA_SAFE_JSON_DECREF(pObj);
    return dwError;

error:
    goto cleanup;
}