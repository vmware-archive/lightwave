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

static
BOOLEAN
_VmDirIsSearchForServerStatus(
    PVDIR_OPERATION     pOp
    );

static
BOOLEAN
_VmDirIsSearchForRaftState(
    PVDIR_OPERATION     pOp
    );

/*
 * Return TRUE if search request require special handling.
 * If TRUE, the request will be served within this function.
 *
 * In case of error, pLdapResult->errCode/pszErrMsg will be set.
 */
BOOLEAN
VmDirHandleSpecialSearch(
    PVDIR_OPERATION    pOp,
    PVDIR_LDAP_RESULT  pLdapResult
    )
{
    DWORD       dwError = 0;
    BOOLEAN     bRetVal = FALSE;
    BOOLEAN     bHasTxn = FALSE;
    PVDIR_ENTRY pEntry = NULL;
    VDIR_SPECIAL_SEARCH_ENTRY_TYPE entryType = REGULAR_SEARCH_ENTRY_TYPE;
    static PCSTR pszEntryType[] =
    {
            "DSE Root",
            "Schema Entry",
            "Server Status"
    };

    if ( !pOp || !pLdapResult )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirIsSearchForDseRootEntry( pOp ))
    {
        entryType = SPECIAL_SEARCH_ENTRY_TYPE_DSE_ROOT;

        dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = pOp->pBEIF->pfnBESimpleIdToEntry(DSE_ROOT_ENTRY_ID, pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                "%s Entry search failed.", pszEntryType[entryType]);

        dwError = VmDirBuildComputedAttribute( pOp, pEntry );
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                "%s Entry send failed.", pszEntryType[entryType]);
    }
    else if (VmDirIsSearchForSchemaEntry( pOp ))
    {
        entryType = SPECIAL_SEARCH_ENTRY_TYPE_SCHEMA_ENTRY;

        dwError = VmDirSubSchemaSubEntry( &pEntry );
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                "%s Entry search failed.", pszEntryType[entryType]);
    }
    else if (_VmDirIsSearchForServerStatus(pOp))
    {
        entryType = SPECIAL_SEARCH_ENTRY_TYPE_SERVER_STATUS;

        dwError = VmDirServerStatusEntry(&pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                "%s Entry search failed.", pszEntryType[entryType]);
    } else if (_VmDirIsSearchForRaftState(pOp))
    {
        entryType = SPECIAL_SEARCH_ENTRY_TYPE_RAFT_STATUS;

        dwError = VmDirRaftStateEntry(&pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                "%s Entry search failed.", pszEntryType[entryType]);
    }

    if (entryType != REGULAR_SEARCH_ENTRY_TYPE)
    {
        bRetVal = TRUE;

        dwError = pOp->pBEIF->pfnBETxnBegin(pOp->pBECtx, VDIR_BACKEND_TXN_READ);
        BAIL_ON_VMDIR_ERROR(dwError);

        bHasTxn = TRUE;

        dwError = VmDirSendSearchEntry( pOp, pEntry );
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                "%s Entry send failed.", pszEntryType[entryType]);

        if (pOp->request.searchReq.bStoreRsltInMem && pEntry->bSearchEntrySent)
        {
            pOp->internalSearchEntryArray.iSize = 1;
            pOp->internalSearchEntryArray.pEntry = pEntry;
            pEntry = NULL;
        }
    }

cleanup:
    if (bHasTxn)
    {
        pOp->pBEIF->pfnBETxnCommit(pOp->pBECtx);
    }
    VmDirFreeEntry(pEntry);
    return bRetVal;

error:
    VmDirLog( LDAP_DEBUG_ANY, "VmDirHandleSpecialSearch: (%d)(%s)",
            dwError, VDIR_SAFE_STRING(pLdapResult->pszErrMsg) );
    pLdapResult->errCode = dwError;
    goto cleanup;
}

/* From RFC 4512 (section 5.1): An LDAP server SHALL provide information about itself and other information that is
 * specific to each server. This is represented as a group of attributes located in the root DSE, which is named with
 * the DN with zero RDNs. These attributes are retrievable, subject to access control and other restrictions, if a
 * client performs a Search operation with an empty baseObject, scope of baseObject, the filter "(objectClass=*),
 * and the attributes field listing the names of the desired attributes. It is noted that root DSE attributes are
 * operational and, like other operational attributes, are not returned in search requests unless requested by name.
 * The root DSE SHALL NOT be included if the client performs a subtree search starting from the root.
*/

BOOLEAN
VmDirIsSearchForDseRootEntry(
    PVDIR_OPERATION pOp
    )
{
    BOOLEAN     bRetVal = FALSE;
    SearchReq * pSearchReq = NULL;

    pSearchReq = &(pOp->request.searchReq);

    if ( pSearchReq->scope == LDAP_SCOPE_BASE && pOp->reqDn.lberbv.bv_len == 0       &&
         pSearchReq->filter->choice == LDAP_FILTER_PRESENT                           &&
         pSearchReq->filter->filtComp.present.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN &&
         pSearchReq->filter->filtComp.present.lberbv.bv_val != NULL                  &&
         VmDirStringNCompareA( ATTR_OBJECT_CLASS, pSearchReq->filter->filtComp.present.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0
       )
    {
        bRetVal = TRUE;
    }

    return bRetVal;
}

/* From RFC 4512 (section 4.4): To read schema attributes from the subschema (sub)entry, clients MUST issue a Search
 * operation where baseObject is the DN of the subschema (sub)entry, scope is baseObject, filter is
 * "(objectClass=subschema)", and the attributes field lists the names of the desired schema attributes (as they are
 * operational). Note: the "(objectClass=subschema)" filter allows LDAP servers that gateway to X.500 to detect that
 * subentry information is being requested.
*/

BOOLEAN
VmDirIsSearchForSchemaEntry(
    PVDIR_OPERATION  pOp
    )
{
    BOOLEAN     bRetVal = FALSE;
    SearchReq * pSearchReq = NULL;

    pSearchReq = &(pOp->request.searchReq);

    // scope must be base
    if (pSearchReq->scope == LDAP_SCOPE_BASE &&
        VmDirStringCompareA( pOp->reqDn.lberbv.bv_val, SUB_SCHEMA_SUB_ENTRY_DN, FALSE) == 0)
    {
        // filter can be (objectClass=subschema)
        if (pSearchReq->filter->choice == LDAP_FILTER_EQUALITY                              &&
            pSearchReq->filter->filtComp.ava.type.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN    &&
            pSearchReq->filter->filtComp.ava.type.lberbv.bv_val != NULL                     &&
            VmDirStringNCompareA( ATTR_OBJECT_CLASS, pSearchReq->filter->filtComp.ava.type.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0 &&
            pSearchReq->filter->filtComp.ava.value.lberbv.bv_len == OC_SUB_SCHEMA_LEN       &&
            pSearchReq->filter->filtComp.ava.value.lberbv.bv_val != NULL                    &&
            VmDirStringNCompareA( OC_SUB_SCHEMA, pSearchReq->filter->filtComp.ava.value.lberbv.bv_val, OC_SUB_SCHEMA_LEN, FALSE) == 0
            )
        {
            bRetVal = TRUE;
        }
        // filter can be (objectClass=*)
        else if (pSearchReq->filter->choice == LDAP_FILTER_PRESENT                              &&
                 pSearchReq->filter->filtComp.present.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN    &&
                 pSearchReq->filter->filtComp.present.lberbv.bv_val != NULL                     &&
                 VmDirStringNCompareA( ATTR_OBJECT_CLASS, pSearchReq->filter->filtComp.present.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0
                 )
        {
            bRetVal = TRUE;
        }
    }

    return bRetVal;
}

/*
 * For server runtime status
 * The search pattern is :
 * BASE:    cn=serverstatus
 * SCOPE:   BASE
 * FILTER:  (objectclass=*)
 */
static
BOOLEAN
_VmDirIsSearchForServerStatus(
    PVDIR_OPERATION     pOp
    )
{
    BOOLEAN         bRetVal = FALSE;
    SearchReq*      pSearchReq = NULL;
    PVDIR_FILTER    pFilter = NULL;

    pSearchReq = &(pOp->request.searchReq);
    pFilter = pSearchReq->filter;

    assert( pFilter != NULL );

    if (pSearchReq->scope == LDAP_SCOPE_BASE
        &&
        (
         pOp->reqDn.lberbv.bv_val != NULL                                               &&
         VmDirStringCompareA(pOp->reqDn.lberbv.bv_val, SERVER_STATUS_DN, FALSE) == 0
        )
        &&
        (
         pFilter->choice == LDAP_FILTER_PRESENT                                         &&
         pFilter->filtComp.present.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN               &&
         pFilter->filtComp.present.lberbv.bv_val != NULL                                &&
         VmDirStringNCompareA( ATTR_OBJECT_CLASS, pFilter->filtComp.present.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0
        )
        )
    {
        bRetVal = TRUE;
    }

    return bRetVal;

}

/*
 * For raft state in cluster scope
 * The search pattern is :
 * BASE:    cn=raftstate
 * SCOPE:   BASE
 * FILTER:  (objectclass=*)
 */
static
BOOLEAN
_VmDirIsSearchForRaftState(
    PVDIR_OPERATION     pOp
    )
{
    BOOLEAN         bRetVal = FALSE;
    SearchReq*      pSearchReq = NULL;
    PVDIR_FILTER    pFilter = NULL;

    pSearchReq = &(pOp->request.searchReq);
    pFilter = pSearchReq->filter;

    assert( pFilter != NULL );

    if (pSearchReq->scope == LDAP_SCOPE_BASE &&
        (pOp->reqDn.lberbv.bv_val != NULL && VmDirStringCompareA(pOp->reqDn.lberbv.bv_val, RAFT_STATE_DN, FALSE) == 0) &&
        (pFilter->choice == LDAP_FILTER_PRESENT &&
         pFilter->filtComp.present.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN &&
         pFilter->filtComp.present.lberbv.bv_val != NULL &&
         VmDirStringNCompareA( ATTR_OBJECT_CLASS, pFilter->filtComp.present.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0))
    {
        bRetVal = TRUE;
    }

    return bRetVal;
}
