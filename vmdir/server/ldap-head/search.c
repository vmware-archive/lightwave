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
DWORD
_VmDirLogSearchAttributes(
    PVDIR_OPERATION pOperation,
    ber_len_t   iNumAttr
    )
{
    DWORD   dwError = 0;
    PSTR    pszAttributes = NULL;
    size_t  currLen = 0;
    size_t  msgSize = 0;
    int     iCnt = 0;
    SearchReq *pSReq = &(pOperation->request.searchReq);

    if (iNumAttr > 0)
    {
        for (iCnt = 0, msgSize = 0; iCnt<iNumAttr; iCnt++)
        {
            msgSize += pSReq->attrs[iCnt].lberbv.bv_len + 2 /* for a ',' and ' ' */;
        }

        dwError = VmDirAllocateMemory(msgSize + 1, (PVOID *)&pszAttributes);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (iCnt = 0, currLen = 0; iCnt<iNumAttr; iCnt++)
        {
            VmDirStringNPrintFA(
                    pszAttributes + currLen,
                    msgSize + 1 - currLen,
                    msgSize - currLen,
                    "%s, ",
                    pSReq->attrs[iCnt].lberbv.bv_val);
            BAIL_ON_VMDIR_ERROR(dwError);
            currLen += pSReq->attrs[iCnt].lberbv.bv_len + 2;
        }
        pszAttributes[currLen - 2] = '\0';

        dwError = VmDirAllocateStringA(
                VDIR_SAFE_STRING(pszAttributes),
                &pOperation->conn->SuperLogRec.opInfo.searchInfo.pszAttributes);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (VmDirLogGetMask() & LDAP_DEBUG_ARGS)
        {
            VMDIR_LOG_VERBOSE(LDAP_DEBUG_ARGS, "    Required attributes: %s", pszAttributes);
        }
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszAttributes);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
            "_VmDirLogSearchAttributes: dwError: %lu, msgSize: %lu, iNumAttr: %lu",
            dwError, msgSize, iNumAttr);
    for (iCnt = 0; iCnt<iNumAttr; iCnt++)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "    attr[%d] len: %lu, val: \"%.*s\"",
                iCnt, pSReq->attrs[iCnt].lberbv.bv_len, 256,
                VDIR_SAFE_STRING(pSReq->attrs[iCnt].lberbv.bv_val));
    }
    goto cleanup;
}

static
DWORD
_VmDirLogSearchParameters(
    PVDIR_OPERATION pOperation
    )
{
    DWORD dwError = 0;
    PVDIR_CONNECTION pConn = pOperation->conn;
    SearchReq sr = pOperation->request.searchReq;
    VDIR_BERVALUE strFilter = VDIR_BERVALUE_INIT;
    static PCSTR pcszScopeStr[] = { "BASE", "ONE", "SUB" };

    dwError = FilterToStrFilter(sr.filter, &strFilter);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            VDIR_SAFE_STRING(strFilter.lberbv.bv_val),
            &pConn->SuperLogRec.pszOperationParameters);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            VDIR_SAFE_STRING(pOperation->reqDn.lberbv.bv_val),
            &pConn->SuperLogRec.opInfo.searchInfo.pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            VDIR_SAFE_STRING(pOperation->pszFilters),
            &pConn->SuperLogRec.opInfo.searchInfo.pszIndexResults);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (sr.scope < 0 || sr.scope > 2)
    {
        VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "_VmDirLogSearchParameters: Unknown search scope (%d)", sr.scope );
        dwError = ERROR_INVALID_PARAMETER;
    }
    else
    {
        dwError = VmDirAllocateStringA(
                pcszScopeStr[sr.scope],
                &pConn->SuperLogRec.opInfo.searchInfo.pszScope);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    pConn->SuperLogRec.opInfo.searchInfo.dwReturned = pOperation->dwSentEntries;
    if (pOperation->request.searchReq.filter->candidates)
    {
        pConn->SuperLogRec.opInfo.searchInfo.dwScanned = sr.filter->candidates->size;
    }

    if (VmDirLogGetLevel() >= VMDIR_LOG_VERBOSE && VmDirLogGetMask() & LDAP_DEBUG_ARGS)
    {
        VMDIR_LOG_VERBOSE(LDAP_DEBUG_ARGS, "    Filter: %s", pConn->SuperLogRec.pszOperationParameters);
    }

cleanup:
    VmDirFreeBervalContent(&strFilter);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirSearchPreCondition(
    PVDIR_OPERATION     pOperation,
    PVDIR_LDAP_RESULT   pResult
    )
{
    DWORD   dwError = 0;
    PSTR    pszLocalErrorMsg = NULL;

    // Is sync request operable?
    if (pOperation->syncReqCtrl)
    {
        if (VmDirdState() != VMDIRD_STATE_NORMAL &&
            VmDirdState() != VMDIRD_STATE_READ_ONLY)
        {
            // Why block out-bound replication when catching up during restore mode?
            //
            // Reason: Partners have high-water-mark (lastLocalUsn) corresponding to this replica that is being
            // restored. If out-bound replication is not blocked while restore/catching-up is going on, originating
            // or replicated updates (if this replica is the only partner) made between the current-local-usn and
            // high-water-marks, that partners remember, will not get replicated out (even if the invocationId
            // has been fixed/changed).

            dwError = pResult->errCode = LDAP_UNWILLING_TO_PERFORM;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrorMsg,
                    "Server in not in normal mode, not allowing outward replication.");
        }

        // Sync request must acquire replication (read) lock
        VMDIR_RWLOCK_READLOCK(pOperation->conn->bInReplLock, gVmdirGlobals.replRWLock, 1000);
        if (!pOperation->conn->bInReplLock)
        {
            dwError = pResult->errCode = LDAP_BUSY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return dwError;

error:
    if (pszLocalErrorMsg)
    {
        VMDIR_APPEND_ERROR_MSG(pResult->pszErrMsg, pszLocalErrorMsg);
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: %s",
                __FUNCTION__, pszLocalErrorMsg);
    }
    goto cleanup;
}

/* PerformSearch: Parse the search request on the wire, and call middle-layer Search functionality.
 *
 * From RFC 4511:
 *    SearchRequest ::= [APPLICATION 3] SEQUENCE {
             baseObject      LDAPDN,
             scope           ENUMERATED {
                  baseObject              (0),
                  singleLevel             (1),
                  wholeSubtree            (2),
                  ...  },
             derefAliases    ENUMERATED {
                  neverDerefAliases       (0),
                  derefInSearching        (1),
                  derefFindingBaseObj     (2),
                  derefAlways             (3) },
             sizeLimit       INTEGER (0 ..  maxInt),
             timeLimit       INTEGER (0 ..  maxInt),
             typesOnly       BOOLEAN,
             filter          Filter,
             attributes      AttributeSelection }

        AttributeSelection ::= SEQUENCE OF selector LDAPString
                        -- The LDAPString is constrained to
                        -- <attributeSelector> in Section 4.5.1.8
 *
 */

int
VmDirPerformSearch(
    PVDIR_OPERATION   pOperation
    )
{
   ber_len_t     size = 0;
   SearchReq *   sr = &(pOperation->request.searchReq);
   int           retVal = LDAP_SUCCESS;
   BerValue*           pLberBerv = NULL;
   PSTR                pszLocalErrorMsg = NULL;
   PVDIR_LDAP_RESULT   pResult = &(pOperation->ldapResult);
   BOOLEAN             bSetAccessInfo = FALSE;

    if (pOperation->conn->AccessInfo.pszBindedObjectSid == NULL)
    {
        retVal = VmDirMLSetupAnonymousAccessInfo(&pOperation->conn->AccessInfo);
        BAIL_ON_VMDIR_ERROR(retVal);

        bSetAccessInfo = TRUE;
    }

   // Parse base object, scope, deref alias, sizeLimit, timeLimit and typesOnly search parameters.
   if ( ber_scanf( pOperation->ber, "{miiiib", &(pOperation->reqDn.lberbv), &sr->scope, &sr->derefAlias, &sr->sizeLimit,
                   &sr->timeLimit, &sr->attrsOnly ) == LBER_ERROR )
   {
      VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "PerformSearch: Decoding baseDN, ... attrsOnly error." );
      pResult->errCode = LDAP_PROTOCOL_ERROR;
      retVal = LDAP_NOTICE_OF_DISCONNECT;
      BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                      "Decoding error while parsing baseDN, ... attrsOnly.");
   }

   VMDIR_LOG_VERBOSE( LDAP_DEBUG_ARGS, "Search Request: base: \"%s\", scope: %d, deref: %d, sizeLimit: %d, timeLimit: %d,"
             "attrsOnly: %d", pOperation->reqDn.lberbv.bv_val, sr->scope, sr->derefAlias, sr->sizeLimit, sr->timeLimit,
             sr->attrsOnly);

   if (sr->scope != LDAP_SCOPE_BASE && sr->scope != LDAP_SCOPE_ONELEVEL &&
       sr->scope != LDAP_SCOPE_SUBTREE)
   {
       pResult->errCode = retVal = LDAP_PROTOCOL_ERROR;
       BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg), "Invalid scope" );
   }

   if (sr->sizeLimit < 0 || sr->sizeLimit > LDAP_MAXINT)
   {
       pResult->errCode = retVal = LDAP_PROTOCOL_ERROR;
       BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg), "Invalid size limit: %d", sr->sizeLimit  );
   }

   if (sr->timeLimit < 0 || sr->timeLimit > LDAP_MAXINT)
   {
       pResult->errCode = retVal = LDAP_PROTOCOL_ERROR;
       BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg), "Invalid time limit: %d", sr->timeLimit );
   }

   if (sr->derefAlias != LDAP_DEREF_NEVER && sr->derefAlias != LDAP_DEREF_SEARCHING &&
       sr->derefAlias != LDAP_DEREF_FINDING && sr->derefAlias != LDAP_DEREF_ALWAYS)
   {
      pResult->errCode = retVal = LDAP_PROTOCOL_ERROR;
      BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg), "Invalid dereference alias parameter");
   }

   // Parse filter
   retVal = ParseFilter( pOperation, &sr->filter, pResult );
   BAIL_ON_VMDIR_ERROR(retVal);

   // Parse attributes. 'M' => attribute names point within (in-place) the ber.
   size = sizeof( BerValue ); // Size of the structure is passed-in, and number of attributes are returned back in
                              // the same parameter.
   if ( ber_scanf( pOperation->ber, "{M}}", &pLberBerv, &size, 0 ) == LBER_ERROR )
   {
      pResult->errCode = LDAP_PROTOCOL_ERROR;
      retVal = LDAP_NOTICE_OF_DISCONNECT;
      BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg), "Decoding error while parsing required attributes.");
   }

   // copy pLberBerv content into sr->attrs
   if (pLberBerv && size > 0)
   {
       int iCnt = 0;

       if (VmDirAllocateMemory(sizeof(VDIR_BERVALUE) * (size+1), (PVOID*)&sr->attrs) != 0)
       {
           pResult->errCode = retVal = LDAP_OPERATIONS_ERROR;
           BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg), "no memory");
       }

       for (iCnt = 0; iCnt < size; iCnt++)
       {
           sr->attrs[iCnt].lberbv.bv_val = pLberBerv[iCnt].bv_val;
           sr->attrs[iCnt].lberbv.bv_len = pLberBerv[iCnt].bv_len;
       }
   }

   // Log list of the required attributes
   retVal = _VmDirLogSearchAttributes(pOperation, size);
   BAIL_ON_VMDIR_ERROR(retVal);

   // Parse LDAP controls present (if any) in the request.
   retVal = ParseRequestControls(pOperation, pResult);  // ldapResult.errCode set inside
   BAIL_ON_VMDIR_ERROR( retVal );

   // Check all pre-conditions before processing operation
   retVal = _VmDirSearchPreCondition(pOperation, pResult);
   BAIL_ON_VMDIR_ERROR(retVal);

   retVal = pResult->errCode = VmDirMLSearch(pOperation);
   BAIL_ON_VMDIR_ERROR(retVal);

   retVal = _VmDirLogSearchParameters(pOperation);
   BAIL_ON_VMDIR_ERROR(retVal);

cleanup:
    if (retVal != LDAP_NOTICE_OF_DISCONNECT)
    {
        VmDirSendLdapResult( pOperation );
    }
    if (bSetAccessInfo)
    {
        VmDirFreeAccessInfo(&pOperation->conn->AccessInfo);
    }

    VMDIR_SAFE_FREE_MEMORY(pLberBerv);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return retVal;

error:
    VMDIR_APPEND_ERROR_MSG(pResult->pszErrMsg, pszLocalErrorMsg);
    goto cleanup;
}

void
VmDirFreeSearchRequest(
   SearchReq * sr,
   BOOLEAN     freeSelf
   )
{
    if (sr)
    {
        if (sr->attrs)
        {
            int i = 0;
            for (i = 0; sr->attrs[i].lberbv.bv_val; i++)
            {
                VmDirFreeBervalContent(&sr->attrs[i]);
            }
            VMDIR_SAFE_FREE_MEMORY(sr->attrs);
        }

        DeleteFilter(sr->filter);

        if (freeSelf)
        {
            VMDIR_SAFE_FREE_MEMORY(sr);
        }
    }

    return;
}
