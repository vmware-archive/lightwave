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

VOID
VmDirFreeCtrlContent(
    LDAPControl*    pCtrl
    )
{
    if (pCtrl)
    {
        if (pCtrl->ldctl_value.bv_val)
        {
            ber_memfree(pCtrl->ldctl_value.bv_val);
        }
        memset(pCtrl, 0, sizeof(LDAPControl));
    }
}

int
VmDirCreateRaftPingCtrlContent(
    PCSTR           pszLeader,
    uint32_t        term,
    LDAPControl*    pPingCtrl
    )
{
    int             retVal = LDAP_SUCCESS;
    BerElement*     pBer = NULL;
    BerValue        localLeaderBV = {0};

    if (!pszLeader || !pPingCtrl)
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if ((pBer = ber_alloc()) == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_NO_MEMORY);
    }

    localLeaderBV.bv_val = (char*)pszLeader;
    localLeaderBV.bv_len = VmDirStringLenA(pszLeader);

    if ( ber_printf( pBer, "{iO}", term, &localLeaderBV) == -1)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: ber_printf failed.", __FUNCTION__ );
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_NO_MEMORY);
    }

    memset( pPingCtrl, 0, sizeof( LDAPControl ));
    pPingCtrl->ldctl_oid = LDAP_RAFT_PING_CONTROL;
    pPingCtrl->ldctl_iscritical = '1';
    if (ber_flatten2(pBer, &pPingCtrl->ldctl_value, 1))
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_NO_MEMORY);
    }

cleanup:

    if (pBer)
    {
        ber_free(pBer, 1);
    }
    return retVal;

error:
    VmDirFreeCtrlContent(pPingCtrl);
    goto cleanup;
}

int
VmDirCreateRaftVoteCtrlContent(
    PCSTR           pszCandiateId,
    uint32_t        term,
    LDAPControl*    pVoteCtrl
    )
{
    int             retVal = LDAP_SUCCESS;
    BerElement*     pBer = NULL;
    BerValue        localCandidateIdBV = {0};

    if (!pszCandiateId || !pVoteCtrl)
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if ((pBer = ber_alloc()) == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_NO_MEMORY);
    }

    localCandidateIdBV.bv_val = (char*)pszCandiateId;
    localCandidateIdBV.bv_len = VmDirStringLenA(pszCandiateId);

    if ( ber_printf( pBer, "{iO}", term, &localCandidateIdBV) == -1)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: ber_printf failed.", __FUNCTION__ );
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_NO_MEMORY);
    }

    memset( pVoteCtrl, 0, sizeof( LDAPControl ));
    pVoteCtrl->ldctl_oid = LDAP_RAFT_VOTE_CONTROL;
    pVoteCtrl->ldctl_iscritical = '1';
    if (ber_flatten2(pBer, &pVoteCtrl->ldctl_value, 1))
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_NO_MEMORY);
    }

cleanup:

    if (pBer)
    {
        ber_free(pBer, 1);
    }
    return retVal;

error:
    VmDirFreeCtrlContent(pVoteCtrl);
    goto cleanup;
}

int
VmDirCreateDbCopyControlContent(
    PVDIR_DB_COPY_CONTROL_VALUE pDbCopyCtrlVal,
    LDAPControl*                pDbCopyCtrl
    )
{
    int             retVal = LDAP_SUCCESS;
    BerElement*     pBer = NULL;
    BerValue        localBV = {0};

    if (!pDbCopyCtrlVal || !pDbCopyCtrlVal->pszPath || !pDbCopyCtrl || pDbCopyCtrlVal->dwBlockSize == 0)
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if ((pBer = ber_alloc()) == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_NO_MEMORY);
    }

    localBV.bv_val = (char*)pDbCopyCtrlVal->pszPath;
    localBV.bv_len = VmDirStringLenA(pDbCopyCtrlVal->pszPath);

    if (ber_printf(pBer, "{Oii}", &localBV, pDbCopyCtrlVal->dwBlockSize, pDbCopyCtrlVal->fd) == -1)
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_IO);
    }

    memset(pDbCopyCtrl, 0, sizeof(LDAPControl));
    pDbCopyCtrl->ldctl_oid = LDAP_DB_COPY_CONTROL;
    pDbCopyCtrl->ldctl_iscritical = '1';

    if (ber_flatten2(pBer, &pDbCopyCtrl->ldctl_value, 1))
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_IO);
    }

cleanup:

    VMDIR_SAFE_FREE_BER(pBer);
    return retVal;

error:
    if (pDbCopyCtrl)
    {
        VmDirFreeCtrlContent(pDbCopyCtrl);
    }
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s, error %d", __FUNCTION__, retVal);
    goto cleanup;
}

int
VmDirCreateDBCopyReplyControlContent(
    int         fd,
    BerValue*   pBerVIn,
    BerValue*   pBerVOut
    )
{
    int         retVal = LDAP_SUCCESS;
    BerElement* pBer = NULL;
    BerValue    localBV = {0};

    if (!pBerVIn || !pBerVOut)
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if ((pBer = ber_alloc()) == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_NO_MEMORY);
    }

    if (ber_printf(pBer, "{iO}", fd, pBerVIn) == -1)
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_IO);
    }

    if (ber_flatten2(pBer, &localBV, 1))
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_IO);
    }

    retVal = VmDirAllocateAndCopyMemory(localBV.bv_val, localBV.bv_len, (PVOID*) &pBerVOut->bv_val);
    BAIL_ON_VMDIR_ERROR(retVal);

    pBerVOut->bv_len = localBV.bv_len;

cleanup:
    VMDIR_SAFE_FREE_BER(pBer);
    ber_memfree(localBV.bv_val);
    return retVal;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s, error %d", __FUNCTION__, retVal);
    goto cleanup;
}

DWORD
VmDirParseDBCopyReplyControlContent(
    LDAPControl*                pDbCopyReplyCtrl,
    PVDIR_DB_COPY_CONTROL_VALUE pDbCopyCtrlVal
    )
{
    BerElement*     pBer          = NULL;
    DWORD           dwError       = 0;
    BerValue        localBerValue = {0};
    ber_tag_t       berTag        = 0;

    if (!pDbCopyCtrlVal || !pDbCopyReplyCtrl)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pBer = ber_init(&pDbCopyReplyCtrl->ldctl_value);
    if (pBer == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_MEMORY);
    }

    berTag = ber_scanf(pBer, "{im}", &pDbCopyCtrlVal->fd, &localBerValue);
    if (berTag == LBER_ERROR)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_IO);
    }

    dwError = VmDirAllocateAndCopyMemory(
                localBerValue.bv_val,
                localBerValue.bv_len,
                (PVOID*)&pDbCopyCtrlVal->pszData);
    BAIL_ON_VMDIR_ERROR(dwError);

    pDbCopyCtrlVal->dwDataLen = localBerValue.bv_len;

cleanup:
    VMDIR_SAFE_FREE_BER(pBer);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s, error %d", __FUNCTION__, dwError);
    goto cleanup;
}
