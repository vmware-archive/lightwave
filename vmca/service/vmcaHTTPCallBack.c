/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ~@~\License~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ~@~\AS IS~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
#include "includes.h"

#ifndef _WIN32

DWORD
VMCARESTWritePayload(
    PCSTR pszPayloadContents,
    PCSTR pszContentName,
    PSTR* ppszResponsePayload
    )
{
    DWORD dwError = 0;
    PSTR pszResponsePayload = NULL;
    PSTR pszSuccessMessage = VMCA_SUCCESS_MESSAGE;
    int nJsonSymbolLength = 8;
    int nCursor = 0;
    int nContentLength = 0;
    int nNameLength = 0;
    int nTotalLength = 0;

    if (!ppszResponsePayload)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pszPayloadContents == NULL )
    {
        dwError = VMCAAllocateStringA(pszSuccessMessage, &pszResponsePayload);
        BAIL_ON_VMREST_ERROR(dwError);
    } else
    {
        nContentLength = strlen(pszPayloadContents);
        nNameLength = strlen(pszContentName);
        nTotalLength = nContentLength + nNameLength + nJsonSymbolLength;

        dwError = VMCAAllocateMemory(
                            nTotalLength + 1,
                            (PVOID*) &pszResponsePayload
                            );
        BAIL_ON_VMCA_ERROR(dwError);

        nCursor = 0;
        strcpy(pszResponsePayload, "{\"");
        nCursor += 2;
        strcpy(pszResponsePayload+nCursor, pszContentName);
        nCursor += nNameLength;
        strcpy(pszResponsePayload+nCursor, "\": \"");
        nCursor += 4;
        strcpy(pszResponsePayload+nCursor, pszPayloadContents);
        nCursor += nContentLength;
        strcpy(pszResponsePayload+nCursor, "\"}\0");
    }
    *ppszResponsePayload = pszResponsePayload;
cleanup:
    return dwError;
error:
    VMCA_SAFE_FREE_MEMORY(pszResponsePayload);
    goto cleanup;
}

DWORD
VMCARemoveNewlineChars(
    PSTR pszString,
    PSTR* ppszResult
    )
{
    DWORD dwError = 0;
    PSTR pszRep = "\n";
    PSTR pszWith = "\\n";
    PSTR pszResult; // the return string
    PSTR pszIns;    // the next insert point
    PSTR pszTmp;    // varies
    int nRepLen;  // length of pszRep (the string to remove)
    int nWithLen; // length of pszWith (the string to replace pszRep pszWith)
    int nRepLastToNext; // distance between pszRep and end of last pszRep
    int nCounter;    // number of replacements

    if (!ppszResult)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMREST_ERROR(dwError);
    }

    if (!pszString || strlen(pszString) == 0)
    {
        goto cleanup;
    }

    nRepLen = strlen(pszRep);
    nWithLen = strlen(pszWith);
    pszIns = pszString; // count the number of replacements needed
    for (nCounter = 0; (pszTmp = strstr(pszIns, pszRep)); ++nCounter)
    {
        pszIns = pszTmp + nRepLen;
    }

    dwError = VMCAAllocateMemory(
                        strlen(pszString) + (nWithLen - nRepLen) * nCounter + 1,
                        (PVOID*) &pszTmp
                        );
    BAIL_ON_VMREST_ERROR(dwError);
    pszResult = pszTmp;

    while (nCounter--) {
        pszIns = strstr(pszString, pszRep);
        nRepLastToNext = pszIns - pszString;
        pszTmp = strncpy(pszTmp, pszString, nRepLastToNext) + nRepLastToNext;
        pszTmp = strcpy(pszTmp, pszWith) + nWithLen;
        pszString += nRepLastToNext + nRepLen;
    }
    strcpy(pszTmp, pszString);

    *ppszResult = pszResult;

cleanup:
    return dwError;

error:
    goto cleanup;

}

DWORD
VMCARESTWriteEnumCerts(
    VMCA_CERTIFICATE_ARRAY* pTempCertArray,
    PSTR* ppszResponsePayload
    )
{
    DWORD dwError = 0;
    PSTR pszResponsePayload = NULL;
    PSTR pszTemp = NULL;
    int nResponsePayloadSize = 0;
    int nCursor = 0;
    int nCounter = 0;

    if (!pTempCertArray || !ppszResponsePayload)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pTempCertArray->certificates == NULL || !pTempCertArray->dwCount)
    {
        dwError = VMCARESTWritePayload(
                                " ",
                                "cert",
                                &pszResponsePayload
                                );
        BAIL_ON_VMCA_ERROR(dwError);
    } else
    {
        nResponsePayloadSize = 7 + strlen(VMCA_ENUM_CERTS_RETURN_KEY);
        nCursor = 0;

        dwError = VMCAAllocateMemory(
                                    nResponsePayloadSize + 1,
                                    (PVOID*) &pszResponsePayload
                                    );
        BAIL_ON_VMCA_ERROR(dwError);

        strcpy(pszResponsePayload, "{\"");
        nCursor += 2;
        strcpy(pszResponsePayload+nCursor, VMCA_ENUM_CERTS_RETURN_KEY);
        nCursor += strlen(VMCA_ENUM_CERTS_RETURN_KEY);
        strcpy(pszResponsePayload+nCursor, "\": [\n");
        nCursor += 5;

        for (nCounter = 0; nCounter < (int)pTempCertArray->dwCount; nCounter++)
        {
            dwError = VMCARemoveNewlineChars(
                                    pTempCertArray->certificates[nCounter].pCert,
                                    &pszTemp
                                    );
            BAIL_ON_VMCA_ERROR(dwError);

            nCursor = nResponsePayloadSize;
            nResponsePayloadSize += strlen(pszTemp);
            nResponsePayloadSize += 4;

            dwError = VMCAReallocateMemory(
                                        pszResponsePayload,
                                        (PVOID*) &pszResponsePayload,
                                        nResponsePayloadSize + 1
                                        );
            BAIL_ON_VMCA_ERROR(dwError);

            strcpy(pszResponsePayload + nCursor, "\"");
            ++nCursor;
            strcpy(pszResponsePayload + nCursor, pszTemp);
            nCursor += strlen(pszTemp);
            strcpy(pszResponsePayload + nCursor, "\",\n");
            nCursor += 3;
            VMCA_SAFE_FREE_MEMORY(pszTemp);
        }

        pszResponsePayload[strlen(pszResponsePayload)-2] = ']';
        pszResponsePayload[strlen(pszResponsePayload)-1] = '}';
    }

    *ppszResponsePayload = pszResponsePayload;

cleanup:

    return dwError;
error:
    VMCA_SAFE_FREE_MEMORY(pszResponsePayload);
    VMCA_SAFE_FREE_MEMORY(pszTemp);
    goto cleanup;
}

DWORD
VMCARESTWriteStatus(
    PCSTR pszStatus,
    PSTR* ppszStatusCode
    )
{
    return VMCAAllocateStringA(pszStatus, ppszStatusCode);
}

DWORD
VMCARESTStringToInt(
    PCSTR pszStr,
    int* pnInt
    )
{
    DWORD dwError = 0;
    PSTR pszEndptr;
    errno = 0;
    long l;

    if (pszStr == NULL ||
        pnInt == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMREST_ERROR(dwError);
    }

    l = strtol(pszStr, &pszEndptr, 0);

    if (errno == ERANGE ||
        *pszEndptr != '\0' ||
        pszStr == pszEndptr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMREST_ERROR(dwError);
    }

    if  (l < INT_MIN ||
        l > INT_MAX)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMREST_ERROR(dwError);
    }

    *pnInt = (int) l;

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VMCAConvertStringInputToJSON(
    const char *pszString,
    json_t **ppJsonObject
    )
{
    DWORD dwError = 0;
    json_t *pObject = NULL;
    json_error_t stError = {0};

    if(IsNullOrEmptyString(pszString) || !ppJsonObject)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pObject = json_loads(pszString, 0, &stError);
    if(!pObject)
    {
        dwError = ERROR_INVALID_PARAMETER;
    }
    BAIL_ON_VMCA_ERROR(dwError);

    *ppJsonObject = pObject;
cleanup:
    return dwError;

error:
    if(pObject)
    {
        json_decref(pObject);
    }
    goto cleanup;
}

DWORD
VMCAHandleEnumCertsParam(
    PSTR pszKey1,
    PSTR pszVal1,
    PSTR pszKey2,
    PSTR pszVal2,
    PSTR* ppszFlag,
    PSTR* ppszNum
    )
{
    DWORD dwError = 0;
    PSTR pszFlag = NULL;
    PSTR pszNum = NULL;

    if (!pszKey1 || !pszKey2 || !pszVal1 || !pszVal2 || !ppszFlag || !ppszNum)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if  (!VMCAStringCompareA(pszKey1, VMCA_ENUM_CERTS_PARAM_KEY_FLAG, 0) )
    {
        pszFlag = pszVal1;
    } else
    if  (!VMCAStringCompareA(pszKey1, VMCA_ENUM_CERTS_PARAM_KEY_NUMBER, 0) )
    {
        pszNum = pszVal1;
    } else
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if  (!VMCAStringCompareA(pszKey2, VMCA_ENUM_CERTS_PARAM_KEY_FLAG, 0) )
    {
        pszFlag = pszVal2;
    } else
    if  (!VMCAStringCompareA(pszKey2, VMCA_ENUM_CERTS_PARAM_KEY_NUMBER, 0) )
    {
        pszNum = pszVal2;
    } else
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!pszFlag || !pszNum)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    *ppszFlag = pszFlag;
    *ppszNum= pszNum;
cleanup:
    return dwError;
error:
    goto cleanup;

}

DWORD
VMCAParseQuery(
    PSTR pszQuery,
    PSTR **pppszParams,
    int** ppnCount
    )
{
    DWORD dwError = 0;
    PSTR *ppszParams = NULL;
    int *pnCount = NULL;
    int nCount = 0;
    int nSizeToAllocate = 0;
    int nNotDone = 1;
    int nIndex = 0;
    PSTR pszUrl = NULL;
    PSTR pszRemaining = NULL;
    PSTR pszKey = NULL;
    PSTR pszVal = NULL;

    if (!pppszParams || !pszQuery)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAStringCountSubstring(pszQuery, "=", &pnCount);
    BAIL_ON_VMCA_ERROR(dwError);

    nCount = *pnCount;

    if (nCount == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    nSizeToAllocate = 2*nCount*sizeof(char*);
    dwError = VMCAAllocateMemory(nSizeToAllocate, (PVOID*) &ppszParams);
    BAIL_ON_VMCA_ERROR(dwError);

    pszUrl = VMCAStringTokA(pszQuery, "?", &pszRemaining);
    if (VMCAStringCompareA(pszRemaining, "", 0) == 0 )
    {
        dwError = VMCA_ERROR_INVALID_URI;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    nNotDone = 1;
    while(nNotDone)
    {

        pszKey = VMCAStringTokA(NULL, "=", &pszRemaining);
        if (VMCAStringCompareA(pszRemaining, "", 0) == 0 )
        {
            dwError = VMCA_ERROR_INVALID_URI;
            BAIL_ON_VMCA_ERROR(dwError);
        } else
        {
            dwError = VMCAAllocateStringA(pszKey, &(ppszParams[nIndex]) );
            BAIL_ON_VMCA_ERROR(dwError);
            nIndex++;
        }

        pszVal = VMCAStringTokA(NULL, "&", &pszRemaining);
        if (VMCAStringCompareA(pszRemaining, "", 0) == 0 )
        {
            if (VMCAStringCompareA(pszVal, "", 0) == 0 )
            {
                dwError = VMCA_ERROR_INVALID_URI;
                BAIL_ON_VMCA_ERROR(dwError);
            } else
            {
                dwError = VMCAAllocateStringA(pszVal, &(ppszParams[nIndex]) );
                BAIL_ON_VMCA_ERROR(dwError);
                nIndex++;
            }
        } else
        {
            dwError = VMCAAllocateStringA(pszVal, &(ppszParams[nIndex]) );
            BAIL_ON_VMCA_ERROR(dwError);
            nIndex++;
        }

        if (!strstr(pszRemaining, "=") )
        {
            nNotDone = 0;
        } else
        {
            pszKey = NULL;
            pszVal = NULL;
        }
    }

    *ppnCount = pnCount;
    *pppszParams = ppszParams;
cleanup:
    return dwError;
error:
    VMCA_SAFE_FREE_MEMORY(ppszParams);
    goto cleanup;
}

DWORD
VMCARESTGetCRL(
    VMCA_HTTP_REQ_OBJ request,
    PSTR* ppszStatusCode,
    PSTR* ppszResponsePayload
    )
{
    DWORD dwError = 0;
    VMCA_FILE_BUFFER* pTempCRLData = NULL;
    PSTR pszCRLJson = NULL;
    PSTR pszStatusCode = NULL;
    PSTR pszResponsePayload = NULL;
    unsigned int dwFileOffset = 0;
    unsigned int dwSize = 65535;

    dwError = VMCAGetCRL(
                    dwFileOffset,
                    dwSize,
                    &pTempCRLData
                    );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARemoveNewlineChars(
                            pTempCRLData->buffer,
                            &pszCRLJson
                            );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARESTWritePayload(
                            pszCRLJson,
                            "crl",
                            &pszResponsePayload
                            );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARESTWriteStatus(
                            "200",
                            &pszStatusCode
                            );
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszResponsePayload = pszResponsePayload;
    *ppszStatusCode = pszStatusCode;
cleanup:
    if (pTempCRLData != NULL)
    {
        VMCA_SAFE_FREE_MEMORY (pTempCRLData->buffer);
        VMCA_SAFE_FREE_MEMORY (pTempCRLData);
    }
    VMCA_SAFE_FREE_MEMORY(pszCRLJson);
    return dwError;
error:
    VMCA_SAFE_FREE_MEMORY (pszStatusCode);
    goto cleanup;
}


DWORD
VMCARESTGetRootCACertificate(
    VMCA_HTTP_REQ_OBJ request,
    PSTR* ppszStatusCode,
    PSTR* ppszResponsePayload
    )
{
    DWORD dwError = 0;
    PVMCA_CERTIFICATE pTempCertificate = NULL;
    PSTR pszCertJson = NULL;
    PSTR pszStatusCode = NULL;
    PSTR pszResponsePayload = NULL;
    DWORD dwCertLength = 0;

    dwError = VMCAGetRootCACertificate(&dwCertLength, &pTempCertificate);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARemoveNewlineChars(pTempCertificate, &pszCertJson);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARESTWritePayload(
                            pszCertJson,
                            "cert",
                            &pszResponsePayload
                            );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARESTWriteStatus(
                            "200",
                            &pszStatusCode
                            );
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszResponsePayload = pszResponsePayload;
    *ppszStatusCode = pszStatusCode;
cleanup:
    VMCA_SAFE_FREE_MEMORY (pTempCertificate);
    VMCA_SAFE_FREE_MEMORY (pszCertJson);
    return dwError;
error:
    VMCA_SAFE_FREE_MEMORY (pszStatusCode);
    goto cleanup;
}

DWORD
VMCARESTSrvPublishRootCerts(
    VMCA_HTTP_REQ_OBJ request,
    PSTR* ppszStatusCode,
    PSTR* ppszResponsePayload
    )
{
    DWORD dwError = 0;
    PSTR pszStatusCode = NULL;
    PSTR pszResponsePayload = NULL;

    dwError = VMCASrvPublishRootCerts();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARESTWritePayload(
                            NULL,
                            NULL,
                            &pszResponsePayload
                            );
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = VMCARESTWriteStatus(
                            "201",
                            &pszStatusCode
                            );;
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszResponsePayload = pszResponsePayload;
    *ppszStatusCode = pszStatusCode;
cleanup:
    return dwError;
error:
    VMCA_SAFE_FREE_MEMORY (pszResponsePayload);
    VMCA_SAFE_FREE_MEMORY (pszStatusCode);
    goto cleanup;
}

DWORD
VMCARESTAddRootCertificate(
    VMCA_HTTP_REQ_OBJ request,
    PSTR* ppszStatusCode,
    PSTR* ppszResponsePayload
    )
{
    DWORD dwError = 0;
    PSTR  pszStatusCode = NULL;
    PSTR  pszResponsePayload = NULL;
    unsigned char* pszRootCertificate = NULL;
    PWSTR pszPassPhrase = NULL;
    unsigned char* pszPrivateKey = NULL;
    unsigned int dwOverWrite = 0;
    json_t *pRoot = NULL;
    json_t *pJsonCert = NULL;
    json_t *pJsonPriv = NULL;
    json_t *pJsonOverWrite = NULL;


    dwError = VMCAConvertStringInputToJSON(*request.pszPayload, &pRoot);
    BAIL_ON_VMREST_ERROR(dwError);
    pJsonCert = json_object_get(pRoot, VMCA_ADD_ROOT_PARAM_KEY_CERT);
    pJsonPriv = json_object_get(pRoot, VMCA_ADD_ROOT_PARAM_KEY_PRIVKEY);
    if (!pJsonCert || !pJsonPriv)
    {
        dwError = VMCA_ERROR_MISSING_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }
    pszRootCertificate = (unsigned char*) json_string_value(pJsonCert);
    pszPrivateKey = (unsigned char*) json_string_value(pJsonPriv);
    pJsonOverWrite = json_object_get(pRoot, VMCA_ADD_ROOT_PARAM_KEY_OVERWRITE);
    if (pJsonOverWrite)
    {
        dwError = VMCARESTStringToInt(json_string_value(pJsonOverWrite), &dwOverWrite);
        BAIL_ON_VMREST_ERROR(dwError);
    } else
    {
        dwOverWrite = 0;
    }

    dwError = VMCAAddRootCertificate(
                                pszRootCertificate,
                                pszPassPhrase,      // can be left null
                                pszPrivateKey,
                                dwOverWrite
                                );
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = VMCARESTWritePayload(
                            NULL,
                            NULL,
                            &pszResponsePayload
                            );
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = VMCARESTWriteStatus(
                            "201",
                            &pszStatusCode
                            );
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszStatusCode = pszStatusCode;
    *ppszResponsePayload = pszResponsePayload;
cleanup:
    return dwError;
error:
    VMCA_SAFE_FREE_MEMORY (pszResponsePayload);
    VMCA_SAFE_FREE_MEMORY (pszStatusCode);
    goto cleanup;
}

DWORD
VMCARESTEnumCertificates(
    VMCA_HTTP_REQ_OBJ request,
    PCSTR pszFlag,
    PCSTR pszNumber,
    PSTR* ppszStatusCode,
    PSTR* ppszResponsePayload
    )
{
    DWORD dwError = 0;
    PSTR  pszStatusCode = NULL;
    unsigned int dwStartIndex = 0;
    unsigned int dwNumCertificates = 2;
    VMCA_CERTIFICATE_ARRAY* pTempCertArray = NULL;
    CERTIFICATE_STATUS dwStatus = CERTIFICATE_ALL;
    //json_t *pRoot = NULL;
    //json_t *pJsonFlag = NULL;
    //json_t *pJsonNumber = NULL;

    //dwError = VMCAConvertStringInputToJSON(*request.pszPayload, &pRoot);
    //BAIL_ON_VMREST_ERROR(dwError);

    //pJsonFlag = json_object_get(pRoot, VMCA_ENUM_CERTS_PARAM_KEY_FLAG);
    //pJsonNumber = json_object_get(pRoot, VMCA_ENUM_CERTS_PARAM_KEY_NUMBER);
    //if (!pJsonFlag || !pJsonNumber)
    //{
    //    dwError = VMCA_ERROR_MISSING_PARAMETER;
    //    BAIL_ON_VMCA_ERROR(dwError);
    //}
    //pszFlag = (unsigned char*) json_string_value(pJsonFlag);

    if ( !strcasecmp(pszFlag, "all") )
        dwStatus = CERTIFICATE_ALL;
    else if ( !strcasecmp(pszFlag, "active") )
        dwStatus = CERTIFICATE_ACTIVE;
    else if ( !strcasecmp(pszFlag, "revoked") )
        dwStatus = CERTIFICATE_REVOKED;
    else if ( !strcasecmp(pszFlag, "expired") )
    {
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCARESTStringToInt(pszNumber, &dwNumCertificates);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAEnumCertificates(
                            dwStartIndex,
                            dwNumCertificates,
                            dwStatus,
                            &pTempCertArray
                            );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARESTWriteStatus("200", &pszStatusCode);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARESTWriteEnumCerts(pTempCertArray, ppszResponsePayload);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszStatusCode = pszStatusCode;
cleanup:
    VMCAFreeCertificateArray(pTempCertArray);

    return dwError;
error:
    VMCA_SAFE_FREE_MEMORY (pszStatusCode);
    goto cleanup;
}


DWORD
VMCARESTGetSignedCertificate(
    VMCA_HTTP_REQ_OBJ request,
    PSTR* ppszStatusCode,
    PSTR* ppszResponsePayload
    )
{
    DWORD dwError = 0;
    PSTR pszStatusCode = NULL;
    PSTR pszResponsePayload = NULL;
    PSTR pszSignedCert = NULL;
    unsigned char *pszPEMEncodedCSRRequest = 0;
    unsigned int dwNotBefore = 0;
    unsigned int nDuration = 1000;
    unsigned int dwNotAfter = 0;
    VMCA_CERTIFICATE_CONTAINER* pTempCertContainer = NULL;
    json_t *pRoot = NULL;
    json_t *pJsonCSR = NULL;
    json_t *pJsonNotBefore = NULL;
    json_t *pJsonDuration = NULL;


    dwError = VMCAConvertStringInputToJSON(*request.pszPayload, &pRoot);
    BAIL_ON_VMREST_ERROR(dwError);

    pJsonCSR = json_object_get(pRoot, VMCA_GET_SIGNED_CERT_PARAM_KEY_CSR);
    pJsonNotBefore = json_object_get(pRoot, VMCA_GET_SIGNED_CERT_PARAM_KEY_NOT_BF);
    pJsonDuration = json_object_get(pRoot, VMCA_GET_SIGNED_CERT_PARAM_KEY_DURATION);
    if (!pJsonCSR || !pJsonNotBefore || !pJsonDuration)
    {
        dwError = VMCA_ERROR_MISSING_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }
    pszPEMEncodedCSRRequest = (unsigned char*) json_string_value(pJsonCSR);
    dwError = VMCARESTStringToInt(json_string_value(pJsonNotBefore), &dwNotBefore);
    BAIL_ON_VMCA_ERROR(dwError);
    dwError = VMCARESTStringToInt(json_string_value(pJsonDuration), &nDuration);
    BAIL_ON_VMCA_ERROR(dwError);

    dwNotAfter = dwNotBefore + nDuration;

    dwError = VMCAGetSignedCertificate(
                                pszPEMEncodedCSRRequest,
                                dwNotBefore,
                                dwNotAfter,
                                &pTempCertContainer
                                );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARemoveNewlineChars(pTempCertContainer->pCert, &pszSignedCert);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARESTWritePayload(
                            pszSignedCert,
                            "cert",
                            &pszResponsePayload
                            );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARESTWriteStatus(
                            "200",
                            &pszStatusCode
                            );
    BAIL_ON_VMCA_ERROR(dwError); 
    *ppszResponsePayload = pszResponsePayload;
    *ppszStatusCode = pszStatusCode;
cleanup:
    VMCAFreeCertificateContainer(pTempCertContainer);
    VMCA_SAFE_FREE_MEMORY(pszSignedCert);
    return dwError;
error:
    VMCA_SAFE_FREE_MEMORY (pszResponsePayload);
    VMCA_SAFE_FREE_MEMORY (pszStatusCode);
    goto cleanup;
}

DWORD
VMCARESTRevokeCertificate(
    VMCA_HTTP_REQ_OBJ request,
    PSTR* ppszStatusCode,
    PSTR* ppszResponsePayload
    )
{
    DWORD dwError = 0;
    PSTR  pszStatusCode = NULL;
    PSTR  pszResponsePayload = NULL;
    unsigned char *pszCertificate = 0;
    json_t *pRoot = NULL;
    json_t *pJsonCert = NULL;

    dwError = VMCAConvertStringInputToJSON(*request.pszPayload, &pRoot);
    BAIL_ON_VMCA_ERROR(dwError);

    pJsonCert = json_object_get(pRoot, VMCA_REVOKE_CERT_PARAM_KEY_CERT);
    if (!pJsonCert)
    {
        dwError = VMCA_ERROR_MISSING_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }
    pszCertificate = (unsigned char*) json_string_value(pJsonCert);

    dwError = VmcaSrvRevokeCertificate(
                                NULL,
                                pszCertificate,
                                VMCA_CRL_REASON_UNSPECIFIED
                                );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARESTWritePayload(
                            NULL,
                            NULL,
                            &pszResponsePayload
                            );
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = VMCARESTWriteStatus(
                            "201",
                            &pszStatusCode
                            );
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszResponsePayload = pszResponsePayload;
    *ppszStatusCode = pszStatusCode;
cleanup:
    return dwError;
error:
    VMCA_SAFE_FREE_MEMORY(pszResponsePayload);
    VMCA_SAFE_FREE_MEMORY(pszStatusCode);
    goto cleanup;
}

DWORD
VMCARESTGetServerVersion (
    VMCA_HTTP_REQ_OBJ request,
    PSTR* ppszStatusCode,
    PSTR* ppszResponsePayload
    )
{
    DWORD dwError = 0;
    PSTR  pszResponsePayload = NULL;
    PSTR  pszStatusCode = NULL;
    PSTR  pszTempServerVersion = NULL;
    PSTR  pszServerVersion = NULL;

    dwError = VMCAGetServerVersion(&pszTempServerVersion);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARemoveNewlineChars(pszTempServerVersion, &pszServerVersion);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARESTWritePayload (
                            pszServerVersion,
                            "version",
                            &pszResponsePayload
                            );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARESTWriteStatus(
                            "200",
                            &pszStatusCode
                            );
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszResponsePayload = pszResponsePayload;
    *ppszStatusCode = pszStatusCode;
cleanup:
    VMCA_SAFE_FREE_STRINGA(pszServerVersion);
    VMCA_SAFE_FREE_STRINGA(pszTempServerVersion);
    return dwError;
error:
    VMCA_SAFE_FREE_MEMORY (pszResponsePayload);
    VMCA_SAFE_FREE_MEMORY (pszStatusCode);
    goto cleanup;
}
#endif
