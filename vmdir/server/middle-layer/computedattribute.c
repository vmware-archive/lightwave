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
 * Filename: computedattribute.c
 *
 * Abstract:
 *
 * computed attribute support
 *
 */

#include "includes.h"

#define VMDIR_COMPUTED_ATTRIBUTE_INITIALIZER \
{                                                                                               \
    {                                                                                           \
    VMDIR_SF_INIT(.pszComputedAttributeName, ATTR_MEMBEROF),                                    \
    VMDIR_SF_INIT(.pfnComputedAttr, _VmDirBuildMemberOfAttribute)                               \
    },                                                                                          \
    {                                                                                           \
    VMDIR_SF_INIT(.pszComputedAttributeName, ATTR_HIGHEST_COMMITTED_USN),                       \
    VMDIR_SF_INIT(.pfnComputedAttr, _VmDirBuildHighestCommittedUSNfAttribute)                   \
    },                                                                                          \
    {                                                                                           \
    VMDIR_SF_INIT(.pszComputedAttributeName, ATTR_ACL_STRING),                                  \
    VMDIR_SF_INIT(.pfnComputedAttr, _VmDirBuildAclStringAttribute)                              \
    },                                                                                          \
    {                                                                                           \
    VMDIR_SF_INIT(.pszComputedAttributeName, ATTR_EID_SEQUENCE_NUMBER),                         \
    VMDIR_SF_INIT(.pfnComputedAttr, _VmDirBuildEIDAttribute)                                    \
    },                                                                                          \
}

static
DWORD
_VmDirBuildHighestCommittedUSNfAttribute(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    PVDIR_ATTRIBUTE*    ppComputedAttr
    );

static
DWORD
_VmDirBuildMemberOfAttribute(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    PVDIR_ATTRIBUTE*	ppComputedAttr
    );

static
DWORD
_VmDirBuildAclStringAttribute(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    PVDIR_ATTRIBUTE*    ppComputedAttr
    );

static
DWORD
_VmDirBuildEIDAttribute(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    PVDIR_ATTRIBUTE*    ppComputedAttr
    );

DWORD
VmDirBuildComputedAttribute(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry
    )
{
    DWORD       dwError = 0;
    SearchReq*  pSearchReq = NULL;
    PVDIR_ATTRIBUTE	pLocalComputedAttr = NULL;
    VDIR_COMPUTED_ATTRIBUTE_INFO computedAttrTbl[] = VMDIR_COMPUTED_ATTRIBUTE_INITIALIZER;
    DWORD       dwTblSize = sizeof(computedAttrTbl)/sizeof(computedAttrTbl[0]);

    if ( !pOperation || !pEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pSearchReq = &(pOperation->request.searchReq);

    if (pSearchReq->attrs != NULL)
    {
        DWORD   dwReqAttrCnt = 0;
        for (dwReqAttrCnt = 0; pSearchReq->attrs[dwReqAttrCnt].lberbv_val != NULL; dwReqAttrCnt++)
        {
            DWORD   dwCnt = 0;
            for (dwCnt = 0; dwCnt < dwTblSize; dwCnt++)
            {
                if (VmDirStringCompareA( pSearchReq->attrs[dwReqAttrCnt].lberbv_val,
                                         computedAttrTbl[dwCnt].pszComputedAttributeName,
                                         FALSE) == 0
                   )
                {
                    VmDirFreeAttribute( pLocalComputedAttr );

                    dwError = computedAttrTbl[dwCnt].pfnComputedAttr(pOperation, pEntry, &pLocalComputedAttr);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    if (pLocalComputedAttr && pLocalComputedAttr->numVals > 0)
                    {
                        PVDIR_ATTRIBUTE pTmpAttr = pEntry->pComputedAttrs;
                        pEntry->pComputedAttrs = pLocalComputedAttr;
                        pEntry->pComputedAttrs->next = pTmpAttr;

                        pLocalComputedAttr = NULL;
                    }

                    break;
                }
            }
        }
    }

cleanup:

    VmDirFreeAttribute( pLocalComputedAttr );

    return dwError;

error:

    goto cleanup;
}

static
DWORD
_VmDirBuildHighestCommittedUSNfAttribute(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    PVDIR_ATTRIBUTE*    ppComputedAttr
    )
{
    DWORD               dwError = 0;
    PVDIR_ATTRIBUTE     pHighestCommittedUSNAttr = NULL;
    USN                 highestCommittedUSN = 0;
    PSTR                pszLocalUSN = NULL;

    if ( !pOperation->pBECtx )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    highestCommittedUSN = pOperation->pBEIF->pfnBEGetHighestCommittedUSN( pOperation->pBECtx );

    dwError = VmDirAttributeAllocate(   ATTR_HIGHEST_COMMITTED_USN,
                                        1,
                                        pEntry->pSchemaCtx,
                                        &pHighestCommittedUSNAttr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(    &pszLocalUSN,
                                            "%ld",
                                            highestCommittedUSN);
    BAIL_ON_VMDIR_ERROR(dwError);

    pHighestCommittedUSNAttr->vals[0].lberbv_val = pszLocalUSN;
    pHighestCommittedUSNAttr->vals[0].lberbv_len = VmDirStringLenA(pszLocalUSN);
    pHighestCommittedUSNAttr->vals[0].bOwnBvVal = TRUE;
    pszLocalUSN = NULL;

    *ppComputedAttr = pHighestCommittedUSNAttr;

cleanup:

    VMDIR_SAFE_FREE_MEMORY( pszLocalUSN );

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirBuildHighestCommittedUSNfAttribute failed (%u)", dwError);
    VmDirFreeAttribute( pHighestCommittedUSNAttr );

    goto cleanup;
}

/* BuildMemberOfAttribute: For the given DN (dn), find out to which groups it belongs (appears in the member attribute),
 * including nested memberships. Return these group DNs as memberOf attribute (memberOfAttr).
 */

static
DWORD
_VmDirBuildMemberOfAttribute(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    PVDIR_ATTRIBUTE*	ppComputedAttr
    )
{
    return VmDirBuildMemberOfAttribute( pOperation, pEntry, ppComputedAttr );
}

static
DWORD
_VmDirBuildAclStringAttribute(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    PVDIR_ATTRIBUTE*    ppComputedAttr
    )
{
    DWORD               dwError = LDAP_SUCCESS;
    PSTR                pszLocalErrorMsg = NULL;
    PVDIR_ATTRIBUTE     pAclStringAttr = NULL;
    PVDIR_ATTRIBUTE     pAttrSD = NULL;
    PSTR                pszLocalAclString = NULL;
    SECURITY_INFORMATION secInfoAll = (OWNER_SECURITY_INFORMATION |
                                       GROUP_SECURITY_INFORMATION |
                                       DACL_SECURITY_INFORMATION |
                                       SACL_SECURITY_INFORMATION);

    pAttrSD = VmDirEntryFindAttribute( ATTR_OBJECT_SECURITY_DESCRIPTOR, pEntry );
    if (!pAttrSD)
    {
        dwError = VMDIR_ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                                      "No ATTR_OBJECT_SECURITY_DESCRIPTOR in entry (%s)", pEntry->dn.lberbv.bv_val );
    }

    dwError = VmDirAttributeAllocate( ATTR_ACL_STRING, 1, pOperation->pSchemaCtx, &pAclStringAttr );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "no memory");

    if ((dwError = LwNtStatusToWin32Error( RtlAllocateSddlCStringFromSecurityDescriptor(
                                                &pszLocalAclString,
                                                (PSECURITY_DESCRIPTOR_RELATIVE) pAttrSD->vals[0].lberbv.bv_val,
                                                SDDL_REVISION_1,
                                                secInfoAll ))) != ERROR_SUCCESS)
    {
        dwError = VMDIR_ERROR_GENERIC;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "RtlAllocateSddlCStringFromSecurityDescriptor failed" );
    }

    pAclStringAttr->vals[0].lberbv_val = pszLocalAclString;
    pAclStringAttr->vals[0].lberbv_len = VmDirStringLenA(pszLocalAclString);
    pAclStringAttr->vals[0].bOwnBvVal = TRUE;
    pszLocalAclString = NULL;

    *ppComputedAttr = pAclStringAttr;

cleanup:
    VMDIR_SAFE_FREE_MEMORY( pszLocalAclString );
    VMDIR_SAFE_FREE_MEMORY( pszLocalErrorMsg );
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirBuildAclStringAttribute failed (%u)(%s)",
                     dwError, VDIR_SAFE_STRING(pszLocalErrorMsg) );
    VmDirFreeAttribute( pAclStringAttr );
    goto cleanup;
}

static
DWORD
_VmDirBuildEIDAttribute(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    PVDIR_ATTRIBUTE*    ppComputedAttr
    )
{
    DWORD               dwError = 0;
    PVDIR_ATTRIBUTE     pEIDAttr = NULL;
    PSTR                pszLocalEID = NULL;

    dwError = VmDirAttributeAllocate( ATTR_EID_SEQUENCE_NUMBER, 1, pOperation->pSchemaCtx, &pEIDAttr );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf( &pszLocalEID,  "%u",  pEntry->eId);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEIDAttr->vals[0].lberbv_val = pszLocalEID;
    pEIDAttr->vals[0].lberbv_len = VmDirStringLenA(pszLocalEID);
    pEIDAttr->vals[0].bOwnBvVal = TRUE;
    pszLocalEID = NULL;

    *ppComputedAttr = pEIDAttr;

cleanup:

    VMDIR_SAFE_FREE_MEMORY( pszLocalEID );

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirBuildEIDAttribute failed (%u)", dwError);

    VmDirFreeAttribute( pEIDAttr );

    goto cleanup;
}
