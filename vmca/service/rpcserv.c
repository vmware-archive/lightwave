/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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


// This API does not work now, most of the information
// returned the RPC stack is NULL.

DWORD
VMCAGetClientName(handle_t IDL_handle)
{
#if DEBUG_RPC
  rpc_binding_handle_t Srv;
  PSTR pszBinding = NULL;
  PSTR pszObjUUID = NULL;
  PSTR pszProtoSeq = NULL;
  PSTR pszNetworkAddress = NULL;
  PSTR pszEndPoint = NULL;
  PSTR pszNetworkOptions = NULL;


  unsigned32 rs = 0;
  rpc_binding_server_from_client (IDL_handle, &Srv, &rs);
  BAIL_ON_VMCA_ERROR(rs);
  rpc_binding_to_string_binding(Srv,
                        (unsigned char **) &pszBinding, &rs);
  printf("Binding Info : %s\n", pszBinding);
  rpc_binding_free(&Srv, &rs);

  rpc_string_binding_parse(pszBinding,
                        (unsigned_char_t **) &pszObjUUID,
                        (unsigned_char_t **) &pszProtoSeq,
                        (unsigned_char_t **) &pszNetworkAddress,
                        (unsigned_char_t **) &pszEndPoint,
                        (unsigned_char_t **) &pszNetworkOptions,
                        &rs);
  printf("Obj UUID : %s, Proto : %s, Network : %s, \
            EndPoint %s, Network Options : %s \n",
            pszObjUUID,
            pszProtoSeq,
            pszNetworkAddress,
            pszEndPoint,
            pszNetworkOptions);

    rpc_string_free((unsigned_char_t **) &pszBinding, &rs);
    rpc_string_free((unsigned_char_t **) &pszObjUUID, &rs);
    rpc_string_free((unsigned_char_t **) &pszProtoSeq, &rs);
    rpc_string_free((unsigned_char_t **) &pszNetworkAddress, &rs);
    rpc_string_free((unsigned_char_t **) &pszEndPoint, &rs);
    rpc_string_free((unsigned_char_t **) &pszNetworkOptions, &rs);
#endif
    return 0;
}


DWORD
VMCACheckAccess(
    handle_t IDL_handle,
    BOOL bNeedAdminPrivilage
    )
{
    DWORD dwError = 0;
    rpc_authz_cred_handle_t hPriv = { 0 };
    DWORD dwProtectLevel = 0;
    ULONG rpc_status = rpc_s_ok;
    unsigned char *authPrinc = NULL;

    rpc_binding_inq_auth_caller(
                            IDL_handle,
                            &hPriv,
                            &authPrinc,
                            &dwProtectLevel,
                            NULL, /* unsigned32 *authn_svc, */
                            NULL, /* unsigned32 *authz_svc, */
                            &rpc_status);

    /* Deny if connection is not encrypted */
    if (dwProtectLevel < rpc_c_protect_level_pkt_privacy)
    {
        dwError = VMCA_KRB_ACCESS_DENIED;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    /* Deny if no auth identity is provided.  */
    if (rpc_status == rpc_s_binding_has_no_auth || !authPrinc || !*authPrinc)
    {
        dwError = VMCA_KRB_ACCESS_DENIED;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    VMCA_LOG_INFO("VMCACheckAccessKrb: Authenticated user %s", authPrinc);

    if (bNeedAdminPrivilage)
    {
        dwError = VMCALdapAccessCheck(authPrinc, VMCA_ADMINISTRATORS);
        BAIL_ON_VMCA_ERROR(dwError);
    }

error:
    if (authPrinc)
    {
        rpc_string_free((unsigned_char_t **)&authPrinc, &rpc_status);
    }
    return dwError;
}

/* Here is the Truth Table that we are using for this Access Check
+-----+---------------+--------+
| KRB | Shared Secret | Allow  |
+-----+---------------+--------+
| NO  | NO            | NO     |
| NO  | YES           | YES    |
| YES | NO            | YES    |
|*YES | YES           | YES    |
+-----+---------------+--------+
Note: *KRB and shared secret are mutually exclusive; Impossible state.
*/

unsigned int
RpcVMCASetServerOption(
    handle_t IDL_handle,
    unsigned int dwOption
    )
{
    DWORD   dwError = 0;
    unsigned int    dwCurrentOption = 0;
    unsigned int    dwNewOption = 0;

    dwError = VMCACheckAccess(IDL_handle, TRUE);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAConfigGetDword(VMCA_REG_KEY_SERVER_OPTION, &dwCurrentOption);
    dwError = dwError == ERROR_FILE_NOT_FOUND ? 0: dwError;
    BAIL_ON_VMCA_ERROR(dwError);

    dwNewOption = dwCurrentOption | dwOption;

    dwError = VMCAConfigSetDword(VMCA_REG_KEY_SERVER_OPTION, dwNewOption);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;

error:
    goto cleanup;
}

unsigned int
RpcVMCAUnsetServerOption(
    handle_t IDL_handle,
    unsigned int dwOption
    )
{
    DWORD   dwError = 0;
    unsigned int    dwCurrentOption = 0;
    unsigned int    dwNewOption = 0;

    dwError = VMCACheckAccess(IDL_handle, TRUE);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAConfigGetDword(VMCA_REG_KEY_SERVER_OPTION, &dwCurrentOption);
    dwError = dwError == ERROR_FILE_NOT_FOUND ? 0: dwError;
    BAIL_ON_VMCA_ERROR(dwError);

    dwNewOption = dwCurrentOption & ~dwOption;

    dwError = VMCAConfigSetDword(VMCA_REG_KEY_SERVER_OPTION, dwNewOption);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;

error:
    goto cleanup;
}

unsigned int
RpcVMCAGetServerOption(
    handle_t IDL_handle,
    unsigned int *pdwOption
    )
{
    DWORD   dwError = 0;
    unsigned int    dwOption = 0;

    if (!pdwOption)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCACheckAccess(IDL_handle, TRUE);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAConfigGetDword(VMCA_REG_KEY_SERVER_OPTION, &dwOption);
    dwError = dwError == ERROR_FILE_NOT_FOUND ? 0: dwError;
    BAIL_ON_VMCA_ERROR(dwError);

    *pdwOption = dwOption;

cleanup:
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;

error:
    goto cleanup;
}

unsigned int
RpcVMCAGetServerVersion(
    handle_t IDL_handle,
    unsigned int *pdwCertLength,
    VMCA_CERTIFICATE_CONTAINER **pServerVersion)
{
    DWORD dwError = 0;
    PSTR pTempServerVersion = NULL; 
    VMCA_CERTIFICATE_CONTAINER *pTempVersion = NULL;
    /*
     * TBD: Probably don't care if this is accessable remotely
     * Currently VMCA API wiki states this should be SRP authenticated. (??)
     */
    dwError = VMCACheckAccess(IDL_handle, FALSE);
    BAIL_ON_VMCA_ERROR(dwError);
    VMCA_LOG_DEBUG("Entering %s", __FUNCTION__);

    if (pServerVersion == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAGetServerVersion(&pTempServerVersion);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARpcAllocateCertificateContainer
            (pTempServerVersion, &pTempVersion);
    BAIL_ON_VMCA_ERROR(dwError);

    strcpy(pTempVersion->pCert,pTempServerVersion);
    *pServerVersion = pTempVersion;

cleanup:
    VMCA_SAFE_FREE_STRINGA(pTempServerVersion);
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;
error:
    if(pdwCertLength)
    {
        *pdwCertLength = 0;
    }

    if (pServerVersion)
    {
        *pServerVersion = NULL;
    }

    if (pTempVersion)
    {
        VMCARpcFreeCertificateContainer(pTempVersion);
        pTempVersion = NULL;
    }

    goto cleanup;
}

unsigned int
RpcVMCAInitEnumCertificatesHandle(
    handle_t IDL_handle,
    unsigned int * pdwHandle)
{
    DWORD dwError = 0;
    VMCA_LOG_DEBUG("Entering %s", __FUNCTION__);

    dwError = VMCACheckAccess(IDL_handle, TRUE);
    BAIL_ON_VMCA_ERROR(dwError);

error:
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);

    return dwError;
}

DWORD
RpcVMCACopyCertificateToRPC(
    VMCA_CERTIFICATE_ARRAY* pInputCertArray,
    VMCA_CERTIFICATE_ARRAY** pOutputCertArray
    )
{
    DWORD dwError                               = 0;
    VMCA_CERTIFICATE_ARRAY* pCertArray          = NULL;

    dwError = VMCARpcAllocateMemory(
                    sizeof(VMCA_CERTIFICATE_ARRAY),
                    (PVOID*)&pCertArray);
    BAIL_ON_VMCA_ERROR(dwError);

    pCertArray->dwCount = pInputCertArray->dwCount;

    if (pCertArray->dwCount > 0)
    {
        DWORD iEntry = 0;
        dwError = VMCARpcAllocateMemory(
                    pCertArray->dwCount * sizeof(pCertArray->certificates[0]),
                    (PVOID*)&pCertArray->certificates);
        BAIL_ON_VMCA_ERROR(dwError);

        for (; iEntry < pCertArray->dwCount; iEntry++)
        {

            pCertArray->certificates[iEntry].dwCount
                    = pInputCertArray->certificates[iEntry].dwCount;

            dwError = VMCARpcAllocateString(
                    (RP_PSTR)pInputCertArray->certificates[iEntry].pCert,
                    (RP_PSTR*)&pCertArray->certificates[iEntry].pCert);

            BAIL_ON_VMCA_ERROR(dwError);
        }
    }

    *pOutputCertArray = pCertArray;

cleanup:

    return dwError;
error:
    if (pCertArray)
    {
        VMCARpcFreeCertificateArray(pCertArray);
    }

    goto cleanup;
}

unsigned int
RpcVMCAEnumCertificates(
    handle_t IDL_handle,
    CERTIFICATE_STATUS dwStatus,
    unsigned int dwStartIndex,
    unsigned int dwNumCertificates,
    VMCA_CERTIFICATE_ARRAY** ppCertArray)
{
    DWORD dwError = 0;
    VMCA_CERTIFICATE_ARRAY* pTempCertArray = NULL;
    VMCA_CERTIFICATE_ARRAY* pCertArray = NULL;
    VMCA_LOG_DEBUG("Entering %s", __FUNCTION__);

    if (ppCertArray == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCACheckAccess(IDL_handle, TRUE);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAEnumCertificates(
                        dwStartIndex,
                        dwNumCertificates,
                        dwStatus,
                        &pTempCertArray);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = RpcVMCACopyCertificateToRPC(
                        pTempCertArray,
                        &pCertArray);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppCertArray = pCertArray;

cleanup:
    if (pTempCertArray)
    {
        VMCAFreeCertificateArray(pTempCertArray);
    }
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;

error:
    if (ppCertArray)
    {
        *ppCertArray = NULL;
    }

    if (pCertArray)
    {
        VMCARpcFreeCertificateArray(pCertArray);
    }

    goto cleanup;
}

unsigned int
RpcVMCAAddRootCertificate(
    handle_t IDL_handle,
    unsigned char *pszRootCertificate,
    wchar16_t *pszPassPhrase,
    unsigned char *pszPrivateKey,
    unsigned int dwOverWrite,
    wchar16_t *pszSharedSecret)
{

    DWORD dwError = 0;

    VMCA_LOG_DEBUG("Entering %s", __FUNCTION__);

    dwError = VMCACheckAccess(IDL_handle, TRUE);
    BAIL_ON_VMCA_ERROR(dwError);

    if (pszRootCertificate == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if(pszPrivateKey == NULL){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAddRootCertificate(
                        pszRootCertificate,
                        pszPassPhrase,
                        pszPrivateKey,
                        dwOverWrite);

    BAIL_ON_VMCA_ERROR(dwError);
error :
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;
}

unsigned int
RpcVMCAGetSignedCertificate(
        handle_t IDL_handle,
        unsigned char *pszPEMEncodedCSRRequest,
        unsigned int dwValidFrom,
        unsigned int dwDurationInSeconds,
        wchar16_t *pszSharedSecret,
        unsigned int *pdwCertLength,
        VMCA_CERTIFICATE_CONTAINER **ppCertContainer)
{
    DWORD dwError = 0;
    VMCA_CERTIFICATE_CONTAINER* pCertContainer = NULL;
    VMCA_CERTIFICATE_CONTAINER* pTempCertContainer = NULL;

    VMCA_LOG_DEBUG("Entering %s", __FUNCTION__);

    if (ppCertContainer == NULL ||
        pszPEMEncodedCSRRequest == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCACheckAccess(IDL_handle, FALSE);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetSignedCertificate(
                        pszPEMEncodedCSRRequest,
                        dwValidFrom,
                        dwDurationInSeconds,
                        &pTempCertContainer);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARpcAllocateCertificateContainer(
                        pTempCertContainer->pCert,
                        &pCertContainer);
    BAIL_ON_VMCA_ERROR(dwError);

    strcpy((char*)pCertContainer->pCert, (char*)pTempCertContainer->pCert);

    *ppCertContainer = pCertContainer;

cleanup:
    VMCAFreeCertificateContainer (pTempCertContainer);
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;
error:
    if (pdwCertLength)
    {
        *pdwCertLength = 0;
    }

    if (ppCertContainer)
    {
        *ppCertContainer = NULL;
    }

    VMCARpcFreeCertificateContainer(pCertContainer);
    goto cleanup;
}


unsigned int
RpcVMCAGetRootCACertificate(
        handle_t IDL_handle,
        unsigned int *pdwCertLength,
        VMCA_CERTIFICATE_CONTAINER **ppCertContainer)
{
    DWORD dwError = 0;
    DWORD dwCertLength = 0;
    VMCA_CERTIFICATE_CONTAINER* pTempCertContainer = NULL;
    PVMCA_CERTIFICATE pTempCertificate = NULL;

    VMCA_LOG_DEBUG("Entering %s", __FUNCTION__);

/*
 * TBD: This cannot be protected now; certool --WaitVMCA uses this
 * interface to determine vmcad is up and responding.
 */
    /* Restrict access to this RPC. Should use VECS APIs */
    dwError = VMCACheckAccess(IDL_handle, FALSE);
    BAIL_ON_VMCA_ERROR(dwError);

    if (ppCertContainer == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAGetRootCACertificate(
                &dwCertLength,
                &pTempCertificate);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARpcAllocateCertificateContainer(pTempCertificate, &pTempCertContainer);
    BAIL_ON_VMCA_ERROR(dwError);

    strcpy((char*) (pTempCertContainer)->pCert, pTempCertificate);

    *ppCertContainer = pTempCertContainer;
    *pdwCertLength = dwCertLength;
    pTempCertContainer = NULL;

cleanup:
    VMCA_SAFE_FREE_STRINGA(pTempCertificate);
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;
error:
    if (pdwCertLength)
    {
        *pdwCertLength = 0;
    }

    if (ppCertContainer)
    {
        *ppCertContainer = NULL;
    }

    if (pTempCertContainer)
    {
        VMCARpcFreeCertificateContainer(pTempCertContainer);
    }

    goto cleanup;
}


unsigned int
RpcVMCARevokeCertificate(
    handle_t IDL_handle,
    unsigned char *pszCertificate,
    wchar16_t *pszSharedSecret)
{
    DWORD dwError = 0;
    VMCA_LOG_DEBUG("Entering %s", __FUNCTION__);

    dwError = VMCACheckAccess(IDL_handle, TRUE);
    BAIL_ON_VMCA_ERROR(dwError);

    if(pszCertificate == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCARevokeCertificate(pszCertificate);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;
error:
    goto cleanup;
}

unsigned int
RpcVMCAVerifyCertificate(
    handle_t IDL_handle,
    unsigned char *pszPEMEncodedCertificate,
    unsigned int  *pdwStatus
)
{
    DWORD dwError = 0;
    DWORD dwTempStatus = 0;

    VMCA_LOG_DEBUG("Entering %s", __FUNCTION__);

    if (pszPEMEncodedCertificate == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCACheckAccess(IDL_handle, TRUE);
    BAIL_ON_VMCA_ERROR(dwError);

    // dw status never used?
    dwError = VMCAVerifyCertificate(
                pszPEMEncodedCertificate,
                &dwTempStatus);
    BAIL_ON_VMCA_ERROR(dwError);

    *pdwStatus = dwTempStatus;

cleanup:
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;
error:
    if (pdwStatus)
    {
        *pdwStatus = 0;
    }
    goto cleanup;
}

unsigned int
RpcVMCAFindCertificates(
    handle_t IDL_handle,
    unsigned int  dwSearchQueryLength,
    wchar16_t *pszSearchQuery,
    unsigned int  *dwCertificateCount,
    VMCA_CERTIFICATE_CONTAINER ** ppCertContainer
)
{
    DWORD dwError = 0;
    VMCA_LOG_DEBUG("Entering %s", __FUNCTION__);
    dwError = VMCACheckAccess(IDL_handle, FALSE);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;

error:
    goto cleanup;
}

unsigned int
RpcVMCAGetCertificateCount(
    handle_t IDL_handle,
    unsigned int dwStatus,
    unsigned int *pdwNumCertificates
)
{
    DWORD dwError = 0;
    DWORD dwTempNumCertificates = 0;

    VMCA_LOG_DEBUG("Entering %s", __FUNCTION__);

    if (pdwNumCertificates == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCACheckAccess(IDL_handle, TRUE);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetCertificateCount(
                dwStatus,
                &dwTempNumCertificates);
    BAIL_ON_VMCA_ERROR(dwError);

    *pdwNumCertificates = dwTempNumCertificates;

cleanup:
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;
error:
    if (pdwNumCertificates)
    {
        *pdwNumCertificates = 0;
    }
    goto cleanup;
}

unsigned int
RpcVMCAGetCRL(
    handle_t IDL_handle,
    unsigned char *pszClientCachedCRLID,
    unsigned int dwFileOffset,
    unsigned int dwSize,
    VMCA_FILE_BUFFER **ppCRLData
)
{
    DWORD dwError = 0;
    VMCA_FILE_BUFFER* pTempCRLData = NULL;
    VMCA_FILE_BUFFER* pCRLData = NULL;

    VMCA_LOG_DEBUG("Entering %s", __FUNCTION__);

    if(ppCRLData == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCACheckAccess(IDL_handle, FALSE);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError =  VMCAGetCRL(
        dwFileOffset,
        dwSize,
        &pTempCRLData);
    BAIL_ON_VMCA_ERROR(dwError);


   dwError = VMCARpcAllocateMemory
            (
            sizeof(pCRLData),
            (PVOID*) &pCRLData
            );
    BAIL_ON_VMCA_ERROR(dwError);
    pCRLData->dwCount = pTempCRLData->dwCount;
    if(pCRLData->dwCount > 0)
    {
        dwError = VMCARpcAllocateMemory(
                pCRLData->dwCount * sizeof(unsigned char),
                (PVOID*) &pCRLData->buffer);
        BAIL_ON_VMCA_ERROR(dwError);
        memcpy(
            (PVOID*) pCRLData->buffer,
            pTempCRLData->buffer,
            (size_t) pCRLData->dwCount);
    }

    *ppCRLData = pCRLData;
    pTempCRLData = NULL;

cleanup:
    if ( pTempCRLData )
    {
        VMCA_SAFE_FREE_MEMORY(pTempCRLData->buffer);
        VMCA_SAFE_FREE_MEMORY(pTempCRLData);
    }
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;
error:
    if (ppCRLData)
    {
        *ppCRLData = NULL;
    }

    if(pCRLData)
    {
        if(pCRLData->buffer)
        {
            VMCARpcFreeMemory((PVOID) pCRLData->buffer);
        }
        VMCARpcFreeMemory((PVOID)pCRLData);
    }

    goto cleanup;
}


/*
 * What is the difference between these two RPC methods?
 *  RpcVMCAReGenCRL()
 *  VMCARpcReGenCRL()
 */
unsigned int
VMCARpcReGenCRL(
    handle_t IDL_handle
    )
{
    DWORD dwError = 0;
    VMCA_LOG_DEBUG("Entering %s", __FUNCTION__);

    dwError = VMCACheckAccess(IDL_handle, TRUE);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaSrvReGenCRL(NULL);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;
error:
    goto cleanup;
}

unsigned int
RpcVMCAReGenCRL (
    handle_t IDL_handle
    )
{
    DWORD dwError = 0;

    VMCA_LOG_DEBUG("Entering %s", __FUNCTION__);

    /*
     * This is depricated. No one is using the client API.
     * However, this should not do anything if the existing
     * server-side RPC is called.
     */
    dwError = ERROR_INVALID_PARAMETER;
    BAIL_ON_VMCA_ERROR (dwError);

cleanup:
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;
error:
    goto cleanup;
}

unsigned int
RpcVMCAPublishRootCerts(
    handle_t IDL_handle
    )
{
    DWORD dwError = 0;

    VMCA_LOG_DEBUG("Entering %s", __FUNCTION__);

    dwError = VMCACheckAccess(IDL_handle, TRUE);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCASrvPublishRootCerts();
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;
error:
    goto cleanup;
}

unsigned int
VMCARpcRevokeCertificate(
    handle_t IDL_handle,
    wchar16_t *pszServerName,
    unsigned char *pszCertificate,
    unsigned int dwCertRevokeReason,
    wchar16_t *pszSharedSecret)
{
    DWORD dwError = 0;
    VMCA_CRL_REASON certRevokeReason = CRL_REASON_UNSPECIFIED;
    VMCA_LOG_DEBUG("Entering %s", __FUNCTION__);

    dwError = VMCACheckAccess(IDL_handle, TRUE);
    BAIL_ON_VMCA_ERROR(dwError);

    if(pszCertificate == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VmcaSrvValidateCRLReason (
                    dwCertRevokeReason,
                    &certRevokeReason
                    );
    BAIL_ON_VMCA_ERROR (dwError);
    /* TODO: Can the server be NULL? */

    dwError = VmcaSrvRevokeCertificate(
                    pszServerName,
                    pszCertificate,
                    certRevokeReason
                    );
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    VMCA_LOG_DEBUG("Exiting %s, Status = %d", __FUNCTION__, dwError);
    return dwError;
error:
    goto cleanup;
}
