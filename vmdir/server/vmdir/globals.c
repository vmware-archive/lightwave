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
 * Module Name: Directory main
 *
 * Filename: globals.c
 *
 * Abstract:
 *
 * Globals
 *
 */

#include "includes.h"

VMDIRD_STATE_GLOBALS gVmdirdStateGlobals =
{
    VMDIR_SF_INIT(.pMutex, NULL),
    VMDIR_SF_INIT(.vmdirdState, VMDIRD_STATE_UNDEFINED),
    VMDIR_SF_INIT(.targetState, VMDIRD_STATE_NORMAL),
};

VMDIR_GLOBALS gVmdirGlobals =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMDIR_SF_INIT(.pszBootStrapSchemaFile, NULL),
        VMDIR_SF_INIT(.bPatchSchema, FALSE),
        VMDIR_SF_INIT(.pszBDBHome, NULL),
        VMDIR_SF_INIT(.bAllowInsecureAuth, 0),
        VMDIR_SF_INIT(.bAllowAdminLockout, 0),
        VMDIR_SF_INIT(.bDisableVECSIntegration, 0),
        VMDIR_SF_INIT(.bEnableRegionalMaster, 0),
        VMDIR_SF_INIT(.pdwLdapListenPorts, NULL),
        VMDIR_SF_INIT(.dwLdapListenPorts, 0),
        VMDIR_SF_INIT(.pdwLdapsListenPorts, NULL),
        VMDIR_SF_INIT(.dwLdapsListenPorts, 0),
        VMDIR_SF_INIT(.pdwLdapConnectPorts, NULL),
        VMDIR_SF_INIT(.dwLdapConnectPorts, 0),
        VMDIR_SF_INIT(.pdwLdapsConnectPorts, NULL),
        VMDIR_SF_INIT(.dwLdapsConnectPorts, 0),
        VMDIR_SF_INIT(.dwHTTPListenPort, 0),
        VMDIR_SF_INIT(.dwHTTPSListenPort, 0),
        VMDIR_SF_INIT(.dwHTTPSApiListenPort, 0),
        VMDIR_SF_INIT(.dwLdapRecvTimeoutSec, 0),
        VMDIR_SF_INIT(.dwLdapConnectTimeoutSec, 0),
        VMDIR_SF_INIT(.dwOperationsThreadTimeoutInMilliSec, 0),
        VMDIR_SF_INIT(.dwReplConsumerThreadTimeoutInMilliSec, 0),
        VMDIR_SF_INIT(.dwEmptyPageCnt, 0),
        VMDIR_SF_INIT(.dwSupplierThrTimeoutInMilliSec, 0),
        VMDIR_SF_INIT(.dwWriteTimeoutInMilliSec, 0),
        VMDIR_SF_INIT(.mutex, NULL),
        VMDIR_SF_INIT(.pSrvThrInfo, NULL),
        VMDIR_SF_INIT(.bReplNow, FALSE),
#if !defined(_WIN32) || defined(HAVE_DCERPC_WIN32)
        VMDIR_SF_INIT(.pRPCServerThread, NULL),
#endif
#ifdef _WIN32
        VMDIR_SF_INIT(.hStopServiceEvent, 0),
#endif
        VMDIR_SF_INIT(.bRegisterTcpEndpoint, TRUE),
        VMDIR_SF_INIT(.replAgrsMutex, NULL),
        VMDIR_SF_INIT(.replAgrsCondition, NULL),
        VMDIR_SF_INIT(.replCycleDoneMutex, NULL),
        VMDIR_SF_INIT(.replCycleDoneCondition, NULL),
        VMDIR_SF_INIT(.dwReplCycleCounter, 0),
        VMDIR_SF_INIT(.limitLocalUsnToBeSupplied, 0),
        VMDIR_SF_INIT(.pOperationThrSyncCounter, NULL),
        VMDIR_SF_INIT(.pPortListenSyncCounter, NULL),
        VMDIR_SF_INIT(.pIPCSrvThrInfo, NULL),
        VMDIR_SF_INIT(.pIPCConn, NULL),
        VMDIR_SF_INIT(.bIPCShutdown, FALSE),
        VMDIR_SF_INIT(.pFlowCtrlMutex, NULL),
        VMDIR_SF_INIT(.dwMaxFlowCtrlThr, 1024),
        VMDIR_SF_INIT(.pLogger, NULL),
        VMDIR_SF_INIT(.iServerStartupTime, 0),
        VMDIR_SF_INIT(.dwMaxIndexScan, 512),
        VMDIR_SF_INIT(.dwSmallCandidateSet,32),
        VMDIR_SF_INIT(.dwMaxSizelimitScan,0),
        VMDIR_SF_INIT(.dwLdapSearchTimeoutSec, 0),
        VMDIR_SF_INIT(.bAllowImportOpAttrs, FALSE),
        VMDIR_SF_INIT(.bTrackLastLoginTime, FALSE),
        VMDIR_SF_INIT(.bPagedSearchReadAhead, FALSE),
        VMDIR_SF_INIT(.dwLdapWrites, 0),
        VMDIR_SF_INIT(.gpVdirSslCtx, NULL)
    };

VMDIR_KRB_GLOBALS gVmdirKrbGlobals =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...,
        VMDIR_SF_INIT(.pmutex, NULL),
        VMDIR_SF_INIT(.pcond, NULL),
        VMDIR_SF_INIT(.pszRealm, NULL),
        VMDIR_SF_INIT(.pszDomainDN, NULL),
        VMDIR_SF_INIT(.bervMasterKey, VDIR_BERVALUE_INIT),
        VMDIR_SF_INIT(.bTryInit, FALSE)
    };

VMDIR_OP_PLUGIN_GLOBALS gVmdirPluginGlobals =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMDIR_SF_INIT(.pPreAddPluginInfo, NULL),
        VMDIR_SF_INIT(.pPostAddCommitPluginInfo, NULL),
        VMDIR_SF_INIT(.pPreModApplyModifyPluginInfo, NULL),
        VMDIR_SF_INIT(.pPreModifyPluginInfo, NULL),
        VMDIR_SF_INIT(.pPostModifyCommitPluginInfo, NULL),
        VMDIR_SF_INIT(.pPreModApplyDeletePluginInfo, NULL)
    };

VMDIR_SERVER_GLOBALS gVmdirServerGlobals =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMDIR_SF_INIT(.invocationId, VDIR_BERVALUE_INIT),
        VMDIR_SF_INIT(.bvDefaultAdminDN, VDIR_BERVALUE_INIT),
        VMDIR_SF_INIT(.serverId, 0),
        VMDIR_SF_INIT(.systemDomainDN, VDIR_BERVALUE_INIT),
        VMDIR_SF_INIT(.delObjsContainerDN, VDIR_BERVALUE_INIT),
        VMDIR_SF_INIT(.bvDCGroupDN, VDIR_BERVALUE_INIT),
        VMDIR_SF_INIT(.bvDCClientGroupDN, VDIR_BERVALUE_INIT),
        VMDIR_SF_INIT(.bvSchemaManagersGroupDN, VDIR_BERVALUE_INIT),
        VMDIR_SF_INIT(.bvServicesRootDN, VDIR_BERVALUE_INIT),
        VMDIR_SF_INIT(.serverObjDN, VDIR_BERVALUE_INIT),
        VMDIR_SF_INIT(.dcAccountDN, VDIR_BERVALUE_INIT),
        VMDIR_SF_INIT(.dcAccountUPN, VDIR_BERVALUE_INIT),
        VMDIR_SF_INIT(.replInterval, -1),
        VMDIR_SF_INIT(.replPageSize, -1),
        VMDIR_SF_INIT(.pUtdVectorCache, NULL),
        VMDIR_SF_INIT(.pszSiteName, NULL),
        VMDIR_SF_INIT(.isIPV4AddressPresent, FALSE),
        VMDIR_SF_INIT(.isIPV6AddressPresent, FALSE),
        VMDIR_SF_INIT(.initialNextUSN, 0),
        VMDIR_SF_INIT(.bvServerObjName, VDIR_BERVALUE_INIT),
        VMDIR_SF_INIT(.dwDomainFunctionalLevel, VDIR_DFL_DEFAULT),
        VMDIR_SF_INIT(.dwTombstoneExpirationPeriod, 0),
        VMDIR_SF_INIT(.dwTombstoneThreadFrequency, 0),
        VMDIR_SF_INIT(.dwMaxInternalSearchLimit, 0),
        VMDIR_SF_INIT(.dwEfficientReadOpTimeMS, 0),
        VMDIR_SF_INIT(.bPromoted, FALSE),
        VMDIR_SF_INIT(.pReplDeadlockDetectionVector, NULL),
    };

VMDIRD_SD_GLOBALS gVmdirdSDGlobals =
    {
        VMDIR_SF_INIT(.pSDdcAdminGX, NULL),
    };

VMDIR_REPLICATION_AGREEMENT * gVmdirReplAgrs = NULL;

VMDIR_TRACK_LAST_LOGIN_TIME gVmdirTrackLastLoginTime =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMDIR_SF_INIT(.pMutex, NULL),
        VMDIR_SF_INIT(.pCond, NULL),
        VMDIR_SF_INIT(.pTSStack, NULL)
    };

VMDIR_INTEGRITY_CHECK_GLOBALS gVmdirIntegrityCheck =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMDIR_SF_INIT(.pMutex, NULL),
        VMDIR_SF_INIT(.pJob, NULL)
    };

VMDIR_BKGD_GLOBALS gVmdirBkgdGlobals =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMDIR_SF_INIT(.pThrInfo, NULL),
        VMDIR_SF_INIT(.bShutdown, FALSE)
    };

VMDIR_SERVER_OPERATIONS_GLOBALS gVmDirServerOpsGlobals =
    {
        VMDIR_SF_INIT(.pMutex, NULL),
        VMDIR_SF_INIT(.maxCommittedUSN, 0),
        VMDIR_SF_INIT(.pWriteQueue, NULL)
    };

PVM_METRICS_HISTOGRAM gpRpcMetrics[METRICS_RPC_OP_COUNT];

PVM_METRICS_GAUGE     gpSrvStatMetrics[METRICS_SRV_STAT_COUNT];

PVM_METRICS_GAUGE     gpLwGitHashMetrics;
