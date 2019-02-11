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
 * Defines cert data
 */
typedef struct _LWCA_DB_CERT_DATA
{
    PSTR                    pszIssuer;
    PSTR                    pszSerialNumber;
    PSTR                    pszIssuerSerialNumber;
    PSTR                    pszSKI;
    PSTR                    pszAKI;
    PSTR                    pszRevokedDate;     // ASN1 TIME Format
    PSTR                    pszTimeValidFrom;   // ASN1 TIME Format
    PSTR                    pszTimeValidTo;     // ASN1 TIME Format
    DWORD                   dwRevokedReason;
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
 * Defines trusted root cert data
 */
typedef struct _LWCA_DB_ROOT_CERT_DATA
{
    PSTR                    pszCAId;
    PLWCA_DB_CERT_DATA      pRootCertData;
    PLWCA_CERTIFICATE       pRootCertPEM;
    PLWCA_KEY               pEncryptedPrivateKey;
    PSTR                    pszChainOfTrust;
    PSTR                    pszCRLNumber;
    PSTR                    pszLastCRLUpdate;   // ASN1 TIME Format
    PSTR                    pszNextCRLUpdate;   // ASN1 TIME Format
} LWCA_DB_ROOT_CERT_DATA, *PLWCA_DB_ROOT_CERT_DATA;

/*
 * Defines root cert data array
 */
typedef struct _LWCA_DB_ROOT_CERT_DATA_ARRAY
{
    DWORD                       dwCount;
    PLWCA_DB_ROOT_CERT_DATA     *ppRootCertData;
} LWCA_DB_ROOT_CERT_DATA_ARRAY, *PLWCA_DB_ROOT_CERT_DATA_ARRAY;

/*
 * Defines CA data
 */
typedef struct _LWCA_DB_CA_DATA
{
    PSTR                        pszSubjectName;
    PSTR                        pszParentCAId;
    PSTR                        pszActiveCertSKI;
    PSTR                        pszAuthBlob;
    LWCA_CA_STATUS              status;
} LWCA_DB_CA_DATA, *PLWCA_DB_CA_DATA;

/*
 * Indicates any GetCerts API to retrieve different kinds of certs.
 */
typedef enum _LWCA_DB_GET_CERTS_FLAGS
{
    LWCA_DB_GET_ALL_CA_CERTS    = 0,
    LWCA_DB_GET_ALL_CERTS       = 1,
    LWCA_DB_GET_ACTIVE_CA_CERT  = 2,
    LWCA_DB_GET_CERT_VIA_SKI    = 3,
    LWCA_DB_GET_CERT_VIA_SERIAL = 4,
    LWCA_DB_GET_REVOKED_CERTS   = 5
} LWCA_DB_GET_CERTS_FLAGS;

/*
 * Defines filters for GetCAs API which retrieve CA's based on the data.
 */
typedef enum _LWCA_DB_CA_FILTER_TYPE
{
    LWCA_DB_CA_NONE         = 0,
    LWCA_DB_CA_SUBJECT_NAME = 1,
    LWCA_DB_CA_PARENT_CA_ID = 2,
    LWCA_DB_CA_STATUS       = 3
} LWCA_DB_CA_FILTER_TYPE, *PLWCA_DB_CA_FILTER_TYPE;

/*
 * Defines CA DB filter
 */
typedef struct _LWCA_DB_CA_FILTER
{
    LWCA_DB_CA_FILTER_TYPE  filter;
    VOID*                   pData;
} LWCA_DB_CA_FILTER, *PLWCA_DB_CA_FILTER;

/*
 * Defines CA DB filter array
 */
typedef struct _LWCA_DB_CA_FILTER_ARRAY
{
    PLWCA_DB_CA_FILTER  *ppFilter;
    DWORD               dwCount;
} LWCA_DB_CA_FILTER_ARRAY, *PLWCA_DB_CA_FILTER_ARRAY;

/*
 * Plugin handle stores plugin context
 */
typedef struct _LWCA_DB_HANDLE *PLWCA_DB_HANDLE;

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
 * Returns plugin version
 */
typedef PCSTR
(*PFN_LWCA_DB_GET_VERSION)(
    VOID
    );

/*
 * Stores CA data with CA ID as the primary key.
 */
typedef DWORD
(*PFN_LWCA_DB_ADD_CA)(
    PLWCA_DB_HANDLE             pHandle,            // IN
    PCSTR                       pcszCAId,           // IN
    PLWCA_DB_CA_DATA            pCAData,            // IN
    PLWCA_DB_ROOT_CERT_DATA     pCARootCertData     // IN
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
 * Get CA Id's
 */
typedef DWORD
(*PFN_LWCA_DB_GET_CA_IDS)(
    PLWCA_DB_HANDLE          pHandle,        //IN
    PLWCA_DB_CA_FILTER_ARRAY pFilter,        //IN
    PLWCA_STRING_ARRAY       *ppCAIds        //OUT
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
 * Free memory allocated to plugin CA data
 */
typedef VOID
(*PFN_LWCA_DB_FREE_CA_DATA)(
    PLWCA_DB_CA_DATA  pCAData       //IN
    );

/*
 * Acquire lock for CA Id
 */
typedef DWORD
(*PFN_LWCA_DB_LOCK_CA)(
    PLWCA_DB_HANDLE         pHandle,        //IN
    PCSTR                   pcszCAId,       //IN
    PSTR                    *ppszUuid       //OUT
    );

/*
 * Release lock for CA Id
 */
typedef DWORD
(*PFN_LWCA_DB_UNLOCK_CA)(
    PLWCA_DB_HANDLE         pHandle,        //IN
    PCSTR                   pcszCAId,       //IN
    PCSTR                   pcszUuid        //IN
    );

/*
 * Store a CA trusted root certificate
 */
typedef DWORD
(*PFN_LWCA_DB_ADD_CA_CERT)(
    PLWCA_DB_HANDLE             pHandle,        //IN
    PCSTR                       pcszCAId,       //IN
    PLWCA_DB_ROOT_CERT_DATA     pCACert         //IN
    );

/*
 * Get all CA's stored trusted root certificates
 */
typedef DWORD
(*PFN_LWCA_DB_GET_CA_CERTS)(
    PLWCA_DB_HANDLE                 pHandle,        // IN
    PCSTR                           pcszCAId,       // IN
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   *ppCACerts      // OUT
    );

/*
 * Get CA's trusted root certificate with a particular SKI
 */
typedef DWORD
(*PFN_LWCA_DB_GET_CA_CERT)(
    PLWCA_DB_HANDLE                 pHandle,        //IN
    PCSTR                           pcszCAId,       //IN - OPTIONAL
    PCSTR                           pcszSKI,        //IN - OPTIONAL
    PLWCA_DB_ROOT_CERT_DATA         *ppCACert       //OUT
    );

typedef DWORD
(*PFN_LWCA_DB_UPDATE_CA_CERT)(
    PLWCA_DB_HANDLE             pHandle,        //IN
    PCSTR                       pcszCAId,       //IN
    PLWCA_DB_ROOT_CERT_DATA     pCACert         //IN
    );
/*
 * Free memory allocated to plugin trusted root certificate data
 */
typedef VOID
(*PFN_LWCA_DB_FREE_ROOT_CERT_DATA)(
    PLWCA_DB_ROOT_CERT_DATA     pRootCertData     //IN
    );

/*
 * Free memory allocated to plugin trusted root certificate data array
 */
typedef VOID
(*PFN_LWCA_DB_FREE_ROOT_CERT_DATA_ARRAY)(
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   pRootCertDataArray    //IN
    );

/*
 * Acquire lock for CA trusted root Cert
 */
typedef DWORD
(*PFN_LWCA_DB_LOCK_CA_CERT)(
    PLWCA_DB_HANDLE         pHandle,            //IN
    PCSTR                   pcszCAId,           //IN
    PCSTR                   pcszSerialNumber,   //IN
    PSTR                    *ppszUuid           //OUT
    );

/*
 * Release lock for CA trusted root Cert
 */
typedef DWORD
(*PFN_LWCA_DB_UNLOCK_CA_CERT)(
    PLWCA_DB_HANDLE         pHandle,            //IN
    PCSTR                   pcszCAId,           //IN
    PCSTR                   pcszSerialNumber,   //IN
    PCSTR                   pcszUuid            //IN
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
 * Get all certificate objects from a CA
 */
typedef DWORD
(*PFN_LWCA_DB_GET_CERTS)(
    PLWCA_DB_HANDLE             pHandle,            //IN
    PCSTR                       pcszCAId,           //IN
    PLWCA_DB_CERT_DATA_ARRAY    *ppCerts            //OUT
    );

/*
 * Get a certificate with a particular serial number certificate
 */
typedef DWORD
(*PFN_LWCA_DB_GET_CERT)(
    PLWCA_DB_HANDLE             pHandle,            //IN
    PCSTR                       pcszCAId,           //IN - OPTIONAL
    PCSTR                       pcszSerialNumber,   //IN - OPTIONAL
    PLWCA_DB_CERT_DATA          *ppCert             //OUT
    );

/*
 * Get all revoked certificates of a CA
 */
typedef DWORD
(*PFN_LWCA_DB_GET_REVOKED_CERTS)(
    PLWCA_DB_HANDLE             pHandle,            //IN
    PCSTR                       pcszCAId,           //IN - OPTIONAL
    PCSTR                       pcszIssuerSKI,      //IN - OPTIONAL
    PLWCA_DB_CERT_DATA_ARRAY    *ppCerts            //OUT
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
 * Free memory allocated to plugin certificate data
 */
typedef VOID
(*PFN_LWCA_DB_FREE_CERT_DATA)(
    PLWCA_DB_CERT_DATA      pCertData   //IN
    );

/*
 * Free memory allocated to plugin certificate data array
 */
typedef VOID
(*PFN_LWCA_DB_FREE_CERT_DATA_ARRAY)(
    PLWCA_DB_CERT_DATA_ARRAY  pCertDataArray    //IN
    );

/*
 * Acquire lock for Cert
 */
typedef DWORD
(*PFN_LWCA_DB_LOCK_CERT)(
    PLWCA_DB_HANDLE         pHandle,            //IN
    PCSTR                   pcszCAId,           //IN
    PCSTR                   pcszSerialNumber,   //IN
    PSTR                    *ppszUuid           //OUT
    );

/*
 * Release lock for Cert
 */
typedef DWORD
(*PFN_LWCA_DB_UNLOCK_CERT)(
    PLWCA_DB_HANDLE         pHandle,            //IN
    PCSTR                   pcszCAId,           //IN
    PCSTR                   pcszSerialNumber,   //IN
    PCSTR                   pcszUuid            //IN
    );

/*
 * Free memory allocated to string
 */
typedef VOID
(*PFN_LWCA_DB_FREE_STRING)(
    PSTR  pszString   //IN
    );

/*
 * Free memory allocated to string
 */
typedef VOID
(*PFN_LWCA_DB_FREE_STRING_ARRAY)(
    PLWCA_STRING_ARRAY  pStrArray   //IN
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
    PFN_LWCA_DB_INITIALIZE                  pFnInit;
    PFN_LWCA_DB_GET_VERSION                 pFnGetVersion;

    PFN_LWCA_DB_ADD_CA                      pFnAddCA;
    PFN_LWCA_DB_GET_CA                      pFnGetCA;
    PFN_LWCA_DB_UPDATE_CA                   pFnUpdateCA;
    PFN_LWCA_DB_LOCK_CA                     pFnLockCA;
    PFN_LWCA_DB_UNLOCK_CA                   pFnUnlockCA;
    PFN_LWCA_DB_FREE_CA_DATA                pFnFreeCAData;

    PFN_LWCA_DB_GET_CA_IDS                  pFnGetCAIds;

    PFN_LWCA_DB_ADD_CA_CERT                 pFnAddCACert;
    PFN_LWCA_DB_GET_CA_CERTS                pFnGetCACerts;
    PFN_LWCA_DB_GET_CA_CERT                 pFnGetCACert;
    PFN_LWCA_DB_UPDATE_CA_CERT              pFnUpdateCACert;
    PFN_LWCA_DB_FREE_ROOT_CERT_DATA         pFnFreeCACertData;
    PFN_LWCA_DB_FREE_ROOT_CERT_DATA_ARRAY   pFnFreeCACertDataArray;
    PFN_LWCA_DB_LOCK_CA_CERT                pFnLockCACert;
    PFN_LWCA_DB_UNLOCK_CA_CERT              pFnUnlockCACert;

    PFN_LWCA_DB_ADD_CERT_DATA               pFnAddCert;
    PFN_LWCA_DB_GET_CERTS                   pFnGetCerts;
    PFN_LWCA_DB_GET_CERT                    pFnGetCert;
    PFN_LWCA_DB_GET_REVOKED_CERTS           pFnGetRevokedCerts;
    PFN_LWCA_DB_UPDATE_CERT_DATA            pFnUpdateCert;
    PFN_LWCA_DB_FREE_CERT_DATA              pFnFreeCertData;
    PFN_LWCA_DB_FREE_CERT_DATA_ARRAY        pFnFreeCertDataArray;
    PFN_LWCA_DB_LOCK_CERT                   pFnLockCert;
    PFN_LWCA_DB_UNLOCK_CERT                 pFnUnlockCert;

    PFN_LWCA_DB_FREE_STRING                 pFnFreeString;
    PFN_LWCA_DB_FREE_STRING_ARRAY           pFnFreeStringArray;
    PFN_LWCA_DB_FREE_HANDLE                 pFnFreeHandle;
} LWCA_DB_FUNCTION_TABLE, *PLWCA_DB_FUNCTION_TABLE;

#ifdef __cplusplus
}
#endif

#endif // __MUTENTCADB_H__
