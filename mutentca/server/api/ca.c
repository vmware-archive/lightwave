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
_LwCACheckCAExist(
    PCSTR pcszCAId
    );

static
DWORD
_LwCACheckCANotExist(
    PCSTR pcszCAId
    );

static
DWORD
_LwCASignX509Certificate(
    X509    *pCert,
    PCSTR   pcszCAId
    );

static
DWORD
_LwCAGetCurrentCACertificate(
    PCSTR               pcszCAId,
    PLWCA_CERTIFICATE   *ppCACert
    );

static
DWORD
_LwCAStoreIntermediateCA(
    X509        *pCert,
    PCSTR       pcszCAId,
    PCSTR       pcszParentCAId,
    PLWCA_KEY   pEncryptedKey
    );

DWORD
LwCACreateRootCA(
    PLWCA_REQ_CONTEXT       pReqCtx,
    PCSTR                   pcszRootCAId,
    PLWCA_CERTIFICATE       pCertificate,
    PCSTR                   pcszPrivateKey,
    PCSTR                   pcszPassPhrase
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE_ARRAY pCertArray = NULL;
    X509 *pCert = NULL;
    PSTR pszSubject = NULL;
    PLWCA_DB_CA_DATA pCAData = NULL;
    BOOLEAN bIsCA = FALSE;

    if (!pReqCtx || IsNullOrEmptyString(pcszRootCAId) ||
        !pCertificate || IsNullOrEmptyString(pcszPrivateKey)
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCACheckCANotExist(pcszRootCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAPEMToX509(pCertificate, &pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509ValidateCertificate(pCert, pcszPrivateKey, pcszPassPhrase);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509CheckIfCACert(pCert, &bIsCA);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bIsCA)
    {
        dwError = LWCA_NOT_CA_CERT;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAX509GetSubjectName(pCert, &pszSubject);
    BAIL_ON_LWCA_ERROR(dwError);

    //TODO: Secure manager APIs must be called to encrypt and store private key.

    dwError = LwCACreateCertArray((PSTR*)&pCertificate, 1 , &pCertArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateCAData(pszSubject,
                                pCertArray,
                                NULL,
                                NULL,
                                LWCA_CA_STATUS_ACTIVE,
                                &pCAData
                                );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbAddCA(pcszRootCAId, pCAData, NULL);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LwCAFreeCertificates(pCertArray);
    LwCADbFreeCAData(pCAData);
    LWCA_SAFE_FREE_STRINGA(pszSubject);
    LwCAX509Free(pCert);

    return dwError;

error:
    goto cleanup;
}

DWORD
LwCAGetCACertificates(
    PLWCA_REQ_CONTEXT       pReqCtx,
    PCSTR                   pcszCAId,
    PLWCA_CERTIFICATE_ARRAY *ppCertificates
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE_ARRAY pCertArray = NULL;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) || !ppCertificates )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCACheckCAExist(pcszCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbGetCACertificates(pcszCAId, &pCertArray);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCertificates = pCertArray;

cleanup:
    return dwError;

error:
    LwCAFreeCertificates(pCertArray);
    if (ppCertificates)
    {
        *ppCertificates = NULL;
    }
    goto cleanup;
}

DWORD
LwCAGetSignedCertificate(
    PLWCA_REQ_CONTEXT       pReqCtx,
    PCSTR                   pcszCAId,
    PLWCA_CERT_REQUEST      pCertRequest,
    PLWCA_CERT_VALIDITY     pValidity,
    LWCA_SIGNING_ALGORITHM  signAlgorithm,
    PLWCA_CERTIFICATE       *ppCertifcate
    )
{
    DWORD dwError = 0;
    X509_REQ *pRequest = NULL;
    X509 *pX509Cert = NULL;
    PLWCA_CERTIFICATE pCACert = NULL;
    PLWCA_CERTIFICATE pCert = NULL;
    BOOLEAN bIsCA = FALSE;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) ||
        !pCertRequest || !pValidity || !ppCertifcate)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAPEMToX509Req(pCertRequest, &pRequest);
    BAIL_ON_LWCA_ERROR(dwError);

    //TODO: Policy plugin must verify subjectname does not have wildcardstring
    //TODO: Policy plugin must verify subjectaltname does not have wildcardstring
    //TODO: Policy plugin must verify multiple SANs are not allowed

    dwError = _LwCACheckCAExist(pcszCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetCurrentCACertificate(pcszCAId, &pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateX509Certificate(pRequest, pValidity, pCACert, &pX509Cert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509CheckIfCACert(pX509Cert, &bIsCA);
    BAIL_ON_LWCA_ERROR(dwError);

    if (bIsCA)
    {
        LWCA_LOG_INFO("Request for a CA certificate is not allowed");
        dwError = LWCA_INVALID_CSR_FIELD;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCASignX509Certificate(pX509Cert, pcszCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509ToPEM(pX509Cert, &pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCertifcate = pCert;

cleanup:
    LwCAX509Free(pX509Cert);
    LwCAX509ReqFree(pRequest);
    return dwError;

error:
    LwCAFreeCertificate(pCert);
    if (ppCertifcate)
    {
        *ppCertifcate = NULL;
    }
    goto cleanup;
}

DWORD
LwCACreateIntermediateCA(
    PLWCA_REQ_CONTEXT               pReqCtx,
    PCSTR                           pcszCAId,
    PCSTR                           pcszParentCAId,
    PLWCA_INT_CA_REQ_DATA           pCARequest,
    PLWCA_CERT_VALIDITY             pValidity,
    PLWCA_CERTIFICATE_ARRAY         *ppCACerts
    )
{
    DWORD                       dwError = 0;
    DWORD                       dwKeyUsageConstraints = 0;
    time_t                      tmNotBefore;
    time_t                      tmNotAfter;
    struct tm                   *tm = NULL;
    BOOLEAN                     bIsCA = FALSE;
    PSTR                        pszPublicKey = NULL;
    PLWCA_KEY                   pEncryptedKey = NULL;
    PLWCA_STRING_ARRAY          pOrganizationList = NULL;
    X509                        *pX509CACert = NULL;
    X509                        *pX509ParentCACert = NULL;
    X509_REQ                    *pRequest = NULL;
    PLWCA_PKCS_10_REQ_DATA      pPKCSReq = NULL;
    PLWCA_CERTIFICATE           pParentCACert = NULL;
    PLWCA_CERTIFICATE           pCACert =  NULL;
    PLWCA_CERTIFICATE_ARRAY     pCACerts = NULL;
    PLWCA_CERT_VALIDITY         pTempValidity = NULL;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) || !ppCACerts ||
        !pCARequest || IsNullOrEmptyString(pcszParentCAId)
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCACheckCANotExist(pcszCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCACheckCAExist(pcszParentCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetCurrentCACertificate(pcszParentCAId, &pParentCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAPEMToX509(pParentCACert, &pX509ParentCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetOrganizations(pX509ParentCACert, &pOrganizationList);
    BAIL_ON_LWCA_ERROR(dwError);

    dwKeyUsageConstraints = (LWCA_KEY_USAGE_FLAG_KEY_CERT_SIGN |
                                LWCA_KEY_USAGE_FLAG_KEY_CRL_SIGN);

    dwError = LwCACreatePKCSRequest(pcszCAId,
                            NULL,
                            pCARequest->pCountryList,
                            pCARequest->pLocalityList,
                            pCARequest->pStateList,
                            pOrganizationList,
                            pCARequest->pOUList,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            dwKeyUsageConstraints,
                            &pPKCSReq
                            );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAKmCreateKeyPair(pcszCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAKmGetPublickey(pcszCAId, &pszPublicKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError =  LwCACreateCertificateSignRequest(pPKCSReq, pszPublicKey, &pRequest);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAKmSignX509Request(pRequest, pcszCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pValidity)
    {
        dwError = LwCACreateCertValidity(pValidity->tmNotBefore,
                                        pValidity->tmNotAfter,
                                        &pTempValidity
                                        );
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else
    {
        tmNotBefore = time(NULL);
        tm = localtime(&tmNotBefore);
        //TODO: Get CA Cert duration from policy engine
        tm->tm_yday += LWCA_MAX_INT_CA_CERT_DURATION;
        tmNotAfter = mktime(tm);

        dwError = LwCACreateCertValidity(tmNotBefore,
                                        tmNotAfter,
                                        &pTempValidity
                                        );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError =  LwCAGenerateX509Certificate(pRequest, pTempValidity, pParentCACert, &pX509CACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509CheckIfCACert(pX509CACert, &bIsCA);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bIsCA)
    {
        dwError = LWCA_UNKNOWN_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAKmSignX509Cert(pX509CACert, pcszParentCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAKmGetEncryptedKey(pcszCAId, &pEncryptedKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509ToPEM(pX509CACert, &pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACreateCertArray((PSTR*)&pCACert, 1 , &pCACerts);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAStoreIntermediateCA(
                                    pX509CACert,
                                    pcszCAId,
                                    pcszParentCAId,
                                    pEncryptedKey
                                    );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCACerts = pCACerts;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszPublicKey);
    LwCAFreeStringArray(pOrganizationList);
    LwCAFreePKCSRequest(pPKCSReq);
    LwCAX509ReqFree(pRequest);
    LwCAX509Free(pX509CACert);
    LwCAX509Free(pX509ParentCACert);
    LwCAFreeCertificate(pParentCACert);
    LwCAFreeCertificate(pCACert);
    LwCAFreeCertValidity(pTempValidity);

    return dwError;

error:
    LwCAFreeCertificates(pCACerts);
    if (ppCACerts)
    {
        *ppCACerts = NULL;
    }
    goto cleanup;
}

DWORD
LwCARevokeCertificate(
    PLWCA_REQ_CONTEXT       pReqCtx,
    PCSTR                   pcszCAId,
    PLWCA_CERTIFICATE       pCertificate
    )
{
    DWORD dwError = 0;
    return dwError;

}
static
DWORD
_LwCACheckCAExist(
    PCSTR pcszCAId
    )
{
    DWORD dwError = 0;
    BOOLEAN bCAExists = FALSE;

    dwError = LwCADbCheckCA(pcszCAId, &bCAExists);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bCAExists)
    {
        dwError = LWCA_CA_MISSING;
    }
    BAIL_ON_LWCA_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
_LwCACheckCANotExist(
    PCSTR pcszCAId
    )
{
    DWORD dwError = 0;
    BOOLEAN bCAExists = FALSE;

    dwError = LwCADbCheckCA(pcszCAId, &bCAExists);
    BAIL_ON_LWCA_ERROR(dwError);

    if (bCAExists)
    {
        dwError = LWCA_CA_ALREADY_EXISTS;
    }
    BAIL_ON_LWCA_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
_LwCAGetCurrentCACertificate(
    PCSTR               pcszCAId,
    PLWCA_CERTIFICATE   *ppCACert
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE_ARRAY pCACerts = NULL;
    PLWCA_CERTIFICATE pCACert = NULL;

    dwError = LwCADbGetCACertificates(pcszCAId, &pCACerts);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pCACerts->dwCount != 1)
    {
        dwError = LWCA_INVALID_CA_DATA;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCACreateCertificate(pCACerts->ppCertificates[0], &pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCACert = pCACert;

cleanup:
    LwCAFreeCertificates(pCACerts);
    return dwError;

error:
    LwCAFreeCertificate(pCACert);
    if (ppCACert)
    {
        *ppCACert = NULL;
    }

    goto cleanup;
}

static
DWORD
_LwCASignX509Certificate(
    X509    *pCert,
    PCSTR   pcszCAId
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CA_DATA pCAData = NULL;
    PSTR pszPrivateKey = NULL;

    //TODO: This method will be re-implemented
    // after secure manager is integrated.

    dwError = LwCADbGetCA(pcszCAId, &pCAData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringWithLengthA(
                        pCAData->pEncryptedPrivateKey->pData,
                        pCAData->pEncryptedPrivateKey->dwLength,
                        &pszPrivateKey
                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509SignCertificate(
                        pCert,
                        pszPrivateKey,
                        NULL
                        );
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszPrivateKey);
    LwCADbFreeCAData(pCAData);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_LwCAStoreIntermediateCA(
    X509        *pCert,
    PCSTR       pcszCAId,
    PCSTR       pcszParentCAId,
    PLWCA_KEY   pEncryptedKey
    )
{
    DWORD                   dwError = 0;
    PLWCA_CERTIFICATE       pCertificate = NULL;
    PLWCA_CERTIFICATE_ARRAY pCertificates = NULL;
    PLWCA_DB_CA_DATA        pCAData = NULL;
    PSTR                    pszSubject = NULL;
    
    dwError = LwCAX509ToPEM(pCert, &pCertificate);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetSubjectName(pCert, &pszSubject);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACreateCertArray((PSTR*)&pCertificate, 1 , &pCertificates);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateCAData(pszSubject,
                                pCertificates,
                                pEncryptedKey,
                                NULL,
                                LWCA_CA_STATUS_ACTIVE,
                                &pCAData
                                );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbAddCA(pcszCAId, pCAData, pcszParentCAId);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LwCAFreeCertificate(pCertificate);
    LwCAFreeCertificates(pCertificates);
    LWCA_SAFE_FREE_STRINGA(pszSubject);
    LwCADbFreeCAData(pCAData);
    return dwError;

error:
    goto cleanup;
}
