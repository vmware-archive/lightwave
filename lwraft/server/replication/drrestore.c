/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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
 * Module Name: Replication
 *
 * Filename: drrestore.c
 *
 * Abstract: DR node restore from backup DB
 *
 */

#include "includes.h"

static
DWORD
_VmDirDeleteDCObject(
    VOID
    );

static
DWORD
_VmDirPingLDAPPort(
    DWORD   dwIPFamily
    );

static
DWORD
_VmDirStartLDAPListener(
    VOID
    );

/* This function re-instantiates the current vmdir instance with a
 * foreign (MDB) database file.
 *
 * 1. foreign DB should be copy to /var/lib/vmware/vmdir/partner directory
 * 2. re-initialize backend with foreign DB (this include schema and index reload)
 * 3. clean up foreign DB
 *    a) delete raft cluster members
 * 4. reload gRaftState from foreign DB
 */
DWORD
VmDirSrvServerReset(
    PDWORD pServerResetState
    )
{
    DWORD dwError = 0;
    const char  *dbHomeDir = LWRAFT_DB_DIR;
    BOOLEAN     bWriteInvocationId = FALSE;
    BOOLEAN     bMdbWalEnable = FALSE;
    BOOLEAN     bFatalError = FALSE;

    VmDirGetMdbWalEnable(&bMdbWalEnable);

    VmDirShutdownDB();

    VmDirMetricsShutdown();

    //swap current vmdir database file with the foriegn one under partner/
    dwError = VmDirSwapDB(dbHomeDir, bMdbWalEnable);
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s Swap DB", __FUNCTION__);

    dwError = VmDirLoadRaftState();
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s Load Raft State", __FUNCTION__);

    dwError = VmDirDeleteAllLogs(gRaftState.commitIndex+1, &bFatalError);
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s Delete uncommitted raft logs > %llu", __FUNCTION__, gRaftState.commitIndex);

    dwError = _VmDirDeleteDCObject();
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s Delete DC objects", __FUNCTION__);

    dwError = LoadServerGlobals(&bWriteInvocationId);
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s Load Server Globals", __FUNCTION__);

    dwError = _VmDirStartLDAPListener();
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s LDAP port is up", __FUNCTION__);

    dwError = VmDirMetricsInitialize();
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirPingLDAPPort(
    DWORD   dwIPFamily
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    BOOLEAN bInReplCycleDoneLock = FALSE;

    for (dwCnt = 0; dwCnt<10; dwCnt++)
    {
        VMDIR_LOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);
        VmDirConditionSignal(gVmdirGlobals.replCycleDoneCondition);
        VMDIR_UNLOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);

        if (dwIPFamily == AF_INET && VmDirPingIPV4AcceptThr(VmDirGetLdapPort()) == 0)
        {
            VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s IPV4 LDAP port is up", __FUNCTION__);
            break;
        }
        else if (dwIPFamily == AF_INET6 && VmDirPingIPV6AcceptThr(VmDirGetLdapPort()) == 0)
        {
            VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s IPV6 LDAP port is up", __FUNCTION__);
            break;
        }

        VmDirSleep(1000);
    }

    if (dwCnt==10)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_IO);
    }

error:
    VMDIR_UNLOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);

    return dwError;
}

static
DWORD
_VmDirStartLDAPListener(
    VOID
    )
{
    DWORD   dwError = 0;
    BOOLEAN bIPV4 = FALSE;
    BOOLEAN bIPV6 = FALSE;

    dwError = VmDirInitConnAcceptThread();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirWhichAddressPresent(&bIPV4, &bIPV6);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bIPV4)
    {
        dwError = _VmDirPingLDAPPort(AF_INET);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (bIPV6)
    {
        dwError = _VmDirPingLDAPPort(AF_INET6);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

// remove all Raft members under ou=Raft Clusters,SYSTEM_DOMAIN
static
DWORD
_VmDirDeleteDCObject(
    VOID
    )
{
    DWORD   dwError = 0;
    int     i = 0;
    PSTR    pszRaftMmemberContainerDN = NULL;
    VDIR_ENTRY_ARRAY entryArray = {0};

    dwError = VmDirAllocateStringPrintf(&
        pszRaftMmemberContainerDN,
        "ou=%s,%s",
        VMDIR_DOMAIN_CONTROLLERS_RDN_VAL,
        gVmdirServerGlobals.systemDomainDN.lberbv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEqualFilterInternalSearch(
        pszRaftMmemberContainerDN,
        LDAP_SCOPE_ONE,
        ATTR_OBJECT_CLASS,
        OC_COMPUTER,
        &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (entryArray.iSize > 0)
    {
        for (i = 0; i < entryArray.iSize; i++)
        {
            dwError = VmDirDeleteEntry(&entryArray.pEntry[i]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    VMDIR_SAFE_FREE_MEMORY(pszRaftMmemberContainerDN);

    return dwError;

error:
    goto cleanup;
}
