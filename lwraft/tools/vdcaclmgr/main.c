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

VOID
VdcHashMapFreeStringPair(
    PLW_HASHMAP_PAIR pPair,
    LW_PVOID pUserData
    )
{
    VMDIR_SAFE_FREE_MEMORY(pPair->pKey);
    VMDIR_SAFE_FREE_MEMORY(pPair->pValue);
}

VOID
VdcFreeHashMap(
    PLW_HASHMAP *ppHashMap
    )
{
    if (*ppHashMap != NULL)
    {
        LwRtlHashMapClear(*ppHashMap, VdcHashMapFreeStringPair, NULL);
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
    PVOID pvState
    )
{
    printf("Usage: vdcaclmgr -H <host> -u <user UPN> [-w <password> | -x <password file>] -o <object DN> [-g <username:permission>] [-r <username:permission>] [-v]\n");
}

DWORD
HandleServerParameterCallback(
    PVOID pContextPointer,
    PCSTR pValue
    )
{
    PCOMMAND_LINE_PARAMETER_STATE pContext = (PCOMMAND_LINE_PARAMETER_STATE)pContextPointer;

    if (pContext->pszServerName != NULL)
    {
        return VMDIR_ERROR_INVALID_PARAMETER;
    }

    pContext->pszServerName = pValue;

    return 0;
}

DWORD
HandleUserNameParameterCallback(
    PVOID pContextPointer,
    PCSTR pValue
    )
{
    PCOMMAND_LINE_PARAMETER_STATE pContext = (PCOMMAND_LINE_PARAMETER_STATE)pContextPointer;

    if (pContext->pszUserName != NULL)
    {
        return VMDIR_ERROR_INVALID_PARAMETER;
    }

    pContext->pszUserName = pValue;

    return 0;
}

DWORD
HandlePasswordFileParameterCallback(
    PVOID pContextPointer,
    PCSTR pValue
    )
{
    PCOMMAND_LINE_PARAMETER_STATE pContext = (PCOMMAND_LINE_PARAMETER_STATE)pContextPointer;

    if (pContext->pszPassword != NULL || pContext->pszPasswordFile != NULL)
    {
        return VMDIR_ERROR_INVALID_PARAMETER;
    }

    pContext->pszPasswordFile = pValue;

    return 0;
}

DWORD
HandlePasswordParameterCallback(
    PVOID pContextPointer,
    PCSTR pValue
    )
{
    PCOMMAND_LINE_PARAMETER_STATE pContext = (PCOMMAND_LINE_PARAMETER_STATE)pContextPointer;

    if (pContext->pszPassword != NULL || pContext->pszPasswordFile != NULL)
    {
        return VMDIR_ERROR_INVALID_PARAMETER;
    }

    pContext->pszPassword = pValue;

    return 0;
}

DWORD
HandleBaseDNParameterCallback(
    PVOID pContextPointer,
    PCSTR pValue
    )
{
    PCOMMAND_LINE_PARAMETER_STATE pContext = (PCOMMAND_LINE_PARAMETER_STATE)pContextPointer;

    if (pContext->pszBaseDN != NULL)
    {
        return VMDIR_ERROR_INVALID_PARAMETER;
    }

    pContext->pszBaseDN = pValue;

    return 0;
}

DWORD
HandleObjectParameterCallback(
    PVOID pContextPointer,
    PCSTR pValue
    )
{
    PCOMMAND_LINE_PARAMETER_STATE pContext = (PCOMMAND_LINE_PARAMETER_STATE)pContextPointer;

    if (pContext->pszObjectName != NULL)
    {
        return VMDIR_ERROR_INVALID_PARAMETER;
    }

    pContext->pszObjectName = pValue;

    return 0;
}

DWORD
HandleGrantParameterCallback(
    PVOID pContextPointer,
    PCSTR pValue
    )
{
    PCOMMAND_LINE_PARAMETER_STATE pContext = (PCOMMAND_LINE_PARAMETER_STATE)pContextPointer;

    if (pContext->pszGrantParameter != NULL)
    {
        return VMDIR_ERROR_INVALID_PARAMETER;
    }

    pContext->pszGrantParameter = pValue;

    return 0;
}

DWORD
HandleRemoveParameterCallback(
    PVOID pContextPointer,
    PCSTR pValue
    )
{
    PCOMMAND_LINE_PARAMETER_STATE pContext = (PCOMMAND_LINE_PARAMETER_STATE)pContextPointer;

    if (pContext->pszRemoveParameter != NULL)
    {
        return VMDIR_ERROR_INVALID_PARAMETER;
    }

    pContext->pszRemoveParameter = pValue;

    return 0;
}

DWORD
HandleVerboseParameterCallback(
    PVOID pContextPointer
    )
{
    PCOMMAND_LINE_PARAMETER_STATE pContext = (PCOMMAND_LINE_PARAMETER_STATE)pContextPointer;

    //
    // We don't check if the user specified '-v' multiple times.
    //
    pContext->bVerbose = TRUE;

    return 0;
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

    return 0;
}

VMDIR_COMMAND_LINE_OPTIONS CommandLineOptions =
{
    ShowUsage,
    PostValidationRoutine,
    {
        {'H', "host", CL_STRING_PARAMETER, HandleServerParameterCallback},
        {'u', "username", CL_STRING_PARAMETER, HandleUserNameParameterCallback},
        {'w', "password", CL_STRING_PARAMETER, HandlePasswordParameterCallback},
        {'b', "basedn", CL_STRING_PARAMETER, HandleBaseDNParameterCallback},
        {'o', "object", CL_STRING_PARAMETER, HandleObjectParameterCallback},
        {'g', "grant", CL_STRING_PARAMETER, HandleGrantParameterCallback},
        {'r', "remove", CL_STRING_PARAMETER, HandleRemoveParameterCallback},
        {'x', "password-file", CL_STRING_PARAMETER, HandlePasswordFileParameterCallback},
        {'v', "verbose", CL_NO_PARAMETER, HandleVerboseParameterCallback},
        {0, 0, 0, 0}
    }
};

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
    } else
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

    dwError = VmDirParseArguments(
                &CommandLineOptions,
                &State,
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
    if (State.pszGrantParameter)
    {
        dwError = VdcGrantPermissionToUser(pLd, pUserToSidMapping, State.pszObjectName, State.pszGrantParameter);
    }
    else if (State.pszRemoveParameter)
    {
        dwError = VdcRemovePermissionFromUser(pLd, pUserToSidMapping, State.pszObjectName, State.pszRemoveParameter);
    }
    else
    {
        dwError = VdcPrintSecurityDescriptorForObject(pLd, pSidToUserMapping, State.pszObjectName, State.bVerbose);
    }

cleanup:
    VdcFreeHashMap(&pUserToSidMapping);
    VdcFreeHashMap(&pSidToUserMapping);
    return dwError;

error:
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
