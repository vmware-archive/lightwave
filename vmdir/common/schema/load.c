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
VmDirLdapSchemaLoadStrLists(
    PVDIR_LDAP_SCHEMA   pSchema,
    PVMDIR_STRING_LIST  pAtStrList,
    PVMDIR_STRING_LIST  pOcStrList,
    PVMDIR_STRING_LIST  pCrStrList
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0, j = 0;

    if (!pSchema || !pAtStrList || !pOcStrList || !pCrStrList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i = 0; i < pAtStrList->dwCount; i++)
    {
        PVDIR_LDAP_ATTRIBUTE_TYPE   pAt = NULL;
        PVDIR_LDAP_ATTRIBUTE_TYPE*  pAtList = NULL;

        dwError = VmDirLdapAtParseStr(pAtStrList->pStringList[i], &pAt);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapAtResolveAliases(pAt, &pAtList);
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirFreeLdapAt(pAt);

        for (j = 0; pAtList && pAtList[j]; j++)
        {
            dwError = VmDirLdapSchemaAddAt(pSchema, pAtList[j]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        VMDIR_SAFE_FREE_MEMORY(pAtList);
    }

    for (i = 0; i < pOcStrList->dwCount; i++)
    {
        PVDIR_LDAP_OBJECT_CLASS pOc = NULL;

        dwError = VmDirLdapOcParseStr(pOcStrList->pStringList[i], &pOc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddOc(pSchema, pOc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i = 0; i < pCrStrList->dwCount; i++)
    {
        PVDIR_LDAP_CONTENT_RULE pCr = NULL;

        dwError = VmDirLdapCrParseStr(pCrStrList->pStringList[i], &pCr);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddCr(pSchema, pCr);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapSchemaResolveAndVerifyAll(pSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirLdapSchemaLoadFile(
    PVDIR_LDAP_SCHEMA   pSchema,
    PCSTR               pszSchemaFilePath
    )
{
    DWORD   dwError = 0;
    PVMDIR_STRING_LIST  pAtStrList = NULL;
    PVMDIR_STRING_LIST  pOcStrList = NULL;
    PVMDIR_STRING_LIST  pCrStrList = NULL;

    if (!pSchema || IsNullOrEmptyString(pszSchemaFilePath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirReadSchemaFile(pszSchemaFilePath,
            &pAtStrList, &pOcStrList, &pCrStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaLoadStrLists(
            pSchema, pAtStrList, pOcStrList, pCrStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirStringListFree(pAtStrList);
    VmDirStringListFree(pOcStrList);
    VmDirStringListFree(pCrStrList);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}
