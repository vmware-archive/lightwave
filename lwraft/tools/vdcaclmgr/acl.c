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

//
// pszPermissionStatement will be of the form "Username:PERMISSION". E.g.,
// "testuser:RP".
//
DWORD
_VdcParsePermissionStatement(
    PCSTR pszPermissionStatement,
    PLW_HASHMAP pUserToSidMapping,
    PSTR *ppszUserSid,
    PSTR *ppszPermission
    )
{
    DWORD dwError = 0;
    PSTR pszUserSid = NULL;
    PSTR pszUserName = NULL;
    PSTR pszPermission = NULL;
    PSTR pszStringEnd = NULL;

    pszStringEnd = strchr(pszPermissionStatement, ':');
    if (pszStringEnd == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringOfLenA(pszPermissionStatement, pszStringEnd - pszPermissionStatement, &pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszStringEnd + 1, &pszPermission);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlHashMapFindKey(pUserToSidMapping, (PVOID*)&pszUserSid, pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszUserSid = pszUserSid;
    *ppszPermission = pszPermission;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUserName);
    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszPermission);
    VMDIR_SAFE_FREE_STRINGA(pszUserSid);
    goto cleanup;
}
DWORD
_VdcParsePermissionsString(
    PCSTR pszPermissions,
    PSTRING_LIST pStringList
    )
{
    DWORD dwError = 0;

    //
    // All permissions are two characters long so the length of the entire string should
    // be even.
    //
    if (strlen(pszPermissions) % 2 != 0)
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
        dwError = VmDirAllocateStringOfLenA(pszPermissions, 2, &pszPermission);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszPermissions += 2;

        dwError = VdcStringListAdd(pStringList, pszPermission);
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
    PSTRING_LIST *ppPermissionList
    )
{
    DWORD dwError = 0;
    PSTR pszStringEnd = NULL;
    PSTR pszPermissions = NULL;
    PSTR pszSid = NULL;
    PSTRING_LIST pPermissionList = NULL;

    assert(pszAce[0] == '(' && pszAce[strlen(pszAce) - 1] == ')');

    dwError = VdcStringListInitialize(&pPermissionList, DEFAULT_PERMISSION_LIST_SIZE);
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
    VdcStringListFree(pPermissionList);
    goto cleanup;
}

DWORD
_VdcParseSecurityDescriptor(
    PCSTR pszSecurityDescriptor,
    PSTR *ppszOwner,
    PSTR *ppszGroup,
    PSTRING_LIST *ppAceList
    )
{
    DWORD dwError = 0;
    PSTR pszOwner = NULL;
    PSTR pszGroup = NULL;
    PSTRING_LIST pAceList = NULL;
    PSTR pszStringEnd = NULL;

    dwError = VdcStringListInitialize(&pAceList, DEFAULT_ACE_LIST_SIZE);
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

    while (*pszSecurityDescriptor)
    {
        PSTR pszAce = NULL;

        if (*pszSecurityDescriptor != '(')
        {
            dwError = VMDIR_ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pszStringEnd = strchr(pszSecurityDescriptor, ')');
        if (pszStringEnd == NULL)
        {
            dwError = VMDIR_ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        //
        // + 1 to grab the trailing ')'. We do this more for consistency than anything.
        //
        dwError = VmDirAllocateStringOfLenA(pszSecurityDescriptor, pszStringEnd - pszSecurityDescriptor + 1, &pszAce);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VdcStringListAdd(pAceList, pszAce);
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
    VdcStringListFree(pAceList);
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
    PSTR pszFilter = NULL;

    dwError = VmDirAllocateStringAVsnprintf(&pszFilter, "%s=*", ATTR_OBJECT_CLASS);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcLdapGetAttributeValue(pLd,
                                       pszBaseDN,
                                       LDAP_SCOPE_BASE,
                                       pszFilter,
                                       ATTR_ACL_STRING,
                                       &pszSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszSecurityDescriptor = pszSecurityDescriptor;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszFilter);
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

    dwError = VmDirAllocateStringAVsnprintf(&pszNewSecurityDescriptor, "%s(A;;%s;;;%s)", pszObjectSD, pszPermission, pszUserSid);
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
    DWORD dwDestinationBufferSize = 0;
    PSTR pszNewSecurityDescriptor = NULL;
    PSTR pszAceStart = NULL;
    PSTR pszTokenizer = NULL;

    //
    // +1 for the null.
    //
    dwDestinationBufferSize = strlen(pszObjectSD) + strlen(pszPermission) + 1;
    dwError = VmDirAllocateMemory(dwDestinationBufferSize, (PVOID*)&pszNewSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszAceStart = strstr(pszObjectSD, pszAce);
    dwError = VmDirStringNCpyA(
                pszNewSecurityDescriptor,
                dwDestinationBufferSize,
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
                dwDestinationBufferSize,
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
                    dwDestinationBufferSize,
                    pszPermission);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        PSTR pszPermissionInString = pszTokenizer;

        while (*pszPermissionInString != ';')
        {
            if (strncmp(pszPermissionInString, pszPermission, strlen(pszPermission)) == 0)
            {
                break;
            }
            else
            {
                pszPermissionInString += 2;
            }
        }

        if (*pszPermissionInString != ';')
        {
            //
            // We found the permission in question. Copy everything before it to the
            // new SD.
            //
            dwError = VmDirStringNCatA(
                        pszNewSecurityDescriptor,
                        dwDestinationBufferSize,
                        pszTokenizer,
                        pszPermissionInString - pszTokenizer);
            BAIL_ON_VMDIR_ERROR(dwError);

            //
            // Skip over the permission to "remove" it. The rest of the ACE will get
            // copied below.
            //
            pszTokenizer = pszPermissionInString + 2;
        }
        else
        {
            //
            // This user doesn't have the permission in question. Let's report an
            // error.
            //
            printf("Error: User doesn't have the %s permission\n", pszPermission);
            dwError = VMDIR_ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    //
    // Now copy the rest of the existing ACE to the end of the new security descriptor.
    //
    dwError = VmDirStringCatA(
                pszNewSecurityDescriptor,
                dwDestinationBufferSize,
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
    PSTRING_LIST pAceList = NULL;
    BOOLEAN bFoundUser = FALSE;
    PSTRING_LIST pPermissionList = NULL;
    PSTR pszNewSecurityDescriptor;

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
        //
        // If the user already has this permission then bail out.
        //
        if (fAddPermission && VdcStringListContains(pPermissionList, pszPermission))
        {
            printf("The user (%s) already has the %s permission\n", pszUserSid, pszPermission);
            dwError = VMDIR_ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

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
    VdcStringListFree(pAceList);
    VdcStringListFree(pPermissionList);

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
    PSTR pszPermission = NULL;
    PSTR pszNewSecurityDescriptor = NULL;

    dwError = _VdcGetObjectSecurityDescriptor(pLd, pszObjectDN, &pszObjectSD);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("Previous SD for %s ==> %s\n", pszObjectDN, pszObjectSD);

    dwError = _VdcParsePermissionStatement(pszPermissionStatement, pUserToSidMapping, &pszUserSid, &pszPermission);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcUpdateAclInSD(pszObjectSD, pszUserSid, pszPermission, TRUE, &pszNewSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("Updated SD ==> %s\n", pszNewSecurityDescriptor);

    dwError = VmDirAllocateStringAVsnprintf(&pszFilter, "%s=*", ATTR_OBJECT_CLASS);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcLdapReplaceAttrOnEntries(pLd,
                                          pszObjectDN,
                                          LDAP_SCOPE_SUB,
                                          pszFilter,
                                          ATTR_ACL_STRING,
                                          pszNewSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszNewSecurityDescriptor);
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
    PSTR pszPermission = NULL;
    PSTR pszNewSecurityDescriptor = NULL;

    dwError = _VdcGetObjectSecurityDescriptor(pLd, pszObjectDN, &pszObjectSD);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("Previous SD for %s ==> %s\n", pszObjectDN, pszObjectSD);

    dwError = _VdcParsePermissionStatement(pszPermissionStatement, pUserToSidMapping, &pszUserSid, &pszPermission);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcUpdateAclInSD(pszObjectSD, pszUserSid, pszPermission, FALSE, &pszNewSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("Updated SD ==> %s\n", pszNewSecurityDescriptor);

    dwError = VmDirAllocateStringAVsnprintf(&pszFilter, "%s=*", ATTR_OBJECT_CLASS);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcLdapReplaceAttrOnEntries(pLd,
                                          pszObjectDN,
                                          LDAP_SCOPE_SUB,
                                          pszFilter,
                                          ATTR_ACL_STRING,
                                          pszNewSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

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
    PSTR pszAce
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    PSTR pszSid = NULL;
    PSTR pszUserUPN = NULL;
    PSTRING_LIST pPermissionList = NULL;

    dwError = _VdcParseAce(pszAce, &pszSid, &pPermissionList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlHashMapFindKey(pUserToSidMapping, (PVOID*)&pszUserUPN, pszSid);
    if (dwError != ERROR_SUCCESS)
    {
        pszUserUPN = pszSid;
    }

    printf("\tACE for user/group %s:\n", pszUserUPN);
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
    VdcStringListFree(pPermissionList);
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
    PSTR pszUserFilter = "objectclass=user";
    PSTR pszGroupFilter = "objectclass=group";

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

    dwError = VdcLdapGetObjectList(pLd, pszBaseDN, LDAP_SCOPE_SUBTREE, pszUserFilter, pUserToSidMapping, pSidToUserMapping);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcLdapGetObjectList(pLd, pszBaseDN, LDAP_SCOPE_SUBTREE, pszGroupFilter, pUserToSidMapping, pSidToUserMapping);
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
    PSTRING_LIST pAceList = NULL;
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
        }

        dwError = LwRtlHashMapFindKey(pSidToUserMapping, (PVOID*)&pszGroup, pszGroupSid);
        if (dwError != 0)
        {
            pszGroup = pszGroupSid;
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
    VdcStringListFree(pAceList);
    VMDIR_SAFE_FREE_MEMORY(pszSecurityDescriptor);

    VdcFreeHashMap(&pPermissionDescriptions);

    return dwError;

error:
    goto cleanup;
}
