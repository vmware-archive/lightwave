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

#define LWCA_KEY_USAGE_FLAG_DIGITAL_SIGNATURE 0x00000001
#define LWCA_KEY_USAGE_FLAG_NON_REPUDIATION   0x00000002
#define LWCA_KEY_USAGE_FLAG_KEY_ENCIPHERMENT  0x00000004
#define LWCA_KEY_USAGE_FLAG_DATA_ENCIPHERMENT 0x00000008
#define LWCA_KEY_USAGE_FLAG_KEY_AGREEMENT     0x00000010
#define LWCA_KEY_USAGE_FLAG_KEY_CERT_SIGN     0x00000020
#define LWCA_KEY_USAGE_FLAG_KEY_CRL_SIGN      0x00000040
#define LWCA_KEY_USAGE_FLAG_ENCIPER_ONLY      0x00000080
#define LWCA_KEY_USAGE_FLAG_DECIPHER_ONLY     0x00000100


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

DWORD
LwCAX509CrlToPEM(
    X509_CRL    *pCrl,
    PSTR        *ppszCrl
    );

VOID
LwCAX509Free(
    X509        *pX509
    );

VOID
LwCAX509ReqFree(
    X509_REQ    *pX509Req
    );

VOID
LwCAX509CrlFree(
    X509_CRL    *pX509Crl
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
LwCAX509GetOrganizations(
    X509                *pCert,
    PLWCA_STRING_ARRAY  *ppOrgList
    );

DWORD
LwCAX509ReqGetSubjectAltNames(
    X509_REQ            *pReq,
    PLWCA_STRING_ARRAY  *ppSANArray
    );

DWORD
LwCAX509ReqGetKeyUsage(
    X509_REQ    *pReq,
    DWORD       *pdwKeyUsage
    );

DWORD
LwCAX509GetSerialNumber(
    X509    *pCert,
    PSTR    *ppszSerialNumber
    );

DWORD
LwCAX509GetTimeValidFrom(
    X509 *pCert,
    PSTR *ppszTimeValidFrom
    );

DWORD
LwCAX509GetTimeValidTo(
    X509 *pCert,
    PSTR *ppszTimeValidTo
    );

DWORD
LwCAGenerateCertRevokedDate(
    PSTR    *ppszRevokedDate
    );

DWORD
LwCAGenerateCRLNumber(
    PSTR *ppszCRlNumber
    );

DWORD
LwCACreateCertificateSignRequest(
    PLWCA_PKCS_10_REQ_DATA  pCertRequest,
    PCSTR                   pcszPublicKey,
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
LwCAGenerateSelfSignX509Certificate(
    X509_REQ                *pRequest,
    PLWCA_CERT_VALIDITY     pValidity,
    X509                    **ppCert
    );

DWORD
LwCAGenerateX509Crl(
    PCSTR                       pcszCRLNumber,
    PCSTR                       pcszLastCRLUpdate,
    PCSTR                       pcszNextCRLUpdate,
    PLWCA_DB_CERT_DATA_ARRAY    pCertDataArray,
    X509                        *pCACert,
    X509_CRL                    **ppCrl
    );

DWORD
LwCAX509SignCertificate(
    X509        *pX509,
    PCSTR       pcszPrivateKey,
    PCSTR       pcszPassPhrase
    );

DWORD
LwCAX509ReqSignRequest(
    X509_REQ    *pReq,
    PCSTR       pcszPrivateKey,
    PCSTR       pcszPassPhrase
    );

DWORD
LwCAX509CrlSign(
    X509_CRL    *pCrl,
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
LwCAVerifyCertificateSign(
    X509    *pCert,
    X509    *pCACert
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
LwCAGetNextCrlNumber(
    PCSTR   pcszCRLNumber,
    PSTR    *ppszNextCRLNumber
    );

DWORD
LwCAGenerateCrlTimestamps(
    time_t  tmCrlValidity,
    PSTR    *ppszLastCRLUpdate,
    PSTR    *ppszNextCRLUpdate
    );

DWORD
LwCACreateKeyPair(
    size_t nKeyLength,
    PSTR *ppszPrivateKey,
    PSTR *ppszPublicKey
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
LwCAFreePKCSRequest(
    PLWCA_PKCS_10_REQ_DATA pPKCSRequest
    );

#ifdef __cplusplus
}
#endif

#endif /* __LWCA_MUTENTCA_PKCS_H__ */
