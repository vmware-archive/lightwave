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

static
DWORD
_LwCACreateSelfSignedRootCACert(
    PLWCA_REQ_CONTEXT       pReqCtx,
    PCSTR                   pcszRootCAId,
    PLWCA_PKCS_10_REQ_DATA  pCARequest,
    X509                    **ppX509CACert,
    PSTR                    *ppszPrivateKey
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

    dwError = LwCAJsonGetStringFromKey(
                                pConfig,
                                FALSE,
                                LWCA_CA_CONFIG_KEY_NAME,
                                &pszName);
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

    dwError = LwCAAllocateStringA(pszName, &gApiGlobals.pszRootCAId);
    BAIL_ON_LWCA_ERROR(dwError);

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
    if (dwError == LWCA_CA_ALREADY_EXISTS)
    {
        dwError = 0;
        goto cleanup;
    }
    LWCA_LOCK_MUTEX_UNLOCK(&gApiGlobals.mutex, bLocked);
    LwCAFreeCACtx();
    goto cleanup;
}

DWORD
LwCAGetRootCAId(
    PSTR *ppszRootCAId
    )
{
    DWORD dwError = 0;
    PSTR pszRootCAId = NULL;
    BOOLEAN bLocked = FALSE;

    if (!ppszRootCAId)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gApiGlobals.mutex, bLocked);

    dwError = LwCAAllocateStringA(gApiGlobals.pszRootCAId, &pszRootCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszRootCAId = pszRootCAId;

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gApiGlobals.mutex, bLocked);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszRootCAId);
    if (ppszRootCAId)
    {
        *ppszRootCAId = NULL;
    }
    goto cleanup;
}

VOID
LwCAFreeCACtx(
    )
{
    BOOLEAN bLocked = FALSE;

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gApiGlobals.mutex, bLocked);

    LWCA_SAFE_FREE_STRINGA(gApiGlobals.pszRootCAId);

    LWCA_LOCK_MUTEX_UNLOCK(&gApiGlobals.mutex, bLocked);
}

DWORD
LwCACreateRootCA(
    PLWCA_REQ_CONTEXT       pReqCtx,
    PCSTR                   pcszRootCAId,
    PLWCA_PKCS_10_REQ_DATA  pCARequest,
    PLWCA_CERTIFICATE       pCertificate,
    PCSTR                   pcszPrivateKey,
    PCSTR                   pcszPassPhrase
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE_ARRAY pCertArray = NULL;
    X509 *pX509CACert = NULL;
    PSTR pszSubject = NULL;
    PSTR pszCRLNumber = NULL;
    PSTR pszLastCRLUpdate = NULL;
    PSTR pszNextCRLUpdate = NULL;
    PSTR pszPrivateKey = NULL;
    PLWCA_DB_CA_DATA pCAData = NULL;
    PLWCA_CERTIFICATE pCACert =  NULL;
    BOOLEAN bIsCA = FALSE;
    PLWCA_KEY pEncryptedKey = NULL;

    if (!pReqCtx || IsNullOrEmptyString(pcszRootCAId) ||
        !((pCertificate && !IsNullOrEmptyString(pcszPrivateKey)) || pCARequest)
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCACheckCANotExist(pcszRootCAId);
    BAIL_ON_LWCA_ERROR(dwError);

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
                                    &pszPrivateKey
                                    );
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

    dwError = LwCAX509ToPEM(pX509CACert, &pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACreateCertArray((PSTR*)&pCACert, 1 , &pCertArray);
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

    dwError = LwCADbCreateCAData(pszSubject,
                                pCertArray,
                                pEncryptedKey,
                                pszCRLNumber,
                                pszLastCRLUpdate,
                                pszNextCRLUpdate,
                                LWCA_CA_STATUS_ACTIVE,
                                &pCAData
                                );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbAddCA(pcszRootCAId, pCAData, NULL);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LwCAFreeCertificate(pCACert);
    LwCAFreeCertificates(pCertArray);
    LwCADbFreeCAData(pCAData);
    LWCA_SAFE_FREE_STRINGA(pszSubject);
    LWCA_SAFE_FREE_STRINGA(pszCRLNumber);
    LWCA_SAFE_FREE_STRINGA(pszLastCRLUpdate);
    LWCA_SAFE_FREE_STRINGA(pszNextCRLUpdate);
    LWCA_SECURE_SAFE_FREE_MEMORY(pszPrivateKey, LwCAStringLenA(pszPrivateKey));
    LwCAX509Free(pX509CACert);
    LwCAFreeKey(pEncryptedKey);

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
    time_t tmNotBefore;
    time_t tmNotAfter;
    PLWCA_CERT_VALIDITY pTempValidity = NULL;
    DWORD dwDuration = 0;
    BOOLEAN bIsValid = FALSE;
    BOOLEAN bIsCA = FALSE;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) ||
        !pCertRequest || !ppCertifcate)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAPEMToX509Req(pCertRequest, &pRequest);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAPolicyValidate(
                gpPolicyCtx,
                pReqCtx,
                pRequest,
                pValidity,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_ALL,
                &bIsValid);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bIsValid)
    {
        dwError = LWCA_POLICY_VALIDATION_ERROR;
        BAIL_ON_LWCA_ERROR_WITH_MSG(
            dwError,
            "Failed to sign certificate due to policy violation");
    }

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

    dwError = _LwCACheckCAExist(pcszCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetCurrentCACertificate(pcszCAId, &pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateX509Certificate(pRequest, pTempValidity, pCACert, &pX509Cert);
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
    DWORD                       dwDuration = 0;
    time_t                      tmNotBefore;
    time_t                      tmNotAfter;
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
    BOOLEAN                     bIsValid = FALSE;

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

    dwError = LwCAPolicyValidate(
                gpPolicyCtx,
                pReqCtx,
                pRequest,
                pValidity,
                LWCA_POLICY_TYPE_CA,
                LWCA_POLICY_CHECK_ALL,
                &bIsValid);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bIsValid)
    {
        dwError = LWCA_POLICY_VALIDATION_ERROR;
        BAIL_ON_LWCA_ERROR_WITH_MSG(
            dwError,
            "Failed to create intermediate CA due to policy violation");
    }

    dwError = LwCASecuritySignX509Request(pcszCAId, pRequest);
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

    dwError =  LwCAGenerateX509Certificate(pRequest, pTempValidity, pParentCACert, &pX509CACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509CheckIfCACert(pX509CACert, &bIsCA);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bIsCA)
    {
        dwError = LWCA_UNKNOWN_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCASecuritySignX509Cert(pcszParentCAId, pX509CACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCASecurityGetEncryptedKey(pcszCAId, &pEncryptedKey);
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
    LwCAFreeKey(pEncryptedKey);

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
    X509 *pCert = NULL;
    PSTR pszSerialNumber = NULL;
    PSTR pszCRLNumber = NULL;
    PSTR pszNextCRLNumber = NULL;
    PSTR pszLastCRLUpdate = NULL;
    PSTR pszNextCRLUpdate = NULL;
    PSTR pszTimeValidTo = NULL;
    PSTR pszTimeValidFrom = NULL;
    PSTR pszRevokedDate = NULL;
    PLWCA_CERTIFICATE_ARRAY pCACerts = NULL;
    PLWCA_DB_CA_DATA pCAData = NULL;
    PLWCA_DB_CERT_DATA pCertData = NULL;
    BOOLEAN bExists = FALSE;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) || !pCertificate)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCACheckCAExist(pcszCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbGetCACertificates(pcszCAId, &pCACerts);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAVerifyCertificate(pCACerts, pCertificate);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAPEMToX509(pCertificate, &pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetSerialNumber(pCert, &pszSerialNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCheckCertData(pcszCAId, pszSerialNumber, &bExists);
    BAIL_ON_LWCA_ERROR(dwError);

    if (bExists)
    {
        dwError = LWCA_CRL_CERT_ALREADY_REVOKED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCADbGetCACRLNumber(pcszCAId, &pszCRLNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetTimeValidFrom(pCert, &pszTimeValidFrom);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetTimeValidTo(pCert, &pszTimeValidTo);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateCertRevokedDate(&pszRevokedDate);
    BAIL_ON_LWCA_ERROR(dwError);

    //TODO: User revoked reason needs to be mapped to CRL reason
    dwError = LwCADbCreateCertData(
                        pszSerialNumber,
                        pszTimeValidFrom,
                        pszTimeValidTo,
                        CRL_REASON_UNSPECIFIED,
                        pszRevokedDate,
                        LWCA_CERT_STATUS_INACTIVE,
                        &pCertData
                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetNextCrlNumber(pszCRLNumber, &pszNextCRLNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateCrlTimestamps(
                        LWCA_CRL_DEFAULT_CRL_VALIDITY,
                        &pszLastCRLUpdate,
                        &pszNextCRLUpdate);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateCAData(NULL,
                                NULL,
                                NULL,
                                pszNextCRLNumber,
                                pszLastCRLUpdate,
                                pszNextCRLUpdate,
                                LWCA_CA_STATUS_ACTIVE,
                                &pCAData
                                );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbUpdateCA(pcszCAId, pCAData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbAddCertData(pcszCAId, pCertData);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszSerialNumber);
    LWCA_SAFE_FREE_STRINGA(pszCRLNumber);
    LWCA_SAFE_FREE_STRINGA(pszNextCRLNumber);
    LWCA_SAFE_FREE_STRINGA(pszTimeValidFrom);
    LWCA_SAFE_FREE_STRINGA(pszTimeValidTo);
    LWCA_SAFE_FREE_STRINGA(pszRevokedDate);
    LWCA_SAFE_FREE_STRINGA(pszLastCRLUpdate);
    LWCA_SAFE_FREE_STRINGA(pszNextCRLUpdate);
    LwCADbFreeCertData(pCertData);
    LwCADbFreeCAData(pCAData);
    if (pCert)
    {
        X509_free(pCert);
    }
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCARevokeIntermediateCA(
    PLWCA_REQ_CONTEXT       pReqCtx,
    PCSTR                   pcszCAId
    )
{
    DWORD dwError = 0;
    PSTR pszParentCAId = NULL;
    PLWCA_CERTIFICATE pCACert = NULL;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCACheckCAExist(pcszCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbGetParentCAId(pcszCAId, &pszParentCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetCurrentCACertificate(pcszCAId, &pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARevokeCertificate(pReqCtx, pszParentCAId, pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbUpdateCAStatus(pcszCAId, LWCA_CA_STATUS_INACTIVE);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszParentCAId);
    LwCAFreeCertificate(pCACert);
    return dwError;

error:
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
    PLWCA_CERTIFICATE pCACert = NULL;
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray = NULL;
    PLWCA_DB_CA_DATA pCAData = NULL;
    PLWCA_CRL pCrl = NULL;
    X509 *pX509CACert = NULL;
    X509_CRL *pX509Crl = NULL;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) || !ppCrl)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCACheckCAExist(pcszCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetCurrentCACertificate(pcszCAId, &pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAPEMToX509(pCACert, &pX509CACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbGetCertData(pcszCAId, &pCertDataArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbGetCA(pcszCAId, &pCAData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateX509Crl(
                    pCAData->pszCRLNumber,
                    pCAData->pszLastCRLUpdate,
                    pCAData->pszNextCRLUpdate,
                    pCertDataArray,
                    pX509CACert,
                    &pX509Crl
                    );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError =  LwCASecuritySignX509Crl(pcszCAId, pX509Crl);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509CrlToPEM(pX509Crl, &pCrl);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCrl = pCrl;

cleanup:
    LwCAFreeCertificate(pCACert);
    LwCADbFreeCertDataArray(pCertDataArray);
    LwCADbFreeCAData(pCAData);
    LwCAX509Free(pX509CACert);
    LwCAX509CrlFree(pX509Crl);
    return dwError;

error:
    LwCAFreeCrl(pCrl);
    if (ppCrl)
    {
        *ppCrl = NULL;
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
    PSTR                    pszCRLNumber = NULL;
    PSTR                    pszLastCRLUpdate = NULL;
    PSTR                    pszNextCRLUpdate = NULL;

    dwError = LwCAX509ToPEM(pCert, &pCertificate);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509GetSubjectName(pCert, &pszSubject);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACreateCertArray((PSTR*)&pCertificate, 1 , &pCertificates);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateCRLNumber(&pszCRLNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateCrlTimestamps(
                        LWCA_CRL_DEFAULT_CRL_VALIDITY,
                        &pszLastCRLUpdate,
                        &pszNextCRLUpdate);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateCAData(pszSubject,
                                pCertificates,
                                pEncryptedKey,
                                pszCRLNumber,
                                pszLastCRLUpdate,
                                pszNextCRLUpdate,
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
    LWCA_SAFE_FREE_STRINGA(pszCRLNumber);
    LWCA_SAFE_FREE_STRINGA(pszLastCRLUpdate);
    LWCA_SAFE_FREE_STRINGA(pszNextCRLUpdate);
    LwCADbFreeCAData(pCAData);
    return dwError;

error:
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

    dwError =  LwCAGenerateSelfSignX509Certificate(pRequest, pValidity, &pX509CACert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAX509SignCertificate(pX509CACert, pszPrivateKey, NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppX509CACert = pX509CACert;
    *ppszPrivateKey = pszPrivateKey;

cleanup:
    LwCAX509ReqFree(pRequest);
    LwCAFreeCertValidity(pValidity);
    LWCA_SECURE_SAFE_FREE_MEMORY(pszPublicKey, LwCAStringLenA(pszPublicKey));
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
