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

/*TODO: Move this to database component
 * and expose it from there. If we are using
 * this is multiple places
 */
static
VOID
VMCAFreeDBEntryFields(
    PVMCA_DB_CERTIFICATE_ENTRY pDBEntry
);

DWORD
VmcaSrvRevokeCertificate(
                         PCWSTR pwszServerName,
                         PVMCA_CERTIFICATE pszCertificate,
                         VMCA_CRL_REASON certRevokeReason
                        )
{
    DWORD dwError = 0;

    PVMCA_DB_CERTIFICATE_ENTRY pEntry = NULL;

    dwError = VMCASrvValidateCA();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCADecodeCert((LPSTR)pszCertificate, &pEntry);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbRevokeCert(
                        pEntry->pwszSerial,
                        pEntry->pwszIssuerName,
                        (DWORD)certRevokeReason
                        );
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:

    if(pEntry != NULL) {
        VMCAFreeDBEntryFields(pEntry);
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
VmcaSrvValidateCRLReason(
                         DWORD dwCrlReason,
                         PVMCA_CRL_REASON pCertRevokeReason
                        )
{
    DWORD dwError = 0;
    VMCA_CRL_REASON crlReason = VMCA_CRL_REASON_UNSPECIFIED;

    if (!pCertRevokeReason)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    switch (dwCrlReason)
    {
        case 1:
          crlReason = VMCA_CRL_REASON_KEY_COMPROMISE;
          break;
        case 2:
          crlReason = VMCA_CRL_REASON_CA_COMPROMISE;
          break;
        case 3:
          crlReason = VMCA_CRL_REASON_AFFILIATION_CHANGED;
          break;
        case 4:
          crlReason = VMCA_CRL_REASON_SUPERSEDED;
          break;
        case 5:
          crlReason = VMCA_CRL_REASON_CESSATION_OF_OPERATION;
          break;
        case 6:
          crlReason = VMCA_CRL_REASON_CERTIFICATE_HOLD;
          break;
        case 7:
          crlReason = VMCA_CRL_REASON_REMOVE_FROM_CRL;
          break;
        case 8:
          crlReason = VMCA_CRL_REASON_PRIVILEGE_WITHDRAWN;
          break;
        case 9:
          crlReason = VMCA_CRL_REASON_AA_COMPROMISE;
          break;
        default:
          break;
    }

    *pCertRevokeReason = crlReason;

cleanup:
    return dwError;

error:
    if (pCertRevokeReason)
    {
        *pCertRevokeReason = VMCA_CRL_REASON_UNSPECIFIED;
    }

    goto cleanup;
}

DWORD
VmcaSrvReGenCRL(
                X509_CRL **ppCrl
               )
{

    DWORD dwError = 0;
    PSTR pszTmpFile = NULL;
    DWORD dwExists = 0;
    DWORD dwCertCount = 0;
    PVMCA_DB_REVOKED_CERTS pRevokedEntryArray = NULL;
    time_t tmNextUpdate = 0;
    time_t tmLastUpdate = 0;
    X509_CRL *pCrl = NULL;
    DWORD dwCrlNum = 0;
    PVMCA_X509_CA pCA = NULL;
    DWORD dwIndex = 0;
    BOOL bIsHoldingMutex = TRUE;

    dwError = VMCAGetTempCRLNamePath(&pszTmpFile);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCACheckFileExists(pszTmpFile,&dwExists);
    if(dwExists == FILE_EXISTS)
    {
        dwError = remove(pszTmpFile);

        if ( dwError == -1) {
            dwError = VMCA_FILE_REMOVE_ERROR;
        }
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCASrvValidateCA();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCASrvGetCA(&pCA);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbGetRevokedCerts(
                                    &pRevokedEntryArray,
                                    &dwCertCount
                                   );
    BAIL_ON_VMCA_ERROR (dwError);

    dwError = VMCAOpenCRLPrivate(
                                 pCA,
                                 pszTmpFile,
                                 &pCrl
                                );
    BAIL_ON_VMCA_ERROR(dwError);

    for ( ; dwIndex < dwCertCount; dwIndex++)
    {
        VMCA_CRL_REASON certRevokeReason = VMCA_CRL_REASON_UNSPECIFIED;

        dwError = VmcaSrvValidateCRLReason(
                              pRevokedEntryArray[dwIndex].dwRevokedReason,
                              &certRevokeReason
                              );
        BAIL_ON_VMCA_ERROR (dwError);

        dwError = VMCAAddCertToCRL_Reason (
                                pCrl,
                                pRevokedEntryArray[dwIndex].pwszSerial,
                                pRevokedEntryArray[dwIndex].dwRevokedDate,
                                certRevokeReason
                                );
        BAIL_ON_VMCA_ERROR (dwError);

    }


    tmLastUpdate = time(NULL) - VMCA_TIME_LAG_OFFSET_CRL;
    dwError = VMCAGetNextUpdateTime(pCrl, &tmNextUpdate);
    if (dwError == VMCA_CRL_NULL_TIME ) {
        tmNextUpdate = tmLastUpdate +
        ( VMCAGetDefaultValidityPeriod() * 24 * 60 * 60);
        dwError = 0;
    }
    BAIL_ON_VMCA_ERROR(dwError);

    pthread_mutex_lock (&gVMCAServerGlobals.mutexCRL);

    bIsHoldingMutex = TRUE;

    dwCrlNum = gVMCAServerGlobals.dwCurrentCRLNumber;

    dwCrlNum++;

    dwError = VMCAUpdateTimeStamps (pCrl, tmLastUpdate,
                                tmNextUpdate, dwCrlNum);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbSetCurrentCRLNumber (dwCrlNum);
    BAIL_ON_VMCA_ERROR (dwError);

    gVMCAServerGlobals.dwCurrentCRLNumber = dwCrlNum;

    pthread_mutex_unlock (&gVMCAServerGlobals.mutexCRL);

    bIsHoldingMutex = FALSE;

    dwError = VMCAUpdateAuthorityKeyIdentifier(
                                  pCrl,
                                  pCA
                                  );
    BAIL_ON_VMCA_ERROR (dwError);

    dwError = VMCASortCRL(pCrl);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCACrlSign(pCrl, pCA);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCACRLToFile(pCrl, pszTmpFile);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCACopyTempCRLtoCRL();
    BAIL_ON_VMCA_ERROR(dwError);

    if (ppCrl)
    {
        *ppCrl = pCrl;

        pCrl = NULL;
    }

cleanup:
    if (pCrl)
    {
        VMCACrlFree(pCrl);
    }

    VMCA_SAFE_FREE_STRINGA(pszTmpFile);

    if (pRevokedEntryArray)
    {
        VmcaDbFreeRevokedCerts(
                              pRevokedEntryArray,
                              dwCertCount
                              );
    }
    if (pCA)
    {
        VMCAReleaseCA(pCA);
    }

    if (bIsHoldingMutex)
    {
        pthread_mutex_unlock(&gVMCAServerGlobals.mutexCRL);
    }
    return dwError;
error:

    if (ppCrl)
    {
        *ppCrl = NULL;
    }

    goto cleanup;
}

static
VOID
VMCAFreeDBEntryFields(
    PVMCA_DB_CERTIFICATE_ENTRY pDBEntry
)
{
    if (pDBEntry)
    {
        if( pDBEntry->pwszCommonName) {
            VMCAFreeMemory(pDBEntry->pwszCommonName);
        }

        if( pDBEntry->pwszAltNames) {
            VMCAFreeMemory(pDBEntry->pwszAltNames);
        }

        if( pDBEntry->pwszOrgName) {
            VMCAFreeMemory(pDBEntry->pwszOrgName);
        }

        if( pDBEntry->pwszOrgUnitName) {
            VMCAFreeMemory(pDBEntry->pwszOrgUnitName);
        }

        if( pDBEntry->pwszIssuerName) {
            VMCAFreeMemory(pDBEntry->pwszIssuerName);
        }

        if(pDBEntry->pwszCountryName) {
            VMCAFreeMemory(pDBEntry->pwszCountryName);
        }

        if( pDBEntry->pwszSerial) {
            VMCAFreeMemory(pDBEntry->pwszSerial);
        }

        if ( pDBEntry->pwszTimeValidFrom){
            VMCAFreeMemory(pDBEntry->pwszTimeValidFrom);
        }

        if ( pDBEntry->pwszTimeValidTo){
            VMCAFreeMemory(pDBEntry->pwszTimeValidTo);
        }

        if(pDBEntry->pCertBlob) {
            VMCAFreeMemory(pDBEntry->pCertBlob);
        }

        VMCAFreeMemory(pDBEntry);
    }
}
