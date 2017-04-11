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

static
PVMAF_CFG_CONNECTION
VmAfPosixCfgAcquireConnection(
    PVMAF_CFG_CONNECTION pConnection
    );

static
VOID
VmAfPosixCfgFreeConnection(
    PVMAF_CFG_CONNECTION pConnection
    );

DWORD
VmAfPosixCfgOpenConnection(
    PVMAF_CFG_CONNECTION* ppConnection
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_CONNECTION pConnection = NULL;

    dwError = VmAfdAllocateMemory(sizeof(*pConnection), (PVOID*)&pConnection);
    BAIL_ON_VMAF_POSIX_ERROR(dwError);

    pConnection->refCount = 1;

    dwError = RegOpenServer(&pConnection->hConnection);
    BAIL_ON_VMAF_POSIX_ERROR(dwError);

    *ppConnection = pConnection;

cleanup:

    return dwError;

error:

    *ppConnection = NULL;

    if (pConnection)
    {
        VmAfPosixCfgCloseConnection(pConnection);
    }

    goto cleanup;
}

DWORD
VmAfPosixCfgOpenRootKey(
	PVMAF_CFG_CONNECTION pConnection,
	PCSTR                pszKeyName,
    DWORD                dwOptions,
    DWORD                dwAccess,
	PVMAF_CFG_KEY*       ppKey
	)
{
	DWORD dwError = 0;
	PVMAF_CFG_KEY pKey = NULL;

	if (!pConnection || IsNullOrEmptyString(pszKeyName) || !ppKey)
	{
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMAF_POSIX_ERROR(dwError);
	}

	if (strcmp(pszKeyName, "HKEY_LOCAL_MACHINE"))
	{
		dwError = ERROR_NOT_SUPPORTED;
		BAIL_ON_VMAF_POSIX_ERROR(dwError);
	}

	dwError = VmAfPosixCfgOpenKey(
					pConnection,
					NULL,
					"HKEY_THIS_MACHINE",
					dwOptions,
					dwAccess,
					&pKey);
	BAIL_ON_VMAF_POSIX_ERROR(dwError);

	*ppKey = pKey;

cleanup:

	return dwError;

error:

	if (ppKey)
	{
		*ppKey = NULL;
	}

//    if (pKey)
//    {
//        VmAfPosixCfgCloseKey(pKey);
//    }

	goto cleanup;
}

DWORD
VmAfPosixCfgOpenKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey,
    DWORD                dwOptions,
    DWORD                dwAccess,
    PVMAF_CFG_KEY*       ppKey
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_KEY pKeyLocal = NULL;

    dwError = VmAfdAllocateMemory(sizeof(*pKeyLocal), (PVOID*)&pKeyLocal);
    BAIL_ON_VMAF_POSIX_ERROR(dwError);

    dwError = RegOpenKeyExA(
                    pConnection->hConnection,
                    (pKey ? pKey->hKey : NULL),
                    pszSubKey,
                    dwOptions,
                    dwAccess,
                    &pKeyLocal->hKey);
    BAIL_ON_VMAF_POSIX_ERROR(dwError);

    pKeyLocal->pConnection = VmAfPosixCfgAcquireConnection(pConnection);

    *ppKey = pKeyLocal;

cleanup:

    return dwError;

error:

    *ppKey = NULL;

    if (pKeyLocal)
    {
        VmAfPosixCfgCloseKey(pKeyLocal);
    }

    goto cleanup;
}

DWORD
VmAfPosixCfgCreateKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey,
    DWORD                dwOptions,
    DWORD                dwAccess,
    PVMAF_CFG_KEY*       ppKey
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_KEY pKeyLocal = NULL;

    dwError = VmAfdAllocateMemory(sizeof(*pKeyLocal), (PVOID*)&pKeyLocal);
    BAIL_ON_VMAF_POSIX_ERROR(dwError);

    dwError = RegCreateKeyExA(
                    pConnection->hConnection,
                    (pKey ? pKey->hKey : NULL),
                    pszSubKey,
                    0,
                    NULL,
                    dwOptions,
                    dwAccess,
                    NULL, /* TODO: Security Descriptor */
                    &pKeyLocal->hKey,
                    NULL);
    BAIL_ON_VMAF_POSIX_ERROR(dwError);

    pKeyLocal->pConnection = VmAfPosixCfgAcquireConnection(pConnection);

    *ppKey = pKeyLocal;

cleanup:

    return dwError;

error:

    *ppKey = NULL;

    if (pKeyLocal)
    {
        VmAfPosixCfgCloseKey(pKeyLocal);
    }

    goto cleanup;
}

DWORD
VmAfPosixCfgDeleteKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey
    )
{
    DWORD dwError = 0;

    dwError = RegDeleteKeyA(
                        pConnection->hConnection,
                        (pKey ? pKey->hKey : NULL),
                        pszSubKey
                        );
    BAIL_ON_VMAF_POSIX_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfPosixCfgEnumKeys(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PSTR                 **pppszKeyNames,
    PDWORD               pdwKeyNameCount
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    DWORD dwCount = 0;
    PSTR  pszKeyName = NULL;
    PSTR  *ppszKeyNames = NULL;
    DWORD dwKeyNameSize = 0;

    while(1)
    {
        dwKeyNameSize = VMAF_REG_KEY_NAME_MAX_LENGTH;
        dwError = VmAfdAllocateMemory(
                                 dwKeyNameSize,
                                 (PVOID *)&pszKeyName
                                 );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = RegEnumKeyExA(
                      pConnection->hConnection,
                      (pKey ? pKey->hKey : NULL),
                      dwCount,
                      pszKeyName,
                      &dwKeyNameSize,
                      NULL,
                      NULL,
                      NULL,
                      NULL
                      );
        if (dwError == LWREG_ERROR_NO_MORE_KEYS_OR_VALUES ||
            !dwKeyNameSize
           )
        {
            dwError = 0;
            break;
        }
        BAIL_ON_VMAFD_ERROR(dwError);
        dwCount++;
        VMAFD_SAFE_FREE_MEMORY(pszKeyName);
    }

    if (dwCount)
    {

        dwError = VmAfdAllocateMemory(
                            sizeof(PSTR)*dwCount,
                            (PVOID *)&ppszKeyNames
                            );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    for (; dwIndex < dwCount; ++dwIndex)
    {
        dwKeyNameSize = VMAF_REG_KEY_NAME_MAX_LENGTH;
        dwError = VmAfdAllocateMemory(
                                dwKeyNameSize,
                                (PVOID *)&pszKeyName
                                );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError  = RegEnumKeyExA(
                      pConnection->hConnection,
                      (pKey ? pKey->hKey : NULL),
                      dwIndex,
                      pszKeyName,
                      &dwKeyNameSize,
                      NULL,
                      NULL,
                      NULL,
                      NULL
                      );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringA(
                                pszKeyName,
                                &ppszKeyNames[dwIndex]
                                );
        BAIL_ON_VMAFD_ERROR(dwError);
        VMAFD_SAFE_FREE_MEMORY(pszKeyName);
    }

    *pppszKeyNames = ppszKeyNames;
    *pdwKeyNameCount = dwCount;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszKeyName);
    return dwError;

error:

    if (pppszKeyNames)
    {
        *pppszKeyNames = NULL;
    }
    if (pdwKeyNameCount)
    {
        *pdwKeyNameCount = 0;
    }
    if (ppszKeyNames)
    {
        VmAfdFreeStringArrayA(ppszKeyNames);
    }
    goto cleanup;
}


DWORD
VmAfPosixCfgReadStringValue(
    PVMAF_CFG_KEY        pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PSTR*               ppszValue
    )
{
    DWORD dwError = 0;
    CHAR  szValue[VMAF_MAX_CONFIG_VALUE_BYTE_LENGTH] = {0};
    DWORD dwszValueSize = sizeof(szValue);
    PSTR  pszValue = NULL;

    dwError = RegGetValueA(
                    pKey->pConnection->hConnection,
                    pKey->hKey,
                    pszSubkey,
                    pszName,
                    RRF_RT_REG_SZ,
                    NULL,
                    szValue,
                    &dwszValueSize);
    if (dwError == LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
    {
        dwError = ERROR_FILE_NOT_FOUND;
    }
    BAIL_ON_VMAF_POSIX_ERROR(dwError);

    dwError = VmAfdAllocateStringA(szValue, &pszValue);
    BAIL_ON_VMAF_POSIX_ERROR(dwError);

    *ppszValue = pszValue;

cleanup:

    return dwError;

error:

    *ppszValue = NULL;

    if (pszValue)
    {
        VmAfdFreeStringA(pszValue);
    }

    goto cleanup;
}

DWORD
VmAfPosixCfgReadDWORDValue(
    PVMAF_CFG_KEY        pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PDWORD              pdwValue
    )
{
    DWORD dwError =0;
    DWORD dwValue = 0;
    DWORD dwValueSize = sizeof(dwValue);

    dwError = RegGetValueA(
                    pKey->pConnection->hConnection,
                    pKey->hKey,
                    pszSubkey,
                    pszName,
                    RRF_RT_REG_DWORD,
                    NULL,
                    (PVOID)&dwValue,
                    &dwValueSize);
    if (dwError == LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
    {
        dwError = ERROR_FILE_NOT_FOUND;
    }
    BAIL_ON_VMAF_POSIX_ERROR(dwError);

    *pdwValue = dwValue;

cleanup:

    return dwError;

error:

    *pdwValue = 0;

    goto cleanup;
}

DWORD
VmAfPosixCfgSetValue(
	PVMAF_CFG_KEY       pKey,
	PCSTR               pszValue,
	DWORD               dwType,
	PBYTE               pValue,
	DWORD               dwSize
	)
{
	DWORD dwError = 0;

	if (!pKey || IsNullOrEmptyString(pszValue))
	{
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMAFD_ERROR(dwError);
	}

	dwError = RegSetValueExA(
					pKey->pConnection->hConnection,
					pKey->hKey,
					pszValue,
					0,
					dwType,
					pValue,
					dwSize);

error:

	return dwError;
}

DWORD
VmAfPosixCfgDeleteValue(
	PVMAF_CFG_KEY       pKey,
	PCSTR               pszValue
	)
{
	DWORD dwError = 0;

	if (!pKey || IsNullOrEmptyString(pszValue))
	{
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMAFD_ERROR(dwError);
	}

	dwError = RegDeleteValueA(
                  pKey->pConnection->hConnection,
                  pKey->hKey,
                  pszValue
                  );

  if (dwError == LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
  {
      dwError = ERROR_FILE_NOT_FOUND;
  }

error:

	return dwError;
}

DWORD
VmAfPosixCfgGetSecurity(
    PVMAF_CFG_KEY         pKey,
    PSTR                 *ppszSecurityDescriptor
    )
{
    DWORD dwError = 0;
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptor = NULL;
    DWORD dwSecurityDescriptorLen = 0;
    SECURITY_INFORMATION secInfoAll = (OWNER_SECURITY_INFORMATION |
                                       GROUP_SECURITY_INFORMATION |
                                       DACL_SECURITY_INFORMATION  |
                                       SACL_SECURITY_INFORMATION);
    PSTR pszSecurityDescriptor = NULL;

    if (!pKey || !ppszSecurityDescriptor)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwSecurityDescriptorLen = SECURITY_DESCRIPTOR_RELATIVE_MAX_SIZE;
    dwError = RegGetKeySecurity(
                  pKey->pConnection->hConnection,
                  pKey->hKey,
                  secInfoAll,
                  NULL,
                  &dwSecurityDescriptorLen);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateMemory(
                  dwSecurityDescriptorLen,
                  (PVOID)&pSecurityDescriptor);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = RegGetKeySecurity(
                  pKey->pConnection->hConnection,
                  pKey->hKey,
                  secInfoAll,
                  pSecurityDescriptor,
                  &dwSecurityDescriptorLen);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = RtlAllocateSddlCStringFromSecurityDescriptor(
                   &pszSecurityDescriptor,
                   (PSECURITY_DESCRIPTOR_RELATIVE)pSecurityDescriptor,
                   SDDL_REVISION_1,
                   secInfoAll);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszSecurityDescriptor = pszSecurityDescriptor;

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pSecurityDescriptor);
    return dwError;

error:
    goto cleanup;
}

VOID
VmAfPosixCfgCloseKey(
    PVMAF_CFG_KEY pKey
    )
{
    if (pKey->pConnection)
    {
        if (pKey->hKey)
        {
            RegCloseKey(pKey->pConnection->hConnection, pKey->hKey);
        }

        VmAfPosixCfgCloseConnection(pKey->pConnection);
    }
    VmAfdFreeMemory(pKey);
}

VOID
VmAfPosixCfgCloseConnection(
    PVMAF_CFG_CONNECTION pConnection
    )
{
    if (InterlockedDecrement(&pConnection->refCount) == 0)
    {
        VmAfPosixCfgFreeConnection(pConnection);
    }
}

static
PVMAF_CFG_CONNECTION
VmAfPosixCfgAcquireConnection(
    PVMAF_CFG_CONNECTION pConnection
    )
{
    InterlockedIncrement(&pConnection->refCount);

    return pConnection;
}

static
VOID
VmAfPosixCfgFreeConnection(
    PVMAF_CFG_CONNECTION pConnection
    )
{
    if (pConnection->hConnection)
    {
        RegCloseServer(pConnection->hConnection);
    }
    VmAfdFreeMemory(pConnection);
}
