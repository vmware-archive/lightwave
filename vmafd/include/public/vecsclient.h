/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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



#ifndef VECSCLIENT_H_
#define VECSCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vmafdtypes.h"

typedef struct _VECS_STORE_ *PVECS_STORE;
typedef struct _VECS_ENUM_CONTEXT *PVECS_ENUM_CONTEXT;

typedef struct _VECS_CERT_ENTRY_W
{
    CERT_ENTRY_TYPE entryType;
    DWORD dwDate;
    PWSTR pwszAlias;
    PWSTR pwszCertificate;
    PWSTR pwszKey;

} VECS_CERT_ENTRY_W, *PVECS_CERT_ENTRY_W;

typedef struct _VECS_CERT_ENTRY_A
{
    CERT_ENTRY_TYPE entryType;
    DWORD dwDate;
    PSTR pszAlias;
    PSTR pszCertificate;
    PSTR pszKey;

} VECS_CERT_ENTRY_A, *PVECS_CERT_ENTRY_A;

typedef struct _VECS_STORE_PERMISSION_CONTAINER_A
{
    PSTR pszUserName;
    DWORD dwAccessMask;
} VECS_STORE_PERMISSION_A, *PVECS_STORE_PERMISSION_A;

/*
 * @brief Creates a certificate store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pszStoreName Name of the store
 * @param[out,optional] Handle to the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsCreateCertStoreA(
        PCSTR pszServerName,
        PCSTR pszStoreName,
        PCSTR pszPassword,
        PVECS_STORE *ppStore
        );

DWORD
VecsCreateCertStoreHA(
        PVMAFD_SERVER pServer,
        PCSTR pszStoreName,
        PCSTR pszPassword,
        PVECS_STORE *ppStore
        );

/*
 * @brief Creates a certificate store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pszStoreName Name of the store
 * @param[out,optional] Handle to the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsCreateCertStoreW(
        PCWSTR pszServerName,
        PCWSTR pszStoreName,
        PCWSTR pszPassword,
        PVECS_STORE *ppStore
        );

DWORD
VecsCreateCertStoreHW(
        PVMAFD_SERVER pServer,
        PCWSTR pszStoreName,
        PCWSTR pszPassword,
        PVECS_STORE *ppStore
        );

/*
 * @brief Opens a certificate store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pszStoreName Name of the store
 * @param[out] Handle to the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsOpenCertStoreA(
    PCSTR pszServerName,
    PCSTR pszStoreName,
    PCSTR pszPassword,
    PVECS_STORE *ppStore
    );

DWORD
VecsOpenCertStoreHA(
    PVMAFD_SERVER pServer,
    PCSTR pszStoreName,
    PCSTR pszPassword,
    PVECS_STORE *ppStore
    );

/*
 * @brief Opens a certificate store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pszStoreName Name of the store
 * @param[out] Handle to the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsOpenCertStoreW(
    PCWSTR pszServerName,
    PCWSTR pszStoreName,
    PCWSTR pszPassword,
    PVECS_STORE *ppStore
    );

DWORD
VecsOpenCertStoreHW(
    PVMAFD_SERVER pServer,
    PCWSTR pszStoreName,
    PCWSTR pszPassword,
    PVECS_STORE *ppStore
    );

/**
 * @brief Enumerates certificate store names
 *
 * @param[in]     pszServerName      Host Server
 * @param[out]    ppszStoreNameArray Names of certificate stores
 * @param[in,out] pdwCount            Number of store names returned
 *
 * @return 0 on success
 */
DWORD
VecsEnumCertStoreA(
    PCSTR pszServerName,
    PSTR** ppszStoreNameArray,
    PDWORD pdwCount
    );

DWORD
VecsEnumCertStoreHA(
    PVMAFD_SERVER pServer,
    PSTR** ppszStoreNameArray,
    PDWORD pdwCount
    );

/**
 * @brief Enumerates certificate store names
 *
 * @param[in]     pszServerName       Host Server
 * @param[out]    ppwszStoreNameArray Names of certificate stores
 * @param[in,out] pdwCount            Number of store names returned
 *
 * @return 0 on success
 */
DWORD
VecsEnumCertStoreW(
    PCWSTR pwszServerName,
    PWSTR** ppwszStoreNameArray,
    PDWORD pdwCount
    );

DWORD
VecsEnumCertStoreHW(
    PVMAFD_SERVER pServer,
    PWSTR** ppwszStoreNameArray,
    PDWORD pdwCount
    );

/*
 * @brief Adds a certificate to the store.
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] entryType Type of entry
 * @param[in] pszAlias Alias of the entry
 * @param[in] pszCertificate Certificate Content
 * @param[in,optional] pszPrivateKey Private Key
 * @param[in] pszPassword Password for the entry
 * @param[in] bAutoRefresh Whether to automatically renew the certificate
 *
 * @return Returns 0 for success
 */
DWORD
VecsAddEntryA(
    PVECS_STORE pStore,
    CERT_ENTRY_TYPE entryType,
    PCSTR pszAlias,
    PCSTR pszCertificate,
    PCSTR pszPrivateKey,
    PCSTR pszPassword,
    BOOLEAN bAutoRefresh
    );

/*
 * @brief Adds a certificate to the store.
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] entryType Type of entry
 * @param[in] pszAlias Alias of the entry
 * @param[in] pszCertificate Certificate Content
 * @param[in,optional] pszPrivateKey Private Key
 * @param[in] pszPassword Password for the entry
 * @param[in] bAutoRefresh Whether to automatically renew the certificate
 *
 * @return Returns 0 for success
 */
DWORD
VecsAddEntryW(
    PVECS_STORE pStore,
    CERT_ENTRY_TYPE entryType,
    PCWSTR pszAlias,
    PCWSTR pszCertificate,
    PCWSTR pszPrivateKey,
    PCWSTR pszPassword,
    BOOLEAN bAutoRefresh
    );

/*
 * @brief Gets type of an entry in the store by alias
 *
 * DEPRECATED. Use GetEntryByAliasA Function instead
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[out] pType Type of the entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetEntryTypeByAliasA(
    PVECS_STORE pStore,
    PCSTR pszAlias,
    CERT_ENTRY_TYPE *pType
    );

/*
 * @brief Gets type of an entry in the store by alias
 *
 * DEPRECATED. Use GetEntryByAliasW Function instead
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pwszAlias Alias of the entry
 * @param[out] pType Type of the entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetEntryTypeByAliasW(
    PVECS_STORE pStore,
    PCWSTR pwszAlias,
    CERT_ENTRY_TYPE *pType
    );

/*
 * @brief Gets date of an entry in the store by alias
 *
 * DEPRECATED. Use GetEntryByAliasA Function instead
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[out] pDate Date of the entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetEntryDateByAliasA(
    PVECS_STORE pStore,
    PCSTR pszAlias,
    PDWORD pdwDate
    );

/*
 * @brief Gets date of an entry in the store by alias
 *
 * DEPRECATED. Use GetEntryByAliasW Function instead
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pwszAlias Alias of the entry
 * @param[out] pDate Date of the entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetEntryDateByAliasW(
    PVECS_STORE pStore,
    PCWSTR pwszAlias,
    PDWORD pdwDate
    );

/*
 * @brief Gets a certificate from the store by alias
 *
 * DEPRECATED. Use GetEntryByAliasA Function instead
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[out] pszCertificate Certificate Content
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetCertificateByAliasA(
    PVECS_STORE pStore,
    PCSTR pszAlias,
    PSTR *ppszCertificate
    );

/*
 * @brief Gets a certificate from the store by alias
 *
 * DEPRECATED. Use GetEntryByAliasW Function instead
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[out] pszCertificate Certificate Content
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetCertificateByAliasW(
    PVECS_STORE pStore,
    PCWSTR pszAlias,
    PWSTR *ppszCertificate
    );

/*
 * @brief Gets an entry from the store by alias
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[in] infoLevel Level of info requied
 * @param[out] pEntry Entry Content
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetEntryByAliasA(
    PVECS_STORE pStore,
    PCSTR pszAlias,
    ENTRY_INFO_LEVEL infoLevel,
    PVECS_CERT_ENTRY_A *ppEntry
    );

/*
 * @brief Gets an entry from the store by alias
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[in] infoLevel Level of info requied
 * @param[out] pEntry Entry Content
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetEntryByAliasW(
    PVECS_STORE pStore,
    PCWSTR pszAlias,
    ENTRY_INFO_LEVEL infoLevel,
    PVECS_CERT_ENTRY_W *ppEntry
    );

/*
 * @brief Gets a key from the store by alias
 *
 * DEPRECATED. Use GetEntryByAliasA Function instead
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[in] pszPassword Password
 * @param[out] pszPrivateKey PrivateKey entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetKeyByAliasA(
    PVECS_STORE pStore,
    PCSTR pszAlias,
    PCSTR pszPassword,
    PSTR *ppszPrivateKey
    );

/*
 * @brief Gets a key from the store by alias
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[in] pszPassword Password
 * @param[out] pszPrivateKey PrivateKey entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetKeyByAliasW(
    PVECS_STORE pStore,
    PCWSTR pszAlias,
    PCWSTR pszPassword,
    PWSTR *ppszPrivateKey
    );

/*
 * @brief Returns number of entries in Store
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in,out] pdwSize number of entries in Store
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetEntryCount(
    PVECS_STORE pStore,
    PDWORD pdwSize
    );

/*
 * @brief Creates an enumeration handle
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] dwEntryCount maximum number of entries returned
 * @param[out] ppEnumContext Enumeration Handle
 *
 * @return Returns 0 for success
 */
DWORD
VecsBeginEnumEntries(
    PVECS_STORE pStore,
    DWORD dwEntryCount,
    ENTRY_INFO_LEVEL infoLevel,
    PVECS_ENUM_CONTEXT *ppEnumContext
    );

/*
 * @brief Enumerates certificates in a store
 *
 * @param[in] pEnumContext Enumeration Handle
 * @param[out] ppEntries Array of Certificate Entries
 * @param[in,out] pdwEntryCount Count of entries
 *
 * @return Returns 0 for success
 */
DWORD
VecsEnumEntriesA(
    PVECS_ENUM_CONTEXT pEnumContext,
    PVECS_CERT_ENTRY_A *ppEntries,
    PDWORD pdwEntryCount
    );

/*
 * @brief Enumerates Entries in a store
 *
 * @param[in] pEnumContext Enumeration Handle
 * @param[out] ppEntries Array of Entries
 * @param[in,out] pdwEntryCount Count of entries
 *
 * @return Returns 0 for success
 */
DWORD
VecsEnumEntriesW(
    PVECS_ENUM_CONTEXT pEnumContext,
    PVECS_CERT_ENTRY_W *ppEntries,
    PDWORD pdwEntryCount
    );

/*
 * @brief Closes an enumeration handle
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pEnumContext Enumeration Handle
 *
 * @return Returns 0 for success
 */
DWORD
VecsEndEnumEntries(
    PVECS_ENUM_CONTEXT pEnumContext
    );

/*
 * @brief Deletes a certificate from the store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsDeleteEntryA(
    PVECS_STORE pStore,
    PCSTR pszAlias
    );

/*
 * @brief Deletes a certificate from the store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pwszAlias Alias of the entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsDeleteEntryW(
    PVECS_STORE pStore,
    PCWSTR pwszAlias
    );

/*
 * @brief Sets Permission for a store.
 *
 * @param[in] pStore Handle to the store
 * @param[in] pszUserName Name of the service/user
 * @param[in] bAllowed Allow or deny specific access
 * @param[in] dwAccessMask AccessMask indicating access
 *
 * @return Returns 0 for success
 */
DWORD
VecsSetPermissionA(
    PVECS_STORE pStore,
    PCSTR pszUserName,
    DWORD dwAccessMask
    );

/*
 * @brief Sets Permission for a store.
 *
 * @param[in] pStore Handle to the store
 * @param[in] pszUserName Name of the service/user
 * @param[in] bAllowed Allow or deny specific access
 * @param[in] dwAccessMask AccessMask indicating access
 *
 * @return Returns 0 for success
 */
DWORD
VecsSetPermissionW(
    PVECS_STORE pStore,
    PCWSTR pszUserName,
    DWORD dwAccessMask
    );

/*
 * @brief Revokes Permission for a store.
 *
 * @param[in] pStore Handle to the store
 * @param[in] pszUserName Name of the service/user
 * @param[in] bAllowed Allow or deny specific access
 * @param[in] dwAccessMask AccessMask indicating access
 *
 * @return Returns 0 for success
 */
DWORD
VecsRevokePermissionA(
    PVECS_STORE pStore,
    PCSTR pszUserName,
    DWORD dwAccessMask
    );


/*
 * @brief Revoke Permission for a store.
 *
 * @param[in] pStore Handle to the store
 * @param[in] pszUserName Name of the service/user
 * @param[in] bAllowed Allow or deny specific access
 * @param[in] dwAccessMask AccessMask indicating access
 *
 * @return Returns 0 for success
 */
DWORD
VecsRevokePermissionW(
    PVECS_STORE pStore,
    PCWSTR pszUserName,
    DWORD dwAccessMask
    );


/*
 * @brief Gets Permission of a store.
 *
 * @param[in] pStore Handle to the store
 * @param[out] ppszOwner Owner of the store
 * @param[out] dwUserCount Number of users who have access to the store
 * @param[out] ppStorePermissions Permissions of the users
 *
 * @return Returns 0 for success
 */

DWORD
VecsGetPermissionsA(
    PVECS_STORE pStore,
    PSTR *ppszOwner,
    PDWORD pdwUserCount,
    PVECS_STORE_PERMISSION_A *ppStorePermissions
    );

/*
 * @brief Gets Permission of a store.
 *
 * @param[in] pStore Handle to the store
 * @param[out] ppszOwner Owner of the store
 * @param[out] dwUserCount Number of users who have access to the store
 * @param[out] ppStorePermissions Permissions of the users
 *
 * @return Returns 0 for success
 */

DWORD
VecsGetPermissionsW(
    PVECS_STORE pStore,
    PWSTR *ppszOwner,
    PDWORD pdwUserCount,
    PVECS_STORE_PERMISSION_W *ppStorePermissions
    );

/*
 * @brief CLoses a certificate store.
 *
 * @param[in] Handle to the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsCloseCertStore(
    PVECS_STORE pStore
    );

/*
 * @brief Deletes a certificate store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pszStoreName Name of the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsDeleteCertStoreA(
    PCSTR pszServerName,
    PCSTR pszStoreName
    );

DWORD
VecsDeleteCertStoreHA(
    PVMAFD_SERVER pServer,
    PCSTR pszStoreName
    );

/*
 * @brief Deletes a certificate store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pszStoreName Name of the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsDeleteCertStoreW(
    PCWSTR pszServerName,
    PCWSTR pszStoreName
    );

DWORD
VecsDeleteCertStoreHW(
    PVMAFD_SERVER pServer,
    PCWSTR pszStoreName
    );

/*
 * @brief Frees an entry
 *
 * @param[in] pCertEntry  Entry
 *
 */
VOID
VecsFreeCertEntryA(
    PVECS_CERT_ENTRY_A pCertEntry
    );

/*
 * @brief Frees an entry
 *
 * @param[in] pCertEntry Entry
 *
 */
VOID
VecsFreeCertEntryW(
    PVECS_CERT_ENTRY_W pCertEntry
    );

/*
 * @brief Frees an array of certificate entries
 *
 * @param[in] pEntries Array of Certificate Entries
 * @param[in] dwCount Count of certificate entries
 *
 * @return Returns 0 for success
 */
VOID
VecsFreeCertEntryArrayA(
    PVECS_CERT_ENTRY_A pCertEntryArray,
    DWORD dwCount
    );

/*
 * @brief Frees an array of certificate entries
 *
 * @param[in] pEntries Array of Certificate Entries
 * @param[in] dwCount Count of certificate entries
 *
 * @return Returns 0 for success
 */
VOID
VecsFreeCertEntryArrayW(
    PVECS_CERT_ENTRY_W pCertEntryArray,
    DWORD dwCount
    );

/*
 * @brief Frees an array of strings
 *
 * @param[in] ppszStringArray Array of strings
 * @param[in] dwCount Count of certificate entries
 *
 * @return Returns 0 for success
 */
VOID
VecsFreeStringArrayA(
    PSTR *pszStringArray,
    DWORD dwCount
    );

/*
 * @brief Frees an array of strings
 *
 * @param[in] pwszStringArray Array of strings
 * @param[in] dwCount Count of certificate entries
 *
 * @return Returns 0 for success
 */
VOID
VecsFreeStringArrayW(
    PWSTR *pwszStringArray,
    DWORD dwCount
    );

/*
 * @brief Frees an array of Store Permissions
 *
 * @param[in] pStorePermissions Array of Permissions
 * @param[in] dwCount Count of entries
 *
 * @return Returns 0 for success
 */

VOID
VecsFreeStorePermissionsArrayA(
    PVECS_STORE_PERMISSION_A pStorePermissions,
    DWORD dwCount
    );

/*
 * @brief Frees an array of Store Permissions
 *
 * @param[in] pStorePermissions Array of Permissions
 * @param[in] dwCount Count of entries
 *
 * @return Returns 0 for success
 */

VOID
VecsFreeStorePermissionsArrayW(
    PVECS_STORE_PERMISSION_W pStorePermissions,
    DWORD dwCount
    );


#ifdef __cplusplus
}
#endif

#endif /* VECSCLIENT_H_ */



