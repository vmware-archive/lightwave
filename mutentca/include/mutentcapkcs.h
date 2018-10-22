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

#ifndef __LWCA_MUTENTCA_PKCS_H__
#define __LWCA_MUTENTCA_PKCS_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _LWCA_PKCS_10_REQ_DATA
{
    PSTR                pszName;
    PSTR                pszDomainName;
    PLWCA_STRING_ARRAY  pCountryList;
    PLWCA_STRING_ARRAY  pLocalityList;
    PLWCA_STRING_ARRAY  pStateList;
    PLWCA_STRING_ARRAY  pOrganizationList;
    PLWCA_STRING_ARRAY  pOUList;
    PLWCA_STRING_ARRAY  pDNSList;
    PLWCA_STRING_ARRAY  pURIList;
    PLWCA_STRING_ARRAY  pEmailList;
    PLWCA_STRING_ARRAY  pIPAddressList;
    DWORD               dwKeyUsageConstraints;
} LWCA_PKCS_10_REQ_DATA, *PLWCA_PKCS_10_REQ_DATA;

enum _LWCA_KEY_USAGE {
    LWCA_DIGITAL_SIGNATURE      = 0,
    LWCA_NON_REPUDIATION        = 1,
    LWCA_KEY_ENCIPHERMENT       = 2,
    LWCA_DATA_ENCIPHERMENT      = 3,
    LWCA_KEY_AGREEMENT          = 4,
    LWCA_KEY_CERT_SIGN          = 5,
    LWCA_KEY_CRL_SIGN           = 6,
    LWCA_ENCIPHER_ONLY          = 7,
    LWCA_DECIPHER_ONLY          = 8
};

DWORD
LwCAPEMToX509(
    PCSTR       pCertificate,
    X509        **ppX509Cert
    );

DWORD
LwCAPEMToX509Req(
    PCSTR       pcszCSR,
    X509_REQ    **ppX509CertReq
    );

DWORD
LwCAX509ToPEM(
    X509*       pCert,
    PSTR*       ppszCertificate
    );

DWORD
LwCAX509ReqToPEM(
    X509_REQ    *pReq,
    PSTR        *ppszRequest
    );

VOID
LwCAX509Free(
    X509        *pX509
    );

VOID
LwCAX509ReqFree(
    X509_REQ    *pX509Req
    );

DWORD
LwCAX509GetSubjectName(
    X509        *pCert,
    PSTR        *ppszSubjectName
    );

DWORD
LwCAX509ReqGetSubjectName(
    X509_REQ    *pReq,
    PSTR        *ppszSubjectName
    );

DWORD
LwCAX509GetCommonName(
    X509        *pCert,
    PSTR        *ppszCommonName
    );

DWORD
LwCAX509ReqGetCommonName(
    X509_REQ    *pReq,
    PSTR        *ppszCommonName
    );

DWORD
LwCAX509GetIssuerName(
    X509        *pCert,
    PSTR        *ppszIssuerName
    );

DWORD
LwCACreateCertificateSignRequest(
    PLWCA_PKCS_10_REQ_DATA  pCertRequest,
    EVP_PKEY                *pPublicKey,
    X509_REQ                **ppReq
    );

DWORD
LwCAGenerateX509Certificate(
    X509_REQ*               pRequest,
    PLWCA_CERT_VALIDITY     pValidity,
    PLWCA_CERTIFICATE       pCACert,
    X509                    **ppCert
    );

DWORD
LwCAX509SignCertificate(
    X509        *pX509,
    PCSTR       pcszPrivateKey,
    PCSTR       pcszPassPhrase
    );

DWORD
LwCAX509ValidateCertificate(
    X509        *pCert,
    PCSTR       pcszPrivateKey,
    PCSTR       pcszPassPhrase
    );

DWORD
LwCAVerifyCertificate(
    PLWCA_CERTIFICATE_ARRAY pCACertChain,
    PLWCA_CERTIFICATE       pCertificate
    );

DWORD
LwCAX509CheckIfCACert(
    X509     *pCert,
    PBOOLEAN pbIsCA
    );

DWORD
LwCACreatePKCSRequest(
    PCSTR                   pcszName,
    PCSTR                   pcszDomainName,
    PLWCA_STRING_ARRAY      pCountryList,
    PLWCA_STRING_ARRAY      pLocalityList,
    PLWCA_STRING_ARRAY      pStateList,
    PLWCA_STRING_ARRAY      pOrganizationList,
    PLWCA_STRING_ARRAY      pOUList,
    PLWCA_STRING_ARRAY      pDNSList,
    PLWCA_STRING_ARRAY      pURIList,
    PLWCA_STRING_ARRAY      pEmailList,
    PLWCA_STRING_ARRAY      pIPAddressList,
    DWORD                   dwKeyUsageConstraints,
    PLWCA_PKCS_10_REQ_DATA  *ppPKCSRequest
    );

VOID
LwCAFreePkcsRequest(
    PLWCA_PKCS_10_REQ_DATA pPKCSRequest
    );

#ifdef __cplusplus
}
#endif

#endif /* __LWCA_MUTENTCA_PKCS_H__ */
