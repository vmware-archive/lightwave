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



#include "includes.h"

/*
 * @brief Creates a certificate store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pszStoreName Name of the store
 *
 * @return Returns 0 for success
 */
DWORD
VecsLocalCreateCertStoreW(
        PVM_AFD_CONNECTION pConnection,
        PCWSTR pszStoreName,
        PCWSTR pszPassword,
        PBYTE *ppStoreHandle
        )
{
    DWORD dwError = 0;
    UINT32 apiType = VECS_IPC_CREATE_CERTSTORE;

    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    VMW_TYPE_SPEC input_spec[] = CREATE_STORE_REQUEST_PARAMS;
    VMW_TYPE_SPEC output_spec[] = OPEN_STORE_OUTPUT_PARAMS;


    if (IsNullOrEmptyString (pszStoreName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!ppStoreHandle)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR)pszStoreName;
    input_spec[1].data.pWString = (PWSTR)pszPassword;

    dwError = VecsLocalIPCRequestH (
                                    pConnection,
                                    apiType,
                                    noOfArgsIn,
                                    noOfArgsOut,
                                    input_spec,
                                    output_spec
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = *(output_spec[0].data.pUint32);

    BAIL_ON_VMAFD_ERROR (dwError);

    *ppStoreHandle = output_spec[1].data.pByte;
    output_spec[1].data.pByte = NULL;

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);
    return dwError;

error:

    if (ppStoreHandle)
    {
        *ppStoreHandle = NULL;
    }
    goto cleanup;
}

/**
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
    )
{
    DWORD dwError = 0;
    UINT32 apiType = VECS_IPC_ENUM_STORES;

    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    PWSTR *pwszStoreArray = NULL;
    DWORD dwStoreCount = 0;

    VMW_TYPE_SPEC output_spec[] = ENUM_STORE_OUTPUT_PARAMS;


    if (!pppwszStoreNameArray ||
        !pdwCount
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest (
                                    apiType,
                                    noOfArgsIn,
                                    noOfArgsOut,
                                    NULL,
                                    output_spec
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = *(output_spec[0].data.pUint32);

    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdUnMarshalStringArray(
                                        *output_spec[2].data.pUint32,
                                        output_spec[1].data.pByte,
                                        &pwszStoreArray,
                                        &dwStoreCount
                                       );
    BAIL_ON_VMAFD_ERROR (dwError);

    *pppwszStoreNameArray = pwszStoreArray;
    *pdwCount = dwStoreCount;

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);
    return dwError;

error:

    if (pppwszStoreNameArray)
    {
        *pppwszStoreNameArray = NULL;
    }
    if (pdwCount)
    {
        *pdwCount = 0;
    }
    if (pwszStoreArray)
    {
        VecsFreeStringArrayW (
                                pwszStoreArray,
                                dwStoreCount
                              );
    }

    goto cleanup;
}

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
    PBYTE *ppStoreHandle
    )
{
    DWORD dwError = 0;
    UINT32 apiType = VECS_IPC_OPEN_CERTSTORE;

    DWORD noOfArgsIn =  0;
    VMW_TYPE_SPEC input_spec[] = CREATE_STORE_REQUEST_PARAMS;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC output_spec[] = OPEN_STORE_OUTPUT_PARAMS;

    if (IsNullOrEmptyString(pszStoreName) ||
        !ppStoreHandle ||
        !pConnection
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pszStoreName;
    input_spec[1].data.pWString = (PWSTR) pszPassword;

    dwError = VecsLocalIPCRequestH (
                        pConnection,
                        apiType,
                        noOfArgsIn,
                        noOfArgsOut,
                        input_spec,
                        output_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppStoreHandle = output_spec[1].data.pByte;
    output_spec[1].data.pByte = NULL;

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);

    return dwError;
error:

    if (ppStoreHandle)
    {
        *ppStoreHandle = NULL;
    }
    goto cleanup;
}

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
    )
{
    DWORD dwError = 0;
    PWSTR pwszName = NULL;

    if (!pStore ||
        IsNullOrEmptyString (pszName)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringWFromA(
                                pszName,
                                &pwszName
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsLocalSetPermissionW (
                                pStore,
                                pwszName,
                                dwAccessMask
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pwszName);

    return dwError;
error:
    goto cleanup;
}


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
    )
{
    DWORD dwError = 0;
    UINT32 apiType = VECS_IPC_SET_PERMISSION;

    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    VMW_TYPE_SPEC input_spec[] = PERMISSIONS_SET_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    if (!pStore ||
        IsNullOrEmptyString (pszName)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pByte = (PBYTE) pStore->pStoreHandle;
    input_spec[2].data.pWString = (PWSTR) pszName;
    input_spec[3].data.pUint32 = &dwAccessMask;

    if (pStore->pConnection)
    {
        dwError = VecsLocalIPCRequestH (
                            pStore->pConnection,
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
    dwError = VecsLocalIPCRequest (
                                    apiType,
                                    noOfArgsIn,
                                    noOfArgsOut,
                                    input_spec,
                            output_spec);
    BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = *(output_spec[0].data.pUint32);

    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);
    return dwError;
error:
    goto cleanup;

}

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
    )
{
    DWORD dwError = 0;
    UINT32 apiType = VECS_IPC_REVOKE_PERMISSION;

    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    VMW_TYPE_SPEC input_spec[] = PERMISSIONS_SET_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    if (!pStore ||
        IsNullOrEmptyString (pszName)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pByte = (PBYTE) pStore->pStoreHandle;
    input_spec[2].data.pWString = (PWSTR) pszName;
    input_spec[3].data.pUint32 = &dwAccessMask;

    if (pStore->pConnection)
    {
        dwError = VecsLocalIPCRequestH (
                        pStore->pConnection,
                        apiType,
                        noOfArgsIn,
                        noOfArgsOut,
                        input_spec,
                        output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        dwError = VecsLocalIPCRequest (
                        apiType,
                        noOfArgsIn,
                        noOfArgsOut,
                        input_spec,
                        output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = *(output_spec[0].data.pUint32);

    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);
    return dwError;
error:
    goto cleanup;

}

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
VecsLocalGetPermissionW(
    PVECS_STORE pStore,
    PWSTR *ppszOwner,
    PDWORD pdwUserCount,
    PVECS_STORE_PERMISSION_W *ppStorePermissions
    )
{
    DWORD dwError = 0;
    PWSTR pszOwner = NULL;
    DWORD dwUserCount = 0;
    PVECS_STORE_PERMISSION_W pStorePermissions = NULL;
    UINT32 apiType = VECS_IPC_GET_PERMISSIONS;

    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    VMW_TYPE_SPEC input_spec[] = PERMISSIONS_GET_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = PERMISSIONS_GET_OUTPUT_PARAMS;

    if (!pStore ||
        !ppszOwner ||
        !pdwUserCount ||
        !ppStorePermissions
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }


    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pByte = (PBYTE) pStore->pStoreHandle;

    if (pStore->pConnection)
    {
        dwError = VecsLocalIPCRequestH (
                            pStore->pConnection,
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        dwError = VecsLocalIPCRequest (
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = *(output_spec[0].data.pUint32);

    BAIL_ON_VMAFD_ERROR (dwError);

    if (IsNullOrEmptyString (output_spec[1].data.pWString))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringW (
                           output_spec[1].data.pWString,
                           &pszOwner
                           );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdUnMarshalPermissionArray(
                            *output_spec[3].data.pUint32,
                            output_spec[2].data.pByte,
                            &dwUserCount,
                            &pStorePermissions
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppStorePermissions = pStorePermissions;
    *ppszOwner = pszOwner;
    *pdwUserCount = dwUserCount;


cleanup:
    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);
    return dwError;
error:
    if (ppStorePermissions)
    {
        *ppStorePermissions = NULL;
    }
    if (ppszOwner)
    {
        *ppszOwner = NULL;
    }
    if (pdwUserCount)
    {
        *pdwUserCount = 0;
    }

    if (pStorePermissions)
    {
        VmAfdFreeStorePermissionArray(
                              pStorePermissions,
                              dwUserCount
                              );
    }
    VMAFD_SAFE_FREE_MEMORY (pszOwner);
    goto cleanup;
}

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
    )
{
    DWORD dwError = 0;
    PWSTR pwszName = NULL;

    if (IsNullOrEmptyString (pszName) ||
        !pStore
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringWFromA (
                                    pszName,
                                    &pwszName
                                    );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsLocalChangeOwnerW(
                                    pStore,
                                    pwszName
                                    );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pwszName);

    return dwError;
error:
    goto cleanup;
}


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
    )
{
    DWORD dwError = 0;
    UINT32 apiType = VECS_IPC_CHANGE_OWNER;


    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    VMW_TYPE_SPEC input_spec[] = CHANGE_OWNER_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;


    if (!pStore ||
        IsNullOrEmptyString (pszName)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pByte = (PBYTE) pStore->pStoreHandle;
    input_spec[2].data.pWString = (PWSTR) pszName;

    if (pStore->pConnection)
    {
        dwError = VecsLocalIPCRequestH (
                            pStore->pConnection,
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        dwError = VecsLocalIPCRequest (
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }


    dwError = *(output_spec[0].data.pUint32);

    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);
    return dwError;
error:
    goto cleanup;
}

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
    )
{

    DWORD dwError = 0;
    UINT32 apiType = VECS_IPC_ADD_ENTRY;

    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    VMW_TYPE_SPEC input_spec[] = ADD_ENTRY_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;


    if (!pStore )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pByte =   pStore->pStoreHandle;
    input_spec[2].data.pUint32 = (PUINT32) &entryType;
    input_spec[3].data.pWString = (PWSTR) pszAlias;
    input_spec[4].data.pWString = (PWSTR) pszCertificate;
    input_spec[5].data.pWString = (PWSTR) pszPrivateKey;
    input_spec[6].data.pWString = (PWSTR) pszPassword;
    input_spec[7].data.pBoolean = &bAutoRefresh;

    if (pStore->pConnection)
    {
        dwError = VecsLocalIPCRequestH (
                            pStore->pConnection,
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
    dwError = VecsLocalIPCRequest (
                                    apiType,
                                    noOfArgsIn,
                                    noOfArgsOut,
                                    input_spec,
                            output_spec);
    BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = *(output_spec[0].data.pUint32);

    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);
    return dwError;
error:
    goto cleanup;

}

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
    PCWSTR pszAlias
    )
{
    DWORD dwError = 0;
    UINT32 apiType = VECS_IPC_DELETE_ENTRY;

    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    VMW_TYPE_SPEC input_spec[] = DELETE_ENTRY_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;


    if (!pStore ||
        IsNullOrEmptyString (pszAlias)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pByte = pStore->pStoreHandle;
    input_spec[2].data.pWString = (PWSTR) pszAlias;

    if (pStore->pConnection)
    {
        dwError = VecsLocalIPCRequestH (
                            pStore->pConnection,
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        dwError = VecsLocalIPCRequest (
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = *(output_spec[0].data.pUint32);

    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);
    return dwError;
error:
    goto cleanup;
}

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
    )
{
    DWORD dwError = 0;

    UINT32 apiType = VECS_IPC_BEGIN_ENUM_ENTRIES;


    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    VMW_TYPE_SPEC input_spec[] = BEGIN_ENUM_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = BEGIN_ENUM_OUTPUT_PARAMS;


    if (!pStore ||
        !ppEnumContext
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pByte = pStore->pStoreHandle;
    input_spec[2].data.pUint32 = (PUINT32) &dwEntryCount;
    input_spec[3].data.pUint32 = (PUINT32) &infoLevel;

    if (pStore->pConnection)
    {
        dwError = VecsLocalIPCRequestH (
                            pStore->pConnection,
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        dwError = VecsLocalIPCRequest (
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppEnumContext = output_spec[2].data.pByte;
    output_spec[2].data.pByte = NULL;
    *pdwLimit = *output_spec[1].data.pUint32;

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);

    return dwError;
error:

    if (ppEnumContext)
    {
        *ppEnumContext = NULL;
    }
    goto cleanup;

}


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
    )
{
    DWORD dwError = 0;
    PVMAFD_CERT_ARRAY pCertArray = NULL;

    vecs_entry_enum_handle_t pEnumHandleToClean = NULL;


    UINT32 apiType = VECS_IPC_ENUM_ENTRIES;

    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    VMW_TYPE_SPEC input_spec[] = ENUM_ENTRIES_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = ENUM_ENTRIES_OUTPUT_PARAMS;


    if (
         !pEnumContext ||
         !ppCertArray
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pByte = (PBYTE) pEnumContext->pEnumHandle;

    if (pEnumContext->pStore && pEnumContext->pStore->pConnection)
    {
        dwError = VecsLocalIPCRequestH (
                            pEnumContext->pStore->pConnection,
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        dwError = VecsLocalIPCRequest (
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdUnMarshalEntryArray (
                                        *output_spec[2].data.pUint32,
                                        output_spec[1].data.pByte,
                                        &pCertArray
                                       );
    BAIL_ON_VMAFD_ERROR (dwError);


    pEnumHandleToClean = pEnumContext->pEnumHandle;

    pEnumContext->pEnumHandle = (vecs_entry_enum_handle_t)output_spec[3].data.pByte;
    output_spec[3].data.pByte = NULL;

    *ppCertArray = pCertArray;

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);
    VMAFD_SAFE_FREE_MEMORY (pEnumHandleToClean);

    return dwError;
error:

   if (ppCertArray)
   {
      *ppCertArray = NULL;
   }
   if (pCertArray)
   {
      VmAfdFreeCertArray (pCertArray);
   }

   goto cleanup;
}


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
    )
{

    DWORD dwError = 0;
    UINT32 apiType = VECS_IPC_END_ENUM_ENTRIES;

    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    VMW_TYPE_SPEC input_spec[] = END_ENUM_REQUEST_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;


    if (!pEnumContext
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pByte = pEnumContext->pEnumHandle;

    if (pEnumContext->pStore && pEnumContext->pStore->pConnection)
    {
        dwError = VecsLocalIPCRequestH (
                            pEnumContext->pStore->pConnection,
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        dwError = VecsLocalIPCRequest (
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = *(output_spec[0].data.pUint32);

    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);

    if (pEnumContext)
    {
        VMAFD_SAFE_FREE_MEMORY (pEnumContext->pEnumHandle);
    }
    return dwError;
error:
    goto cleanup;

}

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
    )
{
    DWORD dwError = 0;
    PVECS_CERT_ENTRY_W pEntry = NULL;

    UINT32 apiType = VECS_IPC_GET_ENTRY_BY_ALIAS;

    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    VMW_TYPE_SPEC input_spec[] = GET_ENTRY_BY_ALIAS_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = GET_ENTRY_BY_ALIAS_OUTPUT_PARAMS;


    if (!pStore ||
        IsNullOrEmptyString (pszAlias) ||
        !ppEntry
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pByte = pStore->pStoreHandle;
    input_spec[2].data.pWString = (PWSTR) pszAlias;
    input_spec[3].data.pUint32 = (PUINT32) &infoLevel;

    if (pStore->pConnection)
    {
        dwError = VecsLocalIPCRequestH (
                            pStore->pConnection,
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        dwError = VecsLocalIPCRequest (
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = *(output_spec[0].data.pUint32);

    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateMemory (
                                   sizeof (VECS_CERT_ENTRY_W),
                                   (PVOID *)&pEntry
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    pEntry->entryType = *output_spec[1].data.pUint32;
    pEntry->dwDate = *output_spec[2].data.pUint32;

    if (!IsNullOrEmptyString(output_spec[3].data.pWString))
    {
        dwError = VmAfdAllocateStringW(
                                  output_spec[3].data.pWString,
                                  &pEntry->pwszCertificate
                                  );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringW(
                                  pszAlias,
                                  &pEntry->pwszAlias
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppEntry = pEntry;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);

    return dwError;
error:

    if (ppEntry)
    {
        *ppEntry = NULL;
    }
    if (pEntry)
    {
        VecsFreeCertEntryW(pEntry);
    }
    goto cleanup;
}


/*
 * @brief Gets a key from the store by alias
 *
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
    )
{
    DWORD dwError = 0;
    PWSTR pszKey = NULL;

    UINT32 apiType = VECS_IPC_GET_KEY_BY_ALIAS;

    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    VMW_TYPE_SPEC input_spec[] = GET_KEY_BY_ALIAS_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = GET_KEY_BY_ALIAS_OUTPUT_PARAMS;

    if (!pStore ||
        IsNullOrEmptyString (pszAlias) ||
        !ppszPrivateKey
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pByte = pStore->pStoreHandle;
    input_spec[2].data.pWString = (PWSTR) pszAlias;
    input_spec[3].data.pWString = (PWSTR) pszPassword;

    if (pStore->pConnection)
    {
        dwError = VecsLocalIPCRequestH (
                            pStore->pConnection,
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        dwError = VecsLocalIPCRequest (
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }



    dwError = *(output_spec[0].data.pUint32);

    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateStringW (
                                     output_spec[1].data.pWString,
                                     &pszKey
                                   );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszPrivateKey = pszKey;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);

    return dwError;
error:

    if (ppszPrivateKey)
    {
        *ppszPrivateKey = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY (pszKey);
    goto cleanup;

}

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
    )
{
    DWORD dwError = 0;

    UINT32 apiType = VECS_IPC_GET_ENTRY_COUNT;


    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    VMW_TYPE_SPEC input_spec[] = GET_ENTRY_COUNT_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = GET_ENTRY_COUNT_OUTPUT_PARAMS;

    if (!pStore ||
        !pdwSize
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pByte = pStore->pStoreHandle;

    if (pStore->pConnection)
    {
        dwError = VecsLocalIPCRequestH (
                            pStore->pConnection,
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        dwError = VecsLocalIPCRequest (
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = *(output_spec[0].data.pUint32);

    BAIL_ON_VMAFD_ERROR (dwError);

    *pdwSize = *output_spec[1].data.pUint32;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);

    return dwError;
error:

    if (pdwSize)
    {
        *pdwSize = 0;
    }
    goto cleanup;
}

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
    )
{
    DWORD dwError = 0;
    PVECS_CERT_ENTRY_W pCertEntry = NULL;
    CERT_ENTRY_TYPE entryType = CERT_ENTRY_TYPE_UNKNOWN;

    if (!pStore ||
        IsNullOrEmptyString (pwszAlias) ||
        !pType
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsLocalGetEntryByAliasW (
                                          pStore,
                                          pwszAlias,
                                          ENTRY_INFO_LEVEL_1,
                                          &pCertEntry
                                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    entryType = pCertEntry->entryType;

    *pType = entryType;

cleanup:
    if (pCertEntry)
    {
      VecsFreeCertEntryW (pCertEntry);
    }
    return dwError;

error:
    if (pType)
    {
        *pType = CERT_ENTRY_TYPE_UNKNOWN;
    }

    goto cleanup;
}

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
    )
{
    DWORD dwError = 0;
    PVECS_CERT_ENTRY_W pCertEntry = NULL;
    DWORD dwDate = 0;

    if (!pStore ||
        IsNullOrEmptyString (pwszAlias) ||
        !pdwDate
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsLocalGetEntryByAliasW (
                                          pStore,
                                          pwszAlias,
                                          ENTRY_INFO_LEVEL_1,
                                          &pCertEntry
                                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwDate = pCertEntry->dwDate;

    *pdwDate = dwDate;

cleanup:
    if (pCertEntry)
    {
      VecsFreeCertEntryW (pCertEntry);
    }
    return dwError;

error:
    if (pdwDate)
    {
        *pdwDate = 0;
    }

    goto cleanup;
}

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
    )
{
    DWORD dwError = 0;
    PVECS_CERT_ENTRY_W pCertEntry = NULL;
    PWSTR pszCertificate = NULL;

    if (!pStore ||
        IsNullOrEmptyString (pszAlias) ||
        !ppszCertificate
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsLocalGetEntryByAliasW (
                                          pStore,
                                          pszAlias,
                                          ENTRY_INFO_LEVEL_2,
                                          &pCertEntry
                                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (pCertEntry->entryType == CERT_ENTRY_TYPE_SECRET_KEY)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (pCertEntry->pwszCertificate)
    {
        dwError = VmAfdAllocateStringW (
                                    pCertEntry->pwszCertificate,
                                    &pszCertificate
                                   );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    *ppszCertificate = pszCertificate;

cleanup:
    if (pCertEntry)
    {
      VecsFreeCertEntryW (pCertEntry);
    }
    return dwError;

error:
    if (ppszCertificate)
    {
        *ppszCertificate = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY (pszCertificate);

    goto cleanup;
}


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
    )
{

    DWORD dwError = 0;
    UINT32 apiType = VECS_IPC_CLOSE_CERTSTORE;

    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    VMW_TYPE_SPEC input_spec[] = CLOSE_STORE_REQUEST_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;


    if (!pStore
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pByte = pStore->pStoreHandle;

    if (pStore->pConnection)
    {
        dwError = VecsLocalIPCRequestH (
                            pStore->pConnection,
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        dwError = VecsLocalIPCRequest (
                            apiType,
                            noOfArgsIn,
                            noOfArgsOut,
                            input_spec,
                            output_spec);
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = *(output_spec[0].data.pUint32);

    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);

    if (pStore)
    {
        VMAFD_SAFE_FREE_MEMORY (pStore->pStoreHandle);
    }
    return dwError;
error:
    goto cleanup;
}

/*
 * @brief Deletes a certificate store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pszStoreName Name of the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsLocalDeleteCertStoreW(
    PCWSTR pszStoreName
    )
{
    DWORD dwError = 0;
    UINT32 apiType = VECS_IPC_DELETE_CERTSTORE;

    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    VMW_TYPE_SPEC input_spec[] = DELETE_STORE_REQUEST_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    if (IsNullOrEmptyString (pszStoreName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR)pszStoreName;

    dwError = VecsLocalIPCRequest (
                                    apiType,
                                    noOfArgsIn,
                                    noOfArgsOut,
                                    input_spec,
                                    output_spec
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = *(output_spec[0].data.pUint32);

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);
    return dwError;

error:
    goto cleanup;

}

DWORD
VecsLocalIPCRequestH(
                    PVM_AFD_CONNECTION pConnection,
                    UINT32 apiType,
                    DWORD noOfArgsIn,
                    DWORD noOfArgsOut,
                    VMW_TYPE_SPEC *input_spec,
                    VMW_TYPE_SPEC *output_spec
                   )
{
    DWORD dwError = 0;

    DWORD dwRequestSize = 0;
    PBYTE pRequest = NULL;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;

    dwError = VmAfdGetMarshalLength (
                            input_spec,
                            noOfArgsIn,
                            &dwRequestSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateMemory (
                            dwRequestSize,
                            (PVOID *) &pRequest
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdMarshal (
                            apiType,
                            VER1_INPUT,
                            noOfArgsIn,
                            input_spec,
                            pRequest,
                            dwRequestSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdMakeServerRequest (
                            pConnection,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_OUTPUT,
                        noOfArgsOut,
                        pResponse,
                        dwResponseSize,
                        output_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pResponse);
    VMAFD_SAFE_FREE_MEMORY (pRequest);

    return dwError;

error:
    goto cleanup;

}

DWORD
VecsLocalIPCRequest(
                    UINT32 apiType,
                    DWORD noOfArgsIn,
                    DWORD noOfArgsOut,
                    VMW_TYPE_SPEC *input_spec,
                    VMW_TYPE_SPEC *output_spec
                    )
{
    DWORD dwError = 0;
    PVM_AFD_CONNECTION pConnection = NULL;

    dwError = VmAfdOpenClientConnection ( &pConnection);
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsLocalIPCRequestH(
                    pConnection,
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:

    if (pConnection)
    {
        VmAfdFreeClientConnection (pConnection);
    }

    return dwError;

error:
    goto cleanup;

}

