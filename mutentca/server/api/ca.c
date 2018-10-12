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
    PSTR pszIssuer = NULL;
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

    dwError = LwCAValidateCertificate(pCert, pcszPrivateKey, pcszPassPhrase);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACheckIfCACert(pCert, &bIsCA);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bIsCA)
    {
        dwError = LWCA_NOT_CA_CERT;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAGetCertSubjectName(pCert, &pszSubject);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetCertIssuerName(pCert, &pszIssuer);
    BAIL_ON_LWCA_ERROR(dwError);

    //TODO: Secure manager APIs must be called to encrypt and store private key.

    dwError = LwCACreateCertArray((PSTR*)&pCertificate, 1 , &pCertArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateCAData(pszIssuer,
                                pszSubject,
                                pCertArray,
                                NULL,
                                NULL,
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
    LWCA_SAFE_FREE_STRINGA(pszIssuer);
    if (pCert)
    {
        X509_free(pCert);
    }

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
    X509 *pX509Cert = NULL;
    PLWCA_CERTIFICATE pCACert = NULL;
    PLWCA_CERTIFICATE_ARRAY pCACerts = NULL;
    PLWCA_CERTIFICATE pCert = NULL;
    BOOLEAN bIsCA = FALSE;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) ||
        !pCertRequest || !pValidity || !ppCertifcate)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    //TODO: Policy plugin must verify subjectname does not have wildcardstring
    //TODO: Policy plugin must verify subjectaltname does not have wildcardstring
    //TODO: Policy plugin must verify multiple SANs are not allowed

    dwError = _LwCACheckCAExist(pcszCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbGetCACertificates(pcszCAId, &pCACerts);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pCACerts->dwCount != 1)
    {
        dwError = LWCA_INVALID_CA_DATA;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pCACert = pCACerts->ppCertificates[0];

    dwError = LwCAGenerateX509Certificate(pCertRequest, pValidity, pCACert, &pX509Cert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACheckIfCACert(pX509Cert, &bIsCA);
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
    if (pX509Cert)
    {
        X509_free(pX509Cert);
    }
    LwCAFreeCertificates(pCACerts);
    return dwError;

error:
    LwCAFreeCertificate(pCert);
    if (ppCertifcate)
    {
        *ppCertifcate = NULL;
    }
    goto cleanup;
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

    dwError = LwCASignX509Certificate(
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
