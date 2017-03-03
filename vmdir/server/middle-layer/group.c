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



/*
 * Module Name: Directory middle layer
 *
 * Filename: group.c
 *
 * Abstract:
 *
 * group
 *
 */

#include "includes.h"

/*
 * Called during entry add
 *
 *  grouptype :
 *
        http://msdn.microsoft.com/en-us/library/windows/desktop/ms675935%28v=vs.85%29.aspx
        Value   Description
        1 (0x00000001)  Specifies a group that is created by the system.
        2 (0x00000002)  Specifies a group with global scope.
        4 (0x00000004)  Specifies a group with domain local scope.
        8 (0x00000008)  Specifies a group with universal scope.
        16 (0x00000010) Specifies an APP_BASIC group for Windows Server Authorization Manager.
        32 (0x00000020) Specifies an APP_QUERY group for Windows Server Authorization Manager.
        2147483648 (0x80000000) Specifies a security group. If this flag is not set, then the group is a distribution group.

    Currently, Lotus only supports global scope (2).
 */
DWORD
VmDirPluginGroupTypePreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD               dwError = 0;
    PSTR                pszLocalErrorMsg = NULL;

    if ( pOperation->opType != VDIR_OPERATION_TYPE_REPL
         &&
         TRUE == VmDirEntryIsObjectclass(pEntry, OC_GROUP)
       )
    {
        PVDIR_ATTRIBUTE pAttrGroupType = VmDirFindAttrByName(pEntry, ATTR_GROUPTYPE);

        if (pAttrGroupType == NULL)
        {
            dwError = VmDirEntryAddSingleValueStrAttribute(pEntry, ATTR_GROUPTYPE, GROUPTYPE_GLOBAL_SCOPE);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            if ( pAttrGroupType->numVals != 1       // grouptype is a single value attribute
                 ||
                 VmDirStringCompareA( VDIR_SAFE_STRING( pAttrGroupType->vals[0].lberbv.bv_val),
                                       GROUPTYPE_GLOBAL_SCOPE, FALSE) != 0
               )
            {
                dwError = ERROR_DATA_CONSTRAINT_VIOLATION;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg, "invalid or unsupported grouptype (%s)",
                                              VDIR_SAFE_STRING( pAttrGroupType->vals[0].lberbv.bv_val));
            }
        }
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "Group check: (%d)(%s)", dwError, VDIR_SAFE_STRING(pszLocalErrorMsg));

    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszLocalErrorMsg);

    goto cleanup;
}

/*
 * Called during entry modify after we have new entry image (i.e. before commit to db)
 *
 * 1. grouptype: only supports global scope 2
 */
DWORD
VmDirPluginGroupTypePreModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD               dwError = 0;
    PSTR                pszLocalErrorMsg = NULL;

    if ( pOperation->opType != VDIR_OPERATION_TYPE_REPL
         &&
         TRUE == VmDirEntryIsObjectclass(pEntry, OC_GROUP)
       )
    {
        PVDIR_ATTRIBUTE pAttrGroupType = VmDirFindAttrByName(pEntry, ATTR_GROUPTYPE);

        if ( pAttrGroupType == NULL             // grouptype is a must attribute
             ||
             pAttrGroupType->numVals != 1       // gruptype is a single value attribute
             ||
             VmDirStringCompareA( pAttrGroupType->vals[0].lberbv.bv_val , GROUPTYPE_GLOBAL_SCOPE, FALSE) != 0
           )
        {
            dwError = ERROR_DATA_CONSTRAINT_VIOLATION;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg, "invalid or unsupported grouptype (%s)",
                                          VDIR_SAFE_STRING( pAttrGroupType->vals[0].lberbv.bv_val));
        }

    }


cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "Group check: (%d)(%s)", dwError, VDIR_SAFE_STRING(pszLocalErrorMsg));

    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszLocalErrorMsg);

    goto cleanup;
}

DWORD
VmDirPluginGroupMemberPreModApplyDelete(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,    // pEntry is NULL
    DWORD            dwPriorResult
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PVDIR_BERVALUE  pMemberDN = NULL;
    PVDIR_BERVALUE  pGroupDN = NULL;
    VDIR_OPERATION  groupOp = {0};
    VDIR_ENTRY_ARRAY    entryArray = {0};

    pMemberDN = &pOperation->request.deleteReq.dn;

    // look up groups by searching "(member=dn)"
    dwError = VmDirSimpleEqualFilterInternalSearch(
            "",
            LDAP_SCOPE_SUBTREE,
            ATTR_MEMBER,
            pMemberDN->lberbv.bv_val,
            &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    // delete the member from groups
    for (i = 0; i < entryArray.iSize; i++)
    {
        pGroupDN = &entryArray.pEntry[i].dn;

        VmDirFreeOperationContent(&groupOp);
        dwError = VmDirInitStackOperation(&groupOp,
                VDIR_OPERATION_TYPE_INTERNAL,
                LDAP_REQ_MODIFY,
                NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        groupOp.pBEIF = VmDirBackendSelect(NULL);
        groupOp.reqDn.lberbv = pGroupDN->lberbv;
        groupOp.request.modifyReq.dn.lberbv = pGroupDN->lberbv;

        dwError = VmDirAppendAMod(&groupOp,
                MOD_OP_DELETE,
                ATTR_MEMBER,
                ATTR_MEMBER_LEN,
                pMemberDN->lberbv.bv_val,
                pMemberDN->lberbv.bv_len);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirMLModify(&groupOp);
        // Handle possible conflicts gracefully:
        // - The member is already removed from group since search
        // - The group entry is deleted since search
        if (dwError == VMDIR_ERROR_NO_SUCH_ATTRIBUTE ||
            dwError == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND)
        {
            dwError = 0;
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    VmDirFreeOperationContent(&groupOp);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );
    goto cleanup;
}
