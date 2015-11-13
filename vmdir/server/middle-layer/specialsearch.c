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
_VmDirIsSearchForSchemaEntry(
    PVDIR_OPERATION     pOp
    );

static
BOOLEAN
_VmDirIsSearchForADClassSchema(
    PVDIR_OPERATION     pOp,
    PSTR*               ppszName
    );

static
BOOLEAN
_VmDirIsSearchForADAttributeSchema(
    PVDIR_OPERATION     pOp,
    PSTR*               ppszName
    );

static
BOOLEAN
_VmDirIsSearchForAllADAttributeSchema(
    PVDIR_OPERATION     pOp
    );

static
BOOLEAN
_VmDirIsSearchForServerStatus(
    PVDIR_OPERATION     pOp
    );

static
BOOLEAN
_VmDirIsSearchForReplicationStatus(
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
    BOOLEAN     bRetVal = TRUE;
    VDIR_ENTRY  schemaEntry = {0};
    VDIR_ENTRY  dseRootEntry = {0};
    PVDIR_ENTRY pEntry = NULL;
    PSTR        pszClassSchemaName = NULL;
    PSTR        pszAttributeSchemaName = NULL;

    if ( !pOp || !pLdapResult )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirIsSearchForDseRootEntry( pOp ))
    {
        dwError = pOp->pBEIF->pfnBESimpleIdToEntry(DSE_ROOT_ENTRY_ID, &dseRootEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pLdapResult->pszErrMsg),
                                      "DSE Root Entry search failed.");

        dwError = VmDirBuildComputedAttribute( pOp, &dseRootEntry );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pLdapResult->pszErrMsg),
                                      "DSE Root Entry send failed.");

        dwError = VmDirSendSearchEntry( pOp, &dseRootEntry );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pLdapResult->pszErrMsg),
                                      "DSE Root Entry send failed.");
    }
    else if (_VmDirIsSearchForSchemaEntry( pOp ))
    {
      dwError = pOp->pBEIF->pfnBESimpleIdToEntry(SUB_SCEHMA_SUB_ENTRY_ID, &schemaEntry);
      BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pLdapResult->pszErrMsg),
                                    "Schema Entry search failed.");

      dwError = VmDirSendSearchEntry( pOp, &schemaEntry );
      BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pLdapResult->pszErrMsg),
                                    "Schema Entry send failed.");
    }
    else if (_VmDirIsSearchForADClassSchema( pOp, &pszClassSchemaName ))
    {
        PVDIR_ENTRY pCachedADEntry = NULL;    // Do NOT free, we don't own pADEntry

        dwError = VmDirADCompatibleSearchClassSchema(   pOp->pSchemaCtx,
                                                        pszClassSchemaName,  // Do NOT free
                                                        &pCachedADEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pLdapResult->pszErrMsg),
                                      "Class Schema %s Entry search failed.",
                                      VDIR_SAFE_STRING(pszClassSchemaName));

        dwError = VmDirSendSearchEntry( pOp, pCachedADEntry );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pLdapResult->pszErrMsg),
                                      "Class Schema %s Entry send failed.",
                                      VDIR_SAFE_STRING(pszClassSchemaName));
    }
    else if (_VmDirIsSearchForADAttributeSchema( pOp, &pszAttributeSchemaName ))
    {
        PVDIR_ENTRY pCachedADEntry = NULL;    // Do NOT free, we don't own pADEntry

        dwError = VmDirADCompatibleSearchAttributeSchema(   pOp->pSchemaCtx,
                                                            pszAttributeSchemaName, // Do NOT free
                                                            &pCachedADEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pLdapResult->pszErrMsg),
                                       "Attribute Schema %s Entry search failed.",
                                       VDIR_SAFE_STRING(pszAttributeSchemaName));

        dwError = VmDirSendSearchEntry( pOp, pCachedADEntry );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pLdapResult->pszErrMsg),
                                      "Attribute Schema %s Entry send failed.",
                                      VDIR_SAFE_STRING(pszAttributeSchemaName));
    }
    else if (_VmDirIsSearchForAllADAttributeSchema(pOp))
    {
        dwError = VmDirADCompatibleSendAllAttributeSchema( pOp );  // find and send all attributeSchema entries back
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pLdapResult->pszErrMsg),
                                      "All Attribute Schema Entry send failed.");
    }
    else if (_VmDirIsSearchForServerStatus(pOp))
    {
        dwError = VmDirServerStatusEntry(&pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pLdapResult->pszErrMsg),
                                      "Server Status Entry search failed.");

        dwError = VmDirSendSearchEntry( pOp, pEntry );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pLdapResult->pszErrMsg),
                                      "Server Status Entry send failed.");
    }
    else if (_VmDirIsSearchForReplicationStatus(pOp))
    {
        dwError = VmDirReplicationStatusEntry(&pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pLdapResult->pszErrMsg),
                                      "Replication Status Entry search failed.");

        dwError = VmDirSendSearchEntry( pOp, pEntry );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pLdapResult->pszErrMsg),
                                      "Replication Status Entry send failed.");
    }
    else
    {
       bRetVal = FALSE;
    }

cleanup:

    if (pEntry)
    {
        VmDirFreeEntry(pEntry);
    }

    if (dseRootEntry.dn.lberbv_val)
    {
        VmDirFreeEntryContent( &dseRootEntry );
    }

    if (schemaEntry.dn.lberbv_val)
    {
        VmDirFreeEntryContent( &schemaEntry );
    }

    return bRetVal;

error:

    VmDirLog( LDAP_DEBUG_ANY, "VmDirHandleSpecialSearch: (%d)(%s)", dwError, VDIR_SAFE_STRING(pLdapResult->pszErrMsg) );
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

static
BOOLEAN
_VmDirIsSearchForSchemaEntry(
    PVDIR_OPERATION  pOp
    )
{
    BOOLEAN     bRetVal = FALSE;
    SearchReq * pSearchReq = NULL;

    pSearchReq = &(pOp->request.searchReq);

    if ( pSearchReq->scope == LDAP_SCOPE_BASE && VmDirStringCompareA( pOp->reqDn.lberbv.bv_val, SUB_SCHEMA_SUB_ENTRY_DN, FALSE) == 0 &&
         pSearchReq->filter->choice == LDAP_FILTER_EQUALITY                             &&
         pSearchReq->filter->filtComp.ava.type.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN   &&
         pSearchReq->filter->filtComp.ava.type.lberbv.bv_val != NULL                    &&
         VmDirStringNCompareA( ATTR_OBJECT_CLASS, pSearchReq->filter->filtComp.ava.type.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0 &&
         pSearchReq->filter->filtComp.ava.value.lberbv.bv_len == OC_SUB_SCHEMA_LEN      &&
         pSearchReq->filter->filtComp.ava.value.lberbv.bv_val != NULL                   &&
         VmDirStringNCompareA( OC_SUB_SCHEMA, pSearchReq->filter->filtComp.ava.value.lberbv.bv_val, OC_SUB_SCHEMA_LEN, FALSE) == 0
       )
    {
        bRetVal = TRUE;
    }

    return bRetVal;
}

/*
 * For ADSI compatibility - trap ADSI ClassSchema query.
 * The search pattern is :
 * BASE:    SCHEMA_NAMING_CONTEXT (in vmdird case, cn=schemacontext)
 * SCOPE:   ONE
 * FILTER:  (&(lDAPDisplayName=VALUE_OF_OBJECTCLASS)(!(isdefunct=TRUE))
 */
static
BOOLEAN
_VmDirIsSearchForADClassSchema(
    PVDIR_OPERATION     pOp,
    PSTR*               ppszName        // caller does not own returned *ppszName
    )
{
    BOOLEAN         bRetVal = FALSE;
    SearchReq*      pSearchReq = NULL;

    pSearchReq = &(pOp->request.searchReq);

    if (pSearchReq->scope == LDAP_SCOPE_ONE                                                         &&
        pOp->reqDn.lberbv.bv_val != NULL                                                            &&
        VmDirStringCompareA(pOp->reqDn.lberbv.bv_val, SCHEMA_NAMING_CONTEXT_DN, FALSE) == 0         &&
        // (&  beginning AND Fiilter
        pSearchReq->filter->choice ==  LDAP_FILTER_AND
       )
    {
        PVDIR_FILTER pFilter1 = pSearchReq->filter->filtComp.complex;
        if (// (lDAPDisplayName=XXX)
            pFilter1 != NULL                                                                            &&
            pFilter1->choice == LDAP_FILTER_EQUALITY                                                    &&
            pFilter1->filtComp.ava.type.lberbv.bv_val != NULL                                           &&
            VmDirStringCompareA(pFilter1->filtComp.ava.type.lberbv.bv_val, "lDAPDisplayName", FALSE) == 0
           )
        {
            PVDIR_FILTER pFilter2 = pFilter1->next;
            if (// (!(isDefunct=TRUE))
                pFilter2 != NULL                                                                            &&
                pFilter2->choice == LDAP_FILTER_NOT                                                         &&
                pFilter2->filtComp.complex != NULL                                                          &&
                pFilter2->filtComp.complex->choice == LDAP_FILTER_EQUALITY                                  &&
                pFilter2->filtComp.complex->filtComp.ava.type.lberbv.bv_val != NULL                         &&
                VmDirStringCompareA(pFilter2->filtComp.complex->filtComp.ava.type.lberbv.bv_val, "isDefunct", FALSE) == 0   &&
                pFilter2->filtComp.complex->filtComp.ava.value.lberbv.bv_val != NULL                        &&
                VmDirStringCompareA(pFilter2->filtComp.complex->filtComp.ava.value.lberbv.bv_val, "TRUE", FALSE) == 0
               )
            {
                *ppszName = pFilter1->filtComp.ava.value.lberbv.bv_val;
                bRetVal = TRUE;
            }
        }
    }

    return bRetVal;
}

/*
 * For ADSI compatibility - trap ADSI AttributeSchema query.
 * The search pattern is :
 * BASE:    SCHEMA_NAMING_CONTEXT (in vmdird case, cn=schemacontext)
 * SCOPE:   ONE
 * FILTER:  (&(objectclass=attributeschema)(lDAPDisplayName=VALUE_OF_ATTRIBUTETYPE)
 */
static
BOOLEAN
_VmDirIsSearchForADAttributeSchema(
    PVDIR_OPERATION     pOp,
    PSTR*               ppszName        // caller does not own returned *ppszName
    )
{
    BOOLEAN         bRetVal = FALSE;
    SearchReq*      pSearchReq = &(pOp->request.searchReq);

    if (pSearchReq->scope == LDAP_SCOPE_ONE                                                             &&
        pOp->reqDn.lberbv.bv_val != NULL                                                                &&
        VmDirStringCompareA(pOp->reqDn.lberbv.bv_val, SCHEMA_NAMING_CONTEXT_DN, FALSE) == 0             &&
        // (&  beginning AND Fiilter
        pSearchReq->filter->choice ==  LDAP_FILTER_AND
       )
    {
        PVDIR_FILTER pFilter1 = pSearchReq->filter->filtComp.complex;
        if (// (objectclass=attributeSchema)
            pFilter1 != NULL                                                                                &&
            pFilter1->choice == LDAP_FILTER_EQUALITY                                                        &&
            pFilter1->filtComp.ava.type.lberbv.bv_val != NULL                                               &&
            VmDirStringCompareA(pFilter1->filtComp.ava.type.lberbv.bv_val, "objectclass", FALSE) == 0       &&
            pFilter1->filtComp.ava.value.lberbv.bv_val != NULL                                              &&
            VmDirStringCompareA(pFilter1->filtComp.ava.value.lberbv.bv_val, "attributeSchema", FALSE) == 0
           )
        {
            PVDIR_FILTER pFilter2 = pFilter1->next;
            if (// (lDAPDisplayName=XXX)
                pFilter2 != NULL                                                                                &&
                pFilter2->choice == LDAP_FILTER_EQUALITY                                                        &&
                pFilter2->filtComp.ava.type.lberbv.bv_val != NULL                                               &&
                VmDirStringCompareA(pFilter2->filtComp.ava.type.lberbv.bv_val, "lDAPDisplayName", FALSE) == 0   &&
                pFilter2->filtComp.ava.value.lberbv.bv_val != NULL
               )
            {
                *ppszName = pFilter2->filtComp.ava.value.lberbv.bv_val;
                bRetVal = TRUE;
            }
        }
    }

    return bRetVal;
}

/*
 * For ADSI compatibility - trap ADSI AttributeSchema query.
 * The search pattern is :
 * BASE:    SCHEMA_NAMING_CONTEXT (in vmdird case, cn=schemacontext)
 * SCOPE:   ONE
 * FILTER:  (&(objectclass=attributeschema)(!(isdefunct=TRUE))
 */
static
BOOLEAN
_VmDirIsSearchForAllADAttributeSchema(
    PVDIR_OPERATION     pOp
    )
{
    BOOLEAN         bRetVal = FALSE;
    SearchReq*      pSearchReq = &(pOp->request.searchReq);

    if (pSearchReq->scope == LDAP_SCOPE_ONE                                                             &&
        pOp->reqDn.lberbv.bv_val != NULL                                                                &&
        VmDirStringCompareA(pOp->reqDn.lberbv.bv_val, SCHEMA_NAMING_CONTEXT_DN, FALSE) == 0             &&
        // (&  beginning AND Fiilter
        pSearchReq->filter->choice ==  LDAP_FILTER_AND
       )
    {
        PVDIR_FILTER    pFilter1 = pSearchReq->filter->filtComp.complex;
        if (// (objectclass=attributeschema)
            pFilter1 != NULL                                                                                &&
            pFilter1->choice == LDAP_FILTER_EQUALITY                                                        &&
            pFilter1->filtComp.ava.type.lberbv.bv_val != NULL                                               &&
            VmDirStringCompareA(pFilter1->filtComp.ava.type.lberbv.bv_val, "objectclass", FALSE) == 0       &&
            pFilter1->filtComp.ava.value.lberbv.bv_val != NULL                                              &&
            VmDirStringCompareA(pFilter1->filtComp.ava.value.lberbv.bv_val, "attributeSchema", FALSE) == 0
           )
        {
            PVDIR_FILTER    pFilter2 = pFilter1->next;
            if (// (!(isDefunct=TRUE))
                pFilter2 != NULL                                                                                &&
                pFilter2->choice == LDAP_FILTER_NOT                                                             &&
                pFilter2->filtComp.complex != NULL                                                              &&
                pFilter2->filtComp.complex->choice == LDAP_FILTER_EQUALITY                                      &&
                pFilter2->filtComp.complex->filtComp.ava.type.lberbv.bv_val != NULL                             &&
                VmDirStringCompareA(pFilter2->filtComp.complex->filtComp.ava.type.lberbv.bv_val, "isDefunct", FALSE) == 0     &&
                pFilter2->filtComp.complex->filtComp.ava.value.lberbv.bv_val != NULL                            &&
                VmDirStringCompareA(pFilter2->filtComp.complex->filtComp.ava.value.lberbv.bv_val, "TRUE", FALSE) == 0
               )
            {
                bRetVal = TRUE;
            }
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
 * For replication runtime status
 * The search pattern is :
 * BASE:    cn=replicationstatus
 * SCOPE:   BASE
 * FILTER:  (objectclass=*)
 */
static
BOOLEAN
_VmDirIsSearchForReplicationStatus(
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
