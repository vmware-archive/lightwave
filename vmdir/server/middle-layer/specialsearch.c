/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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
    DWORD   dwError = 0;
    size_t  i = 0;
    BOOLEAN bHasTxn = FALSE;
    BOOLEAN bRefresh = FALSE;
    PVDIR_ENTRY_ARRAY   pEntryArray = NULL;
    VDIR_SPECIAL_SEARCH_ENTRY_TYPE entryType = REGULAR_SEARCH_ENTRY_TYPE;
    VMDIR_INTEGRITY_CHECK_JOB_STATE integrityCheckStat = INTEGRITY_CHECK_JOB_NONE;

    static PCSTR pszEntryType[] =
    {
            "DSE Root",
            "Schema Entry",
            "Server Status",
            "Replication Status",
            "Schema Repl Status",
            "Integrity Check Status"
    };

    if ( !pOp || !pLdapResult )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntryArray = &pOp->internalSearchEntryArray;

    if (VmDirIsSearchForDseRootEntry( pOp ))
    {
        entryType = SPECIAL_SEARCH_ENTRY_TYPE_DSE_ROOT;
        pEntryArray->iSize = 1;

        dwError = VmDirAllocateMemory(
                sizeof(VDIR_ENTRY), (PVOID*)&pEntryArray->pEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = pOp->pBEIF->pfnBESimpleIdToEntry(
                DSE_ROOT_ENTRY_ID, pEntryArray->pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                "%s Entry search failed.", pszEntryType[entryType]);

        dwError = VmDirBuildComputedAttribute(pOp, pEntryArray->pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                "%s Entry send failed.", pszEntryType[entryType]);
    }
    else if (VmDirIsSearchForSchemaEntry( pOp ))
    {
        entryType = SPECIAL_SEARCH_ENTRY_TYPE_SCHEMA_ENTRY;
        pEntryArray->iSize = 1;

        dwError = VmDirSubSchemaSubEntry(&pEntryArray->pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                "%s Entry search failed.", pszEntryType[entryType]);
    }
    else if (VmDirIsSearchForServerStatus(pOp))
    {
        entryType = SPECIAL_SEARCH_ENTRY_TYPE_SERVER_STATUS;
        pEntryArray->iSize = 1;

        dwError = VmDirServerStatusEntry(&pEntryArray->pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                "%s Entry search failed.", pszEntryType[entryType]);
    }
    else if (VmDirIsSearchForReplicationStatus(pOp))
    {
        entryType = SPECIAL_SEARCH_ENTRY_TYPE_REPL_STATUS;
        pEntryArray->iSize = 1;

        dwError = VmDirReplicationStatusEntry(&pEntryArray->pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                "%s Entry search failed.", pszEntryType[entryType]);
    }
    else if (VmDirIsSearchForSchemaReplStatus(pOp, &bRefresh))
    {
        entryType = SPECIAL_SEARCH_ENTRY_TYPE_SCHEMA_REPL_STATUS;

        if (bRefresh)
        {
            dwError = VmDirSchemaReplStatusEntriesRefresh();
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                    "%s Entry refresh failed.", pszEntryType[entryType]);
        }
        else
        {
            dwError = VmDirSchemaReplStatusEntriesRetrieve(
                    pEntryArray, pOp->request.searchReq.scope);
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                    "%s Entry search failed.", pszEntryType[entryType]);
        }
    }
    else if (VmDirIsSearchForIntegrityCheckStatus(pOp, &integrityCheckStat))
    {
        BOOLEAN bIsMember = FALSE;

        entryType = SPECIAL_SEARCH_ENTRY_TYPE_INTEGRITY_CHECK_STATUS;

        dwError = VmDirIsBindDnMemberOfSystemDomainAdmins(NULL, &pOp->conn->AccessInfo, &bIsMember);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!bIsMember)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INSUFFICIENT_ACCESS);
        }

        if (integrityCheckStat == INTEGRITY_CHECK_JOB_START ||
            integrityCheckStat == INTEGRITY_CHECK_JOB_RECHECK)
        {
            dwError = VmDirIntegrityCheckStart(integrityCheckStat);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (integrityCheckStat == INTEGRITY_CHECK_JOB_STOP)
        {
            VmDirIntegrityCheckStop();
        }
        else if (integrityCheckStat == INTEGRITY_CHECK_JOB_SHOW_SUMMARY)
        {
            dwError = VmDirIntegrityCheckShowStatus(&pEntryArray->pEntry);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (pEntryArray->pEntry)
            {
                pEntryArray->iSize = 1;
            }
        }
    }

    if (entryType != REGULAR_SEARCH_ENTRY_TYPE)
    {

        /*
         * Read txn for preventing server crash (PR 1634501)
         */
        dwError = pOp->pBEIF->pfnBETxnBegin(pOp->pBECtx, VDIR_BACKEND_TXN_READ);
        BAIL_ON_VMDIR_ERROR(dwError);

        bHasTxn = TRUE;

        for (i = 0; i < pEntryArray->iSize; i++)
        {
            dwError = VmDirSendSearchEntry(pOp, &pEntryArray->pEntry[i]);
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                    "%s Entry send failed.", pszEntryType[entryType]);
        }
    }

cleanup:
    if (bHasTxn)
    {
        pOp->pBEIF->pfnBETxnCommit(pOp->pBECtx);
    }

    return entryType != REGULAR_SEARCH_ENTRY_TYPE;

error:
    VmDirLog( LDAP_DEBUG_ANY, "VmDirHandleSpecialSearch: (%d)(%s)",
            dwError, VDIR_SAFE_STRING(pLdapResult->pszErrMsg) );
    pLdapResult->vmdirErrCode = dwError;

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
BOOLEAN
VmDirIsSearchForServerStatus(
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
 * For replication runtime status
 * The search pattern is :
 * BASE:    cn=replicationstatus
 * SCOPE:   BASE
 * FILTER:  (objectclass=*)
 */
BOOLEAN
VmDirIsSearchForReplicationStatus(
    PVDIR_OPERATION     pOp
    )
{
    BOOLEAN         bRetVal = FALSE;
    SearchReq*      pSearchReq = &(pOp->request.searchReq);
    PVDIR_FILTER    pFilter = pSearchReq ? pSearchReq->filter: NULL;

    if (pSearchReq != NULL
        &&
        pFilter != NULL
        &&
        pSearchReq->scope == LDAP_SCOPE_BASE
        &&
        (
         pOp->reqDn.lberbv.bv_val != NULL                                               &&
         VmDirStringCompareA(pOp->reqDn.lberbv.bv_val, REPLICATION_STATUS_DN, FALSE) == 0
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
 * For schema replication status
 * The search pattern is:
 * BASE:    cn=schemareplstatus
 * FILTER:  (objectclass=*)
 * SCOPE:   BASE - returns the pseudo entry that tells if refresh is in progress
 *          ONELEVEL - returns the status entries
 *          SUBTREE - returns the pseudo entry + status entries
 *
 * Optional:
 * ATTRS:   refresh - trigger server to refresh schema replication status
 */
BOOLEAN
VmDirIsSearchForSchemaReplStatus(
    PVDIR_OPERATION     pOp,
    PBOOLEAN            pbRefresh
    )
{
    BOOLEAN         bRetVal = FALSE;
    SearchReq*      pSearchReq = &(pOp->request.searchReq);
    PSTR            pszDN = pOp->reqDn.lberbv.bv_val;
    PVDIR_FILTER    pFilter = pSearchReq ? pSearchReq->filter : NULL;

    if (pSearchReq != NULL                                                  &&
        pszDN != NULL                                                       &&
        VmDirStringCompareA(pszDN, SCHEMA_REPL_STATUS_DN, FALSE) == 0       &&
        pFilter != NULL                                                     &&
        pFilter->choice == LDAP_FILTER_PRESENT                              &&
        pFilter->filtComp.present.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN    &&
        pFilter->filtComp.present.lberbv.bv_val != NULL                     &&
        VmDirStringNCompareA(ATTR_OBJECT_CLASS, pFilter->filtComp.present.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0)
    {
        bRetVal = TRUE;
    }

    if (pbRefresh && pSearchReq->attrs)
    {
        PSTR pszAttr = pSearchReq->attrs[0].lberbv.bv_val;
        if (VmDirStringCompareA(pszAttr, "refresh", FALSE) == 0)
        {
            *pbRefresh = TRUE;
        }
    }

    return bRetVal;
}

/*
 * For integrity check status
 * The search pattern is:
 * BASE:    cn=integritycheckstatus
 * FILTER:  (objectclass=*)
 * SCOPE:   BASE - with optional attribute operation=start|stop|recheck
 *          ONELEVEL - returns job summary with optional attribute detail
 *
 */
BOOLEAN
VmDirIsSearchForIntegrityCheckStatus(
    PVDIR_OPERATION                     pOp,
    PVMDIR_INTEGRITY_CHECK_JOB_STATE    pState
    )
{
    BOOLEAN         bRetVal = FALSE;
    SearchReq*      pSearchReq = &(pOp->request.searchReq);
    PSTR            pszDN = pOp->reqDn.lberbv.bv_val;
    PVDIR_FILTER    pFilter = pSearchReq ? pSearchReq->filter : NULL;

    if (pSearchReq != NULL                                                  &&
        pszDN != NULL                                                       &&
        VmDirStringCompareA(pszDN, INTEGRITY_CHECK_STATUS_DN, FALSE) == 0   &&
        pFilter != NULL                                                     &&
        pFilter->choice == LDAP_FILTER_PRESENT                              &&
        pFilter->filtComp.present.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN    &&
        pFilter->filtComp.present.lberbv.bv_val != NULL                     &&
        VmDirStringNCompareA(ATTR_OBJECT_CLASS, pFilter->filtComp.present.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0)
    {
        bRetVal = TRUE;
    }

    if (pSearchReq->scope == LDAP_SCOPE_BASE && pState && pSearchReq->attrs)
    {
        PSTR pszAttr = pSearchReq->attrs[0].lberbv.bv_val;
        if (VmDirStringCompareA(pszAttr, "start", FALSE) == 0)
        {
            *pState = INTEGRITY_CHECK_JOB_START;
        }
        else if (VmDirStringCompareA(pszAttr, "stop", FALSE) == 0)
        {
            *pState = INTEGRITY_CHECK_JOB_STOP;
        }
        else if (VmDirStringCompareA(pszAttr, "recheck", FALSE) == 0)
        {
            *pState = INTEGRITY_CHECK_JOB_RECHECK;
        }
        else
        {
            *pState = INTEGRITY_CHECK_JOB_NONE;
        }
    }
    else if (pSearchReq->scope == LDAP_SCOPE_ONELEVEL && pState )
    {
        *pState = INTEGRITY_CHECK_JOB_SHOW_SUMMARY;
    }

    return bRetVal;
}
