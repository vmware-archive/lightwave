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
int
_VmDirSASLInteraction(
    LDAP *      pLD,
    unsigned    flags,
    void *      pDefaults,
    void *      pIn
    );


/*
 * Qsort comparison function for char** data type
 */
int
VmDirQsortPPCHARCmp(
    const void*		ppStr1,
    const void*		ppStr2
    )
{

    if ((ppStr1 == NULL || *(char * const *)ppStr1 == NULL) &&
        (ppStr2 == NULL || *(char * const *)ppStr2 == NULL))
    {
        return 0;
    }

    if (ppStr1 == NULL || *(char * const *)ppStr1 == NULL)
    {
       return -1;
    }

    if (ppStr2 == NULL || *(char * const *)ppStr2 == NULL)
    {
       return 1;
    }

   return VmDirStringCompareA(* (char * const *) ppStr1, * (char * const *) ppStr2, TRUE);
}

int
VmDirQsortPEIDCmp(
    const void * pEID1,
    const void * pEID2)
{
    if (pEID1 == NULL  && pEID2 == NULL)
    {
        return 0;
    }

    if (pEID1 == NULL && pEID2 != NULL)
    {
       return -1;
    }

    if (pEID1 != NULL && pEID2 == NULL)
    {
       return 1;
    }

    if (*((ENTRYID *)pEID1) < *((ENTRYID*)pEID2))
    {
        return -1;
    }
    else if (*((ENTRYID *)pEID1) == *((ENTRYID*)pEID2))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void const *
UtdVectorEntryGetKey(
    PLW_HASHTABLE_NODE  pNode,
    PVOID               pUnused
    )
{
    UptoDateVectorEntry * pUtdVectorEntry = LW_STRUCT_FROM_FIELD(pNode, UptoDateVectorEntry, Node);

    return pUtdVectorEntry->invocationId.lberbv.bv_val;
}

/*
 * VmDirGenOriginatingTimeStr(): Generates the current Universal Time (UTC) in YYYYMMDDHHMMSSmmm format
 * Return value:
 *      0: Indicates that the call succeeded.
 *     -1: Indicates that an error occurred, and errno is set to indicate the error.
 */

#define NSECS_PER_MSEC        1000000

int
VmDirGenOriginatingTimeStr(
    char * timeStr)
{
#ifndef _WIN32
    struct timespec tspec = {0};
    struct tm       tmTime = {0};
    int             retVal = 0;

    retVal = clock_gettime( CLOCK_REALTIME, &tspec );
    BAIL_ON_VMDIR_ERROR(retVal);

    if (gmtime_r(&tspec.tv_sec, &tmTime) == NULL)
    {
        retVal = errno;
        BAIL_ON_VMDIR_ERROR(retVal);
    }
    snprintf( timeStr, VMDIR_ORIG_TIME_STR_LEN, "%4d%02d%02d%02d%02d%02d.%03d",
              tmTime.tm_year+1900,
              tmTime.tm_mon+1,
              tmTime.tm_mday,
              tmTime.tm_hour,
              tmTime.tm_min,
              tmTime.tm_sec,
              (int)(tspec.tv_nsec/NSECS_PER_MSEC));

cleanup:
    return retVal;

error:
    goto cleanup;
#else
    int retVal = 0;
    SYSTEMTIME sysTime = {0};

    GetSystemTime( &sysTime );

    if( _snprintf_s(
        timeStr,
        VMDIR_ORIG_TIME_STR_LEN,
        VMDIR_ORIG_TIME_STR_LEN-1,
        "%4d%02d%02d%02d%02d%02d.%03d",
        sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour,
        sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds
        ) == -1 )
    {
        retVal = -1;
    }

    return retVal;
#endif
}

void
VmDirCurrentGeneralizedTime(
    PSTR    pszTimeBuf,
    int     iBufSize)
{
#ifndef _WIN32
    time_t      tNow = time(NULL);
    struct tm   tmpTm = {0};

    assert (pszTimeBuf);

    gmtime_r(&tNow, &tmpTm);

    snprintf(pszTimeBuf, iBufSize, "%04d%02d%02d%02d%02d%02d.0Z",
            tmpTm.tm_year + 1900,
            tmpTm.tm_mon + 1,
            tmpTm.tm_mday,
            tmpTm.tm_hour,
            tmpTm.tm_min,
            tmpTm.tm_sec);

    return;
#else
    SYSTEMTIME sysTime = {0};

    GetSystemTime( &sysTime );

    _snprintf_s(
        pszTimeBuf,
        iBufSize,
        iBufSize-1,
        "%04d%02d%02d%02d%02d%02d.0Z",
        sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour,
        sysTime.wMinute, sysTime.wSecond
        );

#endif

}

VOID
VmDirForceExit(
    VOID
    )
{
    VmDirLog( LDAP_DEBUG_ANY, "Vmdird force exiting ...");

#ifndef _WIN32
    pid_t       processid = getpid();

    if ( kill( processid, SIGINT ) != 0 )
    {
        VmDirLog( LDAP_DEBUG_ANY, "VmDirForceExit failed (%u)", errno);
    }
#else

    if ( gVmdirGlobals.hStopServiceEvent != 0 )
    {
        if (SetEvent( gVmdirGlobals.hStopServiceEvent ) == 0)
        {
            VmDirLog( LDAP_DEBUG_ANY, "VmDirForceExit failed (%u)", GetLastError());
        }
    }
    else
    {
        VmDirLog( LDAP_DEBUG_ANY, "VmDirForceExit failed, invalid handle");
    }

#endif
}

DWORD
VmDirUuidFromString(
    PCSTR pStr,
    uuid_t* pGuid
)
{
    DWORD dwError = ERROR_SUCCESS;

    if ((pGuid == NULL) || (pStr == NULL) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#if !defined(_WIN32) || defined(HAVE_DCERPC_WIN32)
    uuid_parse( (PSTR)pStr, *pGuid);
#else
    dwError = UuidFromStringA( (RPC_CSTR)pStr, pGuid );
    BAIL_ON_VMDIR_ERROR(dwError);
#endif

error:
    return dwError;
}

VOID
VmDirLogStackFrame(
    int     logLevel
    )
{
#ifndef _WIN32

#define BT_SIZE 30
    int     iNum = 0;
    VOID*   pBuffer[BT_SIZE] = {0};
    PSTR*   ppszStrings = NULL;

    iNum = backtrace(pBuffer, BT_SIZE);
    ppszStrings = backtrace_symbols(pBuffer, iNum);

    if (ppszStrings != NULL)
    {
        int iCnt = 0;

        for (iCnt = 0; iCnt < iNum; iCnt++)
        {
            VmDirLog( logLevel, "StackFrame (%d)(%s)", iCnt, VDIR_SAFE_STRING(ppszStrings[iCnt]) );
        }

        VMDIR_SAFE_FREE_MEMORY(ppszStrings);
    }

    return;

#else

    return;

#endif
}

DWORD
VmDirSrvCreateDN(
    PCSTR pszContainerName,
    PCSTR pszDomainDN,
    PSTR* ppszContainerDN
    )
{
    DWORD dwError = 0;
    PSTR  pszContainerDN = NULL;

    dwError = VmDirAllocateStringPrintf(&pszContainerDN, "cn=%s,%s", pszContainerName, pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszContainerDN = pszContainerDN;

cleanup:
    return dwError;

error:
    *ppszContainerDN = NULL;
    goto cleanup;
}

DWORD
VmDirSrvCreateContainerWithEID(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszContainerDN,
    PCSTR            pszContainerName,
    PVMDIR_SECURITY_DESCRIPTOR pSecDesc, // OPTIONAL
    ENTRYID          eID
    )
{
    DWORD dwError = 0;
    PSTR ppszAttributes[] =
    {
            ATTR_OBJECT_CLASS,  OC_TOP,
            ATTR_OBJECT_CLASS,  OC_CONTAINER,
            ATTR_CN,           (PSTR)pszContainerName,
            NULL
    };

    dwError = VmDirSimpleEntryCreate(pSchemaCtx, ppszAttributes, (PSTR)pszContainerDN, eID);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pSecDesc != NULL)
    {
        dwError = VmDirSetSecurityDescriptorForDn(pszContainerDN, pSecDesc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VmDirLog(LDAP_DEBUG_ANY, "VmDirSrvCreateContainerWithEID failed. Error(%u)", dwError);
    goto cleanup;
}

DWORD
VmDirSrvCreateContainer(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszContainerDN,
    PCSTR            pszContainerName
    )
{
    DWORD dwError = 0;

    dwError = VmDirSrvCreateContainerWithEID(pSchemaCtx, pszContainerDN, pszContainerName, NULL, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

// Create THE domain object, and superior domain component objects.

#define MAX_DOMAIN_COMPONENT_VALUE_LEN  1024 // SJ-TBD: Should use a defined constant

DWORD
VmDirSrvCreateDomain(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    BOOLEAN          bSetupHost,
    PCSTR            pszDomainDN
    )
{
    DWORD   dwError = 0;
    char    pszDomainCompName[MAX_DOMAIN_COMPONENT_VALUE_LEN];
    PSTR    ppszDomainAttrs[] =
    {
        ATTR_OBJECT_CLASS,              OC_TOP,
        ATTR_OBJECT_CLASS,              OC_DC_OBJECT,
        ATTR_OBJECT_CLASS,              OC_DOMAIN,
        ATTR_OBJECT_CLASS,              OC_DOMAIN_DNS,
        ATTR_DOMAIN_COMPONENT,          (PSTR)pszDomainCompName,
        ATTR_DOMAIN_FUNCTIONAL_LEVEL,   VDIR_DOMAIN_FUNCTIONAL_LEVEL,
        NULL
    };
    int     domainDNLen = (int) VmDirStringLenA(pszDomainDN);
    int     i = 0;
    int     startOfRdnInd = 0;
    int     startOfRdnValInd = 0;
    int     endOfRdnValInd = 0;
    int     rdnValLen = 0;

    for (i = endOfRdnValInd = domainDNLen - 1; i >= 0; i-- )
    {
        if (i == 0 || pszDomainDN[i] == RDN_SEPARATOR_CHAR)
        {
            startOfRdnInd = (i == 0) ? 0 : i + 1 /* for , */;
            startOfRdnValInd = startOfRdnInd + ATTR_DOMAIN_COMPONENT_LEN + 1 /* for = */;
            rdnValLen = endOfRdnValInd - startOfRdnValInd + 1;

            dwError = VmDirStringNCpyA( pszDomainCompName, MAX_DOMAIN_COMPONENT_VALUE_LEN,
                                        pszDomainDN + startOfRdnValInd, rdnValLen );
            BAIL_ON_VMDIR_ERROR(dwError);

            pszDomainCompName[rdnValLen] = '\0';

            dwError = VmDirSimpleEntryCreate( pSchemaCtx, ppszDomainAttrs, (PSTR)pszDomainDN + startOfRdnInd, 0 );
            if (dwError == VMDIR_ERROR_BACKEND_ENTRY_EXISTS ||
                dwError == VMDIR_ERROR_ENTRY_ALREADY_EXIST)
            {   // pass through if parent exists
                dwError = VMDIR_SUCCESS;
            }
            BAIL_ON_VMDIR_ERROR(dwError);

            endOfRdnValInd = i - 1;
        }
    }

cleanup:
    return dwError;

error:
    VmDirLog(LDAP_DEBUG_ANY, "VmDirSrvCreateDomain failed. Error(%u)", dwError);
    goto cleanup;
}

DWORD
VmDirFindMemberOfAttribute(
    PVDIR_ENTRY pEntry,
    PVDIR_ATTRIBUTE* ppMemberOfAttr
    )
{
    DWORD               dwError = ERROR_SUCCESS;
    PVDIR_ATTRIBUTE     pMemberOfAttr = NULL;
    VDIR_OPERATION      searchOp = {0};
    BOOLEAN             bHasTxn = FALSE;

    if ( pEntry == NULL || ppMemberOfAttr == NULL )
    {
        dwError =  VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirInitStackOperation( &searchOp, VDIR_OPERATION_TYPE_INTERNAL,
LDAP_REQ_SEARCH, NULL );
    BAIL_ON_VMDIR_ERROR(dwError);

    searchOp.pBEIF = VmDirBackendSelect(NULL);

    // start txn
    dwError = searchOp.pBEIF->pfnBETxnBegin( searchOp.pBECtx, VDIR_BACKEND_TXN_READ );
    BAIL_ON_VMDIR_ERROR(dwError);

    bHasTxn = TRUE;

    dwError = VmDirBuildMemberOfAttribute( &searchOp, pEntry, &pMemberOfAttr );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppMemberOfAttr = pMemberOfAttr;

cleanup:
    if (bHasTxn)
    {
        searchOp.pBEIF->pfnBETxnCommit( searchOp.pBECtx);
    }
    VmDirFreeOperationContent(&searchOp);

    return dwError;

error:
    VmDirFreeAttribute(pMemberOfAttr);
    goto cleanup;
}

/* BuildMemberOfAttribute: For the given DN (dn), find out to which groups it belongs (appears in the member attribute),
 * including nested memberships. Return these group DNs as memberOf attribute (memberOfAttr).
 */
#define START_MAX_NUM_MEMBEROF_VALS     20
#define STEP_INC_NUM_MEMBEROF_VALS      10

DWORD
VmDirBuildMemberOfAttribute(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    PVDIR_ATTRIBUTE*    ppComputedAttr
    )
{
    DWORD               dwError = LDAP_SUCCESS;
    USHORT              currMaxNumVals = START_MAX_NUM_MEMBEROF_VALS;
    VDIR_FILTER *       f = NULL;
    VDIR_CANDIDATES *   cl = NULL;
    VDIR_ENTRY          groupEntry = {0};
    VDIR_ENTRY *        pGroupEntry = NULL;
    VDIR_BERVALUE *     currMemberDn = NULL;
    int                 currMemberDnInd = 0;
    int                 i = 0;
    unsigned int        j = 0;
    PSTR                pszLocalErrorMsg = NULL;
    PVDIR_ATTRIBUTE     pMemberofAttr = NULL;

    dwError = VmDirAllocateMemory( sizeof( VDIR_FILTER ), (PVOID *)&f );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "no memory");

    f->choice = LDAP_FILTER_EQUALITY;
    f->filtComp.ava.type.lberbv.bv_val = ATTR_MEMBER;
    f->filtComp.ava.type.lberbv.bv_len = ATTR_MEMBER_LEN;
    if ((f->filtComp.ava.pATDesc = VmDirSchemaAttrNameToDesc( pOperation->pSchemaCtx, ATTR_MEMBER)) == NULL)
    {
        dwError = VmDirSchemaCtxGetErrorCode(pOperation->pSchemaCtx);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "Attribute type member not defined.");
    }

    dwError = VmDirAttributeAllocate( ATTR_MEMBEROF, currMaxNumVals, pOperation->pSchemaCtx, &pMemberofAttr );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "no memory");

    // Start the following 'for' loop with currMemberDn being the passed in memberDn, then in the loop process searched
    // group DNs as the member DNs => recursive group membership computation.
    for (currMemberDn = &(pEntry->dn), currMemberDnInd = -1, pMemberofAttr->numVals = 0;currMemberDn->lberbv.bv_val != NULL;)
    {
        dwError = VmDirNormalizeDN( currMemberDn, pOperation->pSchemaCtx );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "DN normalization failed");

        f->filtComp.ava.value = *currMemberDn;

        dwError = pOperation->pBEIF->pfnBEGetCandidates(pOperation->pBECtx, f, 0);
        if (dwError == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND)
        {   // no candidates found
            dwError = 0;
        }
        if (dwError != 0)
        {
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                                          "Backend GetCandidates failed. (%d)(%s)",
                                          pOperation->pBECtx->dwBEErrorCode,
                                          VDIR_SAFE_STRING(pOperation->pBECtx->pszBEErrorMsg));
        }

        cl = f->candidates;

        for (i = 0; i < cl->size; i++)
        {
            pGroupEntry = &groupEntry;
            // SJ-TBD: We should probably have an EID => DN index. That will avoid reading whole entry just to extract
            // the DN, as done below.
            dwError = pOperation->pBEIF->pfnBEIdToEntry(
                        pOperation->pBECtx,
                        pOperation->pSchemaCtx,
                        cl->eIds[i],
                        pGroupEntry,
                        VDIR_BACKEND_ENTRY_LOCK_READ);
            if (dwError == 0)
            {
                if (pMemberofAttr->numVals == (currMaxNumVals - 1))
                {
                    currMaxNumVals += STEP_INC_NUM_MEMBEROF_VALS;
                    dwError = VmDirReallocateMemoryWithInit( (PVOID)pMemberofAttr->vals, (PVOID *)(&(pMemberofAttr->vals)),
                                                       sizeof(VDIR_BERVALUE) * currMaxNumVals,
                                                       sizeof(VDIR_BERVALUE) * (pMemberofAttr->numVals + 1));
                    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "no memory");
                }

                // Check if the groupDN is self
                if (pGroupEntry->dn.lberbv.bv_len != pEntry->dn.lberbv.bv_len ||
                    VmDirStringCompareA(pGroupEntry->dn.lberbv.bv_val, pEntry->dn.lberbv.bv_val, TRUE) != 0)
                { // No need to normalize DNs in above comparison because we are getting all these DNs from same
                  // source i.e. e->dn

                    // Check if the group DN already included
                    for (j = 0; j < pMemberofAttr->numVals; j++)
                    {
                        if (pGroupEntry->dn.lberbv.bv_len == pMemberofAttr->vals[j].lberbv.bv_len &&
                            VmDirStringCompareA(pGroupEntry->dn.lberbv.bv_val, pMemberofAttr->vals[j].lberbv.bv_val,
                                                TRUE) == 0)
                        { // No need to normalize DNs in above comparison because we are getting all these DNs from same
                          // source i.e. e->dn
                            break;
                        }

                    }
                    if (j == pMemberofAttr->numVals) // No duplicate/match found
                    {
                        dwError = VmDirBervalContentDup( &(pGroupEntry->dn), &(pMemberofAttr->vals[pMemberofAttr->numVals]));
                        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "no memory");

                        pMemberofAttr->numVals++;
                    }
                }

                VmDirFreeEntryContent( pGroupEntry );
                pGroupEntry = NULL; // Reset to NULL so that DeleteEntry is no-op.
            }
            else
            {
                // Ignore BdbEIdToEntry errors. BdbEIdToEntry can fail because the entry may have been deleted
                // since we constructed the candidates list.
            }
        }

        DeleteCandidates( &(f->candidates) );
        currMemberDnInd++;
        currMemberDn = &(pMemberofAttr->vals[currMemberDnInd]);
    }

    if (pMemberofAttr && pMemberofAttr->numVals > 0 && ppComputedAttr)
    {
        *ppComputedAttr = pMemberofAttr;
    }
    else
    {
        VmDirFreeAttribute( pMemberofAttr );
    }

cleanup:

    if (f)
    {
        memset(&(f->filtComp.ava.value), 0, sizeof(VDIR_BERVALUE)); // Since ava.value is NOT owned by filter.
        DeleteFilter( f );
    }
    VmDirFreeEntryContent( pGroupEntry );

    if (pOperation->ldapResult.pszErrMsg == NULL)
    {
        pOperation->ldapResult.pszErrMsg = pszLocalErrorMsg;
    }
    else
    {
        VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    }

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirBuildMemberOfAttribute() failed, error string (%s), error code (%d)",
                     VDIR_SAFE_STRING(pszLocalErrorMsg), dwError );

    VmDirFreeAttribute( pMemberofAttr );

    goto cleanup;
}

DWORD
VmDirSASLGSSBind(
     LDAP*  pLD
     )
{
    DWORD   dwError = 0;

    dwError = ldap_sasl_interactive_bind_s( pLD,
                                            NULL,
                                            "GSSAPI",
                                            NULL,
                                            NULL,
                                            LDAP_SASL_QUIET,
                                            _VmDirSASLInteraction,
                                            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmDirLog(LDAP_DEBUG_ANY, "VmDirSASLGSSBind failed. (%d)(%s)", dwError, ldap_err2string(dwError));

    goto cleanup;
}

static
int
_VmDirSASLInteraction(
    LDAP *      pLD,
    unsigned    flags,
    void *      pDefaults,
    void *      pIn
    )
{
    // dummy function to staisfy ldap_sasl_interactive_bind call
    return LDAP_SUCCESS;
}

/*
 * Given an UPN, return entry DN
 */
DWORD
VmDirUPNToDN(
    PCSTR       pszUPN,
    PSTR*       ppszEntryDN
    )
{
    DWORD               dwError = 0;
    VDIR_ENTRY_ARRAY    entryResultArray = {0};
    PSTR                pszLocalDN = NULL;

    if ( IsNullOrEmptyString(pszUPN) || !ppszEntryDN )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSimpleEqualFilterInternalSearch( "",
                                                    LDAP_SCOPE_SUBTREE,
                                                    ATTR_KRB_UPN,
                                                    pszUPN,
                                                    &entryResultArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( entryResultArray.iSize == 1)
    {
        dwError = VmDirAllocateStringA( entryResultArray.pEntry[0].dn.lberbv.bv_val,
                                        &pszLocalDN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if ( entryResultArray.iSize == 0)
    {
        dwError = VMDIR_ERROR_ENTRY_NOT_FOUND;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszEntryDN = pszLocalDN;
    pszLocalDN = NULL;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalDN);
    VmDirFreeEntryArrayContent(&entryResultArray);

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "UPN (%s) lookup failed (%u)", VDIR_SAFE_STRING(pszUPN), dwError );

    goto cleanup;
}

DWORD
VmDirUPNToDNBerWrap(
    PCSTR           pszUPN,
    PVDIR_BERVALUE  pBervDN
    )
{
    DWORD   dwError = 0;
    PSTR    pszLocalDN = NULL;

    if ( IsNullOrEmptyString(pszUPN) || !pBervDN )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirUPNToDN( pszUPN, &pszLocalDN );
    BAIL_ON_VMDIR_ERROR(dwError);

    pBervDN->lberbv.bv_val = pszLocalDN;
    pBervDN->lberbv.bv_len = VmDirStringLenA( pszLocalDN );
    pBervDN->bOwnBvVal = TRUE;
    pszLocalDN = NULL;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalDN);

    return dwError;

error:
    goto cleanup;
}

/*
 * check if targetDN live under (or equal) to ancestorDN
 */
DWORD
VmDirIsAncestorDN(
    PVDIR_BERVALUE  pBervAncestorDN,
    PVDIR_BERVALUE  pBervTargetDN,
    PBOOLEAN        pbResult
    )
{
    DWORD       dwError = 0;
    BOOLEAN     bRtn = FALSE;

    assert( pBervAncestorDN && pBervTargetDN && pbResult);

    if (pBervAncestorDN->bvnorm_val == NULL || pBervTargetDN->bvnorm_val == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pBervTargetDN->bvnorm_len == pBervAncestorDN->bvnorm_len &&
        VmDirStringCompareA(pBervTargetDN->bvnorm_val, pBervAncestorDN->bvnorm_val, FALSE) == 0
       )
    {
        bRtn = TRUE;
    }

    if (pBervTargetDN->bvnorm_len > pBervAncestorDN->bvnorm_len                                                         &&
        pBervTargetDN->bvnorm_val[pBervTargetDN->bvnorm_len - pBervAncestorDN->bvnorm_len - 1] == RDN_SEPARATOR_CHAR    &&
        VmDirStringCompareA(pBervTargetDN->bvnorm_val + (pBervTargetDN->bvnorm_len - pBervAncestorDN->bvnorm_len),
                            pBervAncestorDN->bvnorm_val,
                            FALSE) == 0
       )
    {
        bRtn = TRUE;
    }

    *pbResult = bRtn;

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirHasSingleAttrValue(
    PVDIR_ATTRIBUTE pAttr
    )
{
    DWORD   dwError = 0;

    if ( pAttr->numVals != 1 || pAttr->vals == NULL || pAttr->vals[0].lberbv_val == NULL )
    {
        dwError = VMDIR_ERROR_BAD_ATTRIBUTE_DATA;
    }

    return dwError;
}

DWORD
VmDirValidatePrincipalName(
    PVDIR_ATTRIBUTE pAttr,
    PSTR*           ppErrMsg
    )
{
    DWORD   dwError = 0;
    PSTR    pszLocalErrMsg = NULL;

    dwError = VmDirHasSingleAttrValue( pAttr );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg, "Invalid %s value", pAttr->type.lberbv_val);

    if ( VmDirStringChrA( pAttr->vals[0].lberbv_val,'@') == NULL )
    {
        dwError = VMDIR_ERROR_BAD_ATTRIBUTE_DATA;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                      "Invalid %s value (%s), char '@' not found.",
                                      pAttr->type.lberbv_val,
                                      pAttr->vals[0].lberbv_val);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return dwError;

error:
    if (ppErrMsg)
    {
        *ppErrMsg = pszLocalErrMsg;
        pszLocalErrMsg = NULL;
    }

    goto cleanup;
}

/*
 * Determine domain functional level
 */
DWORD
VmDirSrvGetDomainFunctionalLevel(
    PDWORD pdwLevel
    )
{
    DWORD               dwError = 0;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    PVDIR_ATTRIBUTE     pAttrDomainLevel = NULL;
    DWORD               dwLevel = 0;

    if (gVmdirServerGlobals.systemDomainDN.bvnorm_val == NULL)
    {
        dwError = ERROR_NOT_JOINED;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirFilterInternalSearch(gVmdirServerGlobals.systemDomainDN.bvnorm_val, LDAP_SCOPE_BASE, "(objectclass=*)", 0, NULL, &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( entryArray.iSize == 1)
    {
        pAttrDomainLevel = VmDirEntryFindAttribute(
                                ATTR_DOMAIN_FUNCTIONAL_LEVEL,
                                &entryArray.pEntry[0]);
        if (!pAttrDomainLevel)
        {
            dwError = VMDIR_ERROR_NO_SUCH_ATTRIBUTE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwLevel = atoi(pAttrDomainLevel->vals[0].lberbv_val);
    }
    else if ( entryArray.iSize == 0)
    {
        dwError = VMDIR_ERROR_ENTRY_NOT_FOUND;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pdwLevel = dwLevel;

cleanup:

    VmDirFreeEntryArrayContent(&entryArray);
    return dwError;

error:
    goto cleanup;
}
