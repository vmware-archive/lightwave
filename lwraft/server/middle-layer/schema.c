/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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
VmDirPluginSchemaLibUpdatePreModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD dwRtn = 0;
    PVDIR_MODIFICATION pMod = NULL;

    if (pOperation->dwSchemaWriteOp)
    {
        for (pMod = pOperation->request.modifyReq.mods; pMod; pMod = pMod->next)
        {
            // reject the following changes:
            // - objectclass
            // - cn
            PSTR pszType = pMod->attr.type.lberbv.bv_val;
            if (VmDirStringCompareA(pszType, ATTR_OBJECT_CLASS, FALSE) == 0 ||
                VmDirStringCompareA(pszType, ATTR_CN, FALSE) == 0)
            {
                dwRtn = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
                BAIL_ON_VMDIR_ERROR(dwRtn);
            }
        }

        dwRtn = VmDirSchemaLibPrepareUpdateViaModify(pOperation, pEntry);
        BAIL_ON_VMDIR_ERROR(dwRtn);
    }

error:
    return dwPriorResult ? dwPriorResult : dwRtn;
}

DWORD
VmDirPluginSchemaLibUpdatePostModifyCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD   dwRtn = 0;

    if (pOperation->dwSchemaWriteOp)
    {
        dwRtn = VmDirSchemaLibUpdate(dwPriorResult);
    }

    return dwPriorResult ? dwPriorResult : dwRtn;
}

DWORD
VmDirPluginSchemaEntryPreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD   dwRtn = 0;
    PVDIR_ATTRIBUTE pCnAttr = NULL;
    PSTR    pszSchemaIdGuid = NULL;

    if (pOperation->dwSchemaWriteOp)
    {
        // lDAPDisplayName attribute takes cn as default
        if (!VmDirFindAttrByName(pEntry, ATTR_LDAP_DISPLAYNAME))
        {
            pCnAttr = VmDirFindAttrByName(pEntry, ATTR_CN);
            if (!pCnAttr)
            {
                dwRtn = VMDIR_ERROR_INVALID_ENTRY;
                BAIL_ON_VMDIR_ERROR(dwRtn);
            }

            dwRtn = VmDirEntryAddSingleValueStrAttribute(
                    pEntry,
                    ATTR_LDAP_DISPLAYNAME,
                    pCnAttr->vals[0].lberbv.bv_val);
            BAIL_ON_VMDIR_ERROR(dwRtn);
        }

        // schemaIDGUID attribute takes a generated guid as default
        if (!VmDirFindAttrByName(pEntry, ATTR_SCHEMAID_GUID))
        {
            dwRtn = VmDirGenerateGUID(&pszSchemaIdGuid);
            BAIL_ON_VMDIR_ERROR(dwRtn);

            dwRtn = VmDirEntryAddSingleValueStrAttribute(
                    pEntry,
                    ATTR_SCHEMAID_GUID,
                    pszSchemaIdGuid);
            BAIL_ON_VMDIR_ERROR(dwRtn);
        }

        if (VmDirEntryIsObjectclass(pEntry, OC_CLASS_SCHEMA))
        {
            // defaultObjectCategory attribute takes dn as default
            if (!VmDirFindAttrByName(pEntry, ATTR_DEFAULT_OBJECT_CATEGORY))
            {
                dwRtn = VmDirEntryAddSingleValueStrAttribute(
                        pEntry,
                        ATTR_DEFAULT_OBJECT_CATEGORY,
                        pEntry->dn.lberbv.bv_val);
                BAIL_ON_VMDIR_ERROR(dwRtn);
            }
        }
    }

error:
    VMDIR_SAFE_FREE_MEMORY(pszSchemaIdGuid);
    return dwPriorResult ? dwPriorResult : dwRtn;
}

DWORD
VmDirPluginSchemaLibUpdatePreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD   dwRtn = 0;

    if (pOperation->dwSchemaWriteOp)
    {
        dwRtn = VmDirSchemaCheck(pEntry);
        BAIL_ON_VMDIR_ERROR(dwRtn);

        dwRtn = VmDirSchemaLibPrepareUpdateViaModify(pOperation, pEntry);
        BAIL_ON_VMDIR_ERROR(dwRtn);
    }

error:
    return dwPriorResult ? dwPriorResult : dwRtn;
}

DWORD
VmDirPluginSchemaLibUpdatePostAddCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwResult
    )
{
    return VmDirPluginSchemaLibUpdatePostModifyCommit(
            pOperation, pEntry, dwResult);
}
