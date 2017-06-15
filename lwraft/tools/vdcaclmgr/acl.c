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
// pszPermissionStatement will be of the form "Username:(PERMISSION)+". E.g.,
// "testuser:RP" or "administrator:RPWP". Note that the username can also be
// a SID.
//
DWORD
_VdcParsePermissionStatement(
    PCSTR pszPermissionStatement,
    PLW_HASHMAP pUserToSidMapping,
    PSTR *ppszUserSid,
    PVMDIR_STRING_LIST *ppPermissionList
    )
{
    DWORD dwError = 0;
    PSTR pszUserSid = NULL;
    PSTR pszUserName = NULL;
    PSTR pszPermission = NULL;
    PSTR pszStringEnd = NULL;
    PVMDIR_STRING_LIST pPermissionList = NULL;

    dwError = VmDirStringListInitialize(&pPermissionList, DEFAULT_PERMISSION_LIST_SIZE);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszStringEnd = strchr(pszPermissionStatement, ':');
    if (pszStringEnd == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringOfLenA(pszPermissionStatement, pszStringEnd - pszPermissionStatement, &pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcLookupUserSid(pUserToSidMapping, pszUserName, &pszUserSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcParsePermissionsString(++pszStringEnd, pPermissionList);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // There should be at least one permission specified.
    //
    if (pPermissionList->dwCount == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    *ppszUserSid = pszUserSid;
    *ppPermissionList = pPermissionList;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUserName);
    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszPermission);
    VMDIR_SAFE_FREE_STRINGA(pszUserSid);
    VmDirStringListFree(pPermissionList);
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
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
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
    PCSTR pszPermission,
    PSTR *ppszNewSecurityDescriptor
    )
{
    DWORD dwError = 0;
    PSTR pszNewSecurityDescriptor = NULL;

    dwError = VmDirAllocateStringPrintf(&pszNewSecurityDescriptor, "%s(A;;%s;;;%s)", pszObjectSD, pszPermission, pszUserSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszNewSecurityDescriptor = pszNewSecurityDescriptor;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
_VdcUpdateSecurityDescriptor(
    PSTR *ppszNewSecurityDescriptor,
    PCSTR pszObjectSD,
    PCSTR pszAce,
    PCSTR pszPermission,
    BOOLEAN fAddPermission
    )
{
    DWORD dwError = 0;
    SIZE_T sDestinationBufferSize = 0;
    PSTR pszNewSecurityDescriptor = NULL;
    PSTR pszAceStart = NULL;
    PSTR pszTokenizer = NULL;

    //
    // +1 for the null.
    //
    sDestinationBufferSize = strlen(pszObjectSD) + strlen(pszPermission) + 1;
    dwError = VmDirAllocateMemory(sDestinationBufferSize, (PVOID*)&pszNewSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszAceStart = strstr(pszObjectSD, pszAce);
    dwError = VmDirStringNCpyA(
                pszNewSecurityDescriptor,
                sDestinationBufferSize,
                pszObjectSD,
                pszAceStart - pszObjectSD);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // Skip ahead to the permssion section of the ACE.
    //
    pszTokenizer = strchr(pszAceStart, ';');
    if (pszTokenizer == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pszTokenizer = strchr(pszTokenizer + 1, ';');
    if (pszTokenizer == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszTokenizer += 1;

    dwError = VmDirStringNCatA(
                pszNewSecurityDescriptor,
                sDestinationBufferSize,
                pszAceStart,
                pszTokenizer - pszAceStart);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (fAddPermission)
    {
        //
        // Add this permission to the ACE.
        //
        dwError = VmDirStringCatA(
                    pszNewSecurityDescriptor,
                    sDestinationBufferSize,
                    pszPermission);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        while (*pszTokenizer != ';')
        {
            if (strncmp(pszTokenizer, pszPermission, SDDL_PERMISSION_LENGTH) == 0)
            {
                pszTokenizer += SDDL_PERMISSION_LENGTH;
                continue;
            }
            else
            {
                dwError = VmDirStringNCatA(
                            pszNewSecurityDescriptor,
                            sDestinationBufferSize,
                            pszTokenizer,
                            SDDL_PERMISSION_LENGTH);
                BAIL_ON_VMDIR_ERROR(dwError);
                pszTokenizer += SDDL_PERMISSION_LENGTH;
            }
        }
    }

    //
    // Now copy the rest of the existing ACE to the end of the new security descriptor.
    //
    dwError = VmDirStringCatA(
                pszNewSecurityDescriptor,
                sDestinationBufferSize,
                pszTokenizer);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszNewSecurityDescriptor = pszNewSecurityDescriptor;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszNewSecurityDescriptor);
    goto cleanup;
}

DWORD
_VdcUpdateAclInSD(
    PCSTR pszObjectSD,
    PCSTR pszUserSid,
    PCSTR pszPermission,
    BOOLEAN fAddPermission,
    PSTR *ppszNewSD
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    DWORD dwUserAce = 0;
    PSTR pszOwnerSid = NULL;
    PSTR pszGroupSid = NULL;
    PVMDIR_STRING_LIST pAceList = NULL;
    BOOLEAN bFoundUser = FALSE;
    PVMDIR_STRING_LIST pPermissionList = NULL;
    PSTR pszNewSecurityDescriptor = NULL;

    dwError = _VdcParseSecurityDescriptor(
                pszObjectSD,
                &pszOwnerSid,
                &pszGroupSid,
                &pAceList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < pAceList->dwCount; ++i)
    {
        PSTR pszSid = NULL;

        dwError = _VdcParseAce(pAceList->pStringList[i], &pszSid, &pPermissionList);
        BAIL_ON_VMDIR_ERROR(dwError);

        bFoundUser = (strcmp(pszSid, pszUserSid) == 0);
        VMDIR_SAFE_FREE_STRINGA(pszSid);

        if (bFoundUser)
        {
            break;
        }
    }
    dwUserAce = i;

    if (bFoundUser)
    {
        dwError = _VdcUpdateSecurityDescriptor(&pszNewSecurityDescriptor, pszObjectSD, pAceList->pStringList[dwUserAce], pszPermission, fAddPermission);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = _VdcAddAceToSecurityDescriptor(pszObjectSD, pszUserSid, pszPermission, &pszNewSecurityDescriptor);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszNewSD = pszNewSecurityDescriptor;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszOwnerSid);
    VMDIR_SAFE_FREE_STRINGA(pszGroupSid);
    VmDirStringListFree(pAceList);
    VmDirStringListFree(pPermissionList);

    return dwError;

error:
    goto cleanup;
}

DWORD
VdcGrantPermissionToUser(
    LDAP *pLd,
    PLW_HASHMAP pUserToSidMapping,
    PCSTR pszObjectDN,
    PCSTR pszPermissionStatement
    )
{
    DWORD dwError = 0;
    PSTR pszFilter = NULL;
    PSTR pszObjectSD = NULL;
    PSTR pszUserSid = NULL;
    PSTR pszNewSecurityDescriptor = NULL;
    PVMDIR_STRING_LIST pPermissionList = NULL;
    DWORD dwIndex = 0;

    dwError = _VdcGetObjectSecurityDescriptor(pLd, pszObjectDN, &pszObjectSD);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcParsePermissionStatement(
                pszPermissionStatement,
                pUserToSidMapping,
                &pszUserSid,
                &pPermissionList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwIndex = 0; dwIndex < pPermissionList->dwCount; ++dwIndex)
    {
        dwError = _VdcUpdateAclInSD(
                    pszObjectSD,
                    pszUserSid,
                    pPermissionList->pStringList[dwIndex],
                    TRUE,
                    &pszNewSecurityDescriptor);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(&pszFilter, "%s=*", ATTR_OBJECT_CLASS);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcLdapReplaceAttrOnEntries(pLd,
                                          pszObjectDN,
                                          LDAP_SCOPE_BASE,
                                          pszFilter,
                                          ATTR_ACL_STRING,
                                          pszNewSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszNewSecurityDescriptor);
    VMDIR_SAFE_FREE_STRINGA(pszFilter);
    VMDIR_SAFE_FREE_STRINGA(pszUserSid);
    VmDirStringListFree(pPermissionList);
    return dwError;

error:
    goto cleanup;
}

DWORD
VdcRemovePermissionFromUser(
    LDAP *pLd,
    PLW_HASHMAP pUserToSidMapping,
    PCSTR pszObjectDN,
    PCSTR pszPermissionStatement
    )
{
    DWORD dwError = 0;
    PSTR pszFilter = NULL;
    PSTR pszObjectSD = NULL;
    PSTR pszUserSid = NULL;
    PSTR pszNewSecurityDescriptor = NULL;
    PVMDIR_STRING_LIST pPermissionList = NULL;
    DWORD dwIndex = 0;

    dwError = _VdcGetObjectSecurityDescriptor(pLd, pszObjectDN, &pszObjectSD);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcParsePermissionStatement(pszPermissionStatement, pUserToSidMapping, &pszUserSid, &pPermissionList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwIndex = 0; dwIndex < pPermissionList->dwCount; ++dwIndex)
    {
        dwError = _VdcUpdateAclInSD(
                    pszObjectSD,
                    pszUserSid,
                    pPermissionList->pStringList[dwIndex],
                    FALSE,
                    &pszNewSecurityDescriptor);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(&pszFilter, "%s=*", ATTR_OBJECT_CLASS);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcLdapReplaceAttrOnEntries(pLd,
                                          pszObjectDN,
                                          LDAP_SCOPE_BASE,
                                          pszFilter,
                                          ATTR_ACL_STRING,
                                          pszNewSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszNewSecurityDescriptor);
    VMDIR_SAFE_FREE_STRINGA(pszUserSid);
    VmDirStringListFree(pPermissionList);
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
    LDAP *pLd,
    PLW_HASHMAP pSidToUserMapping,
    PCSTR pszObjectDN,
    BOOLEAN bVerbose
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

    dwError = _VdcGetObjectSecurityDescriptor(pLd, pszObjectDN, &pszSecurityDescriptor);
    if (bVerbose)
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

        dwError = LwRtlHashMapFindKey(pSidToUserMapping, (PVOID*)&pszOwner, pszOwnerSid);
        if (dwError != 0)
        {
            pszOwner = pszOwnerSid;
            dwError = 0;
        }

        dwError = LwRtlHashMapFindKey(pSidToUserMapping, (PVOID*)&pszGroup, pszGroupSid);
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
            VdcPrintAce(pSidToUserMapping, pPermissionDescriptions, pAceList->pStringList[i]);
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
