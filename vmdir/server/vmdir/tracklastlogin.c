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
 * Module Name: Track Last Login Time thread
 *
 * Filename: tracklastlogin.c
 *
 * Registry key to turn on/off this feature:
 * services\vmdir\parameters\TrackLastLoginTime 0(default)/1
 *
 * Registry key to suppress TrackLastLoginTime for certain accounts under a container:
 * services\vmdir\parameters\SuppressTrackLLTContainer REG_MULTI_SZ - can specify multiple containers.
 *
 */

#include "includes.h"

#define MAX_PENDING_SIZE    256

static PVDIR_THREAD_INFO   _gpTrackLastLoginThrInfo = NULL;
static PVMDIR_STRING_LIST  _gpSuppressDNList = NULL;

static
DWORD
_VmDirTrackLastLoginTimeThreadFun(
    PVOID pArg
    );

static
VOID
_VmDirFreeLoginTime(
    PVMDIR_LOGIN_TIME   pLoginTime
    );

static
DWORD
_VmDirModifyLastLogonTimeStamp(
    PVMDIR_LOGIN_TIME   pLoginTime
    );

static
DWORD
_VmDirGetAccountType(
    PCSTR       pszDN,
    PBOOLEAN    pIsUser,
    PBOOLEAN    pIsComputer
    );

static
BOOLEAN
_VmDirSuppressDN(
    PCSTR       pszDN
    );

DWORD
VmDirInitTrackLastLoginThread(
    VOID
    )
{
    DWORD               dwError = 0;
    PVDIR_THREAD_INFO   pThrInfo = NULL;

    if (!gVmdirGlobals.bTrackLastLoginTime)
    {
        dwError = VMDIR_ERROR_INVALID_CONFIGURATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // if key configure correctly, get suppress DN list
    if (VmDirRegGetMultiSZ(VMDIR_CONFIG_PARAMETER_V1_KEY_PATH,
                           VMDIR_REG_KEY_SUPPRES_TRACK_LLT,
                           &_gpSuppressDNList) == 0)
    {
        DWORD i = 0;

        for (i = 0; i < _gpSuppressDNList->dwCount; ++i)
        {
            VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "TrackLastLogonTime suppress container (%s)",
                            _gpSuppressDNList->pStringList[i]);
        }
    }

    dwError = VmDirSrvThrInit(
                &pThrInfo,
                gVmdirTrackLastLoginTime.pMutex,
                gVmdirTrackLastLoginTime.pCond,
                TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    _gpTrackLastLoginThrInfo = pThrInfo;

    dwError = VmDirCreateThread(
                &pThrInfo->tid,
                pThrInfo->bJoinThr,
                _VmDirTrackLastLoginTimeThreadFun,
                pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pThrInfo);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Enable track last login time" );

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error %d", __FUNCTION__, dwError);

    _gpTrackLastLoginThrInfo = NULL;
    VmDirSrvThrFree(pThrInfo);

    goto cleanup;
}

uint64_t
_VmDirUnixTimeToFileTime(time_t t)
{
    uint64_t filetime = 0;
    filetime = t * (uint64_t)10000000 + (uint64_t)WIN_EPOCH;
    return filetime;
}

/*
 * Update LastLogonTimeStamp of this DN
 */
VOID
VmDirAddTrackLastLoginItem(
    PCSTR   pszDN
    )
{
    DWORD               dwError = 0;
    PVMDIR_LOGIN_TIME   pLoginTime = NULL;

    if (!pszDN)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (_gpTrackLastLoginThrInfo)
    {
        size_t  iStackSize = VmDirGetTSStackSize(gVmdirTrackLastLoginTime.pTSStack);

        if (iStackSize > MAX_PENDING_SIZE)
        {
            VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "%s stack size > %d. stop track last login for %s",
                              __FUNCTION__, MAX_PENDING_SIZE ,VDIR_SAFE_STRING(pszDN));
            goto cleanup;
        }

        if (!_VmDirSuppressDN(pszDN))
        {
            dwError = VmDirAllocateMemory(sizeof(pLoginTime), (PVOID*)&pLoginTime);
            BAIL_ON_VMDIR_ERROR(dwError);

            pLoginTime->loginTime = _VmDirUnixTimeToFileTime(time(NULL));
            dwError = VmDirAllocateStringA(pszDN, &(pLoginTime->pszDN) );
            BAIL_ON_VMDIR_ERROR(dwError);

            // TODO, could have checked whether this DN exists in the stack or not.
            // pTSStack owns pLoginTime
            dwError = VmDirPushTSStack(gVmdirTrackLastLoginTime.pTSStack, pLoginTime);
            BAIL_ON_VMDIR_ERROR(dwError);

            VmDirSrvThrSignal(_gpTrackLastLoginThrInfo);
        }
    }

cleanup:
    return;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                   "%s add (%s) failed, (%d)",
                   __FUNCTION__, VDIR_SAFE_STRING(pszDN), dwError);
    _VmDirFreeLoginTime(pLoginTime);

    goto cleanup;
}

static
BOOLEAN
_VmDirSuppressDN(
    PCSTR       pszDN
    )
{
    BOOLEAN rtn = FALSE;
    DWORD   i = 0;
    PCSTR   pszTmp = NULL;

    if (_gpSuppressDNList)
    {
        for (i = 0; i < _gpSuppressDNList->dwCount; ++i)
        {
            if ((pszTmp = VmDirStringCaseStrA(pszDN, _gpSuppressDNList->pStringList[i])) != NULL
                    &&
                (pszTmp[VmDirStringLenA(_gpSuppressDNList->pStringList[i])] == '\0')
               )
            {
               rtn = TRUE;
               break;
            }
        }
    }

    return rtn;
}

static
VOID
_VmDirFreeLoginTime(
    PVMDIR_LOGIN_TIME   pLoginTime
    )
{
    if (pLoginTime)
    {
        VMDIR_SAFE_FREE_MEMORY(pLoginTime->pszDN);
        VMDIR_SAFE_FREE_MEMORY(pLoginTime);
    }

    return;
}

static
DWORD
_VmDirModifyLastLogonTimeStamp(
    PVMDIR_LOGIN_TIME   pLoginTime
    )
{
    DWORD           dwError = 0;
    VDIR_OPERATION  op = {0};
    PSTR            pszLocalErrMsg = NULL;
    VDIR_BERVALUE   bvTargetDN = VDIR_BERVALUE_INIT;
    char            pBuf[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};

    dwError = VmDirInitStackOperation( &op, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_MODIFY, NULL );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
            "%s:%d VmDirInitStackOperation failed with error code: %d.",
            __FUNCTION__, __LINE__, dwError );

    // Setup target DN
    bvTargetDN.lberbv.bv_val = pLoginTime->pszDN;
    bvTargetDN.lberbv.bv_len = VmDirStringLenA( bvTargetDN.lberbv.bv_val );

    dwError = VmDirNormalizeDN( &bvTargetDN, op.pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirBervalContentDup( &bvTargetDN, &op.reqDn );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
            "%s:%d BervalContentDup failed with error code: %d.",
            __FUNCTION__, __LINE__, dwError );

    op.pBEIF = VmDirBackendSelect(op.reqDn.lberbv.bv_val);
    assert(op.pBEIF);

    dwError = VmDirBervalContentDup( &op.reqDn, &op.request.modifyReq.dn );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                "%s:%d: BervalContentDup failed with error code: %d.",
                __FUNCTION__, __LINE__, dwError );

    // Setup mods.  ATTR_MODIFIERS_NAME will be default to administrator.
    dwError = VmDirStringPrintFA( pBuf, sizeof(pBuf), "%lld", pLoginTime->loginTime);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAppendAMod( &op, MOD_OP_REPLACE,
                               ATTR_LASTLOGONTIMESTAMP,
                               ATTR_LASTLOGONTIMESTAMP_LEN,
                               pBuf,
                               VmDirStringLenA(pBuf) );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                "%s:%d: VmDirAppendAMod failed with error code: %d.",
                __FUNCTION__, __LINE__, dwError );

    dwError = VmDirInternalModifyEntry( &op );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                "%s:%d: InternalModifyEntry failed. DN: %s, Error code: %d, Error string: %s",
                __FUNCTION__, __LINE__, op.reqDn.lberbv.bv_val, dwError, VDIR_SAFE_STRING( op.ldapResult.pszErrMsg ) );

cleanup:

    VmDirFreeBervalContent(&bvTargetDN);
    VmDirFreeOperationContent(&op);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return dwError;

error:
    VmDirLog(LDAP_DEBUG_ANY, VDIR_SAFE_STRING(pszLocalErrMsg) );
    goto cleanup;
}

static
DWORD
_VmDirTrackLastLoginTimeThreadFun(
    PVOID pArg
    )
{
    DWORD               dwError = 0;
    PVMDIR_LOGIN_TIME   pLoginTime = NULL;
    BOOLEAN             bInLock = FALSE;

    while (TRUE)
    {
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }

        _VmDirFreeLoginTime(pLoginTime);
        pLoginTime = NULL;

        while ( VmDirPopTSStack(gVmdirTrackLastLoginTime.pTSStack, (PVOID*)&pLoginTime) == 0 &&
                pLoginTime != NULL
              )
        {
            BOOLEAN bIsUser = FALSE;
            BOOLEAN bIsComputer = FALSE;

            if (_VmDirGetAccountType(pLoginTime->pszDN, &bIsUser, &bIsComputer) == 0    &&
                bIsUser                                                                 &&
                !bIsComputer
               )
            {   // only update lastlogontimestamp for user
                (VOID) _VmDirModifyLastLogonTimeStamp(pLoginTime);
            }

            if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
            {
                goto cleanup;
            }

            _VmDirFreeLoginTime(pLoginTime);
            pLoginTime = NULL;
        }

        VMDIR_LOCK_MUTEX(bInLock, _gpTrackLastLoginThrInfo->mutexUsed);

        // main thread will signal during shutdown
        VmDirConditionTimedWait(
                _gpTrackLastLoginThrInfo->conditionUsed,
                _gpTrackLastLoginThrInfo->mutexUsed,
                10 * 1000);          // wait 10 seconds
        // ignore error

        VMDIR_UNLOCK_MUTEX(bInLock, _gpTrackLastLoginThrInfo->mutexUsed);
    }

cleanup:
    if (_gpSuppressDNList)
    {
        VmDirStringListFree(_gpSuppressDNList);
    }
    // we may have leak pLoginIime(s), but exiting anyway.
    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Exit track last login time thread" );

    return dwError;
}

static
DWORD
_VmDirGetAccountType(
    PCSTR       pszDN,
    PBOOLEAN    pIsUser,
    PBOOLEAN    pIsComputer
    )
{
    DWORD       dwError = 0;
    BOOLEAN     bIsUserAccount = FALSE;
    BOOLEAN     bIsComputerAccount = FALSE;
    PVDIR_ENTRY pEntry = NULL;
    PVDIR_ATTRIBUTE pAttr = NULL;

    dwError = VmDirSimpleDNToEntry(pszDN, &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    pAttr = VmDirFindAttrByName(pEntry, ATTR_OBJECT_CLASS);
    if (pAttr)
    {
        unsigned i=0;
        for (i=0; i<pAttr->numVals; i++)
        {
            if (VmDirStringCompareA(pAttr->vals[i].lberbv_val, OC_USER, FALSE) == 0)
            {
                bIsUserAccount = TRUE;
            }
            else if (VmDirStringCompareA(pAttr->vals[i].lberbv_val, OC_COMPUTER, FALSE) == 0)
            {
                bIsComputerAccount = TRUE;
            }
        }
    }

    *pIsUser = bIsUserAccount;
    *pIsComputer = bIsComputerAccount;

cleanup:
    VmDirFreeEntry(pEntry);

    return dwError;

error:
    goto cleanup;
}
