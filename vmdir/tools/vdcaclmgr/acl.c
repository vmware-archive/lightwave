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

DWORD
_VdcParsePermissionsString(
    PCSTR pszPermissions,
    PVMDIR_STRING_LIST pStringList
    );

DWORD
_VdcParseAceFlagString(
    PCSTR pszAceFlags,
    PVMDIR_STRING_LIST pStringList
    );

static
DWORD
_VdcAppendNewValue(
    PVMDIR_STRING_LIST  pAceTokenList,
    PVMDIR_STRING_LIST  pNewValueList,
    DWORD               dwNum,
    BOOLEAN*            pbHasChange
    );

static
VOID
_VdcSkipNewValue(
    PVMDIR_STRING_LIST  pAceTokenList,
    PVMDIR_STRING_LIST  pNewValueList,
    DWORD               dwNum,
    BOOLEAN*            pbHasChange
    );

//
// Looks up the user's SID give a username. However, pUserName might already
// be the SID, in which case we just return that. On exit the caller owns
// *ppUserSid.
//
DWORD
_VdcLookupUserSid(
    PLW_HASHMAP pUserToSidMapping,
    PCSTR pszUserName,
    PSTR *ppszUserSid
    )
{
    PSTR pszUserSid = NULL;
    PSTR pszPotentialUserSid = NULL;
    DWORD dwError = 0;

    dwError = LwRtlHashMapFindKey(pUserToSidMapping, (PVOID*)&pszPotentialUserSid, pszUserName);
    if (dwError == ERROR_SUCCESS)
    {

        dwError = VmDirAllocateStringA(pszPotentialUserSid, &pszUserSid);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (dwError == LW_STATUS_NOT_FOUND)
    {
        //
        // If it's LW_STATUS_NOT_FOUND then we assume the supplied user name is
        // actually a SID. It might just be an invalid username but we have no
        // simple/fast way to verify that.
        //
        dwError = VmDirAllocateStringA(pszUserName, &pszUserSid);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        //
        // Otherwise, the error is fatal.
        //
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszUserSid = pszUserSid;

cleanup:
    return dwError;
error:
    goto cleanup;
}

//
// pszPermissionStatement :: = GRANTEE:(PERMISSIONS)*:(ACE_FLAGS)*
//
// GRANTEE          : could be cn or SID
// (PERMISSIONS)*   : optional permissions
// (ACE_FLAGS)*     : optional ACE_FLAGS (currently support 'CI' 'OI')
//
// Should have at lest one PERMISSIONS or ACE_FLAGS; otherwise, return VMDIR_ERROR_INVALID_PARAMETER
//
DWORD
VdcParsePermissionStatement(
    PVDC_ACLMGR_CTX   pCtx
    )
{
    DWORD dwError = 0;
    PVMDIR_STRING_LIST  pStrList = NULL;

    PCSTR pszPermissionStatement = pCtx->paramState.pszGrantParameter ?
                                    pCtx->paramState.pszGrantParameter :
                                    pCtx->paramState.pszRemoveParameter;

    if (!pszPermissionStatement)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirStringListInitialize(&pCtx->pPermissionList, DEFAULT_PERMISSION_LIST_SIZE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringListInitialize(&pCtx->pAceFlagList, DEFAULT_PERMISSION_LIST_SIZE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToTokenListExt(pszPermissionStatement, ":", &pStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pStrList || pStrList->dwCount < 2 || pStrList->dwCount > 3)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VdcLdapCNToSid(
        pCtx->pLd,
        pCtx->paramState.pszBaseDN,
        pStrList->pStringList[0],
        &pCtx->pszTargetSID);

    if (dwError == VMDIR_ERROR_ENTRY_NOT_FOUND)
    {   // not found? take user input as is -- assume user supply SID directly.
        dwError = VmDirAllocateStringA(pStrList->pStringList[0], &pCtx->pszTargetSID);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pStrList->pStringList[1][0] != '\0')
    {
        dwError = _VdcParsePermissionsString(pStrList->pStringList[1], pCtx->pPermissionList);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pStrList->dwCount == 3 && pStrList->pStringList[2][0] != '\0')
    {
        dwError = _VdcParseAceFlagString(pStrList->pStringList[2], pCtx->pAceFlagList);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    //
    // no permission and no ace flag
    //
    if (pCtx->pPermissionList->dwCount == 0 && (pCtx->pAceFlagList->dwCount == 0))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

cleanup:
    VmDirStringListFree(pStrList);

    return dwError;

error:
    goto cleanup;
}

DWORD
_VdcParseAceFlagString(
    PCSTR pszAceFlags,
    PVMDIR_STRING_LIST pStringList
    )
{
    DWORD dwError = 0;
    PSTR pszFlag = NULL;

    //
    // All ace flags are two characters long so the length of the entire string should
    // be even.
    //
    if (strlen(pszAceFlags) % SDDL_PERMISSION_LENGTH != 0)
    {
        dwError = VMDIR_ERROR_INVALID_ACE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (*pszAceFlags)
    {
        //
        // All ace flags are two characters long.
        //
        dwError = VmDirAllocateStringOfLenA(pszAceFlags, SDDL_PERMISSION_LENGTH, &pszFlag);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszAceFlags += SDDL_PERMISSION_LENGTH;

        if (VmDirStringCompareA(pszFlag, "CI", TRUE) != 0 &&
            VmDirStringCompareA(pszFlag, "OI", TRUE) != 0 )
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_ACE);
        }

        dwError = VmDirStringListAdd(pStringList, pszFlag);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszFlag = NULL;
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszFlag);
    return dwError;

error:
    goto cleanup;
}

DWORD
_VdcParsePermissionsString(
    PCSTR pszPermissions,
    PVMDIR_STRING_LIST pStringList
    )
{
    DWORD dwError = 0;

    //
    // All permissions are two characters long so the length of the entire string should
    // be even.
    //
    if (strlen(pszPermissions) % SDDL_PERMISSION_LENGTH != 0)
    {
        dwError = VMDIR_ERROR_INVALID_ACE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (*pszPermissions)
    {
        PSTR pszPermission = NULL;

        //
        // All permissions are two characters long.
        //
        dwError = VmDirAllocateStringOfLenA(pszPermissions, SDDL_PERMISSION_LENGTH, &pszPermission);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszPermissions += SDDL_PERMISSION_LENGTH;

        dwError = VmDirStringListAdd(pStringList, pszPermission);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
_VdcParseAce(
    PCSTR pszAce,
    PSTR *ppszSid,
    PVMDIR_STRING_LIST *ppPermissionList
    )
{
    DWORD dwError = 0;
    PSTR pszStringEnd = NULL;
    PSTR pszPermissions = NULL;
    PSTR pszSid = NULL;
    PVMDIR_STRING_LIST pPermissionList = NULL;

    assert(pszAce[0] == '(' && pszAce[strlen(pszAce) - 1] == ')');

    dwError = VmDirStringListInitialize(&pPermissionList, DEFAULT_PERMISSION_LIST_SIZE);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // Skip the ace type.
    //
    pszAce = strchr(pszAce, ';');
    if (pszAce == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszAce += 1;

    //
    // Skip the ace flags.
    //
    pszAce = strchr(pszAce, ';');
    if (pszAce == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszAce += 1;

    pszStringEnd = strchr(pszAce, ';');
    if (pszStringEnd == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringOfLenA(pszAce, pszStringEnd - pszAce, &pszPermissions);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcParsePermissionsString(pszPermissions, pPermissionList);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszAce = pszStringEnd + 1;

    //
    // Skip the object guid.
    //
    pszAce = strchr(pszAce, ';');
    if (pszAce == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pszAce += 1;

    //
    // Skip the inherit object guid.
    //
    pszAce = strchr(pszAce, ';');
    if (pszAce == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pszAce += 1;

    pszStringEnd = strchr(pszAce, ';');
    if (pszStringEnd == NULL)
    {
        pszStringEnd = strchr(pszAce, ')');
        if (pszStringEnd == NULL)
        {
            dwError = VMDIR_ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = VmDirAllocateStringOfLenA(pszAce, pszStringEnd - pszAce, &pszSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppPermissionList = pPermissionList;
    *ppszSid = pszSid;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszSid);
    VmDirStringListFree(pPermissionList);
    goto cleanup;
}

DWORD
_VdcParseSecurityDescriptor(
    PCSTR pszSecurityDescriptor,
    PSTR *ppszOwner,
    PSTR *ppszGroup,
    PVMDIR_STRING_LIST *ppAceList
    )
{
    DWORD dwError = 0;
    PSTR pszOwner = NULL;
    PSTR pszGroup = NULL;
    PVMDIR_STRING_LIST pAceList = NULL;
    PSTR pszStringEnd = NULL;

    dwError = VmDirStringListInitialize(&pAceList, DEFAULT_ACE_LIST_SIZE);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pszSecurityDescriptor[0] != 'O' || pszSecurityDescriptor[1] != ':')
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    //
    // Skip "O:" prefix.
    //
    pszSecurityDescriptor += 2;

    pszStringEnd = strstr(pszSecurityDescriptor, "G:");
    if (pszStringEnd == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringOfLenA(pszSecurityDescriptor, pszStringEnd - pszSecurityDescriptor, &pszOwner);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // Skip past "G:"
    //
    pszSecurityDescriptor = pszStringEnd + 2;

    pszStringEnd = strstr(pszSecurityDescriptor, "D:");
    if (pszStringEnd == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringOfLenA(pszSecurityDescriptor, pszStringEnd - pszSecurityDescriptor, &pszGroup);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // Skip past "D:"
    //
    pszSecurityDescriptor = pszStringEnd + 2;

    //
    // Skip any DACL flags.
    //
    if (*pszSecurityDescriptor != '(')
    {
        pszStringEnd = strchr(pszSecurityDescriptor, '(');
        if (pszStringEnd == NULL)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
        }
        pszSecurityDescriptor = pszStringEnd;
    }

    while (*pszSecurityDescriptor)
    {
        PSTR pszAce = NULL;

        if (*pszSecurityDescriptor != '(')
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
        }

        pszStringEnd = strchr(pszSecurityDescriptor, ')');
        if (pszStringEnd == NULL)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
        }

        //
        // + 1 to grab the trailing ')'. We do this more for consistency than anything.
        //
        dwError = VmDirAllocateStringOfLenA(pszSecurityDescriptor, pszStringEnd - pszSecurityDescriptor + 1, &pszAce);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirStringListAdd(pAceList, pszAce);
        BAIL_ON_VMDIR_ERROR(dwError);

        //
        // Skip past ')' to next ACE (or to the terminating NULL).
        //
        pszSecurityDescriptor = pszStringEnd + 1;
    }

    *ppszOwner = pszOwner;
    *ppszGroup = pszGroup;
    *ppAceList = pAceList;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszOwner);
    VMDIR_SAFE_FREE_STRINGA(pszGroup);
    VmDirStringListFree(pAceList);
    goto cleanup;
}

//
// Fills a hash table with a mapping of different permissions to their description.
//
DWORD
_VdcInitializePermissionDescriptions(
    PLW_HASHMAP *ppHashMap
    )
{
    DWORD dwError = 0;
    PLW_HASHMAP pHashMap = NULL;

    dwError = LwRtlCreateHashMap(
                &pHashMap,
                LwRtlHashDigestPstr,
                LwRtlHashEqualPstr,
                NULL
                );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "GR", "Generic Read");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "GE", "Generic Execute");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "GW", "Generic Write");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "GX", "Generic Execute");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "GA", "Generic All");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "SD", "Delete an object");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "DT", "Delete an object and all of its child objects");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "RC", "Read security information");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "WD", "Change security information");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "WO", "Change owner information");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "LC", "List child objects");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "CC", "Create a child object");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "DC", "Delete a child object");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "WS", "Write to a self object");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "RP", "Read a property");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "WP", "Write to a property");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "CA", "Control access");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcAddCopiesToHashTable(pHashMap, "LO", "List the object access");
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppHashMap = pHashMap;

cleanup:
    return dwError;

error:
    VdcFreeHashMap(&pHashMap);
    goto cleanup;
}

DWORD
_VdcGetObjectSecurityDescriptor(
    LDAP *pLd,
    PCSTR pszBaseDN,
    PSTR *ppszSecurityDescriptor
    )
{
    DWORD dwError = 0;
    PSTR pszSecurityDescriptor = NULL;

    dwError = VdcLdapGetAttributeValue(pLd,
                                       pszBaseDN,
                                       ATTR_ACL_STRING,
                                       &pszSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszSecurityDescriptor = pszSecurityDescriptor;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
_VdcAddAceToSecurityDescriptor(
    PCSTR pszObjectSD,
    PCSTR pszUserSid,
    PVMDIR_STRING_LIST  pPermissionList,
    PVMDIR_STRING_LIST  pAceFlagList,
    BOOLEAN bVerbose,
    PSTR *ppszNewSecurityDescriptor
    )
{
    DWORD dwError = 0;
    PSTR pszPermission = NULL;
    PSTR pszAceFlag = NULL;
    PSTR pszNewSecurityDescriptor = NULL;
    PSTR pszNewAce = NULL;
    SIZE_T  dwTmpSize = 0;
    DWORD   dwIdx = 0;

    if (pPermissionList)
    {
        dwTmpSize = pPermissionList->dwCount * 2 + 1; //  +1 for null
        dwError = VmDirAllocateMemory(dwTmpSize, (PVOID*)&pszPermission);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (dwIdx = 0; dwIdx < pPermissionList->dwCount; dwIdx++)
        {
            dwError = VmDirStringCatA(
                        pszPermission,
                        dwTmpSize,
                        pPermissionList->pStringList[dwIdx]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    if (pAceFlagList)
    {
        dwTmpSize = pAceFlagList->dwCount * 2 + 1; //  +1 for null
        dwError = VmDirAllocateMemory(dwTmpSize, (PVOID*)&pszAceFlag);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (dwIdx = 0; dwIdx < pAceFlagList->dwCount; dwIdx++)
        {
            dwError = VmDirStringCatA(
                        pszAceFlag,
                        dwTmpSize,
                        pAceFlagList->pStringList[dwIdx]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    if (!pszPermission && !pszAceFlag)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateStringPrintf(
                &pszNewAce, "(A;%s;%s;;;%s)",
                pszAceFlag ? pszAceFlag : "",
                pszPermission ? pszPermission : "",
                pszUserSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bVerbose)
    {
        printf("New ACE: %s\n\n", pszNewAce);
    }

    dwError = VmDirAllocateStringPrintf(
                &pszNewSecurityDescriptor,
                "%s%s",
                pszObjectSD,
                pszNewAce);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszNewSecurityDescriptor = pszNewSecurityDescriptor;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszAceFlag);
    VMDIR_SAFE_FREE_MEMORY(pszPermission);
    VMDIR_SAFE_FREE_MEMORY(pszNewAce);
    return dwError;

error:
    goto cleanup;
}

/*
 * OUT VAR *ppszNewSecurityDescriptor  could be NULL if there is no change needed.
 */
DWORD
_VdcUpdateSecurityDescriptor(
    PSTR *ppszNewSecurityDescriptor,
    PCSTR pszObjectSD,
    PCSTR pszTargetAce,
    PVMDIR_STRING_LIST  pPermissionList,
    PVMDIR_STRING_LIST  pAceFlagList,
    BOOLEAN fAddPermission,
    BOOLEAN bVerbose
    )
{
    DWORD dwError = 0;
    SIZE_T sDestinationBufferSize = 0;
    PSTR pszNewSecurityDescriptor = NULL;
    PSTR pszAceStart = NULL;
    PSTR pszRemainingSD = NULL;
    PSTR pszNewAce = NULL;
    PVMDIR_STRING_LIST  pLocalStrList = NULL;
    BOOLEAN bHasChange = FALSE;

    //
    // calculate buffer size
    //
    sDestinationBufferSize = strlen(pszObjectSD) +
                             (pPermissionList ? pPermissionList->dwCount * 2 : 0) +   // permission is 2 chars
                             (pAceFlagList ? pAceFlagList->dwCount * 2 : 0) +         // ace flag is 2 chars
                             1;                                                       // +1 for null
    dwError = VmDirAllocateMemory(sDestinationBufferSize, (PVOID*)&pszNewSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszAceStart = strstr(pszObjectSD, pszTargetAce);
    dwError = VmDirStringNCpyA(
                pszNewSecurityDescriptor,
                sDestinationBufferSize,
                pszObjectSD,
                pszAceStart - pszObjectSD);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszRemainingSD = pszAceStart + VmDirStringLenA(pszTargetAce);

    // separate pszTargetAce into token
    dwError = VmDirStringToTokenListExt(pszTargetAce, ";", &pLocalStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    // proper ACE should have 6 parts
    if (pLocalStrList->dwCount != 6)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_ACE);
    }

    if (fAddPermission)
    {
        if (pPermissionList && pPermissionList->dwCount > 0)
        {
            dwError = _VdcAppendNewValue(pLocalStrList, pPermissionList, 2, &bHasChange);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (pAceFlagList && pAceFlagList->dwCount > 0)
        {
            dwError = _VdcAppendNewValue(pLocalStrList, pAceFlagList, 1, &bHasChange);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {   // delete permission
        if (pPermissionList && pPermissionList->dwCount > 0)
        {
            _VdcSkipNewValue(pLocalStrList, pPermissionList, 2, &bHasChange);
        }

        if (pAceFlagList && pAceFlagList->dwCount > 0)
        {
            _VdcSkipNewValue(pLocalStrList, pAceFlagList, 1, &bHasChange);
        }
    }

    // make sure we have at least one permission; otherwise, skip this ACE.
    if (pLocalStrList->pStringList[2][0] != '\0')
    {
        dwError = VmDirAllocateStringPrintf(
                    &pszNewAce, "%s;%s;%s;%s;%s;%s",
                    pLocalStrList->pStringList[0],
                    pLocalStrList->pStringList[1],
                    pLocalStrList->pStringList[2],
                    pLocalStrList->pStringList[3],
                    pLocalStrList->pStringList[4],
                    pLocalStrList->pStringList[5]);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (bVerbose)
        {
            if (bHasChange)
            {
                printf("Old ACE: %s\n", pszTargetAce);
                printf("New ACE: %s\n\n", pszNewAce);
            }
            else
            {
                printf("No ACE change needed\n\n");
            }
        }

        dwError = VmDirStringCatA(
                    pszNewSecurityDescriptor,
                    sDestinationBufferSize,
                    pszNewAce);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        if (bVerbose)
        {
            if (bHasChange)
            {
                printf("Old ACE: %s\n", pszTargetAce);
                printf("New ACE: %s\n\n", pszNewAce);
            }
            else
            {
                printf("No ACE change needed\n\n");
            }
        }
    }

    //
    // Now copy the rest of the existing ACE to the end of the new security descriptor.
    //
    dwError = VmDirStringCatA(
                pszNewSecurityDescriptor,
                sDestinationBufferSize,
                pszRemainingSD);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bHasChange)
    {
        *ppszNewSecurityDescriptor = pszNewSecurityDescriptor;
        pszNewSecurityDescriptor = NULL;
    }

cleanup:
    VmDirStringListFree(pLocalStrList);
    VMDIR_SAFE_FREE_MEMORY(pszNewAce);
    VMDIR_SAFE_FREE_STRINGA(pszNewSecurityDescriptor);
    return dwError;

error:
    goto cleanup;
}

/*
 * OUT VAR *ppszNewSD could be NULL if no change needed.
 */
DWORD
_VdcUpdateAclInSD(
    PCSTR pszObjectSD,
    PCSTR pszUserSid,
    PVMDIR_STRING_LIST pPermissionList,
    PVMDIR_STRING_LIST pAceFlagList,
    BOOLEAN fAddPermission,
    BOOLEAN bVerbose,
    PSTR *ppszNewSD
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    DWORD dwUserAce = 0;
    PSTR pszOwnerSid = NULL;
    PSTR pszGroupSid = NULL;
    PSTR pszSid = NULL;
    PVMDIR_STRING_LIST pAceList = NULL;
    BOOLEAN bFoundUser = FALSE;
    PVMDIR_STRING_LIST pTmpList = NULL;
    PSTR pszNewSecurityDescriptor = NULL;

    dwError = _VdcParseSecurityDescriptor(
                pszObjectSD,
                &pszOwnerSid,
                &pszGroupSid,
                &pAceList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < pAceList->dwCount; ++i)
    {
        dwError = _VdcParseAce(pAceList->pStringList[i], &pszSid, &pTmpList);
        BAIL_ON_VMDIR_ERROR(dwError);

        bFoundUser = (strcmp(pszSid, pszUserSid) == 0);
        VMDIR_SAFE_FREE_STRINGA(pszSid);
        VmDirStringListFree(pTmpList);
        pTmpList = NULL;

        if (bFoundUser)
        {
            break;
        }
    }
    dwUserAce = i;

    if (bFoundUser)
    {
        dwError = _VdcUpdateSecurityDescriptor(
                    &pszNewSecurityDescriptor,
                    pszObjectSD,
                    pAceList->pStringList[dwUserAce],
                    pPermissionList,
                    pAceFlagList,
                    fAddPermission,
                    bVerbose);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (fAddPermission)
    {
        dwError = _VdcAddAceToSecurityDescriptor(
                    pszObjectSD,
                    pszUserSid,
                    pPermissionList,
                    pAceFlagList,
                    bVerbose,
                    &pszNewSecurityDescriptor);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {   // delete permission but no such ACE exists
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_ACE_NOT_FOUND);
    }

    *ppszNewSD = pszNewSecurityDescriptor;
    pszNewSecurityDescriptor = NULL;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszOwnerSid);
    VMDIR_SAFE_FREE_STRINGA(pszGroupSid);
    VMDIR_SAFE_FREE_STRINGA(pszSid);
    VMDIR_SAFE_FREE_STRINGA(pszNewSecurityDescriptor);
    VmDirStringListFree(pAceList);
    VmDirStringListFree(pTmpList);

    return dwError;

error:
    goto cleanup;
}

DWORD
VdcGrantPermissionToUser(
    PVDC_ACLMGR_CTX   pCtx,
    PCSTR pszObjectDN
    )
{
    DWORD dwError = 0;
    PSTR pszFilter = NULL;
    PSTR pszObjectSD = NULL;
    PSTR pszNewSecurityDescriptor = NULL;

    dwError = _VdcGetObjectSecurityDescriptor(pCtx->pLd, pszObjectDN, &pszObjectSD);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcUpdateAclInSD(
                pszObjectSD,
                pCtx->pszTargetSID,
                pCtx->pPermissionList,
                pCtx->pAceFlagList,
                TRUE,
                pCtx->paramState.bVerbose,
                &pszNewSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pCtx->paramState.bDryrun &&
        pszNewSecurityDescriptor)    // could be NULL if no change needed
    {
        dwError = VmDirAllocateStringPrintf(&pszFilter, "%s=*", ATTR_OBJECT_CLASS);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VdcLdapReplaceAttrOnEntries(
                    pCtx->pLd,
                    pszObjectDN,
                    LDAP_SCOPE_BASE,
                    pszFilter,
                    ATTR_ACL_STRING,
                    pszNewSecurityDescriptor);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszNewSecurityDescriptor);
    VMDIR_SAFE_FREE_STRINGA(pszFilter);
    return dwError;

error:
    goto cleanup;
}

DWORD
VdcRemovePermissionFromUser(
    PVDC_ACLMGR_CTX   pCtx,
    PCSTR pszObjectDN
    )
{
    DWORD dwError = 0;
    PSTR pszFilter = NULL;
    PSTR pszObjectSD = NULL;
    PSTR pszNewSecurityDescriptor = NULL;

    dwError = _VdcGetObjectSecurityDescriptor(pCtx->pLd, pszObjectDN, &pszObjectSD);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcUpdateAclInSD(
                pszObjectSD,
                pCtx->pszTargetSID,
                pCtx->pPermissionList,
                pCtx->pAceFlagList,
                FALSE,
                pCtx->paramState.bVerbose,
                &pszNewSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pCtx->paramState.bDryrun &&
        pszNewSecurityDescriptor)    // could be NULL if no change needed
    {
        dwError = VmDirAllocateStringPrintf(&pszFilter, "%s=*", ATTR_OBJECT_CLASS);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VdcLdapReplaceAttrOnEntries(
                    pCtx->pLd,
                    pszObjectDN,
                    LDAP_SCOPE_BASE,
                    pszFilter,
                    ATTR_ACL_STRING,
                    pszNewSecurityDescriptor);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszNewSecurityDescriptor);
    return dwError;

error:
    goto cleanup;
}

DWORD
VdcPrintAce(
    PLW_HASHMAP pUserToSidMapping,
    PLW_HASHMAP pPermissionDescriptions,
    PCSTR pszAce
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    PSTR pszSid = NULL;
    PSTR pszUserUPN = NULL;
    PVMDIR_STRING_LIST pPermissionList = NULL;

    dwError = _VdcParseAce(pszAce, &pszSid, &pPermissionList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlHashMapFindKey(pUserToSidMapping, (PVOID*)&pszUserUPN, pszSid);
    if (dwError != ERROR_SUCCESS)
    {
        pszUserUPN = pszSid;
    }

    printf("\tACE for security principal %s:\n", pszUserUPN);
    for (i = 0; i < pPermissionList->dwCount; ++i)
    {
        PSTR pszDescription = NULL;

        dwError = LwRtlHashMapFindKey(pPermissionDescriptions, (PVOID*)&pszDescription, pPermissionList->pStringList[i]);
        if (dwError == ERROR_SUCCESS) {
            printf("\t\tPermission: %s\n", pszDescription);
        }
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszSid);
    VmDirStringListFree(pPermissionList);
    return dwError;

error:
    goto cleanup;
}

DWORD
VdcLoadUsersAndGroups(
    LDAP *pLd,
    PCSTR pszBaseDN,
    PLW_HASHMAP *ppUserToSidMapping,
    PLW_HASHMAP *ppSidToUserMapping
    )
{
    DWORD dwError = 0;
    PLW_HASHMAP pUserToSidMapping = NULL;
    PLW_HASHMAP pSidToUserMapping = NULL;
    PCSTR pszUserFilter = "objectclass=user";
    PCSTR pszGroupFilter = "objectclass=group";

    dwError = LwRtlCreateHashMap(
                &pUserToSidMapping,
                LwRtlHashDigestPstr,
                LwRtlHashEqualPstr,
                NULL
                );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
                &pSidToUserMapping,
                LwRtlHashDigestPstr,
                LwRtlHashEqualPstr,
                NULL
                );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcLdapGetObjectSidMappings(
                pLd,
                pszBaseDN,
                pszUserFilter,
                pUserToSidMapping,
                pSidToUserMapping);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcLdapGetObjectSidMappings(
                pLd,
                pszBaseDN,
                pszGroupFilter,
                pUserToSidMapping,
                pSidToUserMapping);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppUserToSidMapping = pUserToSidMapping;
    *ppSidToUserMapping = pSidToUserMapping;

cleanup:
    return dwError;

error:
    VdcFreeHashMap(&pUserToSidMapping);
    VdcFreeHashMap(&pSidToUserMapping);
    goto cleanup;
}

DWORD
VdcPrintSecurityDescriptorForObject(
    PVDC_ACLMGR_CTX   pCtx,
    PCSTR pszObjectDN
    )
{
    DWORD dwError = 0;
    PSTR pszSecurityDescriptor = NULL;
    PSTR pszOwner = NULL;
    PSTR pszOwnerSid = NULL;
    PSTR pszGroup = NULL;
    PSTR pszGroupSid = NULL;
    PVMDIR_STRING_LIST pAceList = NULL;
    PLW_HASHMAP pPermissionDescriptions = NULL;


    dwError = _VdcGetObjectSecurityDescriptor(pCtx->pLd, pszObjectDN, &pszSecurityDescriptor);
    if (pCtx->paramState.bVerbose)
    {
        DWORD i = 0;

        dwError = _VdcInitializePermissionDescriptions(&pPermissionDescriptions);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VdcParseSecurityDescriptor(
                    pszSecurityDescriptor,
                    &pszOwnerSid,
                    &pszGroupSid,
                    &pAceList);
        BAIL_ON_VMDIR_ERROR(dwError);

        // TBD, pCtx->pUserToSidMapping is not yet provisioned.
        // thus, verbose mode would not print CN properly.
        // should provision pUserToSidMapping on the go.
        dwError = LwRtlHashMapFindKey(pCtx->pSidToUserMapping, (PVOID*)&pszOwner, pszOwnerSid);
        if (dwError != 0)
        {
            pszOwner = pszOwnerSid;
            dwError = 0;
        }

        dwError = LwRtlHashMapFindKey(pCtx->pSidToUserMapping, (PVOID*)&pszGroup, pszGroupSid);
        if (dwError != 0)
        {
            pszGroup = pszGroupSid;
            dwError = 0;
        }

        printf("SD for %s\n", pszObjectDN);
        printf("Owner: %s\n", pszOwner);
        printf("Primary group: %s\n", pszGroup);

        for (i = 0; i < pAceList->dwCount; ++i)
        {
            VdcPrintAce(pCtx->pSidToUserMapping, pPermissionDescriptions, pAceList->pStringList[i]);
        }
    }
    else
    {
        printf("SD for %s is %s\n", pszObjectDN, pszSecurityDescriptor);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszOwnerSid);
    VMDIR_SAFE_FREE_STRINGA(pszGroupSid);
    VmDirStringListFree(pAceList);
    VMDIR_SAFE_FREE_MEMORY(pszSecurityDescriptor);

    VdcFreeHashMap(&pPermissionDescriptions);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VdcAppendNewValue(
    PVMDIR_STRING_LIST  pAceTokenList,
    PVMDIR_STRING_LIST  pNewValueList,
    DWORD               dwNum,
    BOOLEAN*            pbHasChange)
{
    DWORD dwError;
    DWORD dwIdx = 0;
    SIZE_T dwNewSize = 0;
    DWORD dwLen = 0;

    dwNewSize = VmDirStringLenA(pAceTokenList->pStringList[dwNum]) + (pNewValueList->dwCount*2) + 1;

    dwError = VmDirReallocateMemory(
                (PVOID)pAceTokenList->pStringList[dwNum],
                (PVOID*)&(pAceTokenList->pStringList[dwNum]),
                dwNewSize);
    BAIL_ON_VMDIR_ERROR(dwError);
    //
    // Add new values to proper ACE token
    //
    for (dwIdx = 0; dwIdx < pNewValueList->dwCount; dwIdx++)
    {
        BOOLEAN bSameValueFound = FALSE;

        for (dwLen = 0; dwLen < VmDirStringLenA(pAceTokenList->pStringList[dwNum]); dwLen += SDDL_PERMISSION_LENGTH)
        {
            if (VmDirStringNCompareA(pNewValueList->pStringList[dwIdx],
                                     pAceTokenList->pStringList[dwNum]+dwLen,
                                     SDDL_PERMISSION_LENGTH,
                                     TRUE) == 0)
            {
                bSameValueFound = TRUE;
                break;
            }
        }

        if (!bSameValueFound)
        {
            dwError = VmDirStringCatA(
                        (PSTR)pAceTokenList->pStringList[dwNum],
                        dwNewSize,
                        pNewValueList->pStringList[dwIdx]);
            BAIL_ON_VMDIR_ERROR(dwError);

            *pbHasChange = TRUE;
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
VOID
_VdcSkipNewValue(
    PVMDIR_STRING_LIST  pAceTokenList,
    PVMDIR_STRING_LIST  pNewValueList,
    DWORD               dwNum,
    BOOLEAN*            pbHasChange)
{
    DWORD dwIdx = 0;
    PSTR  pszHead = NULL;
    PSTR  pszCurrent = NULL;

    pszHead = pszCurrent = (PSTR)pAceTokenList->pStringList[dwNum];

    while (*pszCurrent != '\0')
    {
        for (dwIdx = 0; dwIdx < pNewValueList->dwCount; dwIdx++)
        {
            if (strncmp(pszCurrent, pNewValueList->pStringList[dwIdx], SDDL_PERMISSION_LENGTH) == 0)
            {
                *pbHasChange = TRUE;
                break;
            }
        }

        if (dwIdx == pNewValueList->dwCount)
        {
            if (pszHead != pszCurrent)
            {
                *(pszHead)   = *(pszCurrent);
                *(pszHead+1) = *(pszCurrent+1);
            }

            pszHead += 2;
        }

        pszCurrent += 2;
    }

    *pszHead = '\0';

    return;
}
