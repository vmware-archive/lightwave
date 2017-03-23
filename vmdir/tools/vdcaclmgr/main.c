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
VOID
_FreeCLStateContent(
    PCOMMAND_LINE_PARAMETER_STATE pState
    )
{
    if (pState)
    {
        VMDIR_SAFE_FREE_MEMORY(pState->pszServerName);
        VMDIR_SAFE_FREE_MEMORY(pState->pszUserName);
        VMDIR_SECURE_FREE_STRINGA(pState->pszPassword);
        VMDIR_SAFE_FREE_MEMORY(pState->pszObjectName);
        VMDIR_SAFE_FREE_MEMORY(pState->pszBaseDN);
        VMDIR_SAFE_FREE_MEMORY(pState->pszGrantParameter);
        VMDIR_SAFE_FREE_MEMORY(pState->pszRemoveParameter);
        VMDIR_SECURE_FREE_STRINGA(pState->pszPasswordFile);
    }
}

VOID
VdcFreeHashMap(
    PLW_HASHMAP *ppHashMap
    )
{
    if (*ppHashMap != NULL)
    {
        LwRtlHashMapClear(*ppHashMap, VmDirSimpleHashMapPairFree, NULL);
        LwRtlFreeHashMap(ppHashMap);
    }
}

DWORD
_VdcAddCopiesToHashTable(
    PLW_HASHMAP pHashMap,
    PCSTR pszKey,
    PCSTR pszValue
    )
{
    DWORD dwError = 0;
    PSTR pszKeyCopy = NULL;
    PSTR pszValueCopy = NULL;

    dwError = VmDirAllocateStringA(pszKey, &pszKeyCopy);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszValue, &pszValueCopy);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlHashMapInsert(pHashMap, pszKeyCopy, pszValueCopy, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszKeyCopy);
    VMDIR_SAFE_FREE_STRINGA(pszValueCopy);
    goto cleanup;
}

VOID
ShowUsage(
    PVOID pvContext
    )
{
    printf("Usage: vdcaclmgr -H <host> -u <user UPN> [-w <password> | -x <password file>] -b <base DN> -o <object DN> [-g <username:permissions>] [-d <username:permissions>] [-r] [-v]\n");
}

DWORD
PostValidationRoutine(
    PVOID pvContext
    )
{
    PCOMMAND_LINE_PARAMETER_STATE pContext = (PCOMMAND_LINE_PARAMETER_STATE)pvContext;

    //
    // These parameters are all required.
    //
    if (pContext->pszServerName == NULL ||
        pContext->pszUserName == NULL ||
        pContext->pszBaseDN == NULL ||
        pContext->pszObjectName == NULL)
    {
        return VMDIR_ERROR_INVALID_PARAMETER;
    }

    //
    // Grant and Remove are optional but only one can be specified.
    //
    if (pContext->pszGrantParameter && pContext->pszRemoveParameter)
    {
        return VMDIR_ERROR_INVALID_PARAMETER;
    }

    //
    // Only one password is expected
    //
    if (pContext->pszPassword && pContext->pszPasswordFile)
    {
        return VMDIR_ERROR_INVALID_PARAMETER;
    }

    return 0;
}

DWORD
VdcGetUsersPassword(
    PCOMMAND_LINE_PARAMETER_STATE pParameters,
    PSTR pszPasswordBuf,
    DWORD dwBufferSize
    )
{
    DWORD dwError = 0;

    if (pParameters->pszPasswordFile != NULL)
    {
        dwError = VmDirReadStringFromFile(pParameters->pszPasswordFile, pszPasswordBuf, sizeof(pszPasswordBuf));
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (pParameters->pszPassword)
    {
        //
        // VmDirStringCpyA expects the buffer size to *not* include the terminating null.
        //
        dwError = VmDirStringCpyA(pszPasswordBuf, dwBufferSize - 1, pParameters->pszPassword);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        //
        // VmDirReadString expects the buffer size to include the terminating null.
        //
        VmDirReadString("password: ", pszPasswordBuf, dwBufferSize, FALSE);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
int
VmDirMain(int argc, char* argv[])
{
    DWORD dwError = 0;
    LDAP *pLd = NULL;
    COMMAND_LINE_PARAMETER_STATE State = { 0 };
    PLW_HASHMAP pUserToSidMapping = NULL; // Used to store "user/group SID" => "display name" mapping.
    PLW_HASHMAP pSidToUserMapping = NULL; // Used to store "display name" => "user/group SID" mapping.
    CHAR pszPasswordBuf[VMDIR_MAX_PWD_LEN + 1] = { 0 };
    PVMDIR_STRING_LIST pObjectDNs = NULL;
    DWORD dwStringIndex = 0;
    PSTR pszErrorMessage = NULL;

    VMDIR_COMMAND_LINE_OPTION Options[] =
    {
            {'H', "host", CL_STRING_PARAMETER, &State.pszServerName},
            {'u', "username", CL_STRING_PARAMETER, &State.pszUserName},
            {'w', "password", CL_STRING_PARAMETER, &State.pszPassword},
            {'b', "basedn", CL_STRING_PARAMETER, &State.pszBaseDN},
            {'o', "object", CL_STRING_PARAMETER, &State.pszObjectName},
            {'g', "grant", CL_STRING_PARAMETER, &State.pszGrantParameter},
            {'d', "delete", CL_STRING_PARAMETER, &State.pszRemoveParameter},
            {'x', "password-file", CL_STRING_PARAMETER, &State.pszPasswordFile},
            {'v', "verbose", CL_NO_PARAMETER, &State.bVerbose},
            {'r', "recursive", CL_NO_PARAMETER, &State.bRecursive},

            {0, 0, 0, 0}
    };

    VMDIR_PARSE_ARG_CALLBACKS Callbacks =
    {
            PostValidationRoutine,
            ShowUsage,
            &State
    };

    dwError = VmDirParseArguments(
                Options,
                &Callbacks,
                argc,
                argv);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcGetUsersPassword(&State, pszPasswordBuf, VMDIR_ARRAY_SIZE(pszPasswordBuf));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBind(
                &pLd,
                State.pszServerName,
                State.pszUserName,
                pszPasswordBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcLoadUsersAndGroups(pLd, State.pszBaseDN, &pUserToSidMapping, &pSidToUserMapping);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // We're either granting a user/group privileges on an object or just showing the
    // existing privileges on it.
    //
    dwError = VdcLdapEnumerateObjects(
                pLd,
                State.pszObjectName,
                State.bRecursive ? LDAP_SCOPE_SUBTREE : LDAP_SCOPE_BASE,
                &pObjectDNs);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwStringIndex = 0; dwStringIndex < pObjectDNs->dwCount; ++dwStringIndex)
    {
        if (State.pszGrantParameter)
        {
            dwError = VdcGrantPermissionToUser(
                        pLd,
                        pUserToSidMapping,
                        pObjectDNs->pStringList[dwStringIndex],
                        State.pszGrantParameter);
        }
        else if (State.pszRemoveParameter)
        {
            dwError = VdcRemovePermissionFromUser(
                        pLd,
                        pUserToSidMapping,
                        pObjectDNs->pStringList[dwStringIndex],
                        State.pszRemoveParameter);
        }
        else
        {
            dwError = VdcPrintSecurityDescriptorForObject(
                        pLd,
                        pSidToUserMapping,
                        pObjectDNs->pStringList[dwStringIndex],
                        State.bVerbose);
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VdcFreeHashMap(&pUserToSidMapping);
    VdcFreeHashMap(&pSidToUserMapping);
    _FreeCLStateContent(&State);
    VMDIR_SAFE_FREE_STRINGA(pszErrorMessage);
    return dwError;

error:
    VmDirGetErrorMessage(dwError, &pszErrorMessage);
    printf("vdcaclmgr failed. Error[%d] - %s\n", dwError, VDIR_SAFE_STRING(pszErrorMessage));
    goto cleanup;
}

#ifdef _WIN32

int wmain(int argc, wchar_t* argv[])
{
    DWORD dwError = 0;
    PSTR* ppszArgs = NULL;
    int   iArg = 0;

    dwError = VmDirAllocateMemory(sizeof(PSTR) * argc, (PVOID*)&ppszArgs);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (; iArg < argc; iArg++)
    {
        dwError = VmDirAllocateStringAFromW(argv[iArg], &ppszArgs[iArg]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirMain(argc, ppszArgs);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    if (ppszArgs)
    {
        for (iArg = 0; iArg < argc; iArg++)
        {
            VMDIR_SAFE_FREE_MEMORY(ppszArgs[iArg]);
        }

        VmDirFreeMemory(ppszArgs);
    }

    return dwError;
}

#else

int main(int argc, char* argv[])
{
    return VmDirMain(argc, argv);
}

#endif
