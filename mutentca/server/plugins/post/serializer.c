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

static
DWORD
_LwCAGetAttributes(
    PLWCA_JSON_OBJECT   pJson,
    PLWCA_JSON_OBJECT   *ppAttr
    );

static
DWORD
_LwCAGetString(
    PLWCA_JSON_OBJECT   pJson,
    PCSTR               pcszKey,
    PSTR                *ppszValue
    );

static
DWORD
_LwCAGetInteger(
    PLWCA_JSON_OBJECT   pJson,
    PCSTR               pcszKey,
    int                 *pValue
    );

static
DWORD
_LwCAGetStringArray(
    PLWCA_JSON_OBJECT   pJson,
    PCSTR               pcszKey,
    PLWCA_STRING_ARRAY  *ppValue
    );

static
DWORD
_LwCAGetBytes(
    PLWCA_JSON_OBJECT   pJson,
    PCSTR               pcszKey,
    PLWCA_KEY           *ppValue
    );

static
DWORD
_LwCAJsonReplaceString(
    PCSTR               pcszType,
    PCSTR               pcszValue,
    PLWCA_JSON_OBJECT   pJson
    );

static
DWORD
_LwCAJsonReplaceStringArray(
    PCSTR               pcszType,
    PLWCA_STRING_ARRAY  pValue,
    PLWCA_JSON_OBJECT   pJson
    );

static
DWORD
_LwCAJsonReplaceCertArray(
    PCSTR                   pcszType,
    PLWCA_CERTIFICATE_ARRAY pValue,
    PLWCA_JSON_OBJECT       pJson
    );

static
DWORD
_LwCAJsonReplaceBytes(
    PCSTR               pcszType,
    PLWCA_KEY           pValue,
    PLWCA_JSON_OBJECT   pJson
    );

static
DWORD
_LwCAJsonReplaceInteger(
    PCSTR               pcszType,
    int                 value,
    PLWCA_JSON_OBJECT   pJson
    );

static
DWORD
_LwCATransformAttrJsonToKV(
    PLWCA_JSON_OBJECT   pOrigAttr,
    PLWCA_JSON_OBJECT   *ppAttr
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

DWORD
LwCADeserializeJSONToCA(
    PCSTR               pcszResponse,
    PLWCA_DB_CA_DATA    *ppCaData
    )
{
    DWORD                       dwError = 0;
    PLWCA_JSON_OBJECT           pRespJson = NULL;
    PLWCA_JSON_OBJECT           pAttrJson = NULL;
    DWORD                       dwCount = 0;
    PSTR                        pszSubjectName = NULL;
    PLWCA_STRING_ARRAY          pszCertArray = NULL;
    PLWCA_CERTIFICATE_ARRAY     pCertArray = NULL;
    PSTR                        pszCRLNumber = NULL;
    int                         caStatus = 0;
    PLWCA_KEY                   pEncryptedPrivateKey = NULL;
    PLWCA_DB_CA_DATA            pCaData = NULL;

    if (IsNullOrEmptyString(pcszResponse) || !ppCaData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonLoadObjectFromString(pcszResponse, &pRespJson);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetUnsignedIntegerFromKey(pRespJson,
                                                FALSE,
                                                LWCA_RESP_RESULT_COUNT,
                                                &dwCount
                                                );
    BAIL_ON_LWCA_ERROR(dwError);

    if (dwCount == 0)
    {
        dwError = LWCA_ERROR_ENTRY_NOT_FOUND;
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else if (dwCount > 1)
    {
        dwError = LWCA_ERROR_INVALID_ENTRY;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAGetAttributes(pRespJson, &pAttrJson);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetString(pAttrJson, LWCA_POST_CA_SUBJ_NAME, &pszSubjectName);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetStringArray(pAttrJson,
                                  LWCA_POST_CA_CERTIFICATES,
                                  &pszCertArray
                                  );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACreateCertArray(pszCertArray->ppData,
                                  pszCertArray->dwCount,
                                  &pCertArray
                                  );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetString(pAttrJson, LWCA_POST_CA_CRL_NUM, &pszCRLNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetInteger(pAttrJson, LWCA_POST_CA_STATUS, &caStatus);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetBytes(pAttrJson,
                            LWCA_POST_CA_ENCR_PRIV_KEY,
                            &pEncryptedPrivateKey
                            );
    BAIL_ON_LWCA_ERROR(dwError);

    // TODO pszLastCRLUpdate and pszNextCRLUpdate will be added separately
    dwError = LwCADbCreateCAData(pszSubjectName,
                                 pCertArray,
                                 pEncryptedPrivateKey,
                                 pszCRLNumber,
                                 NULL,
                                 NULL,
                                 caStatus,
                                 &pCaData
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCaData = pCaData;

cleanup:
    LWCA_SAFE_JSON_DECREF(pRespJson);
    LWCA_SAFE_FREE_STRINGA(pszSubjectName);
    LwCAFreeStringArray(pszCertArray);
    LwCAFreeCertificates(pCertArray);
    LWCA_SAFE_FREE_STRINGA(pszCRLNumber);
    LwCAFreeKey(pEncryptedPrivateKey);
    return dwError;

error:
    // free the whole structure
    LwCADbFreeCAData(pCaData);
    if (ppCaData)
    {
        *ppCaData = NULL;
    }
    goto cleanup;
}

DWORD
LwCAGenerateCAPatchRequestBody(
    PLWCA_DB_CA_DATA    pCaData,
    PSTR                *ppszBody
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pJsonBody = NULL;
    PSTR                pszBody = NULL;

    if (!pCaData || !ppszBody)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonArrayCreate(&pJsonBody);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!IsNullOrEmptyString(pCaData->pszSubjectName))
    {
        dwError = _LwCAJsonReplaceString(LWCA_POST_CA_SUBJ_NAME,
                                         pCaData->pszSubjectName,
                                         pJsonBody
                                         );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCaData->pCertificates)
    {
        dwError = _LwCAJsonReplaceCertArray(LWCA_POST_CA_CERTIFICATES,
                                            pCaData->pCertificates,
                                            pJsonBody
                                            );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCaData->pEncryptedPrivateKey)
    {
        dwError = _LwCAJsonReplaceBytes(LWCA_POST_CA_ENCR_PRIV_KEY,
                                        pCaData->pEncryptedPrivateKey,
                                        pJsonBody
                                        );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCaData->pszCRLNumber))
    {
        dwError = _LwCAJsonReplaceString(LWCA_POST_CA_CRL_NUM,
                                         pCaData->pszCRLNumber,
                                         pJsonBody
                                         );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    // status is not a pointer, hence it will always be updated
    dwError = _LwCAJsonReplaceInteger(LWCA_POST_CA_STATUS,
                                      pCaData->status,
                                      pJsonBody
                                      );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonDumps(pJsonBody, JSON_DECODE_ANY, &pszBody);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszBody = pszBody;

cleanup:
    LWCA_SAFE_JSON_DECREF(pJsonBody);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszBody);
    if (ppszBody)
    {
        *ppszBody = pszBody;
    }
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

    dwError = LwCAJsonSetStringToObject(pEntry, LWCA_LDAP_ATTR_TYPE, pcszType);
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

    dwError = LwCAJsonSetJsonToObject(pEntry, LWCA_LDAP_ATTR_VALUE, pObj);
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

    dwError = LwCAJsonSetStringToObject(pObj, LWCA_LDAP_ATTR_TYPE, pcszType);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonArrayCreate(&pEntry);
    BAIL_ON_LWCA_ERROR(dwError);

    for (i = 0; i < pszArray->dwCount; ++i)
    {
        dwError = LwCAJsonAppendStringToArray(pEntry, pszArray->ppData[i]);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonSetJsonToObject(pObj, LWCA_LDAP_ATTR_VALUE, pEntry);
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

    dwError = LwCAJsonSetStringToObject(pObj, LWCA_LDAP_ATTR_TYPE, pcszType);
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

    dwError = LwCAJsonSetJsonToObject(pObj, LWCA_LDAP_ATTR_VALUE, pEntry);
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

static
DWORD
_LwCATransformAttrJsonToKV(
    PLWCA_JSON_OBJECT   pOrigAttr,
    PLWCA_JSON_OBJECT   *ppAttr
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pAttr = NULL;
    PLWCA_JSON_OBJECT   pElem = NULL; // borrowed ref
    PLWCA_JSON_OBJECT   pValue = NULL; // borrowed ref
    PLWCA_JSON_OBJECT   pAttrValue = NULL;
    PSTR                pszElemType = NULL;
    SIZE_T              attrLen = 0;
    SIZE_T              i = 0;

    if (!pOrigAttr || !ppAttr)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    attrLen = LwCAJsonArraySize(pOrigAttr);

    dwError = LwCAJsonObjectCreate(&pAttr);
    BAIL_ON_LWCA_ERROR(dwError);

    for (i = 0; i < attrLen; ++i)
    {
        dwError = LwCAJsonArrayGetBorrowedRef(pOrigAttr, i, &pElem);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAJsonGetStringFromKey(pElem,
                                           FALSE,
                                           LWCA_LDAP_ATTR_TYPE,
                                           &pszElemType
                                           );
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAJsonGetObjectFromKey(pElem,
                                           FALSE,
                                           LWCA_LDAP_ATTR_VALUE,
                                           &pValue
                                           );
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAJsonArrayStringCopy(pValue, &pAttrValue);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAJsonSetJsonToObject(pAttr, pszElemType, pAttrValue);
        BAIL_ON_LWCA_ERROR(dwError);

        LWCA_SAFE_FREE_STRINGA(pszElemType);
        LWCA_SAFE_JSON_DECREF(pAttrValue);
    }

    *ppAttr = pAttr;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszElemType);
    LWCA_SAFE_JSON_DECREF(pAttrValue);
    return dwError;

error:
    LWCA_SAFE_JSON_DECREF(pAttr);
    if (ppAttr)
    {
        *ppAttr = NULL;
    }

    goto cleanup;
}

static
DWORD
_LwCAGetAttributes(
    PLWCA_JSON_OBJECT   pJson,
    PLWCA_JSON_OBJECT   *ppAttr
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pResult = NULL; // borrowed ref
    PLWCA_JSON_OBJECT   pResultElem = NULL; // borrowed ref
    PLWCA_JSON_OBJECT   pRawAttr = NULL; // borrowed ref
    PLWCA_JSON_OBJECT   pRawAttrElem = NULL; // borrowed ref
    PLWCA_JSON_OBJECT   pAttr = NULL;

    if (!pJson || !ppAttr)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetObjectFromKey(pJson,
                                       FALSE,
                                       LWCA_RESP_RESULT,
                                       &pResult
                                       );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonArrayGetBorrowedRef(pResult, 0, &pResultElem);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetObjectFromKey(pResultElem,
                                       FALSE,
                                       LWCA_POST_JSON_ATTR,
                                       &pRawAttr
                                       );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonArrayGetBorrowedRef(pRawAttr, 0, &pRawAttrElem);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCATransformAttrJsonToKV(pRawAttr, &pAttr);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppAttr = pAttr;

cleanup:
    return dwError;

error:
    LWCA_SAFE_JSON_DECREF(pAttr);
    if (ppAttr)
    {
        *ppAttr = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAGetString(
    PLWCA_JSON_OBJECT   pJson,
    PCSTR               pcszKey,
    PSTR                *ppszValue
    )
{
    DWORD               dwError = 0;
    PSTR                pszValue = NULL;
    PLWCA_JSON_OBJECT   pArray = NULL;

    if (IsNullOrEmptyString(pcszKey) || !pJson || !ppszValue)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetObjectFromKey(pJson, FALSE, pcszKey, &pArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonArrayGetStringAtIndex(pArray, 0, &pszValue);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszValue = pszValue;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszValue);
    if (ppszValue)
    {
        *ppszValue = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAGetInteger(
    PLWCA_JSON_OBJECT   pJson,
    PCSTR               pcszKey,
    int                 *pValue
    )
{
    DWORD               dwError = 0;
    PSTR                pszValue = NULL;
    PLWCA_JSON_OBJECT   pArray = NULL;
    PSTR                pszTmp = NULL;
    int                 value = 0;

    if (IsNullOrEmptyString(pcszKey) || !pJson || !pValue)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetObjectFromKey(pJson, FALSE, pcszKey, &pArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonArrayGetStringAtIndex(pArray, 0, &pszValue);
    BAIL_ON_LWCA_ERROR(dwError);

    value = (int)strtol(pszValue, &pszTmp, 10);

    if (errno)
    {
        dwError = errno + LWCA_ERROR_BASE + LWCA_ERRNO_BASE;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *pValue = value;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszValue);
    return dwError;

error:
    if (pValue)
    {
        *pValue = 0;
    }
    goto cleanup;
}

static
DWORD
_LwCAGetStringArray(
    PLWCA_JSON_OBJECT   pJson,
    PCSTR               pcszKey,
    PLWCA_STRING_ARRAY  *ppValue
    )
{
    DWORD               dwError = 0;
    PLWCA_STRING_ARRAY  pValue = NULL;
    PLWCA_JSON_OBJECT   pArray = NULL;
    PLWCA_JSON_OBJECT   pValueElem = NULL;
    SIZE_T              arrayLen = 0;
    SIZE_T              index = 0;
    PSTR                pszJsonStr = NULL;

    if (IsNullOrEmptyString(pcszKey) || !pJson || !ppValue)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }
    dwError = LwCAJsonGetObjectFromKey(pJson, FALSE, pcszKey, &pArray);
    BAIL_ON_LWCA_ERROR(dwError);

    arrayLen = LwCAJsonArraySize(pArray);

    dwError = LwCAAllocateMemory(sizeof(*pValue), (PVOID *)&pValue);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(PSTR) * arrayLen,
                                 (PVOID *)&pValue->ppData
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    json_array_foreach(pArray, index, pValueElem)
    {
        pszJsonStr = (PSTR)json_string_value(pValueElem);
        if (!pszJsonStr)
        {
            dwError = LWCA_JSON_PARSE_ERROR;
            BAIL_ON_LWCA_ERROR(dwError);
        }
        dwError = LwCAAllocateStringA(pszJsonStr, &pValue->ppData[index]);
        BAIL_ON_LWCA_ERROR(dwError);

        pValue->dwCount++;
    }

    *ppValue = pValue;

cleanup:
    return dwError;

error:
    LwCAFreeStringArray(pValue);
    if (ppValue)
    {
        *ppValue = pValue;
    }
    goto cleanup;
}

static
DWORD
_LwCAGetBytes(
    PLWCA_JSON_OBJECT   pJson,
    PCSTR               pcszKey,
    PLWCA_KEY           *ppValue
    )
{
    DWORD               dwError = 0;
    PBYTE               pJsonStr = NULL;
    PLWCA_KEY           pValue = NULL;

    if (!pJson || IsNullOrEmptyString(pcszKey) || !ppValue)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAGetString(pJson, pcszKey, (PSTR *)&pJsonStr);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACreateKey(pJsonStr, LwCAStringLenA(pJsonStr), &pValue);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppValue = pValue;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pJsonStr);
    return dwError;

error:
    LwCAFreeKey(pValue);
    if (ppValue)
    {
        *ppValue = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAJsonReplaceString(
    PCSTR               pcszType,
    PCSTR               pcszValue,
    PLWCA_JSON_OBJECT   pJson
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pElem = NULL;
    PLWCA_JSON_OBJECT   pObj = NULL;
    PLWCA_JSON_OBJECT   pValueArr = NULL;

    if (IsNullOrEmptyString(pcszType) ||
        IsNullOrEmptyString(pcszValue) ||
        !pJson
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonObjectCreate(&pElem);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetStringToObject(pElem, LWCA_LDAP_ATTR_TYPE, pcszType);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonArrayCreate(&pValueArr);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonAppendStringToArray(pValueArr, pcszValue);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetJsonToObject(pElem, LWCA_LDAP_ATTR_VALUE, pValueArr);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonObjectCreate(&pObj);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetStringToObject(pObj,
                                        LWCA_LDAP_OPERATION,
                                        LWCA_LDAP_REPLACE
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetJsonToObject(pObj, LWCA_LDAP_UPDATE_ATTR, pElem);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonAppendJsonToArray(pJson, pObj);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_JSON_DECREF(pValueArr);
    LWCA_SAFE_JSON_DECREF(pElem);
    LWCA_SAFE_JSON_DECREF(pObj);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_LwCAJsonReplaceCertArray(
    PCSTR                   pcszType,
    PLWCA_CERTIFICATE_ARRAY pValue,
    PLWCA_JSON_OBJECT       pJson
    )
{
    DWORD               dwError = 0;
    PLWCA_STRING_ARRAY  pStrArray = NULL;

    if (IsNullOrEmptyString(pcszType) || !pValue || !pJson)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCACreateStringArrayFromCertArray(pValue, &pStrArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAJsonReplaceStringArray(pcszType, pStrArray, pJson);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LwCAFreeStringArray(pStrArray);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_LwCAJsonReplaceStringArray(
    PCSTR               pcszType,
    PLWCA_STRING_ARRAY  pValue,
    PLWCA_JSON_OBJECT   pJson
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pElem = NULL;
    PLWCA_JSON_OBJECT   pObj = NULL;
    PLWCA_JSON_OBJECT   pValueArr = NULL;
    int                 i = 0;

    if (IsNullOrEmptyString(pcszType) || !pValue || !pJson)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonObjectCreate(&pElem);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetStringToObject(pElem, LWCA_LDAP_ATTR_TYPE, pcszType);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonArrayCreate(&pValueArr);
    BAIL_ON_LWCA_ERROR(dwError);

    for (i = 0; i < pValue->dwCount; ++i)
    {
        dwError = LwCAJsonAppendStringToArray(pValueArr, pValue->ppData[i]);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonSetJsonToObject(pElem, LWCA_LDAP_ATTR_VALUE, pValueArr);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonObjectCreate(&pObj);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetStringToObject(pObj,
                                        LWCA_LDAP_OPERATION,
                                        LWCA_LDAP_REPLACE
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetJsonToObject(pObj, LWCA_LDAP_UPDATE_ATTR, pElem);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonAppendJsonToArray(pJson, pObj);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_JSON_DECREF(pElem);
    LWCA_SAFE_JSON_DECREF(pValueArr);
    LWCA_SAFE_JSON_DECREF(pObj);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_LwCAJsonReplaceBytes(
    PCSTR               pcszType,
    PLWCA_KEY           pValue,
    PLWCA_JSON_OBJECT   pJson
    )
{
    DWORD               dwError = 0;

    if (IsNullOrEmptyString(pcszType) || !pValue || !pJson)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAJsonReplaceString(pcszType, (PSTR)pValue->pData, pJson);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_LwCAJsonReplaceInteger(
    PCSTR               pcszType,
    int                 value,
    PLWCA_JSON_OBJECT   pJson
    )
{
    DWORD   dwError = 0;
    PSTR    pszValue = NULL;

    dwError = LwCAAllocateStringPrintfA(&pszValue, "%d", value);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAJsonReplaceString(pcszType, pszValue, pJson);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszValue);
    return dwError;

error:
    goto cleanup;
}
