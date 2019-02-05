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
_LwCAStoreIntermediateCA(
    X509                            *pCert,
    PCSTR                           pcszCAId,
    X509                            *pParentCACert,
    PLWCA_DB_ROOT_CERT_DATA         pParentRootCertData,
    PCSTR                           pcszParentCAId,
    PLWCA_KEY                       pEncryptedKey,
    PCSTR                           pcszAuthBlob,
    PLWCA_CERTIFICATE_ARRAY         *ppCACerts
    );

static
DWORD
_LwCACreateSelfSignedRootCACert(
    PLWCA_REQ_CONTEXT       pReqCtx,
    PCSTR                   pcszRootCAId,
    PLWCA_PKCS_10_REQ_DATA  pCARequest,
    X509                    **ppX509CACert,
    PSTR                    *ppszPrivateKey
    );

static
DWORD
_LwCACAExists(
    PCSTR               pcszCAId,
    PBOOLEAN            pbCAExists,
    PBOOLEAN            pbCAIsActive
    );

static
DWORD
_LwCABuildChainOfTrust(
    PLWCA_DB_ROOT_CERT_DATA     pIssuerCARootData,
    PLWCA_CERTIFICATE           pCARootCert,
    PSTR                        *ppszChainOfTrust
    );

static
DWORD
_LwCAGenerateCACertCRLInfo(
    PCSTR                       pcszCRLNumber,
    PSTR                        *ppszNextCRLNumber,
    PSTR                        *ppszLastCRLUpdate,
    PSTR                        *ppszNextCRLUpdate
    );

static
DWORD
_LwCAGenerateCertRevocationInfo(
    X509        *pX509Cert,
    PSTR        *ppszRevokedDate,
    PSTR        *ppszTimeValidFrom,
    PSTR        *ppszTimeValidTo
    );


/*
 * Lightwave Root CA will be initialized based on the config file provided.
 * This method takes json object which is refered to "config" section
 *
 * Required fields:
 *    - name
 *
 * Below is the sample config file:
 *{
 * "config": {
 *      "name": "LightwaveCA-US",
 *      "domainName": "lightwave.local",
 *      "organization": "VMware Inc",
 *      "organizationUnit": "VMware Inc",
 *      "country": "US",
 *      "state": "WA",
 *      "locality": "Bellevue",
 *      "certificateFile": "<file path to certificate>",
 *      "privateKeyFile": "<file path to privatekey>",
 *      "passphraseFile": "<file path to passphrase>"
 *      }
 *}
*/

DWORD
LwCAInitCA(
    PLWCA_JSON_OBJECT pConfig
    )
{
    DWORD dwError = 0;
    DWORD dwKeyUsageConstraints = 0;
    PSTR pszName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszOrganziation = NULL;
    PSTR pszOrganizationUnit = NULL;
    PSTR pszCountry = NULL;
    PSTR pszState = NULL;
    PSTR pszLocality = NULL;
    PSTR pszCertificate = NULL;
    PSTR pszPrivateKey = NULL;
    PSTR pszPassPhrase = NULL;
    PSTR pszCertificateFile = NULL;
    PSTR pszPrivateKeyFile = NULL;
    PSTR pszPassPhraseFile = NULL;
    PLWCA_STRING_ARRAY pOrganizationList = NULL;
    PLWCA_STRING_ARRAY pOUList = NULL;
    PLWCA_STRING_ARRAY pCountryList = NULL;
    PLWCA_STRING_ARRAY pStateList = NULL;
    PLWCA_STRING_ARRAY pLocalityList = NULL;
    PLWCA_PKCS_10_REQ_DATA pPKCSReq = NULL;
    PLWCA_REQ_CONTEXT pReqCtx = NULL;
    BOOLEAN bLocked = FALSE;

    if (!pConfig)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAGetRootCAId(&pszName);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(
                                pConfig,
                                TRUE,
                                LWCA_CA_CONFIG_KEY_DOMAIN_NAME,
                                &pszDomainName);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(
                                pConfig,
                                TRUE,
                                LWCA_CA_CONFIG_KEY_ORGANIZATION,
                                &pszOrganziation);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(
                                pConfig,
                                TRUE,
                                LWCA_CA_CONFIG_KEY_ORGANIZATION_UNIT,
                                &pszOrganizationUnit);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(
                                pConfig,
                                TRUE,
                                LWCA_CA_CONFIG_KEY_COUNTRY,
                                &pszCountry);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(
                                pConfig,
                                TRUE,
                                LWCA_CA_CONFIG_KEY_STATE,
                                &pszState);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(
                                pConfig,
                                TRUE,
                                LWCA_CA_CONFIG_KEY_LOCALITY,
                                &pszLocality);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(
                                pConfig,
                                TRUE,
                                LWCA_CA_CONFIG_KEY_CERTIFICATE_FILE,
                                &pszCertificateFile);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(
                                pConfig,
                                TRUE,
                                LWCA_CA_CONFIG_KEY_PRIVATEKEY_FILE,
                                &pszPrivateKeyFile);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(
                                pConfig,
                                TRUE,
                                LWCA_CA_CONFIG_KEY_PASSPHRASE_FILE,
                                &pszPassPhraseFile);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!IsNullOrEmptyString(pszOrganziation))
    {
        dwError = LwCACreateStringArray((PSTR*)&pszOrganziation, 1, &pOrganizationList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszOrganizationUnit))
    {
        dwError = LwCACreateStringArray((PSTR*)&pszOrganizationUnit, 1, &pOUList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszCountry))
    {
        dwError = LwCACreateStringArray((PSTR*)&pszCountry, 1, &pCountryList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszState))
    {
        dwError = LwCACreateStringArray((PSTR*)&pszState, 1, &pStateList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszLocality))
    {
        dwError = LwCACreateStringArray((PSTR*)&pszLocality, 1, &pLocalityList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszCertificateFile))
    {
        dwError = LwCAReadFileToString(pszCertificateFile, &pszCertificate);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszPrivateKeyFile))
    {
        dwError = LwCAReadFileToString(pszPrivateKeyFile, &pszPrivateKey);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszPassPhraseFile))
    {
        dwError = LwCAReadFileToString(pszPassPhraseFile, &pszPassPhrase);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwKeyUsageConstraints = (LWCA_KEY_USAGE_FLAG_KEY_CERT_SIGN |
                                LWCA_KEY_USAGE_FLAG_KEY_CRL_SIGN);

    dwError = LwCACreatePKCSRequest(
                            pszName,
                            pszDomainName,
                            pCountryList,
                            pLocalityList,
                            pStateList,
                            pOrganizationList,
                            pOUList,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            dwKeyUsageConstraints,
                            &pPKCSReq
                            );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(LWCA_REQ_CONTEXT), (PVOID *)&pReqCtx);
    BAIL_ON_LWCA_ERROR(dwError);

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gApiGlobals.mutex, bLocked);

    dwError = LwCACreateRootCA(
                        pReqCtx,
                        pszName,
                        pPKCSReq,
                        pszCertificate,
                        pszPrivateKey,
                        pszPassPhrase);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gApiGlobals.mutex, bLocked);
    LWCA_SAFE_FREE_STRINGA(pszName);
    LWCA_SAFE_FREE_STRINGA(pszDomainName);
    LWCA_SAFE_FREE_STRINGA(pszOrganziation);
    LWCA_SAFE_FREE_STRINGA(pszOrganizationUnit);
    LWCA_SAFE_FREE_STRINGA(pszCountry);
    LWCA_SAFE_FREE_STRINGA(pszState);
    LWCA_SAFE_FREE_STRINGA(pszLocality);
    LWCA_SAFE_FREE_STRINGA(pszCertificate);
    LWCA_SECURE_SAFE_FREE_MEMORY(pszPrivateKey, LwCAStringLenA(pszPrivateKey));
    LWCA_SECURE_SAFE_FREE_MEMORY(pszPassPhrase, LwCAStringLenA(pszPassPhrase));
    LWCA_SAFE_FREE_STRINGA(pszCertificateFile);
    LWCA_SAFE_FREE_STRINGA(pszPrivateKeyFile);
    LWCA_SAFE_FREE_STRINGA(pszPassPhraseFile);
    LwCAFreeStringArray(pOrganizationList);
    LwCAFreeStringArray(pOUList);
    LwCAFreeStringArray(pCountryList);
    LwCAFreeStringArray(pStateList);
    LwCAFreeStringArray(pLocalityList);
    LwCAFreePKCSRequest(pPKCSReq);
    LwCARequestContextFree(pReqCtx);
    return dwError;

error:
    if (dwError == LWCA_ROOT_CA_ALREADY_EXISTS)
    {
        dwError = 0;
        goto cleanup;
    }
    LWCA_LOCK_MUTEX_UNLOCK(&gApiGlobals.mutex, bLocked);
    LwCAFreeCACtx();
    goto cleanup;
}

VOID
LwCAFreeCACtx(
    )
{
    BOOLEAN bLocked = FALSE;

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gApiGlobals.mutex, bLocked);

    LWCA_LOCK_MUTEX_UNLOCK(&gApiGlobals.mutex, bLocked);
}

DWORD
LwCACreateRootCA(
    PLWCA_REQ_CONTEXT           pReqCtx,
    PCSTR                       pcszRootCAId,
    PLWCA_PKCS_10_REQ_DATA      pCARequest,
    PLWCA_CERTIFICATE           pCertificate,
    PCSTR                       pcszPrivateKey,
    PCSTR                       pcszPassPhrase
    )
{
    DWORD                       dwError = 0;
    X509                        *pX509CACert = NULL;
    PSTR                        pszSubject = NULL;
    PSTR                        pszSerial = NULL;
    PSTR                        pszSKI = NULL;
    PSTR                        pszAKI = NULL;
    PSTR                        pszCRLNumber = NULL;
    PSTR                        pszLastCRLUpdate = NULL;
    PSTR                        pszNextCRLUpdate = NULL;
    PSTR                        pszPrivateKey = NULL;
    PSTR                        pszAuthBlob = NULL;
    PLWCA_DB_CA_DATA            pCAData = NULL;
    PLWCA_DB_CERT_DATA          pCertData = NULL;
    PLWCA_DB_ROOT_CERT_DATA     pRootCertData = NULL;
    PLWCA_CERTIFICATE           pCACert =  NULL;
    BOOLEAN                     bDoesCAExist = FALSE;
    BOOLEAN                     bIsCA = FALSE;
    PLWCA_KEY                   pEncryptedKey = NULL;
    LWCA_METRICS_RESPONSE_CODES responseCode = LWCA_METRICS_RESPONSE_SUCCESS;
    uint64_t                    iStartTime = LwCAGetTimeInMilliSec();
    uint64_t                    iEndTime = iStartTime;

    if (!pReqCtx || IsNullOrEmptyString(pcszRootCAId) ||
        !((pCertificate && !IsNullOrEmptyString(pcszPrivateKey)) || pCARequest)
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCACAExists(pcszRootCAId, &bDoesCAExist, NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    if (bDoesCAExist)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ROOT_CA_ALREADY_EXISTS);
    }

    if (pCertificate)
    {
        dwError = LwCAPEMToX509(pCertificate, &pX509CACert);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAX509ValidateCertificate(pX509CACert, pcszPrivateKey, pcszPassPhrase);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAAllocateStringA(pcszPrivateKey, &pszPrivateKey);
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else
    {
        dwError = _LwCACreateSelfSignedRootCACert(
                                pReqCtx,
                                pcszRootCAId,
                                pCARequest,
                                &pX509CACert,
                                &pszPrivateKey);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAX509CheckIfCACert(pX509CACert, &bIsCA);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bIsCA)
    {
        dwError = LWCA_NOT_CA_CERT;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCASecurityAddKeyPair(pcszRootCAId, pszPrivateKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetSubjectName(pX509CACert, &pszSubject);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetSerialNumber(pX509CACert, &pszSerial);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetSubjectKeyIdentifier(pX509CACert, &pszSKI);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetAuthorityKeyIdentifier(pX509CACert, &pszAKI);
    if (dwError == LWCA_SSL_INVALID_AKI)
    {
        dwError = 0;
    }
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509ToPEM(pX509CACert, &pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateCRLNumber(&pszCRLNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateCrlTimestamps(
                        LWCA_CRL_DEFAULT_CRL_VALIDITY,
                        &pszLastCRLUpdate,
                        &pszNextCRLUpdate);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCASecurityGetEncryptedKey(pcszRootCAId, &pEncryptedKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateCertData(
                    NULL,
                    pszSerial,
                    NULL,
                    pszSKI,
                    pszAKI,
                    NULL,
                    NULL,
                    NULL,
                    0,
                    LWCA_CERT_STATUS_ACTIVE,
                    &pCertData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateRootCertData(
                    pcszRootCAId,
                    pCertData,
                    pCACert,
                    pEncryptedKey,
                    (PSTR)pCACert,
                    pszCRLNumber,
                    pszLastCRLUpdate,
                    pszNextCRLUpdate,
                    &pRootCertData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateCAData(
                    pszSubject,
                    NULL,
                    pszSKI,
                    pszAuthBlob,
                    LWCA_CA_STATUS_ACTIVE,
                    &pCAData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbAddCA(pcszRootCAId, pCAData, pRootCertData);
    BAIL_ON_LWCA_ERROR(dwError);


cleanup:
    LwCAFreeCertificate(pCACert);
    LwCADbFreeCertData(pCertData);
    LwCADbFreeRootCertData(pRootCertData);
    LwCADbFreeCAData(pCAData);
    LWCA_SAFE_FREE_STRINGA(pszSubject);
    LWCA_SAFE_FREE_STRINGA(pszSerial);
    LWCA_SAFE_FREE_STRINGA(pszSKI);
    LWCA_SAFE_FREE_STRINGA(pszAKI);
    LWCA_SAFE_FREE_STRINGA(pszCRLNumber);
    LWCA_SAFE_FREE_STRINGA(pszLastCRLUpdate);
    LWCA_SAFE_FREE_STRINGA(pszNextCRLUpdate);
    LWCA_SAFE_FREE_STRINGA(pszAuthBlob);
    LWCA_SECURE_SAFE_FREE_MEMORY(pszPrivateKey, LwCAStringLenA(pszPrivateKey));
    LwCAX509Free(pX509CACert);
    LwCAFreeKey(pEncryptedKey);

    iEndTime = LwCAGetTimeInMilliSec();
    LwCAApiMetricsUpdate(LWCA_METRICS_API_CREATE_ROOT_CA,
                        responseCode,
                        iStartTime,
                        iEndTime);

    return dwError;

error:
    responseCode = LWCA_METRICS_RESPONSE_ERROR;

    LWCA_LOG_ERROR(
        "[%s:%d] Failed to create root CA. requestID: (%s), error: (%d)",
        __FUNCTION__,
        __LINE__,
        LWCA_SAFE_STRING(pReqCtx->pszRequestId),
        dwError);

    goto cleanup;
}

DWORD
LwCAGetCACertificates(
    PLWCA_REQ_CONTEXT               pReqCtx,
    PCSTR                           pcszCAId,
    LWCA_DB_GET_CERTS_FLAGS         certsToGet,
    PCSTR                           pcszSKI,
    PLWCA_CERTIFICATE_ARRAY         *ppCertificates
    )
{
    DWORD                           dwError = 0;
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   pCACertArray= NULL;
    PLWCA_CERTIFICATE_ARRAY         pCertArray = NULL;
    BOOLEAN                         bDoesCAExist = FALSE;
    BOOLEAN                         bIsCAActive = FALSE;
    BOOLEAN                         bAuthorized = FALSE;
    LWCA_METRICS_RESPONSE_CODES     responseCode = LWCA_METRICS_RESPONSE_SUCCESS;
    uint64_t                        iStartTime = LwCAGetTimeInMilliSec();
    uint64_t                        iEndTime = iStartTime;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) || !ppCertificates)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCACAExists(pcszCAId, &bDoesCAExist, &bIsCAActive);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bDoesCAExist)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CA_MISSING);
    }
    if (!bIsCAActive)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CA_REVOKED);
    }

    dwError = LwCAAuthZCheckAccess(
                    pReqCtx,
                    pcszCAId,
                    NULL,
                    LWCA_AUTHZ_GET_CA_CERT_PERMISSION,
                    &bAuthorized);
    BAIL_ON_LWCA_ERROR(dwError);
    if (!bAuthorized)
    {
        LWCA_LOG_ALERT(
                "[%s:%d] UPN (%s) is unauthorized to get the root cert of CA ID: (%s) Req ID: (%s)",
                __FUNCTION__,
                __LINE__,
                LWCA_SAFE_STRING(pReqCtx->pszBindUPN),
                pcszCAId,
                LWCA_SAFE_STRING(pReqCtx->pszRequestId));
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_AUTHZ_UNAUTHORIZED);
    }

    dwError = LwCADbGetCACerts(certsToGet, pcszCAId, pcszSKI, &pCACertArray);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!pCACertArray || pCACertArray->dwCount == 0)
    {
        // If the response is empty, that means an active CA does not have CA
        // certs.  That is wrong, and the data is corrupt.
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_INVALID_CA_DATA);
    }

    dwError = LwCARootCertArrayToCertArray(pCACertArray, &pCertArray);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCertificates = pCertArray;

cleanup:

    LwCADbFreeRootCertDataArray(pCACertArray);

    iEndTime = LwCAGetTimeInMilliSec();
    LwCAApiMetricsUpdate(LWCA_METRICS_API_GET_CA_CERTIFICATES,
                        responseCode,
                        iStartTime,
                        iEndTime);

    return dwError;

error:
    responseCode = LWCA_METRICS_RESPONSE_ERROR;

    LWCA_LOG_ERROR(
        "[%s:%d] Failed to get CA certificates. caID: (%s), requestID: (%s), error: (%d)",
        __FUNCTION__,
        __LINE__,
        pcszCAId,
        LWCA_SAFE_STRING(pReqCtx->pszRequestId),
        dwError);

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
    LWCA_AUTHZ_X509_DATA x509Data = { 0 };
    time_t tmNotBefore;
    time_t tmNotAfter;
    PLWCA_CERT_VALIDITY pTempValidity = NULL;
    DWORD dwDuration = 0;
    BOOLEAN bAuthorized = FALSE;
    BOOLEAN bIsCA = FALSE;
    BOOLEAN bDoesCAExist = FALSE;
    BOOLEAN bIsCAActive = FALSE;
    LWCA_METRICS_RESPONSE_CODES responseCode = LWCA_METRICS_RESPONSE_SUCCESS;
    uint64_t iStartTime = LwCAGetTimeInMilliSec();
    uint64_t iEndTime = iStartTime;
    PSTR pszCAIssuers = NULL;
    PLWCA_DB_ROOT_CERT_DATA_ARRAY pCACertArray = NULL;
    PLWCA_CERTIFICATE pCert = NULL;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) ||
        !pCertRequest || !ppCertifcate)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCACAExists(pcszCAId, &bDoesCAExist, &bIsCAActive);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bDoesCAExist)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CA_MISSING);
    }
    if (!bIsCAActive)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CA_REVOKED);
    }

    dwError = LwCAPEMToX509Req(pCertRequest, &pRequest);
    BAIL_ON_LWCA_ERROR(dwError);

    x509Data.pX509Req = pRequest;

    dwError = LwCAAuthZCheckAccess(
                    pReqCtx,
                    pcszCAId,
                    &x509Data,
                    LWCA_AUTHZ_CERT_SIGN_PERMISSION,
                    &bAuthorized);
    BAIL_ON_LWCA_ERROR(dwError);
    if (!bAuthorized)
    {
        LWCA_LOG_ALERT(
                "[%s:%d] UPN (%s) is unauthorized to obtain a signed certificate! CA ID: (%s) Req ID: (%s)",
                __FUNCTION__,
                __LINE__,
                LWCA_SAFE_STRING(pReqCtx->pszBindUPN),
                pcszCAId,
                LWCA_SAFE_STRING(pReqCtx->pszRequestId));
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_AUTHZ_UNAUTHORIZED);
    }

    dwError = LwCAPolicyValidate(
                gpPolicyCtx,
                pReqCtx,
                pRequest,
                pValidity,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_ALL);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pValidity)
    {
        dwError = LwCACreateCertValidity(
                        pValidity->tmNotBefore,
                        pValidity->tmNotAfter,
                        &pTempValidity);
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else
    {
        dwError = LwCAPolicyGetCertDuration(gpPolicyCtx, LWCA_POLICY_TYPE_CERTIFICATE,  &dwDuration);
        BAIL_ON_LWCA_ERROR(dwError);

        if (dwDuration == 0)
        {
            dwDuration = LWCA_DEFAULT_CERT_DURATION;
        }

        tmNotBefore = time(NULL);
        tmNotAfter = tmNotBefore + dwDuration;

        dwError = LwCACreateCertValidity(tmNotBefore, tmNotAfter, &pTempValidity);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCADbGetCACerts(LWCA_DB_GET_ACTIVE_CA_CERT, pcszCAId, NULL, &pCACertArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetCAEndpoint(pcszCAId, &pszCAIssuers);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateX509Certificate(
                        pRequest,
                        pTempValidity,
                        pCACertArray->ppRootCertData[0]->pRootCertPEM,
                        pszCAIssuers,
                        &pX509Cert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509CheckIfCACert(pX509Cert, &bIsCA);
    BAIL_ON_LWCA_ERROR(dwError);

    if (bIsCA)
    {
        LWCA_LOG_INFO("Request for a CA certificate is not allowed");
        dwError = LWCA_INVALID_CSR_FIELD;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCASecuritySignX509Cert(pcszCAId, pX509Cert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509ToPEM(pX509Cert, &pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCertifcate = pCert;

cleanup:
    LwCAX509Free(pX509Cert);
    LwCADbFreeRootCertDataArray(pCACertArray);
    LwCAX509ReqFree(pRequest);
    LwCAFreeCertValidity(pTempValidity);
    LWCA_SAFE_FREE_STRINGA(pszCAIssuers);

    iEndTime = LwCAGetTimeInMilliSec();
    LwCAApiMetricsUpdate(LWCA_METRICS_API_GET_SIGNED_CERTIFICATE,
                        responseCode,
                        iStartTime,
                        iEndTime);
    return dwError;

error:
    responseCode = LWCA_METRICS_RESPONSE_ERROR;

    LWCA_LOG_ERROR(
        "[%s:%d] Failed to sign certificate. caID: (%s), requestID: (%s), error: (%d)",
        __FUNCTION__,
        __LINE__,
        pcszCAId,
        LWCA_SAFE_STRING(pReqCtx->pszRequestId),
        dwError);

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
    DWORD                           dwError = 0;
    DWORD                           dwKeyUsageConstraints = 0;
    DWORD                           dwDuration = 0;
    time_t                          tmNotBefore;
    time_t                          tmNotAfter;
    BOOLEAN                         bIsCA = FALSE;
    BOOLEAN                         bAuthorized = FALSE;
    BOOLEAN                         bDoesCAExist = FALSE;
    BOOLEAN                         bIsCAActive = FALSE;
    PSTR                            pszPublicKey = NULL;
    PSTR                            pszParentCAId = NULL;
    PSTR                            pszAuthBlob = NULL;
    PSTR                            pszCAIssuers = NULL;
    PLWCA_KEY                       pEncryptedKey = NULL;
    PLWCA_STRING_ARRAY              pOrganizationList = NULL;
    X509                            *pX509CACert = NULL;
    X509                            *pX509ParentCACert = NULL;
    X509_REQ                        *pRequest = NULL;
    PLWCA_PKCS_10_REQ_DATA          pPKCSReq = NULL;
    LWCA_AUTHZ_X509_DATA            x509Data = { 0 };
    PLWCA_DB_CA_DATA                pCAData = NULL;
    PLWCA_DB_CA_DATA                pParentCAData = NULL;
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   pParentCACert = NULL;
    PLWCA_CERTIFICATE               pCACert =  NULL;
    PLWCA_CERTIFICATE_ARRAY         pCACerts = NULL;
    PLWCA_CERT_VALIDITY             pTempValidity = NULL;
    LWCA_METRICS_RESPONSE_CODES     responseCode = LWCA_METRICS_RESPONSE_SUCCESS;
    uint64_t                        iStartTime = LwCAGetTimeInMilliSec();
    uint64_t                        iEndTime = iStartTime;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) || !ppCACerts || !pCARequest)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszParentCAId))
    {
        dwError =  LwCAAllocateStringA(pcszParentCAId, &pszParentCAId);
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else
    {
        dwError = LwCAGetRootCAId(&pszParentCAId);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCACAExists(pszParentCAId, &bDoesCAExist, &bIsCAActive);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bDoesCAExist)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CA_MISSING);
    }
    if (!bIsCAActive)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CA_REVOKED);
    }

    bDoesCAExist = FALSE;
    dwError = _LwCACAExists(pcszCAId, &bDoesCAExist, NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    if (bDoesCAExist)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CA_ALREADY_EXISTS);
    }

    dwError = LwCADbGetCACerts(LWCA_DB_GET_ACTIVE_CA_CERT, pszParentCAId, NULL, &pParentCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAPEMToX509(pParentCACert->ppRootCertData[0]->pRootCertPEM, &pX509ParentCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetOrganizations(pX509ParentCACert, &pOrganizationList);
    BAIL_ON_LWCA_ERROR(dwError);

    dwKeyUsageConstraints = (LWCA_KEY_USAGE_FLAG_KEY_CERT_SIGN |
                             LWCA_KEY_USAGE_FLAG_KEY_CRL_SIGN);

    dwError = LwCACreatePKCSRequest(
                        pcszCAId,
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
                        &pPKCSReq);
    BAIL_ON_LWCA_ERROR(dwError);

    /*
     * in the normal security workflow, db layer will be called
     * from within the CreateKeyPair implementation to store
     * encrypted key. Since this is during creation, db layer
     * is not ready to store partial info. therefore, a storage
     * cache is maintained which caches encrypted key and clears
     * on GetEncryptedKey call. Fetches are always done from db
     * layer.
     */
    dwError = LwCASecurityCreateKeyPair(pcszCAId, &pszPublicKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError =  LwCACreateCertificateSignRequest(pPKCSReq, pszPublicKey, &pRequest);
    BAIL_ON_LWCA_ERROR(dwError);

    x509Data.pX509Req = pRequest;

    dwError = LwCAAuthZCheckAccess(
                    pReqCtx,
                    pszParentCAId,
                    &x509Data,
                    LWCA_AUTHZ_CA_CREATE_PERMISSION,
                    &bAuthorized);
    BAIL_ON_LWCA_ERROR(dwError);
    if (!bAuthorized)
    {
        LWCA_LOG_ALERT(
                "[%s:%d] UPN (%s) is unauthorized to create an intermediate! CA ID (%s) Req ID: (%s)",
                __FUNCTION__,
                __LINE__,
                LWCA_SAFE_STRING(pReqCtx->pszBindUPN),
                pcszCAId,
                LWCA_SAFE_STRING(pReqCtx->pszRequestId));
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_AUTHZ_UNAUTHORIZED);
    }

    dwError = LwCAPolicyValidate(
                gpPolicyCtx,
                pReqCtx,
                pRequest,
                pValidity,
                LWCA_POLICY_TYPE_CA,
                LWCA_POLICY_CHECK_ALL);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCASecuritySignX509Request(pcszCAId, pRequest);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pValidity)
    {
        dwError = LwCACreateCertValidity(
                            pValidity->tmNotBefore,
                            pValidity->tmNotAfter,
                            &pTempValidity);
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else
    {
        dwError = LwCAPolicyGetCertDuration(gpPolicyCtx, LWCA_POLICY_TYPE_CA, &dwDuration);
        BAIL_ON_LWCA_ERROR(dwError);

        if (dwDuration == 0)
        {
            dwDuration = LWCA_DEFAULT_CERT_DURATION;
        }

        tmNotBefore = time(NULL);
        tmNotAfter = tmNotBefore + dwDuration;

        dwError = LwCACreateCertValidity(tmNotBefore, tmNotAfter, &pTempValidity);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAGetCAEndpoint(pszParentCAId, &pszCAIssuers);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateX509Certificate(
                        pRequest,
                        pTempValidity,
                        pParentCACert->ppRootCertData[0]->pRootCertPEM,
                        pszCAIssuers,
                        &pX509CACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509CheckIfCACert(pX509CACert, &bIsCA);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bIsCA)
    {
        dwError = LWCA_UNKNOWN_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCASecuritySignX509Cert(pszParentCAId, pX509CACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCASecurityGetEncryptedKey(pcszCAId, &pEncryptedKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509ToPEM(pX509CACert, &pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAStoreIntermediateCA(
                        pX509CACert,
                        pcszCAId,
                        pX509ParentCACert,
                        pParentCACert->ppRootCertData[0],
                        pszParentCAId,
                        pEncryptedKey,
                        pszAuthBlob,
                        &pCACerts);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCACerts = pCACerts;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszPublicKey);
    LWCA_SAFE_FREE_STRINGA(pszParentCAId);
    LWCA_SAFE_FREE_STRINGA(pszAuthBlob);
    LWCA_SAFE_FREE_STRINGA(pszCAIssuers);
    LwCAFreeStringArray(pOrganizationList);
    LwCAFreePKCSRequest(pPKCSReq);
    LwCAX509ReqFree(pRequest);
    LwCAX509Free(pX509CACert);
    LwCAX509Free(pX509ParentCACert);
    LwCADbFreeCAData(pCAData);
    LwCADbFreeCAData(pParentCAData);
    LwCADbFreeRootCertDataArray(pParentCACert);
    LwCAFreeCertificate(pCACert);
    LwCAFreeCertValidity(pTempValidity);
    LwCAFreeKey(pEncryptedKey);

    iEndTime = LwCAGetTimeInMilliSec();
    LwCAApiMetricsUpdate(LWCA_METRICS_API_CREATE_INTERMEDIATE_CA,
                        responseCode,
                        iStartTime,
                        iEndTime);

    return dwError;

error:
    responseCode = LWCA_METRICS_RESPONSE_ERROR;

    LWCA_LOG_ERROR(
        "[%s:%d] Failed to create intermedidate CA. caID: (%s), parentcaID: (%s), requestID: (%s), error: (%d)",
        __FUNCTION__,
        __LINE__,
        pcszCAId,
        pcszParentCAId,
        LWCA_SAFE_STRING(pReqCtx->pszRequestId),
        dwError);

    LwCAFreeCertificates(pCACerts);
    if (ppCACerts)
    {
        *ppCACerts = NULL;
    }
    goto cleanup;
}

DWORD
LwCARevokeCertificate(
    PLWCA_REQ_CONTEXT           pReqCtx,
    PCSTR                       pcszCAId,
    PLWCA_CERTIFICATE           pCertificate
    )
{
    DWORD dwError = 0;
    X509 *pX509Cert = NULL;
    X509 *pX509CACert = NULL;
    PSTR pszCertSKI = NULL;
    PSTR pszCertAKI = NULL;
    PSTR pszIssuer = NULL;
    PSTR pszSerialNumber = NULL;
    PSTR pszCRLNumber = NULL;
    PSTR pszNextCRLNumber = NULL;
    PSTR pszLastCRLUpdate = NULL;
    PSTR pszNextCRLUpdate = NULL;
    PSTR pszTimeValidTo = NULL;
    PSTR pszTimeValidFrom = NULL;
    PSTR pszRevokedDate = NULL;
    PSTR pszAuthBlob = NULL;
    PLWCA_DB_ROOT_CERT_DATA_ARRAY pCACertArray = NULL;
    PLWCA_DB_ROOT_CERT_DATA pCACert = NULL;
    PLWCA_DB_CERT_DATA_ARRAY pCertArray = NULL;
    PLWCA_DB_CERT_DATA pCert = NULL;
    LWCA_AUTHZ_X509_DATA x509Data = { 0 };
    BOOLEAN bAuthorized = FALSE;
    BOOLEAN bDoesCAExist = FALSE;
    BOOLEAN bIsCAActive = FALSE;
    LWCA_METRICS_RESPONSE_CODES responseCode = LWCA_METRICS_RESPONSE_SUCCESS;
    uint64_t iStartTime = LwCAGetTimeInMilliSec();
    uint64_t iEndTime = iStartTime;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) || !pCertificate)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCACAExists(pcszCAId, &bDoesCAExist, &bIsCAActive);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bDoesCAExist)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CA_MISSING);
    }
    if (!bIsCAActive)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CA_REVOKED);
    }

    dwError = LwCAPEMToX509(pCertificate, &pX509Cert);
    BAIL_ON_LWCA_ERROR(dwError);

    x509Data.pX509Cert = pX509Cert;

    dwError = LwCAX509GetSubjectKeyIdentifier(pX509Cert, &pszCertSKI);
    if (dwError == LWCA_CERT_DECODE_FAILURE)
    {
        // NOTE: SKI is not required to be in CSR.  However, it is required in
        // CA trusted root certs.  That's why this exception is only here.
        dwError = 0;
    }
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetAuthorityKeyIdentifier(pX509Cert, &pszCertAKI);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetIssuerCommonName(pX509Cert, &pszIssuer);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbGetCACerts(LWCA_DB_GET_CERT_VIA_SKI, pcszCAId, pszCertAKI, &pCACertArray);
    BAIL_ON_LWCA_ERROR(dwError);

    if (LwCAStringCompareA(pszIssuer, pcszCAId, FALSE) ||
        LwCAStringCompareA(pszIssuer, pCACertArray->ppRootCertData[0]->pszCAId, FALSE))
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_INVALID_CA_FOR_CERT_REVOKE);
    }

    dwError = LwCAPEMToX509(pCACertArray->ppRootCertData[0]->pRootCertPEM, &pX509CACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAuthZCheckAccess(
                    pReqCtx,
                    pcszCAId,
                    &x509Data,
                    LWCA_AUTHZ_CERT_REVOKE_PERMISSION,
                    &bAuthorized);
    BAIL_ON_LWCA_ERROR(dwError);
    if (!bAuthorized)
    {
        LWCA_LOG_ALERT(
                "[%s:%d] UPN (%s) is unauthorized to revoke a certificate! CA ID: (%s) Req ID: (%s)",
                __FUNCTION__,
                __LINE__,
                LWCA_SAFE_STRING(pReqCtx->pszBindUPN),
                pcszCAId,
                LWCA_SAFE_STRING(pReqCtx->pszRequestId));
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_AUTHZ_UNAUTHORIZED);
    }

    dwError = LwCAVerifyCertificateSign(pX509Cert, pX509CACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetSerialNumber(pX509Cert, &pszSerialNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbGetCerts(LWCA_DB_GET_CERT_VIA_SERIAL, pcszCAId, pszSerialNumber, pszCertAKI, &pCertArray);
    if (dwError == LWCA_ERROR_ENTRY_NOT_FOUND)
    {
        // if no cert, proceed as it can be revoked
        dwError = 0;
    }
    BAIL_ON_LWCA_ERROR(dwError);

    if (pCertArray &&
        pCertArray->dwCount > 0 &&
        // TODO: update the last condition to not use 0 when we support multiple roots
        pCertArray->ppCertData[0]->status == LWCA_CERT_STATUS_INACTIVE)
    {
        // If found, if the status is inactive, we cannot revoke.
        // This happens if a CA root cert is already revoked.
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CRL_CERT_ALREADY_REVOKED);
    }


    dwError = _LwCAGenerateCertRevocationInfo(
                        pX509Cert,
                        &pszRevokedDate,
                        &pszTimeValidFrom,
                        &pszTimeValidTo);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateCertData(
                        pszIssuer,
                        pszSerialNumber,
                        pCACertArray->ppRootCertData[0]->pRootCertData->pszSerialNumber,
                        pszCertSKI,
                        pszCertAKI,
                        pszRevokedDate,
                        pszTimeValidFrom,
                        pszTimeValidTo,
                        CRL_REASON_UNSPECIFIED,
                        LWCA_CERT_STATUS_INACTIVE,
                        &pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbAddCert(pcszCAId, pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGenerateCACertCRLInfo(
                            pCACertArray->ppRootCertData[0]->pszCRLNumber,
                            &pszNextCRLNumber,
                            &pszLastCRLUpdate,
                            &pszNextCRLUpdate);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateRootCertData(
                            NULL,
                            pCACertArray->ppRootCertData[0]->pRootCertData,
                            NULL,
                            NULL,
                            NULL,
                            pszNextCRLNumber,
                            pszLastCRLUpdate,
                            pszNextCRLUpdate,
                            &pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbUpdateCACert(pcszCAId, pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszIssuer);
    LWCA_SAFE_FREE_STRINGA(pszCertSKI);
    LWCA_SAFE_FREE_STRINGA(pszCertAKI);
    LWCA_SAFE_FREE_STRINGA(pszSerialNumber);
    LWCA_SAFE_FREE_STRINGA(pszCRLNumber);
    LWCA_SAFE_FREE_STRINGA(pszNextCRLNumber);
    LWCA_SAFE_FREE_STRINGA(pszTimeValidFrom);
    LWCA_SAFE_FREE_STRINGA(pszTimeValidTo);
    LWCA_SAFE_FREE_STRINGA(pszRevokedDate);
    LWCA_SAFE_FREE_STRINGA(pszLastCRLUpdate);
    LWCA_SAFE_FREE_STRINGA(pszNextCRLUpdate);
    LWCA_SAFE_FREE_STRINGA(pszAuthBlob);
    LwCADbFreeRootCertDataArray(pCACertArray);
    LwCADbFreeRootCertData(pCACert);
    LwCADbFreeCertDataArray(pCertArray);
    LwCADbFreeCertData(pCert);
    LwCAX509Free(pX509Cert);
    LwCAX509Free(pX509CACert);

    iEndTime = LwCAGetTimeInMilliSec();
    LwCAApiMetricsUpdate(LWCA_METRICS_API_REVOKE_CERTIFICATE,
                        responseCode,
                        iStartTime,
                        iEndTime);

    return dwError;

error:
    responseCode = LWCA_METRICS_RESPONSE_ERROR;

    LWCA_LOG_ERROR(
        "[%s:%d] Failed to revoke certificate. caID: (%s), requestID: (%s), error: (%d)",
        __FUNCTION__,
        __LINE__,
        pcszCAId,
        LWCA_SAFE_STRING(pReqCtx->pszRequestId),
        dwError);

    if (dwError == LWCA_SSL_CERT_VERIFY_ERR ||
        dwError == LWCA_CERT_IO_FAILURE)
    {
        dwError = LWCA_ERROR_INVALID_REQUEST;
    }
    goto cleanup;
}

DWORD
LwCARevokeIntermediateCA(
    PLWCA_REQ_CONTEXT       pReqCtx,
    PCSTR                   pcszCAId
    )
{
    DWORD dwError = 0;
    PSTR pszTimeValidFrom = NULL;
    PSTR pszTimeValidTo = NULL;
    PSTR pszRevokedDate = NULL;
    PSTR pszNextCRLNumber = NULL;
    PSTR pszLastCRLUpdate = NULL;
    PSTR pszNextCRLUpdate = NULL;
    BOOLEAN bDoesCAExist = FALSE;
    BOOLEAN bIsCAActive = FALSE;
    BOOLEAN bAuthorized = FALSE;
    PLWCA_DB_CA_DATA pCAData = NULL;
    PLWCA_DB_CERT_DATA pCACert = NULL;
    PLWCA_DB_ROOT_CERT_DATA pIssuerCACert = NULL;
    PLWCA_DB_ROOT_CERT_DATA_ARRAY pCACertArray = NULL;
    PLWCA_DB_ROOT_CERT_DATA_ARRAY pIssuerCACertArray = NULL;
    X509 *pX509Cert = NULL;
    LWCA_AUTHZ_X509_DATA x509Data = { 0 };
    LWCA_METRICS_RESPONSE_CODES responseCode = LWCA_METRICS_RESPONSE_SUCCESS;
    uint64_t iStartTime = LwCAGetTimeInMilliSec();
    uint64_t iEndTime = iStartTime;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCACAExists(pcszCAId, &bDoesCAExist, &bIsCAActive);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bDoesCAExist)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CA_MISSING);
    }
    if (!bIsCAActive)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CA_ALREADY_REVOKED);
    }

    /*
     * TODO:
     *
     *      In the future, when we support multiple root certs per CA, we need to
     *      modify this logic such that it is able to revoke either the entire CA
     *      (All trusted roots for a CA), or a particular CA root cert.
     *
     *      Currently, it assumes there is only one CA root cert (the active one) and
     *      it revokes that particular one, in effect, revoking the whole CA too.
     */
    dwError = LwCADbGetCACerts(LWCA_DB_GET_ACTIVE_CA_CERT, pcszCAId, NULL, &pCACertArray);
    BAIL_ON_LWCA_ERROR(dwError);

    /*
     * TODO:
     *
     *      Same as TODO above
     */
    dwError = LwCADbGetCACerts(
                        LWCA_DB_GET_ACTIVE_CA_CERT,
                        pCACertArray->ppRootCertData[0]->pRootCertData->pszIssuer,
                        NULL,
                        &pIssuerCACertArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAPEMToX509(pCACertArray->ppRootCertData[0]->pRootCertPEM, &pX509Cert);
    BAIL_ON_LWCA_ERROR(dwError);

    x509Data.pX509Cert = pX509Cert;

    dwError = LwCAAuthZCheckAccess(
                    pReqCtx,
                    pcszCAId,
                    &x509Data,
                    LWCA_AUTHZ_CA_REVOKE_PERMISSION,
                    &bAuthorized);
    BAIL_ON_LWCA_ERROR(dwError);
    if (!bAuthorized)
    {
        LWCA_LOG_ALERT(
                "[%s:%d] UPN (%s) is unauthorized to revoke a certificate! CA ID: (%s) Req ID: (%s)",
                __FUNCTION__,
                __LINE__,
                LWCA_SAFE_STRING(pReqCtx->pszBindUPN),
                pcszCAId,
                LWCA_SAFE_STRING(pReqCtx->pszRequestId));
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_AUTHZ_UNAUTHORIZED);
    }

    dwError = _LwCAGenerateCertRevocationInfo(
                        pX509Cert,
                        &pszRevokedDate,
                        &pszTimeValidFrom,
                        &pszTimeValidTo);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGenerateCACertCRLInfo(
                        pIssuerCACertArray->ppRootCertData[0]->pszCRLNumber,
                        &pszNextCRLNumber,
                        &pszLastCRLUpdate,
                        &pszNextCRLUpdate);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateCertData(
                        NULL,
                        pCACertArray->ppRootCertData[0]->pRootCertData->pszSerialNumber,
                        NULL,
                        NULL,
                        NULL,
                        pszRevokedDate,
                        pszTimeValidFrom,
                        pszTimeValidTo,
                        CRL_REASON_UNSPECIFIED,
                        LWCA_CERT_STATUS_INACTIVE,
                        &pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateRootCertData(
                        NULL,
                        pIssuerCACertArray->ppRootCertData[0]->pRootCertData,
                        NULL,
                        NULL,
                        NULL,
                        pszNextCRLNumber,
                        pszLastCRLUpdate,
                        pszNextCRLUpdate,
                        &pIssuerCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    /*
     * TODO:
     *
     *      Since we only support one trusted root per CA, it is safe to assume
     *      that revoking a CA's trusted root cert is revoking the active, and
     *      only, cert.  This means we must set the CA's status to 0.
     *      Once we support multiple roots per CA, this logic should be modified
     *      check if there are other active root certs, update the CA object's
     *      attribute, and only mark the CA as inactive if there are no more active
     *      ones.
     */
    dwError = LwCADbCreateCAData(
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        LWCA_CA_STATUS_INACTIVE,
                        &pCAData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbUpdateCert(pcszCAId, pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbUpdateCACert(
                        pCACertArray->ppRootCertData[0]->pRootCertData->pszIssuer,
                        pIssuerCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbUpdateCA(pcszCAId, pCAData);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LwCADbFreeCertData(pCACert);
    LwCADbFreeRootCertData(pIssuerCACert);
    LwCADbFreeCAData(pCAData);
    LwCADbFreeRootCertDataArray(pIssuerCACertArray);
    LwCADbFreeRootCertDataArray(pCACertArray);
    LwCAX509Free(pX509Cert);
    LWCA_SAFE_FREE_STRINGA(pszNextCRLNumber);
    LWCA_SAFE_FREE_STRINGA(pszLastCRLUpdate);
    LWCA_SAFE_FREE_STRINGA(pszNextCRLUpdate);
    LWCA_SAFE_FREE_STRINGA(pszRevokedDate);
    LWCA_SAFE_FREE_STRINGA(pszTimeValidFrom);
    LWCA_SAFE_FREE_STRINGA(pszTimeValidTo);

    iEndTime = LwCAGetTimeInMilliSec();
    LwCAApiMetricsUpdate(LWCA_METRICS_API_REVOKE_INTERMEDIATE_CA,
                        responseCode,
                        iStartTime,
                        iEndTime);

    return dwError;

error:
    responseCode = LWCA_METRICS_RESPONSE_ERROR;

    LWCA_LOG_ERROR(
        "[%s:%d] Failed to revoke CA. caID: (%s), requestID: (%s), error: (%d)",
        __FUNCTION__,
        __LINE__,
        pcszCAId,
        LWCA_SAFE_STRING(pReqCtx->pszRequestId),
        dwError);

    if (dwError == LWCA_CRL_CERT_ALREADY_REVOKED
        || dwError == LWCA_CA_REVOKED)
    {
        dwError = LWCA_CA_ALREADY_REVOKED;
    }
    goto cleanup;
}

DWORD
LwCAGetCACrl(
    PLWCA_REQ_CONTEXT       pReqCtx,
    PCSTR                   pcszCAId,
    PLWCA_CRL               *ppCrl
    )
{
    DWORD dwError = 0;
    BOOLEAN bDoesCAExist = FALSE;
    BOOLEAN bIsCAActive = FALSE;
    BOOLEAN bAuthorized = FALSE;
    PLWCA_DB_ROOT_CERT_DATA_ARRAY pCACertArray = NULL;
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray = NULL;
    X509 *pX509CACert = NULL;
    X509_CRL *pX509Crl = NULL;
    PSTR pszCAIssuers = NULL;
    LWCA_AUTHZ_X509_DATA x509Data = { 0 };
    LWCA_METRICS_RESPONSE_CODES responseCode = LWCA_METRICS_RESPONSE_SUCCESS;
    uint64_t iStartTime = LwCAGetTimeInMilliSec();
    uint64_t iEndTime = iStartTime;
    PLWCA_CRL pCrl = NULL;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) || !ppCrl)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCACAExists(pcszCAId, &bDoesCAExist, &bIsCAActive);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bDoesCAExist)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CA_MISSING);
    }
    if (!bIsCAActive)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CA_REVOKED);
    }

    /*
     * TODO:
     *      When we support multiple CA root certs, modify this line and subsequent code
     *      to (1) get all CA root certs, and (2) build CRL that has all of the CRL entries
     *      Currently, we assume only one cert and use the active CA cert
     */
    dwError = LwCADbGetCACerts(LWCA_DB_GET_ACTIVE_CA_CERT, pcszCAId, NULL, &pCACertArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAPEMToX509(pCACertArray->ppRootCertData[0]->pRootCertPEM, &pX509CACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbGetCerts(
                    LWCA_DB_GET_REVOKED_CERTS,
                    pcszCAId,
                    NULL,
                    pCACertArray->ppRootCertData[0]->pRootCertData->pszSKI,
                    &pCertDataArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetCAEndpoint(pcszCAId, &pszCAIssuers);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateX509Crl(
                    pCACertArray->ppRootCertData[0]->pszCRLNumber,
                    pCACertArray->ppRootCertData[0]->pszLastCRLUpdate,
                    pCACertArray->ppRootCertData[0]->pszNextCRLUpdate,
                    pCertDataArray,
                    pX509CACert,
                    pszCAIssuers,
                    &pX509Crl);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError =  LwCASecuritySignX509Crl(pcszCAId, pX509Crl);
    BAIL_ON_LWCA_ERROR(dwError);

    x509Data.pX509Crl = pX509Crl;

    dwError = LwCAAuthZCheckAccess(
                    pReqCtx,
                    pcszCAId,
                    &x509Data,
                    LWCA_AUTHZ_GET_CA_CRL_PERMISSION,
                    &bAuthorized);
    BAIL_ON_LWCA_ERROR(dwError);
    if (!bAuthorized)
    {
        LWCA_LOG_ALERT(
                "[%s:%d] UPN (%s) is unauthorized to get the crl of CA ID: (%s) Req ID: (%s)",
                __FUNCTION__,
                __LINE__,
                LWCA_SAFE_STRING(pReqCtx->pszBindUPN),
                pcszCAId,
                LWCA_SAFE_STRING(pReqCtx->pszRequestId));
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_AUTHZ_UNAUTHORIZED);
    }

    dwError = LwCAX509CrlToPEM(pX509Crl, &pCrl);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCrl = pCrl;

cleanup:
    LwCADbFreeCertDataArray(pCertDataArray);
    LwCADbFreeRootCertDataArray(pCACertArray);
    LwCAX509Free(pX509CACert);
    LwCAX509CrlFree(pX509Crl);
    LWCA_SAFE_FREE_STRINGA(pszCAIssuers);

    iEndTime = LwCAGetTimeInMilliSec();
    LwCAApiMetricsUpdate(LWCA_METRICS_API_GET_CA_CRL,
                        responseCode,
                        iStartTime,
                        iEndTime);

    return dwError;

error:
    responseCode = LWCA_METRICS_RESPONSE_ERROR;

    LWCA_LOG_ERROR(
        "[%s:%d] Failed to get CRL. caID: (%s), requestID: (%s), error: (%d)",
        __FUNCTION__,
        __LINE__,
        pcszCAId,
        LWCA_SAFE_STRING(pReqCtx->pszRequestId),
        dwError);

    LwCAFreeCrl(pCrl);
    if (ppCrl)
    {
        *ppCrl = NULL;
    }
    goto cleanup;
}

DWORD
LwCAGetChainOfTrust(
    PLWCA_REQ_CONTEXT               pReqCtx,
    PCSTR                           pcszCAId,
    PSTR                            *ppszChainOfTrust
    )
{
    DWORD                           dwError = 0;
    BOOLEAN                         bDoesCAExist = FALSE;
    BOOLEAN                         bIsCAActive = FALSE;
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   pCACertArray = NULL;
    PSTR                            pszChainOfTrust = NULL;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) || !ppszChainOfTrust)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    dwError = _LwCACAExists(pcszCAId, &bDoesCAExist, &bIsCAActive);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bDoesCAExist)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CA_MISSING);
    }
    if (!bIsCAActive)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_CA_REVOKED);
    }

    /*
     * TODO:
     *
     *      This must be modified once we support multiple CA root certs.
     *      Currently, it assumes that there is only one, the active root cert,
     *      and retrieves the chain of trust of it.
     */
    dwError = LwCADbGetCACerts(LWCA_DB_GET_ACTIVE_CA_CERT, pcszCAId, NULL, &pCACertArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pCACertArray->ppRootCertData[0]->pszChainOfTrust, &pszChainOfTrust);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszChainOfTrust = pszChainOfTrust;

cleanup:

    LwCADbFreeRootCertDataArray(pCACertArray);

    return dwError;

error:

    LWCA_SAFE_FREE_STRINGA(pszChainOfTrust);
    if (ppszChainOfTrust)
    {
        *ppszChainOfTrust = NULL;
    }

    goto cleanup;
}


static
DWORD
_LwCAStoreIntermediateCA(
    X509                            *pCert,
    PCSTR                           pcszCAId,
    X509                            *pParentCACert,
    PLWCA_DB_ROOT_CERT_DATA         pParentRootCertData,
    PCSTR                           pcszParentCAId,
    PLWCA_KEY                       pEncryptedKey,
    PCSTR                           pcszAuthBlob,
    PLWCA_CERTIFICATE_ARRAY         *ppCACerts
    )
{
    DWORD                           dwError = 0;
    PLWCA_CERTIFICATE               pCertificate = NULL;
    PLWCA_DB_CA_DATA                pCAData = NULL;
    PLWCA_DB_CERT_DATA              pCertData = NULL;
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   pCACertDataArray = NULL;
    PLWCA_DB_ROOT_CERT_DATA         pRootCertData = NULL;
    PSTR                            pszSubject = NULL;
    PSTR                            pszSerialNumber = NULL;
    PSTR                            pszIssuerSerialNumber = NULL;
    PSTR                            pszSKI = NULL;
    PSTR                            pszAKI = NULL;
    PSTR                            pszChainOfTrust = NULL;
    PSTR                            pszCRLNumber = NULL;
    PSTR                            pszLastCRLUpdate = NULL;
    PSTR                            pszNextCRLUpdate = NULL;
    PLWCA_CERTIFICATE_ARRAY         pCACerts = NULL;

    dwError = LwCAX509ToPEM(pCert, &pCertificate);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetSubjectName(pCert, &pszSubject);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetSerialNumber(pCert, &pszSerialNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetSerialNumber(pParentCACert, &pszIssuerSerialNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetSubjectKeyIdentifier(pCert, &pszSKI);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetAuthorityKeyIdentifier(pCert, &pszAKI);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCABuildChainOfTrust(pParentRootCertData, pCertificate, &pszChainOfTrust);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateCRLNumber(&pszCRLNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateCrlTimestamps(
                        LWCA_CRL_DEFAULT_CRL_VALIDITY,
                        &pszLastCRLUpdate,
                        &pszNextCRLUpdate);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateCertData(
                        pcszParentCAId,
                        pszSerialNumber,
                        pszIssuerSerialNumber,
                        pszSKI,
                        pszAKI,
                        NULL,
                        NULL,
                        NULL,
                        0,
                        LWCA_CERT_STATUS_ACTIVE,
                        &pCertData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateRootCertData(
                        pcszCAId,
                        pCertData,
                        pCertificate,
                        pEncryptedKey,
                        pszChainOfTrust,
                        pszCRLNumber,
                        pszLastCRLUpdate,
                        pszNextCRLUpdate,
                        &pRootCertData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateCAData(
                        pszSubject,
                        pcszParentCAId,
                        pszSKI,
                        pcszAuthBlob,
                        LWCA_CA_STATUS_ACTIVE,
                        &pCAData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbAddCA(pcszCAId, pCAData, pRootCertData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_ROOT_CERT_DATA_ARRAY), (PVOID *)&pCACertDataArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(PLWCA_DB_ROOT_CERT_DATA), (PVOID *)&pCACertDataArray->ppRootCertData);
    BAIL_ON_LWCA_ERROR(dwError);

    pCACertDataArray->dwCount = 1;

    dwError = LwCADbCopyRootCertData(pRootCertData, &pCACertDataArray->ppRootCertData[0]);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARootCertArrayToCertArray(pCACertDataArray, &pCACerts);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCACerts = pCACerts;


cleanup:
    LwCAFreeCertificate(pCertificate);
    LWCA_SAFE_FREE_STRINGA(pszSubject);
    LWCA_SAFE_FREE_STRINGA(pszSerialNumber);
    LWCA_SAFE_FREE_STRINGA(pszIssuerSerialNumber);
    LWCA_SAFE_FREE_STRINGA(pszSKI);
    LWCA_SAFE_FREE_STRINGA(pszAKI);
    LWCA_SAFE_FREE_STRINGA(pszChainOfTrust);
    LWCA_SAFE_FREE_STRINGA(pszCRLNumber);
    LWCA_SAFE_FREE_STRINGA(pszLastCRLUpdate);
    LWCA_SAFE_FREE_STRINGA(pszNextCRLUpdate);
    LwCADbFreeCertData(pCertData);
    LwCADbFreeRootCertData(pRootCertData);
    LwCADbFreeRootCertDataArray(pCACertDataArray);
    LwCADbFreeCAData(pCAData);
    return dwError;

error:
    LwCAFreeCertificates(pCACerts);
    if (ppCACerts)
    {
        *ppCACerts = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCACreateSelfSignedRootCACert(
    PLWCA_REQ_CONTEXT       pReqCtx,
    PCSTR                   pcszRootCAId,
    PLWCA_PKCS_10_REQ_DATA  pCARequest,
    X509                    **ppX509CACert,
    PSTR                    *ppszPrivateKey
    )
{
    DWORD dwError = 0;
    X509_REQ *pRequest = NULL;
    X509 *pX509CACert = NULL;
    PSTR pszPublicKey = NULL;
    PSTR pszPrivateKey = NULL;
    PSTR pszCAIssuers = NULL;
    time_t tmNotBefore;
    time_t tmNotAfter;
    PLWCA_CERT_VALIDITY pValidity =  NULL;

    if (!pReqCtx || IsNullOrEmptyString(pcszRootCAId) || !pCARequest)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCACreateKeyPair(LWCA_MIN_CA_CERT_PRIV_KEY_LENGTH, &pszPrivateKey, &pszPublicKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACreateCertificateSignRequest(pCARequest, pszPublicKey, &pRequest);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509ReqSignRequest(pRequest, pszPrivateKey, NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    tmNotBefore = time(NULL);
    tmNotAfter = tmNotBefore + LWCA_DEFAULT_ROOTCA_CERT_DURATION;

    dwError = LwCACreateCertValidity(tmNotBefore,
                                    tmNotAfter,
                                    &pValidity
                                    );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetCAEndpoint(pcszRootCAId, &pszCAIssuers);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateSelfSignX509Certificate(pRequest, pValidity, pszCAIssuers, &pX509CACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509SignCertificate(pX509CACert, pszPrivateKey, NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppX509CACert = pX509CACert;
    *ppszPrivateKey = pszPrivateKey;

cleanup:
    LwCAX509ReqFree(pRequest);
    LwCAFreeCertValidity(pValidity);
    LWCA_SECURE_SAFE_FREE_MEMORY(pszPublicKey, LwCAStringLenA(pszPublicKey));
    LWCA_SAFE_FREE_STRINGA(pszCAIssuers);
    return dwError;

error:
    LWCA_SECURE_SAFE_FREE_MEMORY(pszPrivateKey, LwCAStringLenA(pszPrivateKey));
    LwCAX509Free(pX509CACert);
    if (ppX509CACert)
    {
        *ppX509CACert = NULL;
    }
    if (ppszPrivateKey)
    {
        *ppszPrivateKey = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCACAExists(
    PCSTR               pcszCAId,
    PBOOLEAN            pbCAExists,
    PBOOLEAN            pbCAIsActive
    )
{
    DWORD               dwError = 0;
    PLWCA_DB_CA_DATA    pCA = NULL;
    BOOLEAN             bCAExists = FALSE;
    BOOLEAN             bCAIsActive = FALSE;

    dwError = LwCADbGetCA(pcszCAId, &pCA);
    if (dwError == LWCA_ERROR_ENTRY_NOT_FOUND)
    {
        dwError = 0;
        bCAExists = FALSE;
        goto cleanup;
    }
    BAIL_ON_LWCA_ERROR(dwError);

    bCAExists = TRUE;

    bCAIsActive = (pCA->status == LWCA_CA_STATUS_ACTIVE) ? TRUE : FALSE;

cleanup:
    LwCADbFreeCAData(pCA);

    if (pbCAExists)
    {
        *pbCAExists = bCAExists;
    }
    if (pbCAIsActive)
    {
        *pbCAIsActive = bCAIsActive;
    }

    return dwError;

error:
    bCAExists = FALSE;
    bCAIsActive = FALSE;
    goto cleanup;
}

static
DWORD
_LwCABuildChainOfTrust(
    PLWCA_DB_ROOT_CERT_DATA     pIssuerCARootData,
    PLWCA_CERTIFICATE           pCARootCert,
    PSTR                        *ppszChainOfTrust
    )
{
    DWORD                       dwError = 0;
    PSTR                        pszChainOfTrust = NULL;

    dwError = LwCAAllocateStringPrintfA(
                    &pszChainOfTrust,
                    "%s\n%s",
                    (PSTR)pCARootCert,
                    pIssuerCARootData->pszChainOfTrust);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszChainOfTrust = pszChainOfTrust;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszChainOfTrust);
    if (ppszChainOfTrust)
    {
        *ppszChainOfTrust = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAGenerateCACertCRLInfo(
    PCSTR                       pcszCRLNumber,
    PSTR                        *ppszNextCRLNumber,
    PSTR                        *ppszLastCRLUpdate,
    PSTR                        *ppszNextCRLUpdate
    )
{
    DWORD                       dwError = 0;
    PSTR                        pszNextCRLNumber = NULL;
    PSTR                        pszLastCRLUpdate = NULL;
    PSTR                        pszNextCRLUpdate = NULL;

    dwError = LwCAGetNextCrlNumber(pcszCRLNumber, &pszNextCRLNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateCrlTimestamps(
                        LWCA_CRL_DEFAULT_CRL_VALIDITY,
                        &pszLastCRLUpdate,
                        &pszNextCRLUpdate);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszNextCRLNumber = pszNextCRLNumber;
    *ppszLastCRLUpdate = pszLastCRLUpdate;
    *ppszNextCRLUpdate = pszNextCRLUpdate;


cleanup:

    return dwError;

error:

    LWCA_SAFE_FREE_STRINGA(pszNextCRLNumber);
    if (ppszNextCRLNumber)
    {
        *ppszNextCRLNumber = NULL;
    }
    LWCA_SAFE_FREE_STRINGA(pszLastCRLUpdate);
    if (ppszLastCRLUpdate)
    {
        *ppszLastCRLUpdate = NULL;
    }
    LWCA_SAFE_FREE_STRINGA(pszNextCRLUpdate);
    if (ppszNextCRLUpdate)
    {
        *ppszNextCRLUpdate = NULL;
    }

    goto cleanup;
}

static
DWORD
_LwCAGenerateCertRevocationInfo(
    X509        *pX509Cert,
    PSTR        *ppszRevokedDate,
    PSTR        *ppszTimeValidFrom,
    PSTR        *ppszTimeValidTo
    )
{
    DWORD       dwError = 0;
    PSTR        pszRevokedDate = NULL;
    PSTR        pszTimeValidFrom = NULL;
    PSTR        pszTimeValidTo = NULL;

    dwError = LwCAGenerateCertRevokedDate(&pszRevokedDate);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetTimeValidFrom(pX509Cert, &pszTimeValidFrom);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetTimeValidTo(pX509Cert, &pszTimeValidTo);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszRevokedDate = pszRevokedDate;
    *ppszTimeValidFrom = pszTimeValidFrom;
    *ppszTimeValidTo = pszTimeValidTo;


cleanup:

    return dwError;

error:

    LWCA_SAFE_FREE_STRINGA(pszRevokedDate);
    if (ppszRevokedDate)
    {
        *ppszRevokedDate = NULL;
    }
    LWCA_SAFE_FREE_STRINGA(pszTimeValidFrom);
    if (ppszTimeValidFrom)
    {
        *ppszTimeValidFrom = NULL;
    }
    LWCA_SAFE_FREE_STRINGA(pszTimeValidTo);
    if (ppszTimeValidTo)
    {
        *ppszTimeValidTo = NULL;
    }

    goto cleanup;
}
