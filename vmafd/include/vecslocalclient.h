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



#ifndef VECSLOCALCLIENT_H_
#define VECSLOCALCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief Creates a certificate store.
 *
 * @param[in] pszStoreName Name of the store
 *
 * @return Returns 0 for success
 */
DWORD
VecsLocalCreateCertStoreW(
        PVM_AFD_CONNECTION pConnection,
        PCWSTR pszStoreName,
        PCWSTR pszPassword,
        PBYTE *ppStore
        );

/*
 * @brief Enumerates certificate store names
 *
 * @param[out]    ppwszStoreNameArray Names of certificate stores
 * @param[in,out] pdwCount            Number of store names returned
 *
 * @return 0 on success
 */
DWORD
VecsLocalEnumCertStoreW(
    PWSTR** pppwszStoreNameArray,
    PDWORD pdwCount
    );

/*
 * @brief Open a cert store.
 *
 * @param[in] pszStoreName Name of the store
 * @param[in] pszPassword Password for the store
 * @param[out] pStore Handle to a store
 *
 * @return Returns 0 for sucess
 */
DWORD
VecsLocalOpenCertStoreW (
    PVM_AFD_CONNECTION pConnection,
    PCWSTR pszStoreName,
    PCWSTR pszPassword,
    PBYTE *ppStore
    );


/*
 * @brief Adds a certificate to the store.
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] entryType Type of entry
 * @param[in] pszAlias Alias of the entry
 * @param[in] pszCertificate Certificate Content
 * @param[in,optional] pszPrivateKey Private Key
 * @param[in, optional] pszPassword Password for the entry
 * @param[in] bAutoRefresh Whether to automatically renew the certificate
 *
 * @return Returns 0 for success
 */
DWORD
VecsLocalAddEntryW(
    PVECS_STORE pStore,
    CERT_ENTRY_TYPE entryType,
    PCWSTR pszAlias,
    PCWSTR pszCertificate,
    PCWSTR pszPrivateKey,
    PCWSTR pszPassword,
    BOOLEAN bAutoRefresh
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
VecsLocalDeleteEntryW(
    PVECS_STORE pStore,
    PCWSTR pwszAlias
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
VecsLocalBeginEnumEntries(
    PVECS_STORE pStore,
    DWORD dwEntryCount,
    ENTRY_INFO_LEVEL infoLevel,
    PBYTE *ppEnumContext,
    PDWORD pdwLimit
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
VecsLocalEnumEntriesW(
    PVECS_ENUM_CONTEXT pEnumContext,
    PVMAFD_CERT_ARRAY *ppCertArray
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
VecsLocalEndEnumEntries(
    PVECS_ENUM_CONTEXT pEnumContext
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
VecsLocalGetEntryByAliasW(
    PVECS_STORE pStore,
    PCWSTR pszAlias,
    ENTRY_INFO_LEVEL infoLevel,
    PVECS_CERT_ENTRY_W *ppEntry
    );

/*
 * @brief Gets a key from the store by alias
 *
 * DEPRECATED. Use GetEntryByAliasW Function instead
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[in] pszPassword Password
 * @param[out] pszPrivateKey PrivateKey entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsLocalGetKeyByAliasW(
    PVECS_STORE pStore,
    PCWSTR pszAlias,
    PCWSTR pszPassword,
    PWSTR *ppszPrivateKey
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
VecsLocalGetEntryTypeByAliasW(
    PVECS_STORE pStore,
    PCWSTR pwszAlias,
    CERT_ENTRY_TYPE *pType
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
VecsLocalGetEntryDateByAliasW(
    PVECS_STORE pStore,
    PCWSTR pwszAlias,
    PDWORD pdwDate
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
VecsLocalGetCertificateByAliasW(
    PVECS_STORE pStore,
    PCWSTR pszAlias,
    PWSTR *ppszCertificate
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
VecsLocalGetEntryCount(
    PVECS_STORE pStore,
    PDWORD pdwSize
    );

/*
 * @brief Closes a certificate store.
 *
 * @param[in] pStore Handle the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsLocalCloseCertStore(
    PVECS_STORE pStore
    );

/*
 * @brief Deletes a certificate store.
 *
 * @param[in] pszStoreName Name of the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsLocalDeleteCertStoreW(
    PCWSTR pszStoreName
    );

/*
 * @brief Sets Permission for a store.
 *
 * @param[in] pStore Handle to the store
 * @param[in] pszName Name of the service/user
 * @param[in] bAllowed Allow or deny specific access
 * @param[in] dwAccessMask AccessMask indicating access
 *
 * @return Returns 0 for success
 */
DWORD
VecsLocalSetPermissionA(
    PVECS_STORE pStore,
    PCSTR pszName,
    DWORD dwAccessMask
    );

/*
 * @brief Sets Permission for a store.
 *
 * @param[in] pStore Handle to the store
 * @param[in] pszName Name of the service/user
 * @param[in] bAllowed Allow or deny specific access
 * @param[in] dwAccessMask AccessMask indicating access
 *
 * @return Returns 0 for success
 */
DWORD
VecsLocalSetPermissionW(
    PVECS_STORE pStore,
    PCWSTR pszName,
    DWORD dwAccessMask
    );

/*
 * @brief Revoke Permission for a store.
 *
 * @param[in] pStore Handle to the store
 * @param[in] pszName Name of the service/user
 * @param[in] dwAccessMask AccessMask indicating access
 *
 * @return Returns 0 for success
 */
DWORD
VecsLocalRevokePermissionW(
    PVECS_STORE pStore,
    PCWSTR pszName,
    DWORD dwAccessMask
    );



/*
 * @brief Sets Permission for a store.
 *
 * @param[in] pStore Handle to the store
 * @param[in] pszName Name of the service/user
 * @param[in] bAllowed Allow or deny specific access
 * @param[in] dwAccessMask AccessMask indicating access
 *
 * @return Returns 0 for success
 */
DWORD
VecsLocalAddPermissionA(
    PVECS_STORE pStore,
    PCSTR pszName,
    DWORD dwAccessMask
    );

/*
 * @brief Sets Permission for a store.
 *
 * @param[in] pStore Handle to the store
 * @param[in] pszName Name of the service/user
 * @param[in] bAllowed Allow or deny specific access
 * @param[in] dwAccessMask AccessMask indicating access
 *
 * @return Returns 0 for success
 */
DWORD
VecsLocalAddPermissionW(
    PVECS_STORE pStore,
    PCWSTR pszName,
    DWORD dwAccessMask
    );

/*
 * @brief Sets Permission for a store.
 *
 * @param[in] pStore Handle to the store
 * @param[in] pszName Name of the service/user
 * @param[in] bAllowed Allow or deny specific access
 * @param[in] dwAccessMask AccessMask indicating access
 *
 * @return Returns 0 for success
 */
DWORD
VecsLocalGetPermissionW(
    PVECS_STORE pStore,
    PWSTR *ppszOwner,
    PDWORD pdwUserCount,
    PVECS_STORE_PERMISSION_W *ppStorePermissions
    );

/*
 * @brief Change the owner of a store.
 *
 * @param[in] pStore Handle to the store
 * @param[in] pszName Name of the service/user
 *
 * @return Returns 0 for success
 */
DWORD
VecsLocalChangeOwnerA(
    PVECS_STORE pStore,
    PCSTR pszName
    );

/*
 * @brief  Change the owner of a store.
 *
 * @param[in] pStore Handle to the store
 * @param[in] pszName Name of the service/user
 *
 * @return Returns 0 for success
 */
DWORD
VecsLocalChangeOwnerW(
    PVECS_STORE pStore,
    PCWSTR pszName
    );


#ifdef __cplusplus
}
#endif

#endif /* VECSLOCALCLIENT_H_ */
