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
VmDirFixLegacySchemaDefSyntaxErr(
    PSTR    pszDef,
    PSTR*   ppszFixedDef
    )
{
    static PCSTR    ppcszDefFixes[] =
    {
        "( VMWare.LKUP.attribute.27 NAME vmwLKUPLegacyIds DESC 'VMware Lookup Service - service identifier' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{256} )",
        "( VMWare.LKUP.attribute.27 NAME 'vmwLKUPLegacyIds' DESC 'VMware Lookup Service - service identifier' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{256} )",
        "( VMWare.STS.objectclass.25 NAME 'vmwExternalIdpUser' DESC 'VMWare external idp user' AUXILIARY MUST ( vmwSTSEntityId vmwSTSExternalIdpUserId ) )",
        "( VMWare.STS.objectclass.25 NAME 'vmwExternalIdpUser' DESC 'VMWare external idp user' AUXILIARY MUST ( vmwSTSEntityId $ vmwSTSExternalIdpUserId ) )",
        NULL
    };

    DWORD   dwError = 0;
    DWORD   i = 0;
    PSTR    pszFixedDef = NULL;

    for (i = 0; ppcszDefFixes[i]; i += 2)
    {
        if (VmDirStringCompareA(pszDef, ppcszDefFixes[i], FALSE) == 0)
        {
            dwError = VmDirAllocateStringA(ppcszDefFixes[i+1], &pszFixedDef);
            BAIL_ON_VMDIR_ERROR(dwError);
            break;
        }
    }

    if (!pszFixedDef)
    {
        dwError = VmDirAllocateStringA(pszDef, &pszFixedDef);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszFixedDef = pszFixedDef;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszFixedDef);
    goto cleanup;
}

BOOLEAN
VmDirIsMultiNameAttribute(
    PSTR    pszName
    )
{
    static PCSTR    ppcszMultiNameAttrs[] =
    {
        "emailAddress",
        "email",
        "pkcs9email",
        "aliasedEntryName",
        "aliasedObjectName",
        NULL
    };

    DWORD   i = 0;

    assert(pszName);
    for (i = 0; ppcszMultiNameAttrs[i]; i++)
    {
        if (VmDirStringCompareA(pszName, ppcszMultiNameAttrs[i], FALSE) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}
