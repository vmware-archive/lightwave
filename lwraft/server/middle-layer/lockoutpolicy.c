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
 * Filename: lockoutpolicy.c
 *
 * Abstract:
 *
 * lockout policy enforcement
 *
 */

#include "includes.h"

static
DWORD
LockoutCacheRecGet(
    PCSTR               pszNormDN,
    PVDIR_LOCKOUT_REC*  ppLockoutRec
    );

static
DWORD
LockoutCacheRecSet(
    PCSTR               pszNormDN,
    PVDIR_LOCKOUT_REC   pLockoutRec
    );

static
VOID
LockoutRecFree(
    PVDIR_LOCKOUT_REC   pLockoutRec
    );

static
LW_PCVOID
LockoutRecGetKey(
    PLW_HASHTABLE_NODE  pNode,
    PVOID               pUnused
    );

static
DWORD
LockoutPolicyCheck(
    PVDIR_LOCKOUT_REC   pLockoutRec,
    PVDIR_ENTRY         pEntry,
    PBOOLEAN            bBoolLockout
    );

static
DWORD
LockoutRecInit(
    PCSTR                       pszNormDN,
    PVDIR_LOCKOUT_REC           pLockoutRec,
    PVDIR_PASSWD_LOCKOUT_POLICY pPolicy
    );

static
VOID
LockoutPolicyLoadFromEntry(
    PVDIR_ENTRY                 pPolicyEntry,
    PVDIR_PASSWD_LOCKOUT_POLICY pPolicy
    );

static
BOOLEAN
_VmDirExemptUserFromLockoutPolicy(
    ENTRYID             entryID,
    PCSTR               pszNormDN,
    PVDIR_ACCESS_INFO   pAccessInfo
    );

VOID
VdirLockoutCacheRemoveRec(
    PCSTR               pszNormDN
    )
{
    DWORD                   dwError = 0;
    BOOLEAN                 bInLock = FALSE;
    PCSTR                   pszErrorContext = NULL;
    PLW_HASHTABLE_NODE      pNode = NULL;
    PVDIR_LOCKOUT_REC       pLockoutRec = NULL;

    assert(pszNormDN);

    VMDIR_LOCK_MUTEX(bInLock, gVdirLockoutCache.mutex);

    if (gVdirLockoutCache.pHashTbl)
    {
        pszErrorContext = " lockout cache find key.";
        dwError = LwRtlHashTableFindKey(
                        gVdirLockoutCache.pHashTbl,
                        &pNode,
                        (PVOID)pszNormDN);
        dwError = LwNtStatusToWin32Error(dwError);
        if (dwError == ERROR_NOT_FOUND)
        {
            dwError = 0;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pNode)
        {

            dwError = LwRtlHashTableRemove(gVdirLockoutCache.pHashTbl, pNode);
            assert(dwError == 0);

            pLockoutRec = (PVDIR_LOCKOUT_REC)LW_STRUCT_FROM_FIELD(pNode, VDIR_LOCKOUT_REC, Node);

        }
     }

 cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirLockoutCache.mutex);

    if (pLockoutRec)
    {   // BUGBUG, potential race condition to free this?
        // Safer to have a clean up thread to free this on a regular bases.
        LockoutRecFree(pLockoutRec);
    }

     return;

 error:

     VmDirLog(LDAP_DEBUG_ANY,
             "VdirLockoutCacheRemoveRec (%s)",
             VDIR_SAFE_STRING(pszErrorContext));

     goto cleanup;
}

/*
 * Called AFTER password is verified in bind.c
 *
 * We block login if -
 * 1. userAccountControl disable flag is on
 * 2. userAccountControl lockout flag is on
 * 3. userAccountControl password expire flag is on
 *
 * if Lockout policy auto unlock is set and time expired, unlock account.
 *
 * Assume pEntry->dn has normalized form.
 */
DWORD
VdirLoginBlocked(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry
    )
{
    DWORD               dwError = 0;
    BOOLEAN             bLoginBlocked = FALSE;
    PSTR                pszLocalErrMsg = NULL;
    PVDIR_LOCKOUT_REC   pLockoutRec = NULL;
    PVDIR_ATTRIBUTE     pUserActCtlAttr = NULL;
    int64_t             iPasswdExpireFlag  = 0;
    BOOLEAN             bCheckLockoutFlag = TRUE;
    int64_t             iLockoutFlag  = 0;
    int64_t             iDisableFlag  = 0;
    VDIR_LOCKOUT_REC    myTmpLockoutRec = {0};
    BOOLEAN             bRemoveLockoutCacheRec = FALSE;

    if ( !pEntry || !pOperation )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // bypass lockout policy check for default Administrator and DC Account user
    if ( _VmDirExemptUserFromLockoutPolicy( pEntry->eId, BERVAL_NORM_VAL(pEntry->dn), &pOperation->conn->AccessInfo) )
    {
        dwError = 0;
        goto cleanup;
    }

    dwError = LockoutCacheRecGet(
                    BERVAL_NORM_VAL(pEntry->dn),
                    &pLockoutRec);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, " Lockout cache get failed");

    if (pLockoutRec)
    {
        time_t  tNow = time(NULL);

        // handle lockout and auto unlock
        if (pLockoutRec->lockoutTime > 0 && pLockoutRec->iAutoUnlockIntervalSec > 0)
        {
            if ((tNow - pLockoutRec->lockoutTime) <= pLockoutRec->iAutoUnlockIntervalSec)
            {
                dwError = VMDIR_ERROR_USER_LOCKOUT;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            {
                // auto unlock, modify entry to clear lockout flag
                pLockoutRec->bAutoUnlockAccount = FALSE;
                pLockoutRec->lockoutTime = 0;
                VdirUserActCtlFlagUnset(pEntry, USER_ACCOUNT_CONTROL_LOCKOUT_FLAG);  // ignore error
                bCheckLockoutFlag = FALSE;  // need this as pEntry itself is not updated
            }
        }
    }

    // check if userAccontControl password expired and lockout flag set
    pUserActCtlAttr = VmDirFindAttrByName(pEntry, ATTR_USER_ACCOUNT_CONTROL);
    if (pUserActCtlAttr && pUserActCtlAttr->vals)
    {
        iDisableFlag = iLockoutFlag = iPasswdExpireFlag = VmDirStringToLA(pUserActCtlAttr->vals[0].lberbv.bv_val, NULL, 10);

        iPasswdExpireFlag &= USER_ACCOUNT_CONTROL_PASSWORD_EXPIRE_FLAG;
        iLockoutFlag      &= USER_ACCOUNT_CONTROL_LOCKOUT_FLAG;
        iDisableFlag      &= USER_ACCOUNT_CONTROL_DISABLE_FLAG;

        bRemoveLockoutCacheRec = TRUE;
        if ( iPasswdExpireFlag )
        {
            dwError = VMDIR_ERROR_PASSWORD_EXPIRED;
        }
        else if ( bCheckLockoutFlag && iLockoutFlag )
        {
            dwError = VMDIR_ERROR_ACCOUNT_LOCKED;
        }
        else if ( iDisableFlag )
        {
            dwError = VMDIR_ERROR_ACCOUNT_DISABLED;
        }
        else
        {
            bRemoveLockoutCacheRec = FALSE;
        }
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Account access blocked");
    }

    {
        VDIR_PASSWD_LOCKOUT_POLICY  policy = {0};

        // BUGBUG PERFORMANCE BUGBUG, should cache policy?
        dwError = VdirGetPasswdAndLockoutPolicy(
                        BERVAL_NORM_VAL(pEntry->dn),
                        &policy);
        if (dwError == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND)
        {
            dwError = 0;
            goto cleanup;   // no policy, bypass checking
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        if (policy.bEnabled)
        {
            dwError = LockoutRecInit(
                            BERVAL_NORM_VAL(pEntry->dn),
                            &myTmpLockoutRec,
                            &policy);
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Lockout cache init failed");

            // lockout policy check  (if password expired)
            dwError = LockoutPolicyCheck(
                            &myTmpLockoutRec,
                            pEntry,
                            &bLoginBlocked);
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Lockout policy check failed");

            if (bLoginBlocked)
            {
                dwError = VMDIR_ERROR_PASSWORD_EXPIRED;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(myTmpLockoutRec.pszNormDN);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    if (pLockoutRec &&
        (bLoginBlocked == FALSE || bRemoveLockoutCacheRec))
    {
        // remove lockout record from cache if we
        // 1. no longer blocked (i.e. login success)         OR
        // 2. userAccountControl blocks login (one of the flag set)
        VdirLockoutCacheRemoveRec(BERVAL_NORM_VAL(pEntry->dn));
    }

    return dwError;

error:

    bLoginBlocked = TRUE;

    VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "LoginBlocked DN (%s), error (%u)(%s)", BERVAL_NORM_VAL(pEntry->dn),
                             dwError, VDIR_SAFE_STRING(pszLocalErrMsg));

    goto cleanup;
}


/*
 * called via
 * 1. bind/login fail or
 * 2. password modify fail - pEntry parameter is NULL
 *
 */
DWORD
VdirPasswordFailEvent(
    PVDIR_OPERATION     pOperation,
    PCSTR               pszNormDN,
    PVDIR_ENTRY         pEntry
    )
{
    DWORD       dwError = 0;
    PSTR        pszLocalErrMsg = NULL;
    BOOLEAN     bAddRecToCache = FALSE;
    BOOLEAN     bAccountLockout = FALSE;
    PVDIR_LOCKOUT_REC           pLockoutRec = NULL;
    VDIR_PASSWD_LOCKOUT_POLICY  policy = {0};

    if ( !pszNormDN || !pOperation )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // bypass lockout policy check for default Administrator and DC Account user
    if ( pEntry && _VmDirExemptUserFromLockoutPolicy( pEntry->eId, pszNormDN, &pOperation->conn->AccessInfo) )
    {
        goto cleanup;
    }

    dwError = VdirGetPasswdAndLockoutPolicy(
                    pszNormDN,
                    &policy);
    if (dwError == ERROR_BACKEND_ENTRY_NOTFOUND)
    {
        goto cleanup;   // no policy, bypass checking
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!policy.bEnabled)
    {
        goto cleanup;
    }

    dwError = LockoutCacheRecGet(
                    pszNormDN,
                    &pLockoutRec);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Lockout cache get failed");

    if (! pLockoutRec)
    {
        dwError = VmDirAllocateMemory(
                        sizeof(*pLockoutRec),
                        (PVOID)&pLockoutRec);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LockoutRecInit(
                        pszNormDN,
                        pLockoutRec,
                        &policy);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Lockout cache init failed");

        pLockoutRec->iFailedAttempt = 1;
        pLockoutRec->firstFailedTime = time(NULL);

        bAddRecToCache = TRUE;
    }
    else
    {
        time_t  tNow = time(NULL);

        if ( (pLockoutRec->lockoutTime > 0 && pLockoutRec->iAutoUnlockIntervalSec > 0)
             &&
             ((tNow - pLockoutRec->lockoutTime) > pLockoutRec->iAutoUnlockIntervalSec)
           )
        {
            // auto unlock matched, though current login attempt failed again.
            pLockoutRec->bAutoUnlockAccount = TRUE; // will be unlock in LockoutPolicyCheck later.
            // reset to first time failure parameters
            pLockoutRec->lockoutTime = 0;
            pLockoutRec->iFailedAttempt = 1;
            pLockoutRec->firstFailedTime = tNow;
        }
        else if (tNow - pLockoutRec->firstFailedTime > pLockoutRec->iFailedAttemptIntervalSec)
        {   // over FailedAttemptIntervalSec, reset count and first failed time
            pLockoutRec->iFailedAttempt = 1;
            pLockoutRec->firstFailedTime = tNow;
        }
        else
        {   // within FailedAttemptIntervalSec, increment count
            pLockoutRec->iFailedAttempt++;
        }
    }

    if (pEntry)
    {
        // lockout policy check
        dwError = LockoutPolicyCheck(
                     pLockoutRec,
                     pEntry,
                     &bAccountLockout);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Lockout policy check failed");
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    if (bAddRecToCache)
    {   // ignore error
        LockoutCacheRecSet(pszNormDN, pLockoutRec);
    }

    return dwError;

error:

    if (pLockoutRec && bAddRecToCache == FALSE)
    {
        LockoutRecFree(pLockoutRec);
    }


    VmDirLog(LDAP_DEBUG_ANY, "VdirPasswordFailEvent (%u)(%s)", dwError,
                             VDIR_SAFE_STRING(pszLocalErrMsg));

    goto cleanup;
}

/*
 * populate pPolicy based on entry DN
 * if entry has NO password policy, this call is a no-op.
 */
DWORD
VdirGetPasswdAndLockoutPolicy(
    PCSTR                           pszNormEntryDN,
    PVDIR_PASSWD_LOCKOUT_POLICY     pPolicy
    )
{


    DWORD           dwError = 0;
    PSTR            pszPolicyDN = NULL;
    PCSTR           pszDomainDN = NULL;
    PVDIR_ENTRY     pPolicyEntry = NULL;

    assert(pszNormEntryDN && pPolicy);

    // get the domain DN this entry belongs to
    pszDomainDN = VmDirFindDomainDN(pszNormEntryDN);
    if (pszDomainDN)
    {
        // default policy entry lives under domain entry with fix cn
        dwError = VmDirAllocateStringAVsnprintf(
                        &pszPolicyDN,
                        "cn=%s,%s",
                        PASSWD_LOCKOUT_POLICY_DEFAULT_CN,
                        pszDomainDN);
        BAIL_ON_VMDIR_ERROR(dwError)

        ///////////////////////////////////////////////////////////////////////
        // BUGBUG PERFORMANCE BUGBUG should  consider caching policies.
        ///////////////////////////////////////////////////////////////////////
        dwError = VmDirSimpleDNToEntry(pszPolicyDN, &pPolicyEntry);
        if (gVmdirGlobals.bIsLDAPPortOpen &&
            (dwError == VMDIR_ERROR_ENTRY_NOT_FOUND || dwError == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND))
        {
            dwError = 0;
            VmDirGetDefaultPasswdLockoutPolicy(pPolicy);
            goto cleanup;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        LockoutPolicyLoadFromEntry(pPolicyEntry, pPolicy);
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszPolicyDN);

    if (pPolicyEntry)
    {
        VmDirFreeEntry(pPolicyEntry);
    }

    return dwError;

error:

    goto cleanup;
}

/*
 * Enforce policy definition integrity
 */
DWORD
VdirLockoutPolicyIntegrityCheck(
    PVDIR_ENTRY     pPolicyEntry
    )
{
    DWORD           dwError = 0;
    VDIR_PASSWD_LOCKOUT_POLICY  policy = {0};

    assert(pPolicyEntry != NULL);

    LockoutPolicyLoadFromEntry(pPolicyEntry, &policy);

    if (policy.iAutoUnlockIntervalSec       < 0         ||
        policy.iExpireInDay                 < 0         ||
        policy.iFailedAttemptIntervalSec    < 0         ||
        policy.iMaxFailedAttempt            < 0         ||
        policy.iMaxLen                      < 0         ||
        policy.iMinAlphaCnt                 < 0         ||
        policy.iMinLen                      < 0         ||
        policy.iMinLowerCaseCnt             < 0         ||
        policy.iMinNumericCnt               < 0         ||
        policy.iMinSpecialCharCnt           < 0         ||
        policy.iMinUpperCaseCnt             < 0         ||
        policy.iRecycleCnt                  < 0         ||
        policy.iMaxSameAdjacentCharCnt      < 1
        )
    {
        dwError = LDAP_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ((policy.iMaxLen < policy.iMinLen)
        ||
        (policy.iMinLen < policy.iMinAlphaCnt + policy.iMinNumericCnt + policy.iMinSpecialCharCnt)
        ||
        (policy.iMinAlphaCnt < policy.iMinLowerCaseCnt + policy.iMinUpperCaseCnt)
        )
    {
        dwError = LDAP_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }


 error:

     return dwError;
}

/*
 * set userAccountContorl iTargetFlag to FALSE for this entry in DB
 * NOTE, pEntry content is NOT changed.
 */
DWORD
VdirUserActCtlFlagUnset(
    PVDIR_ENTRY         pEntry,
    int64_t             iTargetFlag
    )
{
    DWORD               dwError = 0;
    PVDIR_ATTRIBUTE     pUserActCtlAttr = NULL;
    VDIR_BERVALUE       berUserActCtl = VDIR_BERVALUE_INIT;
    int64_t             iUserActCtl  = 0;
    CHAR                pszUserActCtl[sizeof(iUserActCtl) + 1] = {0};

    assert(pEntry);

    pUserActCtlAttr = VmDirFindAttrByName(pEntry, ATTR_USER_ACCOUNT_CONTROL);
    if (pUserActCtlAttr && pUserActCtlAttr->vals)
    {
        iUserActCtl = VmDirStringToLA(pUserActCtlAttr->vals[0].lberbv.bv_val, NULL, 10);
    }

    iUserActCtl &= (~(iTargetFlag));
    VmDirStringNPrintFA(pszUserActCtl, sizeof(pszUserActCtl), sizeof(pszUserActCtl) - 1, "%ld", iUserActCtl);

    berUserActCtl.lberbv.bv_val = pszUserActCtl;
    berUserActCtl.lberbv.bv_len = VmDirStringLenA(pszUserActCtl);

    dwError = VmDirInternalEntryAttributeReplace(
                    pEntry->pSchemaCtx,
                    BERVAL_NORM_VAL(pEntry->dn),
                    ATTR_USER_ACCOUNT_CONTROL,
                    &berUserActCtl);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
                    "User account control - (%s): (%x) flag unset, new value=(%x)",
                    VDIR_SAFE_STRING(BERVAL_NORM_VAL(pEntry->dn)),
                    iTargetFlag, iUserActCtl );

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                    "User account control - (%s): (%x) flag unset, new value=(%x) failed",
                    VDIR_SAFE_STRING(BERVAL_NORM_VAL(pEntry->dn)),
                    iTargetFlag, iUserActCtl );

    goto cleanup;
}

/*
 * set userAccountContorl iTargetFlag to TRUE for this entry in DB
 * NOTE, pEntry content is NOT changed.
 */
DWORD
VdirUserActCtlFlagSet(
    PVDIR_ENTRY         pEntry,
    int64_t             iTargetFlag
    )
{
    DWORD               dwError = 0;
    PVDIR_ATTRIBUTE     pUserActCtlAttr = NULL;
    VDIR_BERVALUE       berUserActCtl = VDIR_BERVALUE_INIT;
    int64_t             iUserActCtl  = 0;
    CHAR                pszUserActCtl[sizeof(iUserActCtl) + 1] = {0};

    assert(pEntry);

    pUserActCtlAttr = VmDirFindAttrByName(pEntry, ATTR_USER_ACCOUNT_CONTROL);
    if (pUserActCtlAttr && pUserActCtlAttr->vals)
    {
        iUserActCtl = VmDirStringToLA(pUserActCtlAttr->vals[0].lberbv.bv_val, NULL, 10);
    }

    iUserActCtl |= iTargetFlag;
    VmDirStringNPrintFA(pszUserActCtl, sizeof(pszUserActCtl), sizeof(pszUserActCtl) - 1, "%ld", iUserActCtl);

    berUserActCtl.lberbv.bv_val = pszUserActCtl;
    berUserActCtl.lberbv.bv_len = VmDirStringLenA(pszUserActCtl);

    dwError = VmDirInternalEntryAttributeReplace(
                    pEntry->pSchemaCtx,
                    BERVAL_NORM_VAL(pEntry->dn),
                    ATTR_USER_ACCOUNT_CONTROL,
                    &berUserActCtl);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
                    "User account control - (%s): (%x) flag set, new value=(%x)",
                    VDIR_SAFE_STRING(BERVAL_NORM_VAL(pEntry->dn)),
                    iTargetFlag, iUserActCtl );

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                    "User account control - (%s): (%x) flag set, new value=(%x) failed",
                    VDIR_SAFE_STRING(BERVAL_NORM_VAL(pEntry->dn)),
                    iTargetFlag, iUserActCtl );
    goto cleanup;
}

static
VOID
LockoutRecFree(
    PVDIR_LOCKOUT_REC   pLockoutRec
    )
{
    if (pLockoutRec)
    {
        VMDIR_SAFE_FREE_MEMORY(pLockoutRec->pszNormDN);
        VMDIR_SAFE_FREE_MEMORY(pLockoutRec);
    }
}

/*
 * caller does NOT own *ppLockoutRec, do NOT free it.
 */
static
DWORD
LockoutCacheRecGet(
    PCSTR               pszNormDN,
    PVDIR_LOCKOUT_REC*  ppLockoutRec
    )
{
    DWORD                   dwError = 0;
    BOOLEAN                 bInLock = FALSE;
    PCSTR                   pszErrorContext = NULL;
    PLW_HASHTABLE_NODE      pNode = NULL;
    PVDIR_LOCKOUT_REC       pLockoutRec = NULL;

    assert(pszNormDN && ppLockoutRec);

    VMDIR_LOCK_MUTEX(bInLock, gVdirLockoutCache.mutex);

    if (gVdirLockoutCache.pHashTbl)
    {
        pszErrorContext = "lockout cache lookup";
        dwError = LwRtlHashTableFindKey(
                        gVdirLockoutCache.pHashTbl,
                        &pNode,
                        (PVOID)pszNormDN);
        dwError = LwNtStatusToWin32Error(dwError);
        if (dwError == ERROR_NOT_FOUND)
        {
            dwError = 0;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pNode)
        {
            // access but NOT own lockout record
            pLockoutRec = (PVDIR_LOCKOUT_REC)LW_STRUCT_FROM_FIELD(pNode, VDIR_LOCKOUT_REC, Node);
        }
    }

    *ppLockoutRec = pLockoutRec;

cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirLockoutCache.mutex);

    return dwError;

error:

    goto cleanup;
}

static
LW_PCVOID
LockoutRecGetKey(
    PLW_HASHTABLE_NODE      pNode,
    PVOID                   pUnused
    )
{
    PVDIR_LOCKOUT_REC pLockoutRec = LW_STRUCT_FROM_FIELD(pNode, VDIR_LOCKOUT_REC, Node);

    return pLockoutRec->pszNormDN;
}

/*
 * Add LockoutRec into cache
 * if success, gVdirLockoutCache.pHashTbl takes over pLockoutRec
 *
 * TODO, cache aging
 */
static
DWORD
LockoutCacheRecSet(
    PCSTR               pszNormDN,
    PVDIR_LOCKOUT_REC   pLockoutRec
    )
{
    DWORD       dwError = 0;
    BOOLEAN     bInLock = FALSE;
    PLW_HASHTABLE_NODE pNode = NULL;

    assert(pszNormDN && pLockoutRec);

    VMDIR_LOCK_MUTEX(bInLock, gVdirLockoutCache.mutex);

    if (! gVdirLockoutCache.pHashTbl)
    {
        dwError = LwRtlCreateHashTable(
                        &gVdirLockoutCache.pHashTbl,
                        LockoutRecGetKey,
                        LwRtlHashDigestPstr,
                        LwRtlHashEqualPstr,
                        NULL,
                        VMDIR_LOCKOUT_VECTOR_HASH_TABLE_SIZE);
        dwError = LwNtStatusToWin32Error(dwError);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    LwRtlHashTableResizeAndInsert(
            gVdirLockoutCache.pHashTbl,
            &pLockoutRec->Node,
            &pNode);
    assert(pNode == NULL);  // assert the key(DN) of added node is unique

cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirLockoutCache.mutex);

    return dwError;

error:

    goto cleanup;
}


static
BOOLEAN
VmDirCheckForPasswordExpiration(
    PVDIR_LOCKOUT_REC pLockoutRec,
    PVDIR_ENTRY pEntry
    )
{
    int64_t iPwdLastSet =  0;
    int64_t iExpirationInSeconds = 0;
    time_t tNow = time(NULL);
    BOOLEAN bPasswordExpired = FALSE;
    PVDIR_ATTRIBUTE pPwdLastSetAttr = NULL;
    PVDIR_ATTRIBUTE pPwdNeverExpiresAttr = NULL;

    if (pLockoutRec->iExpireInDay == 0)
    {
        VMDIR_LOG_DEBUG(LDAP_DEBUG_ANY, "Lockout policy check - password policy ExpireInDay is 0, implying never expire.");
        goto cleanup;
    }

    pPwdNeverExpiresAttr = VmDirFindAttrByName(pEntry, ATTR_PASSWORD_NEVER_EXPIRES);
    if (pPwdNeverExpiresAttr && pPwdNeverExpiresAttr->vals &&
        VmDirStringCompareA(pPwdNeverExpiresAttr->vals[0].lberbv.bv_val, VDIR_LDAP_BOOLEN_SYNTAX_TRUE_STR, FALSE) == 0)
    {
        VMDIR_LOG_DEBUG(LDAP_DEBUG_ANY, "Lockout policy check - password never expires. (%s)",
                  VDIR_SAFE_STRING(BERVAL_NORM_VAL(pEntry->dn)) );
        goto cleanup;
    }

    pPwdLastSetAttr = VmDirFindAttrByName(pEntry, ATTR_PWD_LAST_SET);
    if (pPwdLastSetAttr && pPwdLastSetAttr->vals)
    {
        iPwdLastSet = VmDirStringToLA(pPwdLastSetAttr->vals[0].lberbv.bv_val, NULL, 10);
        iExpirationInSeconds = (int64_t)pLockoutRec->iExpireInDay * 24 * 60 * 60;

        if ((tNow - iPwdLastSet) > iExpirationInSeconds)
        {
            bPasswordExpired = TRUE;
            VMDIR_LOG_DEBUG(
                LDAP_DEBUG_ANY,
                "Lockout policy check - password expired. (%s)",
                VDIR_SAFE_STRING(BERVAL_NORM_VAL(pEntry->dn)));

            (VOID)VdirUserActCtlFlagSet(
                    pEntry,
                    USER_ACCOUNT_CONTROL_PASSWORD_EXPIRE_FLAG);
        }
    }
    else
    {
        VMDIR_LOG_DEBUG(LDAP_DEBUG_ANY,
                "Lockout policy check - no pwdLastSet attribute (%s)",
                VDIR_SAFE_STRING(BERVAL_NORM_VAL(pEntry->dn)));
    }

cleanup:
    return bPasswordExpired;
}

/*
 * *bBoolLockout is true if
 * 1. MaxFailAttempt reached    OR
 * 2. password expired
 */
static
DWORD
LockoutPolicyCheck(
    PVDIR_LOCKOUT_REC   pLockoutRec,
    PVDIR_ENTRY         pEntry,
    PBOOLEAN            bBoolLockout
    )
{
    DWORD dwError = 0;

    *bBoolLockout = FALSE;

    if (pLockoutRec->bAutoUnlockAccount)
    {
        pLockoutRec->bAutoUnlockAccount = FALSE;
        pLockoutRec->lockoutTime = 0;
        (VOID)VdirUserActCtlFlagUnset(pEntry, USER_ACCOUNT_CONTROL_LOCKOUT_FLAG);

        goto cleanup;
    }

    if (pLockoutRec->iFailedAttempt >= pLockoutRec->iMaxFailedAttempt)
    {
        *bBoolLockout = TRUE;

        if (pLockoutRec->lockoutTime == 0)
        {
            pLockoutRec->lockoutTime = time(NULL);

            // modify entry to set lockout flag
            (VOID)VdirUserActCtlFlagSet(pEntry, USER_ACCOUNT_CONTROL_LOCKOUT_FLAG);
        }

        VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                "Lockout policy check - account lockout. (%s)",
                VDIR_SAFE_STRING(pLockoutRec->pszNormDN));

        goto cleanup;
    }

    *bBoolLockout = VmDirCheckForPasswordExpiration(pLockoutRec, pEntry);

cleanup:

    return dwError;
}

static
DWORD
LockoutRecInit(
    PCSTR                       pszNormDN,
    PVDIR_LOCKOUT_REC           pLockoutRec,
    PVDIR_PASSWD_LOCKOUT_POLICY pPolicy
    )
{
    DWORD   dwError = 0;

    assert(pPolicy && pLockoutRec);

    dwError = VmDirAllocateStringA(
                    pszNormDN,
                    &pLockoutRec->pszNormDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    pLockoutRec->iFailedAttemptIntervalSec = pPolicy->iFailedAttemptIntervalSec;
    pLockoutRec->iMaxFailedAttempt = pPolicy->iMaxFailedAttempt;
    pLockoutRec->iExpireInDay = pPolicy->iExpireInDay;
    pLockoutRec->iAutoUnlockIntervalSec = pPolicy->iAutoUnlockIntervalSec;

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
VOID
LockoutPolicyLoadFromEntry(
    PVDIR_ENTRY                 pPolicyEntry,
    PVDIR_PASSWD_LOCKOUT_POLICY pPolicy
    )
{
    PVDIR_ATTRIBUTE pAttr = NULL;

    assert(pPolicyEntry != NULL && pPolicy != NULL);

    for (pAttr = pPolicyEntry->attrs; pAttr ; pAttr = pAttr->next)
    {
        if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_PASS_AUTO_UNLOCK_SEC, FALSE) == 0)
        {
            pPolicy->iAutoUnlockIntervalSec = VmDirStringToIA(pAttr->vals[0].lberbv.bv_val);
        }
        else if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_PASS_FAIL_ATTEMPT_SEC, FALSE) == 0)
        {
            pPolicy->iFailedAttemptIntervalSec = VmDirStringToIA(pAttr->vals[0].lberbv.bv_val);
        }
        else if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_PASS_MAX_FAIL_ATTEMPT, FALSE) == 0)
        {
            pPolicy->iMaxFailedAttempt = VmDirStringToIA(pAttr->vals[0].lberbv.bv_val);
        }
        else if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_PASS_MAX_SAME_ADJ_CHAR, FALSE) == 0)
        {
            pPolicy->iMaxSameAdjacentCharCnt = VmDirStringToIA(pAttr->vals[0].lberbv.bv_val);
        }
        else if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_PASS_MIN_SP_CHAR, FALSE) == 0)
        {
            pPolicy->iMinSpecialCharCnt = VmDirStringToIA(pAttr->vals[0].lberbv.bv_val);
        }
        else if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_PASS_MIN_MUN_CHAR, FALSE) == 0)
        {
            pPolicy->iMinNumericCnt = VmDirStringToIA(pAttr->vals[0].lberbv.bv_val);
        }
        else if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_PASS_MIN_UPPER_CHAR, FALSE) == 0)
        {
            pPolicy->iMinUpperCaseCnt = VmDirStringToIA(pAttr->vals[0].lberbv.bv_val);
        }
        else if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_PASS_MIN_LOWER_CHAR, FALSE) == 0)
        {
            pPolicy->iMinLowerCaseCnt = VmDirStringToIA(pAttr->vals[0].lberbv.bv_val);
        }
        else if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_PASS_MIN_ALPHA_CHAR, FALSE) == 0)
        {
            pPolicy->iMinAlphaCnt = VmDirStringToIA(pAttr->vals[0].lberbv.bv_val);
        }
        else if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_PASS_MIN_SIZE, FALSE) == 0)
        {
            pPolicy->iMinLen = VmDirStringToIA(pAttr->vals[0].lberbv.bv_val);
        }
        else if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_PASS_MAX_SIZE, FALSE) == 0)
        {
            pPolicy->iMaxLen = VmDirStringToIA(pAttr->vals[0].lberbv.bv_val);
        }
        else if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_PASS_EXP_IN_DAY, FALSE) == 0)
        {
            pPolicy->iExpireInDay = VmDirStringToIA(pAttr->vals[0].lberbv.bv_val);
        }
        else if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_PASS_RECYCLE_CNT, FALSE) == 0)
        {
            pPolicy->iRecycleCnt = VmDirStringToIA(pAttr->vals[0].lberbv.bv_val);
        }
        else if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_ENABLED, FALSE) == 0)
        {
            pPolicy->bEnabled = VmDirStringCompareA(pAttr->vals[0].lberbv.bv_val, VDIR_LDAP_BOOLEN_SYNTAX_TRUE_STR, FALSE) == 0 ? TRUE : FALSE;
        }
        else if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_PASS_SPECIAL_CHARS, FALSE) == 0)
        {
            DWORD   dwTmpError = 0;

            memset(pPolicy->specialChars, 0, MAX_PASSWORD_SPECIAL_CHARS+1);
            dwTmpError = VmDirCopyMemory(
                            pPolicy->specialChars,
                            MAX_PASSWORD_SPECIAL_CHARS,
                            pAttr->vals[0].lberbv.bv_val,
                            pAttr->vals[0].lberbv.bv_len < MAX_PASSWORD_SPECIAL_CHARS ?
                                            pAttr->vals[0].lberbv.bv_len: MAX_PASSWORD_SPECIAL_CHARS );

            if (dwTmpError || pAttr->vals[0].lberbv.bv_len > MAX_PASSWORD_SPECIAL_CHARS )
            {
                VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "Init password special chars failed, (%s)(%d), "
                                                       " first %d chars enforced",
                                                        pPolicyEntry->dn.lberbv.bv_val,
                                                        dwTmpError,
                                                        MAX_PASSWORD_SPECIAL_CHARS);
            }
        }
    }

}

/*
 * In 2015 release, we exempt Administrator and DCadmins/DCClients users from
 * lockout and password policy enforcement.
 */
static
BOOLEAN
_VmDirExemptUserFromLockoutPolicy(
    ENTRYID     entryID,
    PCSTR       pszNormDN,
    PVDIR_ACCESS_INFO   pAccessInfo
    )
{
    DWORD       dwError          = 0;
    BOOLEAN     bExemptUser      = FALSE;

    if ( entryID == DEFAULT_ADMINISTRATOR_ENTRY_ID )
    {
       bExemptUser = (gVmdirGlobals.bAllowAdminLockout == FALSE);
    }
    else
    {
        // exempt if direct member of DCGroup
        dwError = VmDirIsDirectMemberOf( (PSTR)pszNormDN,
                                         VDIR_ACCESS_DCGROUP_MEMBER_INFO,
                                         &pAccessInfo->accessRoleBitmap,
                                         &bExemptUser );
        BAIL_ON_VMDIR_ERROR(dwError);

        if ( ! bExemptUser )
        {
            // exempt if direct member of DCClientGroup
            dwError = VmDirIsDirectMemberOf( (PSTR)pszNormDN,
                                             VDIR_ACCESS_DCCLIENT_GROUP_MEMBER_INFO,
                                             &pAccessInfo->accessRoleBitmap,
                                             &bExemptUser );
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }


cleanup:

    return bExemptUser;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirExemptUserFromLockoutPolicy failed, (%u)", dwError);

    bExemptUser = FALSE;

    goto cleanup;
}
