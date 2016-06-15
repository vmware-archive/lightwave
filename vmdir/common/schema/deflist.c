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
VmDirLdapDefListCreate(
    PVDIR_LDAP_DEFINITION_LIST* ppDefList
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_DEFINITION_LIST  pDefList = NULL;

    if (!ppDefList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_LDAP_DEFINITION_LIST),
            (PVOID*)&pDefList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&pDefList->mutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pDefList->pList);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppDefList = pDefList;

cleanup:
    return dwError;

error:
    VmDirFreeLdapDefList(pDefList);
    goto cleanup;
}

static
VOID
_VmDirLdapDefListDecRefCount_inlock(
    PVDIR_LDAP_DEFINITION_LIST  pDefList,
    PVDIR_LDAP_DEFINITION       pDef
    )
{
    if (pDefList && pDef && pDefList == pDef->pList)
    {
        pDef->iRefCount -= 1;
        if (pDef->iRefCount == 0)
        {
            (VOID)VmDirLinkedListRemove(pDefList->pList, pDef->pNode);
            VmDirFreeLdapDef(pDef);
        }
    }
}

DWORD
VmDirLdapDefListUpdateHead(
    PVDIR_LDAP_DEFINITION_LIST  pDefList,
    PVDIR_LDAP_DEFINITION       pDef
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVDIR_LINKED_LIST_NODE  pHead = NULL;
    PVDIR_LDAP_DEFINITION   pPrevDef = NULL;

    if (!pDefList || !pDef)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOCK_MUTEX(bInLock, pDefList->mutex);

    if (pDefList != pDef->pList)
    {
        dwError = VmDirLinkedListGetHead(pDefList->pList, &pHead);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pHead)
        {
            pPrevDef = (PVDIR_LDAP_DEFINITION)pHead->pElement;
            dwError = VmDirLdapDefAreCompat(pPrevDef, pDef);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirLinkedListInsertHead(pDefList->pList, pDef, &pDef->pNode);
        BAIL_ON_VMDIR_ERROR(dwError);

        (VOID)_VmDirLdapDefListDecRefCount_inlock(pDefList, pPrevDef);
        pDef->pList = pDefList;
    }

    pDef->iRefCount += 1;

error:
    VMDIR_UNLOCK_MUTEX(bInLock, pDefList->mutex);
    return dwError;
}

VOID
VmDirLdapDefListRelease(
    PVDIR_LDAP_DEFINITION_LIST  pDefList,
    PVDIR_LDAP_DEFINITION       pDef
    )
{
    BOOLEAN bInLock = FALSE;

    if (pDefList && pDef)
    {
        VMDIR_LOCK_MUTEX(bInLock, pDefList->mutex);

        (VOID)_VmDirLdapDefListDecRefCount_inlock(pDefList, pDef);

        VMDIR_UNLOCK_MUTEX(bInLock, pDefList->mutex);

        if (VmDirLinkedListIsEmpty(pDefList->pList))
        {
            VmDirFreeLdapDefList(pDefList);
        }
    }
}

VOID
VmDirFreeLdapDefList(
    PVDIR_LDAP_DEFINITION_LIST  pDefList
    )
{
    if (pDefList)
    {
        VMDIR_SAFE_FREE_MUTEX(pDefList->mutex);
        VmDirFreeLinkedList(pDefList->pList);
        VMDIR_SAFE_FREE_MEMORY(pDefList);
    }
}
