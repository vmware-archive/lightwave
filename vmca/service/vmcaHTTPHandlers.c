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
VMCARESTGetPayload(
    PVMREST_HANDLE      pRESTHandle,
    PREST_REQUEST       pRESTRequest,
    VMCA_HTTP_REQ_OBJ*  pVMCARequest
    )
{
    DWORD   dwError = 0;
    DWORD   bytesRead = 0;
    size_t  len = 0;
    PSTR    pszPayload = NULL;

    do
    {
        if (bytesRead || !pszPayload)
        {
            dwError = VMCAReallocateMemory(
                    (PVOID)pszPayload,
                    (PVOID*)&pszPayload,
                    len + VMCARESTMAXPAYLOADLENGTH + 1);    // +1 for NULL char
            BAIL_ON_VMCA_ERROR(dwError);
        }

        bytesRead = 0;
        dwError = VmRESTGetData(
                pRESTHandle,
                pRESTRequest,
                pszPayload + len,
                &bytesRead);

        len += bytesRead;
    }
    while (dwError == REST_ENGINE_MORE_IO_REQUIRED);
    BAIL_ON_VMCA_ERROR(dwError);

    pszPayload[len] = 0;

    pVMCARequest->pszPayload = pszPayload;
    pszPayload = NULL;

cleanup:
    VMCA_SAFE_FREE_MEMORY(pszPayload);
    return dwError;

error:
    goto cleanup;
}

DWORD
VMCARESTParseHttpHeader(
    PREST_REQUEST pRESTRequest,
    VMCA_HTTP_REQ_OBJ** ppVMCARequest
    )
{
    DWORD dwError = 0;
    PSTR  pszBuff = NULL;
    VMCA_HTTP_REQ_OBJ* pVMCARequest;

    HANDLE_NULL_PARAM(ppVMCARequest, dwError);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateMemory(
                            sizeof(VMCA_HTTP_REQ_OBJ),
                            (PVOID*) &pVMCARequest
                            );
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = VmRESTGetHttpMethod(pRESTRequest, &pszBuff);
    BAIL_ON_VMREST_ERROR(dwError);
    pVMCARequest->pszMethod = pszBuff;

    // TRUE - Request c-rest-engine to decode the URI
    dwError = VmRESTGetHttpURI(pRESTRequest, TRUE, &pszBuff);
    BAIL_ON_VMREST_ERROR(dwError);
    pVMCARequest->pszUri = pszBuff;

    dwError = VmRESTGetHttpVersion(pRESTRequest, &pszBuff);
    BAIL_ON_VMREST_ERROR(dwError);
    pVMCARequest->pszVer = pszBuff;

    dwError = VmRESTGetHttpHeader(pRESTRequest,"Content-Length", &pszBuff);
    BAIL_ON_VMREST_ERROR(dwError);
    pVMCARequest->pszContentLength = pszBuff;

    *ppVMCARequest = pVMCARequest;
cleanup:

    return dwError;
error:
    VMCA_SAFE_FREE_STRINGA(pVMCARequest->pszMethod);
    VMCA_SAFE_FREE_STRINGA(pVMCARequest->pszUri);
    VMCA_SAFE_FREE_STRINGA(pVMCARequest->pszVer);
    VMCA_SAFE_FREE_STRINGA(pszBuff);
    VMCA_SAFE_FREE_MEMORY(pVMCARequest);

    goto cleanup;

}

DWORD
VMCARESTCheckGroupPermissions(
    PVMCA_ACCESS_TOKEN pAccessToken
    )
{
    DWORD dwError = 0;
    int nCounter = 0;
    int bGroupFound = 0;

    if  (!pAccessToken || !pAccessToken->tokenType)
    {
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMREST_ERROR(dwError);
    }

    switch(pAccessToken->tokenType)
    {
        case VMCA_AUTHORIZATION_TYPE_HOTK_TOKEN:
        case VMCA_AUTHORIZATION_TYPE_BEARER_TOKEN:
            if (!pAccessToken->pszGroups ||
                pAccessToken->dwGroupSize == 0
                )
            {
                dwError = ERROR_ACCESS_DENIED;
                BAIL_ON_VMREST_ERROR(dwError);
            } else {
                for (nCounter = 0; nCounter < pAccessToken->dwGroupSize; nCounter++)
                {
                    if (strstr(pAccessToken->pszGroups[nCounter], VMCA_GROUP_PERMISSION_STRING))
                    {
                        bGroupFound = 1;
                    }
                }
                if (!bGroupFound)
                {
                    dwError = ERROR_ACCESS_DENIED;
                    BAIL_ON_VMREST_ERROR(dwError);
                }
            }
            break;

        case VMCA_AUTHORIZATION_TOKEN_TYPE_KRB:
            if (!pAccessToken->bKrbTicketValid ||
                *pAccessToken->bKrbTicketValid != 1
                )
            {
                dwError = ERROR_ACCESS_DENIED;
                BAIL_ON_VMREST_ERROR(dwError);
            }
            break;

        default:
            dwError = ERROR_ACCESS_DENIED;
            BAIL_ON_VMREST_ERROR(dwError);
            break;
    }

error:
    return dwError;
}

DWORD
VMCARESTCheckAccess(
    PREST_REQUEST pRESTRequest,
    VMCA_HTTP_REQ_OBJ* pVMCARequest,
    BOOL bNeedAdminPrivilege
    )
{
    DWORD dwError = 0;
    PVMCA_ACCESS_TOKEN pAccessToken = NULL;

    dwError = VMCARESTGetAccessToken(pRESTRequest, &pAccessToken);
    BAIL_ON_VMREST_ERROR(dwError);

    if (bNeedAdminPrivilege)
    {
        dwError = VMCARESTCheckGroupPermissions(pAccessToken);
        BAIL_ON_VMREST_ERROR(dwError);
    }

    pVMCARequest->pAccessToken = pAccessToken;
    pAccessToken = NULL;

cleanup:
    return dwError;

error:
    if (pAccessToken)
    {
        VMCAFreeAccessToken(pAccessToken);
    }
    goto cleanup;
}

DWORD
VMCARESTHandleCRLRequest(
    PREST_REQUEST pRESTRequest,
    VMCA_HTTP_REQ_OBJ* pVMCARequest,
    PSTR* ppszStatusCode,
    PSTR* ppszResponsePayload
    )
{
    DWORD dwError = 0;

    //HANDLE_NULL_PARAM(pVMCARequest, dwError);
    //BAIL_ON_VMCA_ERROR(dwError);

    if (!strcmp(pVMCARequest->pszMethod,"GET"))
    {
        dwError = VMCARESTGetCRL(
                            *pVMCARequest,
                            ppszStatusCode,
                            ppszResponsePayload
                            );
        BAIL_ON_VMREST_ERROR(dwError);
    } else {
        dwError = VMCA_ERROR_INVALID_METHOD;
        BAIL_ON_VMREST_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VMCARESTHandleRootRequest(
    PREST_REQUEST pRESTRequest,
    VMCA_HTTP_REQ_OBJ* pVMCARequest,
    PSTR* ppszStatusCode,
    PSTR* ppszResponsePayload
    )
{
    DWORD dwError = 0;

    //HANDLE_NULL_PARAM(pVMCARequest, dwError);
    //BAIL_ON_VMCA_ERROR(dwError);

    if (!strcmp(pVMCARequest->pszMethod,"GET"))
    {
        dwError = VMCARESTGetRootCACertificate(
                                    *pVMCARequest,
                                    ppszStatusCode,
                                    ppszResponsePayload
                                    );
        BAIL_ON_VMREST_ERROR(dwError);

    } else if (!strcmp(pVMCARequest->pszMethod,"POST"))
    {
        dwError = VMCARESTCheckAccess(pRESTRequest, pVMCARequest, true);
        BAIL_ON_VMREST_ERROR(dwError);

        dwError = VMCARESTAddRootCertificate(
                                    *pVMCARequest,
                                    ppszStatusCode,
                                    ppszResponsePayload
                                    );
        BAIL_ON_VMREST_ERROR(dwError);
    } else if (!strcmp(pVMCARequest->pszMethod,"PUT"))
    {
        dwError = VMCARESTCheckAccess(pRESTRequest, pVMCARequest, true);
        BAIL_ON_VMREST_ERROR(dwError);

        dwError = VMCARESTAddRootCertificate(
                                    *pVMCARequest,
                                    ppszStatusCode,
                                    ppszResponsePayload
                                    );
        BAIL_ON_VMREST_ERROR(dwError);
    } else {
        dwError = VMCA_ERROR_INVALID_METHOD;
        BAIL_ON_VMREST_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VMCARESTHandleCertRequest(
    PREST_REQUEST pRESTRequest,
    VMCA_HTTP_REQ_OBJ* pVMCARequest,
    PSTR* ppszStatusCode,
    PSTR* ppszResponsePayload
    )
{
    DWORD dwError = 0;
    PSTR pszKey1 = NULL;
    PSTR pszVal1 = NULL;
    PSTR pszKey2 = NULL;
    PSTR pszVal2 = NULL;
    PSTR pszFlag = NULL;
    PSTR pszNum = NULL;

    //HANDLE_NULL_PARAM(pVMCARequest, dwError);
    //BAIL_ON_VMCA_ERROR(dwError);

    if (!strcmp(pVMCARequest->pszMethod,"GET"))
    {
        dwError = VMCARESTCheckAccess(pRESTRequest, pVMCARequest, true);
        BAIL_ON_VMREST_ERROR(dwError);

        dwError = VmRESTGetParamsByIndex(pRESTRequest, 2, 1, &pszKey2, &pszVal2);
        BAIL_ON_VMREST_ERROR(dwError);

        dwError = VmRESTGetParamsByIndex(pRESTRequest, 2, 2, &pszKey1, &pszVal1);
        BAIL_ON_VMREST_ERROR(dwError);


        dwError = VMCAHandleEnumCertsParam(
                                    pszKey1,
                                    pszVal1,
                                    pszKey2,
                                    pszVal2,
                                    &pszFlag,
                                    &pszNum);
        BAIL_ON_VMREST_ERROR(dwError);
 
        dwError = VMCARESTEnumCertificates(
                                    *pVMCARequest,
                                    pszFlag,
                                    pszNum,
                                    ppszStatusCode,
                                    ppszResponsePayload
                                    );
        BAIL_ON_VMREST_ERROR(dwError);
    } else if (!strcmp(pVMCARequest->pszMethod,"PUT"))
    {
        dwError = VMCARESTCheckAccess(pRESTRequest, pVMCARequest, false);
        BAIL_ON_VMREST_ERROR(dwError);

        dwError = VMCARESTGetSignedCertificate(
                                    *pVMCARequest,
                                    ppszStatusCode,
                                    ppszResponsePayload
                                    );
        BAIL_ON_VMREST_ERROR(dwError);
    } else if (!strcmp(pVMCARequest->pszMethod,"DELETE"))
    {
        dwError = VMCARESTCheckAccess(pRESTRequest, pVMCARequest, true);
        BAIL_ON_VMREST_ERROR(dwError);

        dwError = VMCARESTRevokeCertificate(
                                    *pVMCARequest,
                                    ppszStatusCode,
                                    ppszResponsePayload
                                    );
        BAIL_ON_VMREST_ERROR(dwError);
    } else {
        dwError = VMCA_ERROR_INVALID_METHOD;
        BAIL_ON_VMREST_ERROR(dwError);
    }

cleanup:
    //VMCA_SAFE_FREE_MEMORY(pszKey1);
    //VMCA_SAFE_FREE_MEMORY(pszKey2);
    //VMCA_SAFE_FREE_MEMORY(pszVal1);
    //VMCA_SAFE_FREE_MEMORY(pszVal2);
    return dwError;

error:
    goto cleanup;
}

DWORD
VMCARESTHandleVMCARequest(
    PREST_REQUEST pRESTRequest,
    VMCA_HTTP_REQ_OBJ* pVMCARequest,
    PSTR* ppszStatusCode,
    PSTR* ppszResponsePayload
    )
{
    DWORD dwError = 0;

    //HANDLE_NULL_PARAM(pVMCARequest, dwError);
    //BAIL_ON_VMCA_ERROR(dwError);

    if (!strcmp(pVMCARequest->pszMethod,"GET"))
    {
        dwError = VMCARESTGetServerVersion(
                                    *pVMCARequest,
                                    ppszStatusCode,
                                    ppszResponsePayload
                                    );
        BAIL_ON_VMREST_ERROR(dwError);
    } else {
        dwError = VMCA_ERROR_INVALID_METHOD;
        BAIL_ON_VMREST_ERROR(dwError);
    }
cleanup:

    return dwError;

error:
    goto cleanup;
}

DWORD
VMCARESTExecuteHttpURI(
    PREST_REQUEST pRESTRequest,
    VMCA_HTTP_REQ_OBJ* pVMCARequest,
    PSTR* ppszStatusCode,
    PSTR* ppszResponsePayload
    )
{
    DWORD dwError = 0;
    PSTR pszStatusCode = NULL;
    PSTR pszResponsePayload = NULL;

    HANDLE_NULL_PARAM(pVMCARequest, dwError);
    BAIL_ON_VMCA_ERROR(dwError);

    if (!pVMCARequest)
    {
        dwError = VMCA_ERROR_INVALID_URI;
        BAIL_ON_VMREST_ERROR(dwError);
    }

    if (strcasestr(pVMCARequest->pszUri,VMCA_CRL_URI) != NULL)
    {
        dwError = VMCARESTHandleCRLRequest(
                                pRESTRequest,
                                pVMCARequest,
                                &pszStatusCode,
                                &pszResponsePayload
                                );
        BAIL_ON_VMREST_ERROR(dwError);
    } else if (strcasestr(pVMCARequest->pszUri,VMCA_ROOT_URI) != NULL)
    {
        dwError = VMCARESTHandleRootRequest(
                                pRESTRequest,
                                pVMCARequest,
                                &pszStatusCode,
                                &pszResponsePayload
                                );
        BAIL_ON_VMREST_ERROR(dwError);
    } else if (strcasestr(pVMCARequest->pszUri,VMCA_CERTS_URI) != NULL)
    {
        dwError = VMCARESTHandleCertRequest(
                                pRESTRequest,
                                pVMCARequest,
                                &pszStatusCode,
                                &pszResponsePayload
                                );
        BAIL_ON_VMREST_ERROR(dwError);
    } else if (strcasestr(pVMCARequest->pszUri,VMCA_URI) != NULL)
    {
        dwError = VMCARESTHandleVMCARequest(
                                pRESTRequest,
                                pVMCARequest,
                                &pszStatusCode,
                                &pszResponsePayload
                                );
        BAIL_ON_VMREST_ERROR(dwError);
    } else {
        dwError = VMCA_ERROR_INVALID_URI;
        BAIL_ON_VMREST_ERROR(dwError);
    }

    *ppszStatusCode = pszStatusCode;

    *ppszResponsePayload = pszResponsePayload;

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VMCARESTSetResponseHeaders(
    PREST_RESPONSE* ppResponse,
    PSTR pszStatusCode
    )
{
    DWORD dwError = 0;

    dwError = VmRESTSetHttpHeader(ppResponse, "VMware", "VMCA");
    BAIL_ON_VMREST_ERROR(dwError);
    dwError = VmRESTSetHttpHeader(ppResponse, "Location", "United States");
    BAIL_ON_VMREST_ERROR(dwError);
    dwError = VmRESTSetHttpStatusCode(ppResponse, pszStatusCode);
    BAIL_ON_VMREST_ERROR(dwError);
    dwError = VmRESTSetHttpStatusVersion(ppResponse,"HTTP/1.1");
    BAIL_ON_VMREST_ERROR(dwError);
    dwError = VmRESTSetHttpReasonPhrase(ppResponse,"OK");
    BAIL_ON_VMREST_ERROR(dwError);
    dwError = VmRESTSetHttpHeader(ppResponse, "Unix", "Linux");
    BAIL_ON_VMREST_ERROR(dwError);
    dwError = VmRESTSetHttpHeader(ppResponse, "Connection", "close");
    BAIL_ON_VMREST_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;

}

DWORD
VMCARESTSetResponsePayload(
    PVMREST_HANDLE  pRESTHandle,
    PREST_RESPONSE* ppResponse,
    PSTR            pszRespPayload
    )
{
    DWORD   dwError = 0;
    DWORD   bytesWritten = 0;
    PSTR    pszPyldLen = NULL;
    size_t  pyldLen = 0;
    size_t  sentLen = 0;

    pyldLen = VMCAStringLenA(VMCA_SAFE_STRING(pszRespPayload));

    dwError = VMCAAllocateStringPrintfA(&pszPyldLen, "%ld", pyldLen);
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = VmRESTSetDataLength(
            ppResponse,
            pyldLen > VMCARESTMAXPAYLOADLENGTH ? NULL : pszPyldLen);
    BAIL_ON_VMREST_ERROR(dwError);

    do
    {
        size_t chunkLen = pyldLen > VMCARESTMAXPAYLOADLENGTH ?
                VMCARESTMAXPAYLOADLENGTH : pyldLen;

        dwError = VmRESTSetData(
                pRESTHandle,
                ppResponse,
                VMCA_SAFE_STRING(pszRespPayload) + sentLen,
                chunkLen,
                &bytesWritten);

        sentLen += bytesWritten;
        pyldLen -= bytesWritten;
    }
    while (dwError == REST_ENGINE_MORE_IO_REQUIRED);
    BAIL_ON_VMREST_ERROR(dwError);

cleanup:
    VMCA_SAFE_FREE_MEMORY(pszPyldLen);
    return dwError;

error:
    goto cleanup;
}

/*
HTTP/1.1 401 Authorization Required
WWW-Authenticate: Negotiate [token]
Content-Type: text/html
Content-Length: 20
*/

uint32_t
VMCARESTRequestNegotiateAuth(
    PVMREST_HANDLE  pRESTHandle,
    PREST_REQUEST   pRequest,
    PREST_RESPONSE* ppResponse,
    const char*     pszToken
    )
{
    uint32_t dwError = 0;
    uint32_t temp = 0;
    const char* pszNegotiate = pszToken ? pszToken : "Negotiate";

    dwError = VmRESTSetHttpStatusVersion(ppResponse, "HTTP/1.1");
    dwError = VmRESTSetHttpStatusCode(ppResponse, "401");
    dwError = VmRESTSetHttpReasonPhrase(ppResponse, "Unauthorized");
    dwError = VmRESTSetHttpHeader(ppResponse, "Connection", "close");
    dwError = VmRESTSetHttpHeader(ppResponse, "Content-Length", "0");
    dwError = VmRESTSetHttpHeader(ppResponse, "WWW-Authenticate", (char *)pszNegotiate);
    dwError = VmRESTSetData(pRESTHandle, ppResponse, "", 0, &temp);
    dwError = EACCES;
    return dwError;
}

DWORD
VMCAHandleHttpRequest(
    PVMREST_HANDLE  pRESTHandle,
    PREST_REQUEST   pRequest,
    PREST_RESPONSE* ppResponse,
    uint32_t        paramsCount
    )
{
    DWORD dwError = 0;
    PSTR pszStatusCode = NULL;
    PSTR pszResponsePayload = NULL;
    VMCA_HTTP_REQ_OBJ* pVMCARequest = NULL;

    dwError = VMCARESTParseHttpHeader(pRequest, &pVMCARequest);
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = VMCARESTGetPayload(pRESTHandle, pRequest, pVMCARequest);
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = VMCARESTExecuteHttpURI(
                                pRequest,
                                pVMCARequest,
                                &pszStatusCode,
                                &pszResponsePayload
                                );
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = VMCARESTSetResponseHeaders(ppResponse, pszStatusCode);
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = VMCARESTSetResponsePayload(
            pRESTHandle, ppResponse, pszResponsePayload);
    BAIL_ON_VMREST_ERROR(dwError);

cleanup:
    VMCA_SAFE_FREE_MEMORY(pszResponsePayload);
    VMCA_SAFE_FREE_MEMORY(pszStatusCode);
    if (pVMCARequest)
    {
        VMCAFreeAccessToken(pVMCARequest->pAccessToken);
        VMCA_SAFE_FREE_MEMORY(pVMCARequest->pszPayload);
        VMCA_SAFE_FREE_MEMORY(pVMCARequest);
    }
    return dwError;

error:
    if (dwError == EACCES)
    {
        dwError = VMCARESTRequestNegotiateAuth(
                pRESTHandle, pRequest, ppResponse, NULL);
    }

    goto cleanup;
}

#endif
