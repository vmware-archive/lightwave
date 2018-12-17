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
    SIZE_T              idx,
    PLWCA_JSON_OBJECT   *ppAttr
    );

static
DWORD
_LwCAGetAttributesFromResponse(
    PCSTR               pcszResponse,
    PLWCA_JSON_OBJECT   *ppAttr
    );

static
DWORD
_LwCAGetArrayAttributesFromResponse(
    PCSTR               pcszResponse,
    PLWCA_JSON_OBJECT   *ppAttr
    );

static
DWORD
_LwCAGetStringFromAttribute(
    PLWCA_JSON_OBJECT   pJson,
    PCSTR               pcszKey,
    PSTR                *ppszValue
    );

static
DWORD
_LwCAGetIntegerFromAttribute(
    PLWCA_JSON_OBJECT   pJson,
    PCSTR               pcszKey,
    int                 *pValue
    );

static
DWORD
_LwCAGetStringArrayFromAttribute(
    PLWCA_JSON_OBJECT   pJson,
    PCSTR               pcszKey,
    PLWCA_STRING_ARRAY  *ppValue
    );

static
DWORD
_LwCAGetBytesFromAttribute(
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
_LwCAJsonReplaceLong(
    PCSTR               pcszType,
    LONG                value,
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

static
DWORD
_LwCAGetEncodedStringArrayFromCertArray(
    PLWCA_CERTIFICATE_ARRAY     pCertArray,
    PLWCA_STRING_ARRAY          *ppEncodedStrArray
    );

DWORD
LwCASerializeContainerToJSON(
    PCSTR       pcszDN,
    PCSTR       pcszCN,
    PSTR        *ppszReqBody
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pRoot = NULL;
    PLWCA_JSON_OBJECT   pAttr = NULL;
    PSTR                pszReqBody = NULL;

    if (IsNullOrEmptyString(pcszDN) ||
        IsNullOrEmptyString(pcszCN) ||
        !ppszReqBody
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonObjectCreate(&pRoot);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetStringToObject(pRoot, LWCA_LDAP_DN, pcszDN);
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

    dwError = _JsonAttrStringCreate(pAttr, LWCA_LDAP_CN, pcszCN, NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetJsonToObject(pRoot, LWCA_POST_JSON_ATTR, pAttr);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonDumps(pRoot, JSON_DECODE_ANY, &pszReqBody);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszReqBody = pszReqBody;

cleanup:
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
    PCSTR               pcszDN,
    PSTR                *ppszReqBody
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pRoot = NULL;
    PLWCA_JSON_OBJECT   pAttr = NULL;
    PSTR                pszDN = NULL;
    PSTR                pszReqBody = NULL;
    PCSTR               pcszDNFormat = NULL;
    PBYTE               pEncoded = NULL;
    DWORD               dwLen = 0;
    PLWCA_STRING_ARRAY  pStrArray = NULL;

    if (IsNullOrEmptyString(pcszCAId) ||
        IsNullOrEmptyString(pcszDN) ||
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
        pcszDNFormat = LWCA_POST_ROOT_CA_DN_ENTRY;
    }
    else
    {
        pcszDNFormat = LWCA_POST_INTERMEDIATE_CA_DN_ENTRY;
    }

    dwError = LwCAAllocateStringPrintfA(&pszDN, pcszDNFormat, pcszCAId, pcszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetStringToObject(pRoot, LWCA_LDAP_DN, pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonArrayCreate(&pAttr);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _JsonAttrStringCreate(pAttr,
                                    LWCA_POST_OBJ_CLASS,
                                    LWCA_POST_CA_OBJ_CLASS,
                                    LWCA_POST_PKICA_AUX_CLASS,
                                    LWCA_POST_LOCK_OBJ_CLASS,
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
        pEncoded = pCAData->pEncryptedPrivateKey->pData;
        dwError = VmEncodeToBase64(pCAData->pEncryptedPrivateKey->pData,
                                   pCAData->pEncryptedPrivateKey->dwLength,
                                   &pEncoded,
                                   &dwLen
                                   );
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = _JsonAttrStringCreate(pAttr,
                                        LWCA_POST_CA_ENCR_PRIV_KEY,
                                        pEncoded,
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
        dwError = _LwCAGetEncodedStringArrayFromCertArray(pCAData->pCertificates,
                                                          &pStrArray
                                                          );
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = _JsonAttrStringArrayCreate(pAttr,
                                             LWCA_POST_CA_CERTIFICATES,
                                             pStrArray
                                             );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCAData->pszLastCRLUpdate))
    {
        dwError = _JsonAttrStringCreate(pAttr,
                                        LWCA_POST_CA_LAST_CRL_UPDATE,
                                        pCAData->pszLastCRLUpdate,
                                        NULL
                                        );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCAData->pszNextCRLUpdate))
    {
        dwError = _JsonAttrStringCreate(pAttr,
                                        LWCA_POST_CA_NEXT_CRL_UPDATE,
                                        pCAData->pszNextCRLUpdate,
                                        NULL
                                        );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCAData->pszAuthBlob))
    {
        dwError = _JsonAttrStringCreate(pAttr,
                                        LWCA_POST_CA_AUTH_BLOB,
                                        pCAData->pszAuthBlob,
                                        NULL
                                        );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    // insert locking mechanism's metadata
    dwError = _JsonAttrStringCreate(pAttr,
                                    LWCA_POST_ATTR_LOCK_OWNER,
                                    LWCA_LOCK_UNOWNED,
                                    NULL
                                    );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _JsonAttrIntCreate(pAttr,
                                 LWCA_POST_ATTR_LOCK_EXPIRE,
                                 1,
                                 LWCA_LOCK_UNOWNED_EXPIRE_TIME
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _JsonAttrIntCreate(pAttr,
                                 LWCA_POST_CA_STATUS,
                                 1,
                                 pCAData->status
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetJsonToObject(pRoot, LWCA_POST_JSON_ATTR, pAttr);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonDumps(pRoot, JSON_DECODE_ANY, &pszReqBody);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszReqBody = pszReqBody;

cleanup:
    LwCAFreeStringArray(pStrArray);
    LWCA_SAFE_FREE_STRINGA(pszDN);
    LWCA_SAFE_FREE_MEMORY(pEncoded);
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
LwCAGetStringAttrFromResponse(
    PCSTR       pcszResponse,
    PCSTR       pcszKey,
    PSTR        *ppszAttrValue
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pAttrJson = NULL;
    PSTR                pszAttrValue = NULL;

    if (IsNullOrEmptyString(pcszResponse) ||
        IsNullOrEmptyString(pcszKey) ||
        !ppszAttrValue
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAGetAttributesFromResponse(pcszResponse, &pAttrJson);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetStringFromAttribute(pAttrJson, pcszKey, &pszAttrValue);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszAttrValue = pszAttrValue;

cleanup:
    LWCA_SAFE_JSON_DECREF(pAttrJson);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszAttrValue);
    if (ppszAttrValue)
    {
        *ppszAttrValue = NULL;
    }
    goto cleanup;
}

DWORD
LwCAGetStringArrayAttrFromResponse(
    PCSTR               pcszResponse,
    PCSTR               pcszKey,
    PLWCA_STRING_ARRAY  *ppStrArray
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pAttrJson = NULL;
    PLWCA_STRING_ARRAY  pStrArray = NULL;

    if (IsNullOrEmptyString(pcszResponse) ||
        IsNullOrEmptyString(pcszKey) ||
        !ppStrArray
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAGetAttributesFromResponse(pcszResponse, &pAttrJson);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetStringArrayFromAttribute(pAttrJson, pcszKey, &pStrArray);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppStrArray = pStrArray;

cleanup:
    LWCA_SAFE_JSON_DECREF(pAttrJson);
    return dwError;

error:
    LwCAFreeStringArray(pStrArray);
    if (ppStrArray)
    {
        *ppStrArray = NULL;
    }

    goto cleanup;
}

DWORD
LwCAGetIntAttrFromResponse(
    PCSTR       pcszResponse,
    PCSTR       pcszKey,
    int         *pValue
    )
{
    DWORD       dwError = 0;
    PSTR        pszValue = NULL;
    PSTR        pszTmp = NULL;
    int         value = 0;

    if (IsNullOrEmptyString(pcszResponse) ||
        IsNullOrEmptyString(pcszKey) ||
        !pValue
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAGetStringAttrFromResponse(pcszResponse, pcszKey, &pszValue);
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
        pValue = 0;
    }
    goto cleanup;

}

DWORD
LwCASerializeCertDataToJSON(
    PCSTR               pcszCAID,
    PLWCA_DB_CERT_DATA  pCertData,
    PCSTR               pcszCADN,
    PSTR                *ppszReqBody
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pRoot = NULL;
    PLWCA_JSON_OBJECT   pAttr = NULL;
    PSTR                pszReqBody = NULL;
    PSTR                pszCertDN = NULL;

    if (IsNullOrEmptyString(pcszCAID) ||
        IsNullOrEmptyString(pcszCADN) ||
        !pCertData ||
        IsNullOrEmptyString(pCertData->pszSerialNumber) ||
        !ppszReqBody
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonObjectCreate(&pRoot);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringPrintfA(&pszCertDN,
                                        LWCA_POST_CERT_DN,
                                        pCertData->pszSerialNumber,
                                        pcszCADN
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetStringToObject(pRoot, LWCA_LDAP_DN, pszCertDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonArrayCreate(&pAttr);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _JsonAttrStringCreate(pAttr,
                                    LWCA_POST_OBJ_CLASS,
                                    LWCA_POST_CERT_OBJ_CLASS,
                                    LWCA_POST_LOCK_OBJ_CLASS,
                                    NULL
                                    );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _JsonAttrStringCreate(pAttr,
                                    LWCA_LDAP_CN,
                                    pCertData->pszSerialNumber,
                                    NULL
                                    );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _JsonAttrStringCreate(pAttr,
                                    LWCA_POST_CERT_SERIAL_NUM,
                                    pCertData->pszSerialNumber,
                                    NULL
                                    );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _JsonAttrStringCreate(pAttr,
                                    LWCA_POST_CERT_ISSUER,
                                    pcszCAID,
                                    NULL
                                    );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _JsonAttrIntCreate(pAttr,
                                 LWCA_POST_CERT_REVOKED_REASON,
                                 1,
                                 pCertData->revokedReason
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    if (!IsNullOrEmptyString(pCertData->pszRevokedDate))
    {
        dwError = _JsonAttrStringCreate(pAttr,
                                        LWCA_POST_CERT_REVOKED_DATE,
                                        pCertData->pszRevokedDate,
                                        NULL
        );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCertData->pszTimeValidFrom))
    {
        dwError = _JsonAttrStringCreate(pAttr,
                                        LWCA_POST_CERT_TIME_VALID_FROM,
                                        pCertData->pszTimeValidFrom,
                                        NULL
        );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCertData->pszTimeValidTo))
    {
        dwError = _JsonAttrStringCreate(pAttr,
                                        LWCA_POST_CERT_TIME_VALID_TO,
                                        pCertData->pszTimeValidTo,
                                        NULL
        );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    // insert locking mechanism's metadata
    dwError = _JsonAttrStringCreate(pAttr,
                                    LWCA_POST_ATTR_LOCK_OWNER,
                                    LWCA_LOCK_UNOWNED,
                                    NULL
                                    );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _JsonAttrIntCreate(pAttr,
                                 LWCA_POST_ATTR_LOCK_EXPIRE,
                                 1,
                                 LWCA_LOCK_UNOWNED_EXPIRE_TIME
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _JsonAttrIntCreate(pAttr,
                                 LWCA_POST_CERT_STATUS,
                                 1,
                                 pCertData->status
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonSetJsonToObject(pRoot, LWCA_POST_JSON_ATTR, pAttr);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonDumps(pRoot, JSON_DECODE_ANY, &pszReqBody);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszReqBody = pszReqBody;

cleanup:
    LWCA_SAFE_JSON_DECREF(pRoot);
    LWCA_SAFE_JSON_DECREF(pAttr);
    LWCA_SAFE_FREE_STRINGA(pszCertDN);
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
LwCADeserializeJSONToCertData(
    PCSTR                       pcszResponse,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCertDataArray
    )
{
    DWORD                       dwError = 0;
    PLWCA_DB_CERT_DATA_ARRAY    pCertDataArray = NULL;
    PLWCA_JSON_OBJECT           pAttrArray = NULL;
    PLWCA_JSON_OBJECT           pJson = NULL;
    PSTR                        pszSerialNumber = NULL;
    DWORD                       dwRevokedReason = 0;
    PSTR                        pszRevokedDate = NULL;
    PSTR                        pszTimeValidFrom = NULL;
    PSTR                        pszTimeValidTo = NULL;
    int                         status = 0;
    DWORD                       idx = 0;
    SIZE_T                      arrLen = 0;

    if (IsNullOrEmptyString(pcszResponse) || !ppCertDataArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAGetArrayAttributesFromResponse(pcszResponse, &pAttrArray);
    BAIL_ON_LWCA_ERROR(dwError);

    arrLen = LwCAJsonArraySize(pAttrArray);
    if (arrLen == 0)
    {
        goto cleanup;
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CERT_DATA_ARRAY),
                                 (PVOID *)&pCertDataArray
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    pCertDataArray->dwCount = arrLen;

    dwError = LwCAAllocateMemory(sizeof(PLWCA_DB_CERT_DATA) * pCertDataArray->dwCount,
                                 (PVOID*)&pCertDataArray->ppCertData
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    json_array_foreach(pAttrArray, idx, pJson)
    {
        dwError = _LwCAGetStringFromAttribute(pJson,
                                              LWCA_POST_CERT_SERIAL_NUM,
                                              &pszSerialNumber
                                              );
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = _LwCAGetIntegerFromAttribute(pJson,
                                               LWCA_POST_CERT_REVOKED_REASON,
                                               (int *)&dwRevokedReason
                                               );
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = _LwCAGetStringFromAttribute(pJson,
                                              LWCA_POST_CERT_REVOKED_DATE,
                                              &pszRevokedDate
                                              );
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = _LwCAGetStringFromAttribute(pJson,
                                              LWCA_POST_CERT_TIME_VALID_FROM,
                                              &pszTimeValidFrom
                                              );
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = _LwCAGetStringFromAttribute(pJson,
                                              LWCA_POST_CERT_TIME_VALID_TO,
                                              &pszTimeValidTo
                                              );
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = _LwCAGetIntegerFromAttribute(pJson,
                                               LWCA_POST_CERT_STATUS,
                                               &status
                                               );
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCADbCreateCertData(pszSerialNumber,
                                       pszTimeValidFrom,
                                       pszTimeValidTo,
                                       dwRevokedReason,
                                       pszRevokedDate,
                                       status,
                                       &pCertDataArray->ppCertData[idx]
                                       );
        BAIL_ON_LWCA_ERROR(dwError);

        LWCA_SAFE_FREE_STRINGA(pszSerialNumber);
        LWCA_SAFE_FREE_STRINGA(pszRevokedDate);
        LWCA_SAFE_FREE_STRINGA(pszTimeValidFrom);
        LWCA_SAFE_FREE_STRINGA(pszTimeValidTo);
    }

    *ppCertDataArray = pCertDataArray;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszSerialNumber);
    LWCA_SAFE_FREE_STRINGA(pszRevokedDate);
    LWCA_SAFE_FREE_STRINGA(pszTimeValidFrom);
    LWCA_SAFE_FREE_STRINGA(pszTimeValidTo);
    LWCA_SAFE_JSON_DECREF(pAttrArray);

    return dwError;

error:
    LwCADbFreeCertDataArray(pCertDataArray);
    if (ppCertDataArray)
    {
        *ppCertDataArray = NULL;
    }
    goto cleanup;
}

DWORD
LwCADeserializeJSONToCA(
    PCSTR               pcszResponse,
    PLWCA_DB_CA_DATA    *ppCaData
    )
{
    DWORD                       dwError = 0;
    PLWCA_JSON_OBJECT           pAttrJson = NULL;
    PSTR                        pszSubjectName = NULL;
    PLWCA_STRING_ARRAY          pszCertArray = NULL;
    PLWCA_CERTIFICATE_ARRAY     pCertArray = NULL;
    PSTR                        pszCRLNumber = NULL;
    PSTR                        pszLastCRLUpdate = NULL;
    PSTR                        pszNextCRLUpdate = NULL;
    PSTR                        pszAuthBlob = NULL;
    int                         caStatus = 0;
    PLWCA_KEY                   pEncryptedPrivateKey = NULL;
    PLWCA_DB_CA_DATA            pCaData = NULL;

    if (IsNullOrEmptyString(pcszResponse) || !ppCaData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAGetAttributesFromResponse(pcszResponse, &pAttrJson);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetStringFromAttribute(pAttrJson,
                                          LWCA_POST_CA_SUBJ_NAME,
                                          &pszSubjectName
                                          );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetStringArrayFromAttribute(pAttrJson,
                                               LWCA_POST_CA_CERTIFICATES,
                                               &pszCertArray
                                               );
    BAIL_ON_LWCA_ERROR(dwError);

    if (pszCertArray)
    {
        dwError = LwCAGetCertArrayFromEncodedStringArray(pszCertArray, &pCertArray);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAGetStringFromAttribute(pAttrJson,
                                          LWCA_POST_CA_CRL_NUM,
                                          &pszCRLNumber
                                          );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetIntegerFromAttribute(pAttrJson,
                                           LWCA_POST_CA_STATUS,
                                           &caStatus
                                           );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetBytesFromAttribute(pAttrJson,
                                         LWCA_POST_CA_ENCR_PRIV_KEY,
                                         &pEncryptedPrivateKey
                                         );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetStringFromAttribute(pAttrJson,
                                          LWCA_POST_CA_NEXT_CRL_UPDATE,
                                          &pszNextCRLUpdate
                                          );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetStringFromAttribute(pAttrJson,
                                          LWCA_POST_CA_LAST_CRL_UPDATE,
                                          &pszLastCRLUpdate
                                          );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetStringFromAttribute(pAttrJson,
                                          LWCA_POST_CA_AUTH_BLOB,
                                          &pszAuthBlob
                                          );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateCAData(pszSubjectName,
                                 pCertArray,
                                 pEncryptedPrivateKey,
                                 pszCRLNumber,
                                 pszLastCRLUpdate,
                                 pszNextCRLUpdate,
                                 pszAuthBlob,
                                 caStatus,
                                 &pCaData
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCaData = pCaData;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszSubjectName);
    LWCA_SAFE_FREE_STRINGA(pszLastCRLUpdate);
    LWCA_SAFE_FREE_STRINGA(pszNextCRLUpdate);
    LWCA_SAFE_FREE_STRINGA(pszAuthBlob);
    LwCAFreeStringArray(pszCertArray);
    LwCAFreeCertificates(pCertArray);
    LWCA_SAFE_FREE_STRINGA(pszCRLNumber);
    LwCAFreeKey(pEncryptedPrivateKey);
    LWCA_SAFE_JSON_DECREF(pAttrJson);
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
LwCAGenerateLockRequestBody(
    PCSTR           pcszUuid,
    ULONG           expireTime,
    PSTR            *ppszReqBody
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pJsonBody = NULL;
    PSTR                pszReqBody = NULL;

    if (IsNullOrEmptyString(pcszUuid) || !ppszReqBody)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonArrayCreate(&pJsonBody);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAJsonReplaceString(LWCA_POST_ATTR_LOCK_OWNER,
                                     pcszUuid,
                                     pJsonBody
                                     );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAJsonReplaceLong(LWCA_POST_ATTR_LOCK_EXPIRE,
                                   expireTime,
                                   pJsonBody
                                   );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonDumps(pJsonBody, JSON_DECODE_ANY, &pszReqBody);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszReqBody = pszReqBody;

cleanup:
    LWCA_SAFE_JSON_DECREF(pJsonBody);
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
LwCAGenerateCertPatchRequestBody(
    PLWCA_DB_CERT_DATA  pCertData,
    PSTR                *ppszBody
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pJsonBody = NULL;
    PSTR                pszBody = NULL;

    if (!pCertData || !ppszBody)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonArrayCreate(&pJsonBody);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAJsonReplaceInteger(LWCA_POST_CERT_REVOKED_REASON,
                                      pCertData->revokedReason,
                                      pJsonBody
                                      );
    BAIL_ON_LWCA_ERROR(dwError);

    if (!IsNullOrEmptyString(pCertData->pszRevokedDate))
    {
        dwError = _LwCAJsonReplaceString(LWCA_POST_CERT_REVOKED_DATE,
                                         pCertData->pszRevokedDate,
                                         pJsonBody
                                         );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCertData->pszTimeValidFrom))
    {
        dwError = _LwCAJsonReplaceString(LWCA_POST_CERT_TIME_VALID_FROM,
                                         pCertData->pszTimeValidFrom,
                                         pJsonBody
                                         );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCertData->pszTimeValidTo))
    {
        dwError = _LwCAJsonReplaceString(LWCA_POST_CERT_TIME_VALID_TO,
                                         pCertData->pszTimeValidTo,
                                         pJsonBody
                                         );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAJsonReplaceInteger(LWCA_POST_CERT_STATUS,
                                      pCertData->status,
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
        *ppszBody = NULL;
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

    if (!IsNullOrEmptyString(pCaData->pszLastCRLUpdate))
    {
        dwError = _LwCAJsonReplaceString(LWCA_POST_CA_LAST_CRL_UPDATE,
                                         pCaData->pszLastCRLUpdate,
                                         pJsonBody
                                         );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCaData->pszNextCRLUpdate))
    {
        dwError = _LwCAJsonReplaceString(LWCA_POST_CA_NEXT_CRL_UPDATE,
                                         pCaData->pszNextCRLUpdate,
                                         pJsonBody
                                         );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCaData->pszAuthBlob))
    {
        dwError = _LwCAJsonReplaceString(LWCA_POST_CA_AUTH_BLOB,
                                         pCaData->pszAuthBlob,
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
    SIZE_T              idx,
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

    dwError = LwCAJsonArrayGetBorrowedRef(pResult, idx, &pResultElem);
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
_LwCAGetStringFromAttribute(
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

    dwError = LwCAJsonGetObjectFromKey(pJson, TRUE, pcszKey, &pArray);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pArray)
    {
        dwError = LwCAJsonArrayGetStringAtIndex(pArray, 0, &pszValue);
        BAIL_ON_LWCA_ERROR(dwError);
    }

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
_LwCAGetIntegerFromAttribute(
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

    dwError = LwCAJsonGetObjectFromKey(pJson, TRUE, pcszKey, &pArray);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pArray)
    {
        dwError = LwCAJsonArrayGetStringAtIndex(pArray, 0, &pszValue);
        BAIL_ON_LWCA_ERROR(dwError);

        value = (int)strtol(pszValue, &pszTmp, 10);

        if (errno)
        {
            dwError = errno + LWCA_ERROR_BASE + LWCA_ERRNO_BASE;
            BAIL_ON_LWCA_ERROR(dwError);
        }
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
_LwCAGetStringArrayFromAttribute(
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
    dwError = LwCAJsonGetObjectFromKey(pJson, TRUE, pcszKey, &pArray);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pArray)
    {

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
_LwCAGetBytesFromAttribute(
    PLWCA_JSON_OBJECT   pJson,
    PCSTR               pcszKey,
    PLWCA_KEY           *ppValue
    )
{
    DWORD               dwError = 0;
    PSTR                pszJsonStr = NULL;
    DWORD               dwLen = 0;
    PBYTE               pDecoded = NULL;
    DWORD               decodedLen = 0;
    PLWCA_KEY           pValue = NULL;

    if (!pJson || IsNullOrEmptyString(pcszKey) || !ppValue)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAGetStringFromAttribute(pJson, pcszKey, &pszJsonStr);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!IsNullOrEmptyString(pszJsonStr))
    {
        dwLen = LwCAStringLenA(pszJsonStr);
        dwError = VmDecodeToBase64((PBYTE)pszJsonStr, dwLen, &pDecoded, &decodedLen);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCACreateKey(pDecoded, decodedLen, &pValue);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppValue = pValue;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszJsonStr);
    LWCA_SAFE_FREE_MEMORY(pDecoded);
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

    dwError = _LwCAGetEncodedStringArrayFromCertArray(pValue, &pStrArray);
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
    DWORD               dwLen = 0;
    PBYTE               pEncoded = NULL;

    if (IsNullOrEmptyString(pcszType) || !pValue || !pJson)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = VmEncodeToBase64(pValue->pData,
                               pValue->dwLength,
                               &pEncoded,
                               &dwLen
                               );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAJsonReplaceString(pcszType, (PSTR)pEncoded, pJson);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_MEMORY(pEncoded);
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

static
DWORD
_LwCAJsonReplaceLong(
    PCSTR               pcszType,
    LONG                value,
    PLWCA_JSON_OBJECT   pJson
    )
{
    DWORD   dwError = 0;
    PSTR    pszValue = NULL;

    dwError = LwCAAllocateStringPrintfA(&pszValue, "%ld", value);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAJsonReplaceString(pcszType, pszValue, pJson);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszValue);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_LwCAGetArrayAttributesFromResponse(
    PCSTR               pcszResponse,
    PLWCA_JSON_OBJECT   *ppAttr
    )
{
    DWORD               dwError = 0;
    DWORD               dwCount = 0;
    PLWCA_JSON_OBJECT   pRespJson = NULL;
    PLWCA_JSON_OBJECT   pAttrJson = NULL;
    PLWCA_JSON_OBJECT   pAttrEntry = NULL;
    SIZE_T              idx = 0;

    if (IsNullOrEmptyString(pcszResponse) || !ppAttr)
    {
        dwError =  LWCA_ERROR_INVALID_PARAMETER;
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

    dwError = LwCAJsonArrayCreate(&pAttrJson);
    BAIL_ON_LWCA_ERROR(dwError);

    for (idx = 0; idx < dwCount; ++idx)
    {
        dwError = _LwCAGetAttributes(pRespJson, idx, &pAttrEntry);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAJsonAppendJsonToArray(pAttrJson, pAttrEntry);
        BAIL_ON_LWCA_ERROR(dwError);

        LWCA_SAFE_JSON_DECREF(pAttrEntry);
    }

    *ppAttr = pAttrJson;

cleanup:
    LWCA_SAFE_JSON_DECREF(pAttrEntry);
    LWCA_SAFE_JSON_DECREF(pRespJson);
    return dwError;

error:
    LWCA_SAFE_JSON_DECREF(pAttrJson);
    if (ppAttr)
    {
        *ppAttr = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAGetAttributesFromResponse(
    PCSTR               pcszResponse,
    PLWCA_JSON_OBJECT   *ppAttr
    )
{
    DWORD               dwError = 0;
    DWORD               dwCount = 0;
    PLWCA_JSON_OBJECT   pRespJson = NULL;
    PLWCA_JSON_OBJECT   pAttrJson = NULL;

    if (IsNullOrEmptyString(pcszResponse) || !ppAttr)
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

    dwError = _LwCAGetAttributes(pRespJson, 0, &pAttrJson);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppAttr = pAttrJson;

cleanup:
    LWCA_SAFE_JSON_DECREF(pRespJson);
    return dwError;

error:
    LWCA_SAFE_JSON_DECREF(pAttrJson);
    if (ppAttr)
    {
        *ppAttr = NULL;
    }
    goto cleanup;

}

static
DWORD
_LwCAGetEncodedStringArrayFromCertArray(
    PLWCA_CERTIFICATE_ARRAY     pCertArray,
    PLWCA_STRING_ARRAY          *ppEncodedStrArray
    )
{
    DWORD                   dwError = 0;
    DWORD                   dwEntry = 0;
    DWORD                   dwLen = 0;
    DWORD                   dwCertLen = 0;
    PLWCA_STRING_ARRAY      pEncodedStrArray = NULL;
    PBYTE                   pInput = NULL;
    PBYTE                   *ppEncodedOutput = NULL;

    if (!pCertArray || !ppEncodedStrArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_STRING_ARRAY),
                                 (PVOID*)&pEncodedStrArray
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    pEncodedStrArray->dwCount = pCertArray->dwCount;

    dwError = LwCAAllocateMemory(sizeof(PSTR) * pEncodedStrArray->dwCount,
                                 (PVOID *)&pEncodedStrArray->ppData
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    for(; dwEntry < pEncodedStrArray->dwCount; ++dwEntry)
    {
        dwCertLen = LwCAStringLenA(pCertArray->ppCertificates[dwEntry]);

        pInput = (PBYTE)pCertArray->ppCertificates[dwEntry];
        ppEncodedOutput = (PBYTE *)&pEncodedStrArray->ppData[dwEntry];

        dwError = VmEncodeToBase64(pInput, dwCertLen, ppEncodedOutput, &dwLen);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppEncodedStrArray = pEncodedStrArray;

cleanup:
    return dwError;

error:
    LwCAFreeStringArray(pEncodedStrArray);
    if (ppEncodedStrArray)
    {
        *ppEncodedStrArray = NULL;
    }
    goto cleanup;
}

DWORD
LwCAGetCertArrayFromEncodedStringArray(
    PLWCA_STRING_ARRAY          pEncodedStrArray,
    PLWCA_CERTIFICATE_ARRAY     *ppCertArray
    )
{
    DWORD                       dwError = 0;
    DWORD                       dwEntry = 0;
    DWORD                       dwLen = 0;
    DWORD                       dwCertLen = 0;
    PLWCA_CERTIFICATE_ARRAY     pCertArray = NULL;
    PBYTE                       pInput = NULL;
    PBYTE                       *ppDecodedOutput = NULL;

    if (!pEncodedStrArray || !ppCertArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_CERTIFICATE_ARRAY),
                                 (PVOID*)&pCertArray
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    pCertArray->dwCount = pEncodedStrArray->dwCount;

    dwError = LwCAAllocateMemory(sizeof(PSTR) * pCertArray->dwCount,
                                 (PVOID *)&pCertArray->ppCertificates
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    for (; dwEntry < pEncodedStrArray->dwCount; ++dwEntry)
    {
        dwLen = LwCAStringLenA(pEncodedStrArray->ppData[dwEntry]);

        pInput = (PBYTE)pEncodedStrArray->ppData[dwEntry];
        ppDecodedOutput = (PBYTE *)&pCertArray->ppCertificates[dwEntry];

        dwError = VmDecodeToBase64(pInput, dwLen, ppDecodedOutput, &dwCertLen);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppCertArray = pCertArray;

cleanup:
    return dwError;

error:
    LwCAFreeCertificates(pCertArray);
    if (ppCertArray)
    {
        *ppCertArray = NULL;
    }
    goto cleanup;
}
