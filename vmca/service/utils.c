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



/*
 * Module Name: utils.c
 *
 * Filename: utils.c
 *
 * Abstract:
 *
 * Utility functions
 *
 */

#include "includes.h"

VMCA_DB_CERTIFICATE_STATUS
VMCAMapToDBStatus(CERTIFICATE_STATUS st)
{
	switch(st)
	{
	case CERTIFICATE_ACTIVE : return VMCA_DB_CERTIFICATE_STATUS_ACTIVE;
	case CERTIFICATE_REVOKED : return VMCA_DB_CERTIFICATE_STATUS_REVOKED;
	case CERTIFIFCATE_EXPIRED : return VMCA_DB_CERTIFICATE_STATUS_EXPIRED;
	case CERTIFICATE_ALL : return VMCA_DB_CERTIFICATE_STATUS_ALL;
	}
     return VMCA_DB_CERTIFICATE_STATUS_ALL;
}

DWORD
VMCAHeartbeatInit(
    PVMAFD_HB_HANDLE *ppHandle
    )
{
    DWORD dwError = 0;
    PVMAFD_HB_HANDLE pHandle = NULL;

    if (!ppHandle)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VmAfdStartHeartbeatA(
                                VMCA_EVENT_SOURCE,
                                2014,
                                &pHandle
                                );
    BAIL_ON_VMCA_ERROR(dwError);

    *ppHandle = pHandle;

cleanup:

    return dwError;
error:

    if (ppHandle)
    {
        *ppHandle = NULL;
    }
    if (pHandle)
    {
        VmAfdStopHeartbeat(pHandle);
    }

    goto cleanup;
}

VOID
VMCAStopHeartbeat(
    PVMAFD_HB_HANDLE pHandle
    )
{
    if (pHandle)
    {
        VmAfdStopHeartbeat(pHandle);
    }
}

static DWORD
VMCASetCSRAuthorityInfoAccess(
    STACK_OF(X509_EXTENSION) *pStack,
    X509 *pCert,
    X509 *pIssuer
    )
{
    DWORD dwError = 0;
    X509V3_CTX ctx;
    X509_EXTENSION *pExtension = NULL;
    PSTR pszIPAddress = NULL;
    PSTR pszAIAString = NULL;

    X509V3_set_ctx_nodb(&ctx);
    X509V3_set_ctx(&ctx, pIssuer, pCert, NULL, NULL, 0);

    dwError = VmAfdGetPNIDA(NULL, &pszIPAddress);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringPrintfA(
                                &pszAIAString,
                                "caIssuers;URI:https://%s/afd/vecs/ssl",
                                pszIPAddress);
    BAIL_ON_VMCA_ERROR(dwError);

    pExtension = X509V3_EXT_conf_nid(
                                NULL,
                                &ctx,
                                NID_info_access,
                                (char*)pszAIAString);
    if (pExtension == NULL)
    {
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    sk_X509_EXTENSION_push(pStack, pExtension);
error:
    VMCA_SAFE_FREE_MEMORY(pszIPAddress);
    VMCA_SAFE_FREE_MEMORY(pszAIAString);
    return dwError;
}

static DWORD
VMCASetAuthorityKeyIdentifier(
    STACK_OF(X509_EXTENSION) *pStack,
    X509 *pCert,
    X509 *pIssuer
    )
{
    DWORD dwError = 0;
    X509V3_CTX ctx;
    X509_EXTENSION *pExtension = NULL;

    X509V3_set_ctx_nodb(&ctx);
    X509V3_set_ctx(&ctx, pIssuer, pCert, NULL, NULL, 0);

    pExtension = X509V3_EXT_conf_nid(
                                NULL,
                                &ctx,
                                NID_authority_key_identifier,
                                "keyid");
    if (pExtension == NULL)
    {
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    sk_X509_EXTENSION_push(pStack, pExtension);
error:
    return dwError;
}

DWORD
VMCACopyExtensions(
    X509 *pCertificate,
    X509 *pCACertificate,
    X509_REQ *pRequest
    )
{
    DWORD dwError = 0;
    STACK_OF(X509_EXTENSION) *pStack  = NULL;
    X509_EXTENSION *pExtension = NULL;
    int extCount = 0;
    int Counter = 0;

    pStack = X509_REQ_get_extensions(pRequest);
    if(pStack == NULL) {
        goto error; // nothing to do here, just get out
    }

    // Copy AuthorityKeyId from CA certificate
    dwError = VMCASetAuthorityKeyIdentifier(pStack, pCertificate, pCACertificate);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCASetCSRAuthorityInfoAccess(pStack, pCertificate, pCACertificate);
    BAIL_ON_VMCA_ERROR(dwError);

    extCount = sk_X509_EXTENSION_num(pStack);
    for(Counter = 0; Counter < extCount; Counter ++)
    {
        pExtension = sk_X509_EXTENSION_value(pStack, Counter);

        // TODO : Clean up the Extensions, and have
        // Policy on duplicate extension ext.
        // We should probably log all this information before
        // returning the certificate too.

        dwError = X509_add_ext(pCertificate, pExtension, -1);
        BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_EXT_ERR);
    }

error:
    if(pStack) {
        sk_X509_EXTENSION_pop_free(pStack, X509_EXTENSION_free);
    }
    return dwError;
}

DWORD
VMCASignedRequestPrivate(
    PVMCA_X509_CA pCA,
    PSTR pszPKCS10Request,
    PSTR *ppszCertificate,
    time_t tmNotBefore,
    time_t tmNotAfter
)
// VMCASignedRequestPrivate takes and CSR and signs the request
//
//Arguments :
//      pCA : The CA class that can sign the request
//      pszPKCS19Request : The Request that needs to be signed
//      ppszCertificate : Points to a PEM encoded Signed Cert
//      tmNotBefore : A Valid Time String that indicates when the Certificate is Valid From
//      tmNotAfter : The End of certificates validity
// Returns :
//  Error Code
{

    DWORD dwError = 0;
    X509_REQ *pRequest = NULL;
    EVP_PKEY *pPublicKey = NULL;
    X509 *pCertificate = NULL;
    X509_NAME *pSubjName = NULL;
    X509_NAME *pCAName = NULL;
    PSTR pszStartTime = NULL;
    PSTR pszEndTime = NULL;
    PSTR pTempCertString = NULL;
    PSTR pTempCertChainString = NULL;
    const EVP_MD *digest = EVP_sha256();
    ASN1_INTEGER *aiSerial = NULL;
    time_t tmNow = 0;

    if  ( (pCA == NULL) ||
          ( pszPKCS10Request == NULL ) ||
          (ppszCertificate == NULL)) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAPEMToCSR(pszPKCS10Request, &pRequest);
    BAIL_ON_VMCA_ERROR(dwError);

    if ((pPublicKey = X509_REQ_get_pubkey(pRequest)) == NULL )
    {
        VMCA_LOG_INFO("VMCASignedRequestPrivate: CSR does not have a public key");
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }
    //
    // Verify the Public Key is good and the signature is
    // indeed for that key.
    //
    if (pPublicKey->type != EVP_PKEY_RSA ||
        BN_num_bits(pPublicKey->pkey.rsa->n) < VMCA_MIN_CA_CERT_PRIV_KEY_LENGTH)
    {
        VMCA_LOG_INFO("VMCASignedRequestPrivate: Key length not supported");
        dwError = VMCA_ERROR_INVALID_KEY_LENGTH;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    time(&tmNow);
    if (tmNotBefore < (tmNow - VMCA_VALIDITY_SYNC_BACK_DATE))
    {
        VMCA_LOG_INFO("VMCASignedRequestPrivate: Invalid start date");
        dwError = VMCA_INVALID_TIME_SPECIFIED;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if ((tmNotAfter - tmNotBefore) > VMCA_MAX_CERT_DURATION)      // 10. year
    {
        VMCA_LOG_INFO("VMCASignedRequestPrivate: Invalid validity period requested");
        dwError = VMCA_INVALID_TIME_SPECIFIED;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = X509_REQ_verify(pRequest, pPublicKey);
    BAIL_ON_SSL_ERROR(dwError, VMCA_INVALID_CSR_FIELD);

    pSubjName = X509_REQ_get_subject_name(pRequest);
    if( pSubjName == NULL)
    {
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if ( X509_NAME_entry_count(pSubjName) == 0 )
    {
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pCertificate = X509_new();
    if(pCertificate == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    // VMCA Supports only X509V3 only
    dwError = X509_set_version(pCertificate, 2);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CERT_IO_FAILURE);

    aiSerial = ASN1_INTEGER_new();
    if (aiSerial == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    VMCAGenerateX509Serial(aiSerial);
    X509_set_serialNumber(pCertificate,aiSerial);

    dwError = X509_set_subject_name(pCertificate, pSubjName);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CERT_IO_FAILURE);

    pCAName = X509_get_subject_name(pCA->pCertificate);
    if ( pCAName == NULL) {
        dwError = VMCA_CERT_IO_FAILURE;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = X509_set_issuer_name(pCertificate, pCAName);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CERT_IO_FAILURE);

    dwError = X509_set_pubkey(pCertificate, pPublicKey);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CERT_IO_FAILURE);

    if (X509_cmp_time(X509_get_notBefore(pCA->pCertificate), &tmNotBefore) >= 0)
    {
        VMCA_LOG_INFO("VMCASignedRequestPrivate: Invalid validity period requested");
        dwError = VMCA_SSL_SET_START_TIME;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!ASN1_TIME_set(X509_get_notBefore(pCertificate), tmNotBefore)){
        dwError = 0;
        BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_SET_START_TIME);
    }

    // If the request is beyond CA cert validity use CA cert validity
    if (X509_cmp_time(X509_get_notAfter(pCA->pCertificate), &tmNotAfter) <= 0)
    {
        VMCA_LOG_INFO("VMCASignedRequestPrivate: Using CA certs not after field");
        if(!ASN1_TIME_set_string(X509_get_notAfter(pCertificate),
                                 X509_get_notAfter(pCA->pCertificate)->data))
        {
            dwError = 0;
            BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_SET_END_TIME);
        }
    }
    else
    {
        if(!ASN1_TIME_set(X509_get_notAfter(pCertificate), tmNotAfter))
        {
            dwError = 0;
            BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_SET_END_TIME);
        }
    }

    dwError = VMCACopyExtensions(pCertificate, pCA->pCertificate, pRequest);
    BAIL_ON_VMCA_ERROR(dwError);

    if (X509_check_ca(pCertificate))
    {
        VMCA_LOG_INFO("Request for a CA certificate is not allowed");
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (((pCertificate->ex_flags & EXFLAG_KUSAGE) &&
         (pCertificate->ex_kusage & KU_KEY_CERT_SIGN)))
    {
        VMCA_LOG_INFO("Request for a certificate signing cert is not allowed");
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (((pCertificate->ex_flags & EXFLAG_KUSAGE) &&
         (pCertificate->ex_kusage & KU_CRL_SIGN)))
    {
        VMCA_LOG_INFO("Request for a CRL signing cert is not allowed");
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (((pCertificate->ex_flags & EXFLAG_KUSAGE) &&
         (pCertificate->ex_kusage & KU_DATA_ENCIPHERMENT)))
    {
        VMCA_LOG_INFO("Request for a cert with data encryption key usage is not allowed");
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAVerifyCertificateName(pCertificate);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAVerifySubjectAltNames(pCertificate);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = X509_sign (pCertificate, pCA->pKey, digest);
    BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_SIGN_FAIL);

    dwError = VMCACertToPEM(pCertificate, &pTempCertString);
    BAIL_ON_VMCA_ERROR(dwError);

    if (!VMCAIsSelfSignedCert(pCA->pCertificate))
    {
        dwError = VMCAAllocateStringPrintfA(
                                            &pTempCertChainString,
                                            "%s\n%s",
                                            pTempCertString,
                                            pCA->pszCertificate
                                           );
        BAIL_ON_VMCA_ERROR(dwError);
    }

    *ppszCertificate = pTempCertChainString?pTempCertChainString:pTempCertString;
cleanup:

    if(pPublicKey != NULL) {
        EVP_PKEY_free(pPublicKey);
    }

    if (pRequest != NULL) {
        X509_REQ_free(pRequest);
    }

    if( pszStartTime != NULL) {
        VMCAFreeStringA(pszStartTime);
    }

    if(pszEndTime != NULL) {
        VMCAFreeStringA(pszEndTime);
    }

    if(pCertificate != NULL) {
        X509_free(pCertificate);
    }
    if(aiSerial != NULL){
        ASN1_INTEGER_free(aiSerial);
    }
    if (pTempCertChainString)
    {
        VMCA_SAFE_FREE_STRINGA (pTempCertString);
    }

    return dwError;
error :
    if (ppszCertificate)
    {
        *ppszCertificate = NULL;
    }
    if(pTempCertString != NULL){
        VMCAFreeStringA(pTempCertString);
        pTempCertString = NULL;
    }
    VMCA_SAFE_FREE_MEMORY (pTempCertChainString);
    goto cleanup;
}
