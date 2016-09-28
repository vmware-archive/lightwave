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

DWORD
VMCACheckFileExists(
    PSTR pszFileName,
    DWORD *pdwExists
)
{

#ifdef _WIN32
    #define access _access
#endif
    DWORD dwError = 0;
    if (pszFileName == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = access(pszFileName, 0);
    if (dwError != -1)
    {
        *pdwExists = FILE_EXISTS;
    } else {
        *pdwExists = FILE_DOES_NOT_EXIST;
    }
    dwError = 0;
error :
    return dwError;

}


DWORD
VMCAReadCRLFromFile(
    PSTR pszFileName,
    X509_CRL **ppszCrlData)
{
    DWORD dwError = 0;
    if ( IsNullOrEmptyString (pszFileName)) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (ppszCrlData == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAFileToCRL(pszFileName, ppszCrlData);
    BAIL_ON_ERROR(dwError);

error :
    return dwError;

}


DWORD
VMCACreateNewCRL(
    PVMCA_X509_CA pCA,
    X509_CRL **ppszCrlData
)
{
    DWORD dwError = 0;
    X509_CRL *pCrl = NULL;
    ASN1_INTEGER *pSerial = NULL;

    pCrl = X509_CRL_new();
    if(pCrl == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    dwError = X509_CRL_set_issuer_name(pCrl,
        X509_get_subject_name(pCA->pCertificate));
    BAIL_ON_SSL_ERROR(dwError, VMCA_CERT_IO_FAILURE);

    dwError = X509_CRL_set_version(pCrl, 1);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CRL_ERROR);

    *ppszCrlData = pCrl;

cleanup :
    if(pSerial != NULL){
        ASN1_INTEGER_free(pSerial);
    }
    return dwError;

error :
    if(pCrl != NULL) {
        X509_CRL_free(pCrl);
    }
    goto cleanup;
}

DWORD
VMCACheckNotAlreadyRevoked(
    X509 *pCert,
    X509_CRL *pCrl
    )
{
    DWORD dwError = 0;
    X509_REVOKED *rev = NULL;
    dwError = X509_CRL_get0_by_cert(pCrl, &rev, pCert);
    if( dwError == 1 ){
        dwError = VMCA_CRL_CERT_ALREADY_REVOKED;
        BAIL_ON_ERROR(dwError);
    }
error :
    // Freeing pCrl should free rev
    // if(rev != NULL){
    //     X509_REVOKED_free(rev);
    // }
    return dwError;
}

DWORD
VMCACheckNotAlreadyRevoked_Serial(
    ASN1_INTEGER *asnSerial,
    X509_CRL *pCrl
    )
{
    DWORD dwError = 0;
    X509_REVOKED *rev = NULL;
    dwError = X509_CRL_get0_by_serial(pCrl, &rev, asnSerial);
    if( dwError == 1 ){
        dwError = VMCA_CRL_CERT_ALREADY_REVOKED;
        BAIL_ON_ERROR(dwError);
    }
error :
    // Freeing pCrl should free rev
    // if(rev != NULL){
    //     X509_REVOKED_free(rev);
    // }
    return dwError;
}


DWORD
VMCACreateRevokedFromCert_Reason(
    ASN1_INTEGER *asnSerial,
    DWORD dwRevokedDate,
    VMCA_CRL_REASON certRevokeReason,
    X509_REVOKED **pRevoked)
{

    DWORD dwError = 0;
    X509_REVOKED *pTempRev = NULL;
    ASN1_TIME *pRevTime = NULL;
    ASN1_ENUMERATED *pCode = NULL;

    pCode = ASN1_ENUMERATED_new();
    if(pCode == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    pTempRev = X509_REVOKED_new();
    if (pTempRev == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    pRevTime = ASN1_TIME_new();
    if (pRevTime == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    ASN1_TIME_set(pRevTime, (time_t)dwRevokedDate);
    dwError = X509_REVOKED_set_serialNumber(pTempRev,
                                            asnSerial);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CRL_SET_SERIAL_FAIL);

    dwError = X509_REVOKED_set_revocationDate(pTempRev, pRevTime);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CRL_SET_TIME_FAIL);

    ASN1_ENUMERATED_set(pCode, certRevokeReason);
    dwError = X509_REVOKED_add1_ext_i2d(pTempRev,
                            NID_crl_reason, pCode, 0, 0);

    BAIL_ON_SSL_ERROR(dwError, VMCA_CRL_REASON_FAIL);
    *pRevoked = pTempRev;

cleanup :
    if(pRevTime != NULL) {
        ASN1_TIME_free(pRevTime);
    }

    if(pCode !=NULL) {
        ASN1_ENUMERATED_free(pCode);
    }
    return dwError;

error:
    if(pTempRev != NULL)
    {
        X509_REVOKED_free(pTempRev);
    }
    goto cleanup;
}


DWORD
VMCACreateRevokedFromCert(
    X509 *pCert,
    X509_REVOKED **pRevoked)
{

    DWORD dwError = 0;
    X509_REVOKED *pTempRev = NULL;
    ASN1_TIME *pRevTime = NULL;
    ASN1_ENUMERATED *pCode = NULL;

    pCode = ASN1_ENUMERATED_new();
    if(pCode == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    pTempRev = X509_REVOKED_new();
    if (pTempRev == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    pRevTime = ASN1_TIME_new();
    if (pRevTime == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    ASN1_TIME_set(pRevTime, time(NULL));
    dwError = X509_REVOKED_set_serialNumber(pTempRev,
                    X509_get_serialNumber(pCert));
    BAIL_ON_SSL_ERROR(dwError, VMCA_CRL_SET_SERIAL_FAIL);

    dwError = X509_REVOKED_set_revocationDate(pTempRev, pRevTime);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CRL_SET_TIME_FAIL);

    //TODO : Fix the UNSPECIFIED to real valid reason
    // which users can pass in.
    ASN1_ENUMERATED_set(pCode, CRL_REASON_UNSPECIFIED);
    dwError = X509_REVOKED_add1_ext_i2d(pTempRev,
                            NID_crl_reason, pCode, 0, 0);

    BAIL_ON_SSL_ERROR(dwError, VMCA_CRL_REASON_FAIL);
    *pRevoked = pTempRev;

cleanup :
    if(pRevTime != NULL) {
        ASN1_TIME_free(pRevTime);
    }

    if(pCode !=NULL) {
        ASN1_ENUMERATED_free(pCode);
    }
    return dwError;

error:
    if(pTempRev != NULL)
    {
        X509_REVOKED_free(pTempRev);
    }
    goto cleanup;
}

DWORD
VMCAAddCertToCRL(
                X509_CRL *pCrl,
                X509 *pCerts,
                DWORD dwCertCount
                )
{
    DWORD dwError = 0;
    UINT ndx = 0;
    X509 *pTempCert = NULL;
    if(pCerts == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    for(ndx = 0; ndx < dwCertCount; ndx ++)
    {
        X509_REVOKED *pRevoked = NULL;
        pTempCert = pCerts + ndx;
        if (pTempCert != NULL) {

            dwError = VMCACheckNotAlreadyRevoked(pTempCert, pCrl);
            BAIL_ON_ERROR(dwError);

            dwError = VMCACreateRevokedFromCert(pTempCert, &pRevoked);
            BAIL_ON_ERROR(dwError);

            // Please Note : 0 functions in Open SSL means
            // when we free parent , children will be freed
            // Automatically.

            dwError = X509_CRL_add0_revoked(pCrl, pRevoked);
            BAIL_ON_SSL_ERROR(dwError, VMCA_CRL_ERROR);
        }
    }
error :
    return dwError;

}

DWORD
VMCAAddCertToCRL_Reason(
                        X509_CRL *pCrl,
                        PWSTR pwszSerial,
                        DWORD dwRevokedDate,
                        VMCA_CRL_REASON certRevokeReason
                       )
{
    DWORD dwError = 0;
    X509 *pCert = NULL;
    X509_REVOKED *pRevoked = NULL;
    ASN1_INTEGER *asnSerial = NULL;

    if (!pCrl ||
        IsNullOrEmptyString (pwszSerial)
       )
    {
       dwError = ERROR_INVALID_PARAMETER;
       BAIL_ON_ERROR(dwError);
    }

    asnSerial = ASN1_ENUMERATED_new();
    if(asnSerial == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAWIntegerToASN1Integer(
                                        pwszSerial,
                                        asnSerial
                                       );
    BAIL_ON_ERROR (dwError);

    dwError = VMCACheckNotAlreadyRevoked_Serial(
                                          asnSerial,
                                          pCrl
                                        );
    BAIL_ON_VMCA_ERROR (dwError);


    dwError = VMCACreateRevokedFromCert_Reason(
                                        asnSerial,
                                        dwRevokedDate,
                                        certRevokeReason,
                                        &pRevoked
                                       );
    BAIL_ON_VMCA_ERROR (dwError);

           // Please Note : 0 functions in Open SSL means
           // when we free parent , children will be freed
           // Automatically.

    dwError = X509_CRL_add0_revoked(pCrl, pRevoked);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CRL_ERROR);

cleanup:
     if (pCert)
     {
        X509_free(pCert);
     }

     if (asnSerial)
     {
        ASN1_INTEGER_free (asnSerial);
     }

     return dwError;
error :

     goto cleanup;
}


DWORD
VMCAOpenCRLPrivate(
    PVMCA_X509_CA pCA,
    PSTR pszCRLFileName,
    X509_CRL **pX509Crl
   )
{
    X509_CRL *pCrl = NULL;
    DWORD dwError = 0;
    DWORD dwFileExists = -1;

    if ( IsNullOrEmptyString (pszCRLFileName)) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCACheckFileExists(pszCRLFileName,&dwFileExists);
    BAIL_ON_ERROR(dwError);

    if ( dwFileExists == FILE_EXISTS) {
        dwError = VMCAReadCRLFromFile(pszCRLFileName, &pCrl);
        BAIL_ON_ERROR(dwError);
    } else {
        dwError = VMCACreateNewCRL(pCA, &pCrl);
        BAIL_ON_ERROR(dwError);
    }


    *pX509Crl = pCrl;

error:
    return dwError;
}


DWORD
VMCASortCRL(
    X509_CRL *pCrl
    )
{
    DWORD dwError = 0;
    dwError = X509_CRL_sort(pCrl);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CRL_SORT_FAILED);

error :
    return dwError;
}

DWORD
VMCACrlSign(
    X509_CRL *pCrl,
    PVMCA_X509_CA pCA
)
{
    DWORD dwError = 0;
    dwError = X509_CRL_sign(pCrl, pCA->pKey, EVP_sha256());
    BAIL_ON_SSL_ERROR(dwError, VMCA_CRL_SIGN_FAIL);
error :
    return dwError;
}

DWORD
VMCACrlFree(
    X509_CRL *pCrl
    )
{
    DWORD dwError = 0;
    if(pCrl != NULL) {
        X509_CRL_free(pCrl);
    }
    return dwError;
}


DWORD
VMCAGetNextCrlNumber(
    X509_CRL *pCrl,
    DWORD *pNextNum
)
{
    DWORD dwError = 0;
    ASN1_INTEGER *pCrlNumber = NULL;
    long nCrlNum = 0;

    if(pCrl == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pCrlNumber = ASN1_INTEGER_new();
    if(pCrlNumber == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    pCrlNumber = X509_CRL_get_ext_d2i(pCrl, NID_crl_number, 0,0);
    nCrlNum = ASN1_INTEGER_get(pCrlNumber);
    nCrlNum++;
    *pNextNum = nCrlNum;
cleanup:
    if(pCrlNumber) {
        ASN1_INTEGER_free(pCrlNumber);
    }
    return dwError;
error :
    goto cleanup;
}

DWORD
VMCAASN1ToTimeT(
        ASN1_TIME *pTime,
        time_t *ptmTime
)
{
    /* Time format is either YYmmddHHMMSS or YYYYmmddHHMMSS */
    DWORD dwError = 0;
    struct tm time1;
    int ndx = 0;
    const char *tmstring = (const char *) pTime->data;

    if(tmstring == NULL) {
        dwError = VMCA_CRL_NULL_TIME;
        BAIL_ON_ERROR(dwError);
    }


    memset(&time1, 0, sizeof(time1));

    // Two digit year
    if(pTime->type == V_ASN1_UTCTIME) {
        time1.tm_year = (tmstring[ndx++] - '0') * 10;
        time1.tm_year += tmstring[ndx++] - '0';
        if( time1.tm_year < 70) {
            time1.tm_year += 100;
        }
    } else if ( pTime->type == V_ASN1_GENERALIZEDTIME){
        // Four digit year
        time1.tm_year  = (tmstring[ndx++] - '0') *  1000;
        time1.tm_year += (tmstring[ndx++] - '0') *   100;
        time1.tm_year += (tmstring[ndx++] - '0') *    10;
        time1.tm_year += (tmstring[ndx++] - '0') *     1;
        time1.tm_year -=1900;
    }

    time1.tm_mon  =  (tmstring[ndx++] - '0') * 10;
    time1.tm_mon += (tmstring[ndx++] - '0') - 1;

    time1.tm_mday  = (tmstring[ndx++] - '0') * 10;
    time1.tm_mday += (tmstring[ndx++] - '0');

    time1.tm_hour  = (tmstring[ndx++] - '0') * 10;
    time1.tm_hour += (tmstring[ndx++] - '0');

    time1.tm_min   = (tmstring[ndx++] - '0') * 10;
    time1.tm_min  +=(tmstring[ndx++] - '0');

    time1.tm_sec   = (tmstring[ndx++] - '0') * 10;
    time1.tm_sec  +=(tmstring[ndx++] - '0');
    // Ignoring Z +hh 'mm' for time1 being.

    *ptmTime = mktime(&time1);
error :
    return dwError;
}



DWORD
VMCAGetNextUpdateTime(
    X509_CRL *pCrl,
    time_t *pNextUpdateTime
)
{
    DWORD dwError = 0;
    ASN1_TIME *pLastUpdate = NULL;
    ASN1_TIME *pNextUpdate = NULL;

    time_t tmLast = 0;
    time_t tmNext = 0;
    time_t tmDiff = 0;
    time_t tmNextUpdate = 0;

    if( (pCrl == NULL) ||
        (pNextUpdateTime == NULL))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pLastUpdate = X509_CRL_get_lastUpdate(pCrl);
    pNextUpdate = X509_CRL_get_nextUpdate(pCrl);

    dwError = VMCAASN1ToTimeT(pLastUpdate, &tmLast);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAASN1ToTimeT(pNextUpdate, &tmNext);
    BAIL_ON_ERROR(dwError);
    tmDiff = tmNext - tmLast;

    tmNextUpdate = time(NULL) + tmDiff;

    *pNextUpdateTime = tmNextUpdate;

cleanup :
    return dwError;

error :
    goto cleanup;

}


DWORD
VMCAUpdateTimeStamps(
    X509_CRL *pCrl,
    time_t tmLastUpdate,
    time_t tmNextUpdate,
    DWORD nCrlNum
)
{
    ASN1_TIME *pAsnLastUpdate = NULL;
    ASN1_TIME *pAsnNextUpdate = NULL;
    DWORD dwError = 0;
    ASN1_INTEGER *pSerial = NULL;

    if(pCrl == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pAsnLastUpdate = ASN1_TIME_new();
    if(pAsnLastUpdate == NULL){
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    pAsnNextUpdate  = ASN1_TIME_new();
    if(pAsnNextUpdate == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    ASN1_TIME_set(pAsnLastUpdate, tmLastUpdate);
    ASN1_TIME_set(pAsnNextUpdate, tmNextUpdate);

    dwError = X509_CRL_set_lastUpdate(pCrl, pAsnLastUpdate);
    BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_SET_START_TIME);

    dwError = X509_CRL_set_nextUpdate(pCrl, pAsnNextUpdate);
    BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_SET_END_TIME);
    pSerial = ASN1_INTEGER_new();
    if (pSerial == NULL){
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }
    ASN1_INTEGER_set(pSerial, nCrlNum);

    X509_CRL_add1_ext_i2d(pCrl, NID_crl_number, pSerial,0,0);

error:
    if ( pAsnLastUpdate != NULL) {
        ASN1_TIME_free(pAsnLastUpdate);
    }

    if( pAsnNextUpdate != NULL) {
        ASN1_TIME_free(pAsnNextUpdate);
    }

    if(pSerial != NULL) {
        ASN1_INTEGER_free(pSerial);
    }
    return dwError;
}

int
VMCAGetDefaultValidityPeriod()
{
    return VMCA_CRL_DEFAULT_CRL_VALIDITY;
}

DWORD
VMCAGetCRLInfoPrivate(
    PSTR pszFileName,
    time_t *ptmLastUpdate,
    time_t *ptmNextUpdate,
    DWORD  *pdwCRLNumber)
{
    DWORD dwError = 0;
    X509_CRL *pCrl = NULL;
    ASN1_TIME *pLastUpdate = NULL;
    ASN1_TIME *pNextUpdate = NULL;
    ASN1_INTEGER *pCrlNumber = NULL;
    long nCrlNum = 0;

    if (   (IsNullOrEmptyString(pszFileName))
        || (ptmNextUpdate == NULL)
        || (ptmLastUpdate == NULL)
        || (pdwCRLNumber == NULL)
        ) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAFileToCRL(pszFileName, &pCrl);
    BAIL_ON_ERROR(dwError);

    pLastUpdate = X509_CRL_get_lastUpdate(pCrl);
    pNextUpdate = X509_CRL_get_nextUpdate(pCrl);

    pCrlNumber = ASN1_INTEGER_new();
    if(pCrlNumber == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }
    pCrlNumber = X509_CRL_get_ext_d2i(pCrl, NID_crl_number, 0,0);
    nCrlNum = ASN1_INTEGER_get(pCrlNumber);
    *pdwCRLNumber = nCrlNum;

    dwError =  VMCAASN1ToTimeT(pLastUpdate,ptmLastUpdate);
    BAIL_ON_ERROR(dwError);

    dwError =  VMCAASN1ToTimeT(pNextUpdate,ptmNextUpdate);
    BAIL_ON_ERROR(dwError);


cleanup:
    if(pCrlNumber){
        ASN1_INTEGER_free(pCrlNumber);
    }
    VMCACrlFree(pCrl);
    return dwError;
error :
    goto cleanup;
}

DWORD
VMCAPrintCRLPrivate(
    PSTR pszFileName,
    PSTR *ppszCRLString
)
{
    DWORD dwError = 0;
    X509_CRL *pCrl = NULL;
    BIO *pBioMem = NULL;
    BUF_MEM *pBuffMem = NULL;
    PSTR pszTempCrl = NULL;
    PSTR pTempCrl = NULL;

    if(IsNullOrEmptyString(pszFileName)){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pBioMem = BIO_new(BIO_s_mem());
    if (pBioMem == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAReadCRLFromFile(pszFileName, &pCrl);
    BAIL_ON_ERROR(dwError);

    dwError = X509_CRL_print(pBioMem, pCrl);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CRL_DECODE_ERROR);

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = VMCAAllocateMemory((DWORD)pBuffMem->length, (PVOID*)&pTempCrl);
    BAIL_ON_ERROR(dwError);

    memcpy(pTempCrl, pBuffMem->data, pBuffMem->length - 1);

    dwError = VMCAAllocateStringA(pTempCrl, &pszTempCrl);
    BAIL_ON_ERROR(dwError);

    *ppszCRLString = pszTempCrl;
    pszTempCrl = NULL;

cleanup:
    if (pCrl)
    {
        VMCACrlFree(pCrl);
    }

    if (pBioMem != NULL)
    {
        BIO_free(pBioMem);
    }

    VMCA_SAFE_FREE_MEMORY(pTempCrl);
    return dwError;

error :
    VMCA_SAFE_FREE_STRINGA(pszTempCrl);
    goto cleanup;
}

DWORD
VMCAUpdateAuthorityKeyIdentifier(
                        X509_CRL *pCrl,
                        PVMCA_X509_CA pCA
                        )
{
    DWORD dwError = 0;

    X509V3_CTX ctx;

    X509_EXTENSION *pExtension = NULL;

    if (!pCA ||
        !pCA->pCertificate ||
        !pCrl
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    X509V3_set_ctx_nodb (&ctx);

    X509V3_set_ctx(
                   &ctx,
                   pCA->pCertificate,
                   NULL,
                   NULL,
                   pCrl,
                   0
                  );

    pExtension = X509V3_EXT_conf_nid(
                                     NULL,
                                     &ctx,
                                     NID_authority_key_identifier,
                                     "keyid"
                                    );

    if (!pExtension)
    {
        goto error;
    }

    X509_CRL_add_ext (pCrl, pExtension, -1);

cleanup:
    if (pExtension)
    {
        X509_EXTENSION_free(pExtension);
    }

    return dwError;

error:
    goto cleanup;
}
