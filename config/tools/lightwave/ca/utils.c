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
UpdateReqData(
    json_t*                 pJsonConfig,
    PVMCA_PKCS_10_REQ_DATAA pCertReqData
    );

static
size_t
CurlWriteMemoryCallback(
    PVOID   contents,
    size_t  size,
    size_t  nmemb,
    PVOID   userp
    );

DWORD
VmwCaAcquireOidcToken(
    PVMW_CA_PARAMS pCaParams,
    PCSTR          pcszScope,
    PSTR*          ppszAccessToken
    )
{
    DWORD                        dwError               = 0;
    PSTR                         pszUpn                = NULL;
    POIDC_CLIENT                 pClient               = NULL;
    POIDC_TOKEN_SUCCESS_RESPONSE pTokenSuccessResponse = NULL;
    POIDC_ERROR_RESPONSE         pTokenErrorResponse   = NULL;
    PSTR                         pszAccessToken        = NULL;

    if (!ppszAccessToken || !pCaParams)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = OidcClientBuild(
                &pClient,
                pCaParams->pszServer,
                VMW_OIDC_PORT,
                pCaParams->pszDomain,
                NULL,
                VMW_DEFAULT_CA_PATH
                );
    if (dwError)
    {
        fprintf(stderr, "Failed to build OIDC Client. SSO Error Code: %d\n", dwError);
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeployAllocateStringPrintf(&pszUpn, "%s@%s", pCaParams->pszUsername, pCaParams->pszDomain);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = OidcClientAcquireTokensByPassword(
                pClient,
                pszUpn,
                pCaParams->pszPassword,
                pcszScope,
                &pTokenSuccessResponse,
                &pTokenErrorResponse
                );
    if (dwError)
    {
        fprintf(stderr, "Failed to acquire OIDC Token. SSO Error Code: %d\n", dwError);
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (pTokenErrorResponse)
    {
        fprintf(
            stderr,
            "Failed to acquire OIDC Token: OIDC Error: %s, ErrorDescription: %s\n",
            OidcErrorResponseGetError(pTokenErrorResponse),
            OidcErrorResponseGetErrorDescription(pTokenErrorResponse)
            );
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if ( !pTokenSuccessResponse || IsNullOrEmptyString(OidcTokenSuccessResponseGetAccessToken(pTokenSuccessResponse)) )
    {
        fprintf(stderr, "Failed to acquire OIDC Token. Empty Success Response.\n");
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeployAllocateStringA(
                    OidcTokenSuccessResponseGetAccessToken(pTokenSuccessResponse),
                    &pszAccessToken
                    );
    BAIL_ON_DEPLOY_ERROR(dwError);

    *ppszAccessToken = pszAccessToken;

cleanup:
    VmwDeployFreeMemory(pszUpn);

    if (pTokenSuccessResponse)
    {
        OidcTokenSuccessResponseDelete(pTokenSuccessResponse);
    }
    if (pTokenErrorResponse)
    {
        OidcErrorResponseDelete(pTokenErrorResponse);
    }
    if (pClient)
    {
        OidcClientDelete(pClient);
    }

    return dwError;

error:
    if (pszAccessToken)
    {
        VmwDeployFreeMemory(pszAccessToken);
    }

    goto cleanup;
}

DWORD
VmwCaGenerateCertSigningRequest(
    PVMW_CA_PARAMS pCaParams,
    PVMCA_CSR*     ppCSR
    )
{
    DWORD                   dwError      = 0;
    PVMCA_KEY               pPrivateKey  = NULL;
    PVMCA_KEY               pPublicKey   = NULL;
    PVMCA_PKCS_10_REQ_DATAA pCertReqData = NULL;
    PVMCA_CSR               pCSR         = NULL;

    if (!ppCSR || !pCaParams)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if ( access(pCaParams->pszPrivKeyFilePath, F_OK) != -1)
    {
        dwError = VmwCaReadFromFile(pCaParams->pszPrivKeyFilePath, &pPrivateKey);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }
    else
    {
        dwError = VMCACreatePrivateKey(
                    NULL,
                    pCaParams->privKeySize,
                    &pPrivateKey,
                    &pPublicKey);
        if (dwError)
        {
            fprintf(stderr, "Failed to create Private Key. VMCA Error Code: %d\n", dwError);
            dwError = VMW_CA_DEFAULT_ERROR;
            BAIL_ON_DEPLOY_ERROR(dwError);
        }

        dwError = VmwCaWriteToFile(pCaParams->pszPrivKeyFilePath, pPrivateKey);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VMCAAllocatePKCS10DataA(&pCertReqData);
    if (dwError)
    {
        fprintf(stderr, "Failed to Allocate certificate request data. VMCA Error Code: %d\n", dwError);
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = UpdateReqData(pCaParams->pJsonConfig, pCertReqData);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError =  VMCACreateSigningRequestA(
                   pCertReqData,
                   pPrivateKey,
                   NULL,
                   &pCSR);
    if (dwError)
    {
        fprintf(stderr, "Failed to create certificate signing request. VMCA Error Code: %d\n", dwError);
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    *ppCSR = pCSR;

cleanup:
    VMCAFreeKey(pPrivateKey);
    VMCAFreeKey(pPublicKey);
    VMCAFreePKCS10DataA(pCertReqData);

    return dwError;

error:
    if (pCSR)
    {
        VMCAFreeCSR(pCSR);
    }

    goto cleanup;
}

static
DWORD
UpdateReqData(
    json_t*                 pJsonConfig,
    PVMCA_PKCS_10_REQ_DATAA pCertReqData
    )
{
    DWORD     dwError    = 0;
    PCSTR     pcszKey    = NULL;
    PCSTR     pcszValue  = NULL;
    PSTR      pszValue   = NULL;
    json_t*   pJsonValue = NULL;
    VMCA_OID  field      = 0;

    if (!pCertReqData || !pJsonConfig)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    json_object_foreach(pJsonConfig, pcszKey, pJsonValue)
    {
        pcszValue = json_string_value(pJsonValue);
        if (!pcszValue)
        {
            fprintf(stderr, "Unexpected value in config file. Expecting string value for key: %s\n", pcszKey);
            dwError = VMW_CA_DEFAULT_ERROR;
            BAIL_ON_DEPLOY_ERROR(dwError);
        }

        if (!strcmp(pcszKey, "common_name"))
        {
                field = VMCA_OID_CN;
        }
        else if (!strcmp(pcszKey, "domain_name"))
        {
                field = VMCA_OID_DC;
        }
        else if (!strcmp(pcszKey, "country"))
        {
                field = VMCA_OID_COUNTRY;
        }
        else if (!strcmp(pcszKey, "state"))
        {
                field = VMCA_OID_STATE;
        }
        else if (!strcmp(pcszKey, "locality"))
        {
                field = VMCA_OID_LOCALITY;
        }
        else if (!strcmp(pcszKey, "organization"))
        {
                field = VMCA_OID_ORGANIZATION;
        }
        else if (!strcmp(pcszKey, "org_unit"))
        {
                field = VMCA_OID_ORG_UNIT;
        }
        else if (!strcmp(pcszKey, "dns"))
        {
                field = VMCA_OID_DNS;
        }
        else if (!strcmp(pcszKey, "uri"))
        {
                field = VMCA_OID_URI;
        }
        else if (!strcmp(pcszKey, "email"))
        {
                field = VMCA_OID_EMAIL;
        }
        else if (!strcmp(pcszKey, "ip_address"))
        {
                field = VMCA_OID_IPADDRESS;
        }
        else
        {
                fprintf(stderr, "Unknown key '%s' in json config file. Ignoring and moving on.\n", pcszKey);
        }

        dwError = VmwDeployAllocateStringA(
                            pcszValue,
                            &pszValue);
        BAIL_ON_DEPLOY_ERROR(dwError);

        dwError = VMCASetCertValueA(
                        field,
                        pCertReqData,
                        pszValue);
        if (dwError)
        {
            fprintf(stderr, "Failed to set certificate value from config file. key: %s, value: %s, VMCA Error Code: %d", pcszKey, pszValue, dwError);
            dwError = VMW_CA_DEFAULT_ERROR;
            BAIL_ON_DEPLOY_ERROR(dwError);
        }

        VmwDeployFreeMemory(pszValue);
        pszValue = NULL;
    }

cleanup:
    return dwError;

error:
    if (pszValue)
    {
        VmwDeployFreeMemory(pszValue);
    }
    goto cleanup;
}

DWORD
VmwCaMakeRestRequest(
    PCSTR   pcszUrl,
    PCSTR   pcszHeader,
    PCSTR   pcszPostData,
    PCSTR   pcszRequestType,
    BOOL    bInsecure,
    PSTR*   ppszOut
    )
{
    DWORD                 dwError        = 0;
    CURL*                 pCurlHandle    = NULL;
    CURLcode              curlRespCode   = 0;
    PVMW_CA_REST_RESPONSE pResponse      = NULL;
    struct curl_slist*    pCurlHeaders   = NULL;
    PSTR                  pszOut         = NULL;

    if (!ppszOut || IsNullOrEmptyString(pcszUrl) || IsNullOrEmptyString(pcszRequestType))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeployAllocateMemory(
                        sizeof(VMW_CA_REST_RESPONSE),
                        (PVOID*) &pResponse);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployAllocateStringA(
                        "",
                        &pResponse->pszBuffer);
    BAIL_ON_DEPLOY_ERROR(dwError);

    pResponse->bufferSize = 1;

    pCurlHandle = curl_easy_init();
    if (!pCurlHandle)
    {
        fprintf(stderr, "Could not initialize CURL handle for REST request\n");
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    curl_easy_setopt(pCurlHandle, CURLOPT_URL, pcszUrl);
    curl_easy_setopt(pCurlHandle, CURLOPT_CUSTOMREQUEST, pcszRequestType);
    curl_easy_setopt(pCurlHandle, CURLOPT_WRITEFUNCTION, CurlWriteMemoryCallback);
    curl_easy_setopt(pCurlHandle, CURLOPT_WRITEDATA, pResponse);
    curl_easy_setopt(pCurlHandle, CURLOPT_CAPATH, VMW_CA_CERT_BUNDLE_PATH);

    if (!IsNullOrEmptyString(pcszHeader))
    {
        pCurlHeaders = curl_slist_append(pCurlHeaders, pcszHeader);
        if (!pCurlHeaders)
        {
            fprintf(stderr, "Could not append curl header.\n");
            dwError = VMW_CA_DEFAULT_ERROR;
            BAIL_ON_DEPLOY_ERROR(dwError);
        }

        curl_easy_setopt(pCurlHandle, CURLOPT_HTTPHEADER, pCurlHeaders);
    }

    if (!IsNullOrEmptyString(pcszPostData))
    {
        curl_easy_setopt(pCurlHandle, CURLOPT_POSTFIELDS, pcszPostData);
    }

    if (bInsecure)
    {
        curl_easy_setopt(pCurlHandle, CURLOPT_SSL_VERIFYPEER, 0L);
    }
    else
    {
        curl_easy_setopt(pCurlHandle, CURLOPT_SSL_VERIFYPEER, 1L);
    }

    curlRespCode = curl_easy_perform(pCurlHandle);
    if (curlRespCode)
    {
        fprintf(stderr, "Error from curl request. Libcurl Error Code: %d\n", curlRespCode);
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pResponse->pszBuffer))
    {
        dwError = VmwDeployAllocateStringA(
                        pResponse->pszBuffer,
                        &pszOut);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    *ppszOut = pszOut;

cleanup:
    if (pCurlHeaders)
    {
        curl_slist_free_all(pCurlHeaders);
    }
    if (pCurlHandle)
    {
        curl_easy_cleanup(pCurlHandle);
    }
    if (pResponse)
    {
        if (pResponse->pszBuffer)
        {
            VmwDeployFreeMemory(pResponse->pszBuffer);
        }
        VmwDeployFreeMemory(pResponse);
    }

    return dwError;

error:
    if (pszOut)
    {
        VmwDeployFreeMemory(pszOut);
    }
    goto cleanup;
}

static
size_t
CurlWriteMemoryCallback(
    PVOID   contents,
    size_t  size,
    size_t  nmemb,
    PVOID   userp
    )
{
    size_t                  numBytesHandled = 0;
    size_t                  totalSize       = 0;
    PVMW_CA_REST_RESPONSE   pResponse       = NULL;

    if (contents == NULL || userp == NULL)
    {
        return 0;
    }

    totalSize = size * nmemb;
    pResponse = (PVMW_CA_REST_RESPONSE) userp;

    pResponse->pszBuffer = realloc((PVOID)pResponse->pszBuffer, pResponse->bufferSize + totalSize);
    if (pResponse->pszBuffer)
    {
        memcpy(pResponse->pszBuffer + pResponse->bufferSize - 1, contents, totalSize);
        pResponse->bufferSize += totalSize;
        pResponse->pszBuffer[pResponse->bufferSize - 1] = '\0';
        numBytesHandled = totalSize;
    }
    else
    {
            fprintf(stderr, "Failed to write REST response. Could not reallocate buffer memory.\n");
            numBytesHandled = 0;
    }

    return numBytesHandled;
}

DWORD
VmwCaWriteToFile(
    PCSTR pcszFilePath,
    PCSTR pcszContents
    )
{
    DWORD dwError = 0;
    FILE* pFile = NULL;

    pFile = fopen(pcszFilePath,"w+");
    if (!pFile)
    {
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);;
    }
    fwrite(pcszContents, 1, strlen(pcszContents), pFile);

cleanup:
    if (pFile)
    {
        fclose(pFile);
    }

    return dwError;

error:
    fprintf(stderr, "Could not create/open file at path %s\n", pcszFilePath);

    goto cleanup;
}

DWORD
VmwCaReadFromFile(
    PCSTR pcszFilePath,
    PSTR* ppszContents
    )
{
    DWORD   dwError     = 0;
    size_t  size        = 0;
    size_t  size_read   = 0;
    FILE*   pFile       = NULL;
    PSTR    pszContents = NULL;

    pFile = fopen(pcszFilePath,"r");
    if (!pFile)
    {
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    fseek(pFile, 0, SEEK_END);
    size = ftell(pFile);
    if (size == -1)
    {
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }
    rewind(pFile);

    dwError = VmwDeployAllocateMemory(
                    (size+1)*sizeof(*pszContents),
                    (PVOID*)&pszContents);
    BAIL_ON_DEPLOY_ERROR(dwError);

    size_read = fread(pszContents, size, 1, pFile);
    if (size_read != size)
    {
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }
    pszContents[size] = '\0';

    *ppszContents = pszContents;

cleanup:
    if (pFile)
    {
        fclose(pFile);
    }

    return dwError;

error:
    fprintf(stderr, "Could not read from file at path %s\n", pcszFilePath);

    if (pszContents)
    {
        VmwDeployFreeMemory(pszContents);
    }
    goto cleanup;
}

BOOL
VmwCaIsIPV6AddrFormat(
    PCSTR   pszAddr
    )
{
    BOOLEAN     bIsIPV6 = pszAddr ? TRUE : FALSE;
    size_t      iSize = 0;
    size_t      iCnt = 0;
    size_t      iColonCnt = 0;

    if ( pszAddr != NULL )
    {
        iSize = strlen(pszAddr);
        for (iCnt=0; bIsIPV6 && iCnt < iSize; iCnt++)
        {
            if ( pszAddr[iCnt] == ':' )
            {
                iColonCnt++;
            }
            else if ( VMW_CA_ASCII_DIGIT( pszAddr[iCnt] )
                      ||
                      VMW_CA_ASCII_aTof( pszAddr[iCnt] )
                      ||
                      VMW_CA_ASCII_AToF( pszAddr[iCnt] )
                    )
            {
            }
            else
            {
                bIsIPV6 = FALSE;
            }
        }

        // should not count on iColonCnt == 7
        if ( iColonCnt < 2 )
        {
            bIsIPV6 = FALSE;
        }
    }

    return bIsIPV6;
}

DWORD
VmwGetKeySize(
    PCSTR   pcszKeySize,
    size_t* pKeySize
    )
{
    DWORD  dwError = 0;
    size_t keySize = 0;

    if (!strcmp(pcszKeySize, "2k") || !strcmp(pcszKeySize, "2K") || !strcmp(pcszKeySize, "2048"))
    {
        keySize = 2048;
    }
    else if (!strcmp(pcszKeySize, "4k") || !strcmp(pcszKeySize, "4K") || !strcmp(pcszKeySize, "4096"))
    {
        keySize = 4096;
    }
    else if (!strcmp(pcszKeySize, "8k") || !strcmp(pcszKeySize, "8K") || !strcmp(pcszKeySize, "8192"))
    {
        keySize = 8192;
    }
    else
    {
        fprintf(stderr, "Unupported keysize '%s', Supported: 2k or 2048, 4k or 4096, 8k or 8192\n", pcszKeySize);
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    *pKeySize = keySize;

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
VmwCaFreeParams(
    PVMW_CA_PARAMS pParams
    )
{
    if (!pParams)
    {
        return;
    }
    if (pParams->pJsonConfig)
    {
        json_decref(pParams->pJsonConfig);
    }
    if (pParams->pszPrivKeyFilePath)
    {
        VmwDeployFreeMemory(pParams->pszPrivKeyFilePath);
    }
    if (pParams->pszCertFilePath)
    {
        VmwDeployFreeMemory(pParams->pszCertFilePath);
    }
    if (pParams->pszCertChainFilePath)
    {
        VmwDeployFreeMemory(pParams->pszCertChainFilePath);
    }
    if (pParams->pszServer)
    {
        VmwDeployFreeMemory(pParams->pszServer);
    }
    if (pParams->pszDomain)
    {
        VmwDeployFreeMemory(pParams->pszDomain);
    }
    if (pParams->pszUsername)
    {
        VmwDeployFreeMemory(pParams->pszUsername);
    }
    if (pParams->pszPassword)
    {
        VmwDeployFreeMemory(pParams->pszPassword);
    }
    if (pParams->pszCertDuration)
    {
        VmwDeployFreeMemory(pParams->pszCertDuration);
    }
    if (pParams->pszCAServer)
    {
        VmwDeployFreeMemory(pParams->pszCAServer);
    }
    if (pParams->pszCAId)
    {
        VmwDeployFreeMemory(pParams->pszCAId);
    }

    VmwDeployFreeMemory(pParams);
}
