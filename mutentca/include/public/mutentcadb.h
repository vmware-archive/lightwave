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

#ifndef __MUTENTCADB_H__
#define __MUTENTCADB_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Defines CA data
 */
typedef struct _LWCA_DB_CA_DATA
{
    PSTR                            pszSubjectName;
    PLWCA_CERTIFICATE_ARRAY         pCertificates;
    PLWCA_KEY                       pEncryptedPrivateKey;
    PSTR                            pszCRLNumber;
    // ASN1 TIME Format
    PSTR                            pszLastCRLUpdate;
    // ASN1 TIME Format
    PSTR                            pszNextCRLUpdate;
    PSTR                            pszAuthBlob;
    LWCA_CA_STATUS                  status;
} LWCA_DB_CA_DATA, *PLWCA_DB_CA_DATA;

/*
 * Defines cert data
 */
typedef struct _LWCA_DB_CERT_DATA
{
    PSTR                    pszSerialNumber;
    DWORD                   revokedReason;
    // ASN1 TIME Format
    PSTR                    pszRevokedDate;
    // ASN1 TIME Format
    PSTR                    pszTimeValidFrom;
    // ASN1 TIME Format
    PSTR                    pszTimeValidTo;
    LWCA_CERT_STATUS        status;
} LWCA_DB_CERT_DATA, *PLWCA_DB_CERT_DATA;

/*
 * Defines cert data array
 */
typedef struct _LWCA_DB_CERT_DATA_ARRAY
{
    DWORD               dwCount;
    PLWCA_DB_CERT_DATA  *ppCertData;
} LWCA_DB_CERT_DATA_ARRAY, *PLWCA_DB_CERT_DATA_ARRAY;

/*
 * Plugin handle stores plugin context
 */
typedef struct _LWCA_DB_HANDLE *PLWCA_DB_HANDLE;

/*
 * Returns plugin version
 */
typedef PCSTR
(*PFN_LWCA_DB_GET_VERSION)(
    VOID
    );

/*
 * Assumption: Plugin will initialize by calling this API. And plugin is responsible for
 * loading its config file from file system (if required) and store its context in handle.
 */
typedef DWORD
(*PFN_LWCA_DB_INITIALIZE)(
    PCSTR           pcszPluginConfigPath,   //IN
    PLWCA_DB_HANDLE *ppHandle               //OUT
    );

/*
 * Stores CA data with CA ID as the primary key.
 */
typedef DWORD
(*PFN_LWCA_DB_ADD_CA)(
    PLWCA_DB_HANDLE        pHandle,       //IN
    PCSTR                  pcszCAId,      //IN
    PLWCA_DB_CA_DATA       pCAData,       //IN
    PCSTR                  pcszParentCAId //IN (OPTIONAL)
    );

/*
 * Stores certificate data issued by CA.
 */
typedef DWORD
(*PFN_LWCA_DB_ADD_CERT_DATA)(
    PLWCA_DB_HANDLE        pHandle,  //IN
    PCSTR                  pcszCAId, //IN
    PLWCA_DB_CERT_DATA     pCertData //IN
    );

/*
 * Check whether CA exists or not
 */
typedef DWORD
(*PFN_LWCA_DB_CHECK_CA)(
    PLWCA_DB_HANDLE        pHandle,      //IN
    PCSTR                  pcszCAId,     //IN
    PBOOLEAN               pbExists      //OUT
    );

/*
 * Check whether certdata exists or not
 */
typedef DWORD
(*PFN_LWCA_DB_CHECK_CERT_DATA)(
    PLWCA_DB_HANDLE        pHandle,             //IN
    PCSTR                  pcszCAId,            //IN
    PCSTR                  pcszSerialNumber,    //IN
    PBOOLEAN               pbExists             //OUT
    );

/*
 * Get CA data
 */
typedef DWORD
(*PFN_LWCA_DB_GET_CA)(
    PLWCA_DB_HANDLE          pHandle,        //IN
    PCSTR                    pcszCAId,       //IN
    PLWCA_DB_CA_DATA         *ppCAData       //OUT
    );

/*
 * Get CA stored certificates.
 */
typedef DWORD
(*PFN_LWCA_DB_GET_CA_CERTIFICATES)(
    PLWCA_DB_HANDLE            pHandle,     //IN
    PCSTR                      pcszCAId,    //IN
    PLWCA_CERTIFICATE_ARRAY    *ppCertArray //OUT
    );

/*
 * Get stored certificate data which is specific to CA.
 */
typedef DWORD
(*PFN_LWCA_DB_GET_CERT_DATA)(
    PLWCA_DB_HANDLE             pHandle,         //IN
    PCSTR                       pcszCAId,        //IN
    PLWCA_DB_CERT_DATA_ARRAY    *ppCertDataArray //OUT
    );

/*
 * Get Cert store id
 */
typedef DWORD
(*PFN_LWCA_DB_GET_CA_CRL_NUMBER)(
    PLWCA_DB_HANDLE             pHandle,         //IN
    PCSTR                       pcszCAId,        //IN
    PSTR                        *ppszCRLNumber     //OUT
    );

/*
 * Get Parent CA info
 */
typedef DWORD
(*PFN_LWCA_DB_GET_PARENT_CA_ID)(
    PLWCA_DB_HANDLE          pHandle,        //IN
    PCSTR                    pcszCAId,       //IN
    PSTR                     *ppszParentCAId //OUT
    );

/*
 * Update CA data
 */
typedef DWORD
(*PFN_LWCA_DB_UPDATE_CA)(
    PLWCA_DB_HANDLE          pHandle,    //IN
    PCSTR                    pcszCAId,   //IN
    PLWCA_DB_CA_DATA         pCAData     //IN
    );

/*
 * Update CA status
 */
typedef DWORD
(*PFN_LWCA_DB_UPDATE_CA_STATUS)(
    PLWCA_DB_HANDLE          pHandle,     //IN
    PCSTR                    pcszCAId,    //IN
    LWCA_CA_STATUS           status       //IN
    );

/*
 * Updates certificate data issued by CA.
 */
typedef DWORD
(*PFN_LWCA_DB_UPDATE_CERT_DATA)(
    PLWCA_DB_HANDLE        pHandle,  //IN
    PCSTR                  pcszCAId, //IN
    PLWCA_DB_CERT_DATA     pCertData //IN
    );

/*
 * Update Cert store id
 */
typedef DWORD
(*PFN_LWCA_DB_UPDATE_CA_CRL_NUMBER)(
    PLWCA_DB_HANDLE             pHandle,         //IN
    PCSTR                       pcszCAId,        //IN
    PCSTR                       pcszCRLNumber      //IN
    );

/*
 * Free memory allocated to plugin CA data
 */
typedef VOID
(*PFN_LWCA_DB_FREE_CA_DATA)(
    PLWCA_DB_CA_DATA  pCAData       //IN
    );

/*
 * Free memory allocated to plugin certificate data array
 */
typedef VOID
(*PFN_LWCA_DB_FREE_CERT_DATA_ARRAY)(
    PLWCA_DB_CERT_DATA_ARRAY  pCertDataArray       //IN
    );

/*
 * Free memory allocated to plugin certificate array
 */
typedef VOID
(*PFN_LWCA_DB_FREE_CERTIFICATE_ARRAY)(
    PLWCA_CERTIFICATE_ARRAY  pCertArray     //IN
    );

/*
 * Free memory allocated to string
 */
typedef VOID
(*PFN_LWCA_DB_FREE_STRING)(
    PSTR  pszString   //IN
    );

/*
 * Free memory allocated to plugin handle
 */
typedef VOID
(*PFN_LWCA_DB_FREE_HANDLE)(
    PLWCA_DB_HANDLE    pHandle      //IN
    );

/*
 * It stores function pointers of various methods to perform db operations.
 */
typedef struct _LWCA_DB_FUNCTION_TABLE
{
    PFN_LWCA_DB_INITIALIZE              pFnInit;
    PFN_LWCA_DB_ADD_CA                  pFnAddCA;
    PFN_LWCA_DB_ADD_CERT_DATA           pFnAddCertData;
    PFN_LWCA_DB_CHECK_CA                pFnCheckCA;
    PFN_LWCA_DB_CHECK_CERT_DATA         pFnCheckCertData;
    PFN_LWCA_DB_GET_CA                  pFnGetCA;
    PFN_LWCA_DB_GET_CA_CERTIFICATES     pFnGetCACertificates;
    PFN_LWCA_DB_GET_CERT_DATA           pFnGetCertData;
    PFN_LWCA_DB_GET_CA_CRL_NUMBER       pFnGetCACRLNumber;
    PFN_LWCA_DB_GET_PARENT_CA_ID        pFnGetParentCAId;
    PFN_LWCA_DB_UPDATE_CA               pFnUpdateCA;
    PFN_LWCA_DB_UPDATE_CA_STATUS        pFnUpdateCAStatus;
    PFN_LWCA_DB_UPDATE_CERT_DATA        pFnUpdateCertData;
    PFN_LWCA_DB_UPDATE_CA_CRL_NUMBER    pFnUpdateCACRLNumber;
    PFN_LWCA_DB_FREE_CA_DATA            pFnFreeCAData;
    PFN_LWCA_DB_FREE_CERT_DATA_ARRAY    pFnFreeCertDataArray;
    PFN_LWCA_DB_FREE_CERTIFICATE_ARRAY  pFnFreeCertArray;
    PFN_LWCA_DB_FREE_STRING             pFnFreeString;
    PFN_LWCA_DB_FREE_HANDLE             pFnFreeHandle;
} LWCA_DB_FUNCTION_TABLE, *PLWCA_DB_FUNCTION_TABLE;

#ifdef __cplusplus
}
#endif

#endif // __MUTENTCADB_H__
