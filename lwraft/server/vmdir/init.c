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

#define VDIR_CFG_ROOT_ENTRY_INITIALIZER     \
{                                           \
    "objectclass",  "vmwDirCfg",            \
    "cn",           "config",               \
    NULL                                    \
}

#define VDIR_OPEN_FILES_MAX 16384

static
DWORD
InitializeCFGEntries(
    PVDIR_SCHEMA_CTX    pSchemaCtx);

static
int
InitializeVmdirdSystemEntries(
    VOID);

static
DWORD
InitializeResouceLimit(
    VOID
    );

static
DWORD
InitializeServerStatusGlobals(
    VOID
    );

static
DWORD
InitializeGlobalVars(
    VOID
    );

static
DWORD
_VmDirWriteBackInvocationId(VOID);

static
DWORD
_VmDirGenerateInvocationId(VOID);

int
LoadServerGlobals(BOOLEAN *pbWriteInvocationId);

static
DWORD
_VmDirSrvCreatePersistedDSERoot(VOID);

/*
 * no krb function needed in lwraft, but we use gVmdirKrbGlobals.pszRealm in various places.
 */
DWORD
VmDirKrbInit(
    VOID
    )
{
    DWORD               dwError = 0;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    PVDIR_ATTRIBUTE     pAttrRootNamingCtx = NULL;
    BOOLEAN             bInLock = FALSE;

    dwError = VmDirSimpleEqualFilterInternalSearch(
                    PERSISTED_DSE_ROOT_DN,
                    LDAP_SCOPE_BASE,
                    ATTR_OBJECT_CLASS,
                    OC_DSE_ROOT,
                    &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (entryArray.iSize != 1
        ||
        ((pAttrRootNamingCtx = VmDirFindAttrByName(&(entryArray.pEntry[0]), ATTR_ROOT_DOMAIN_NAMING_CONTEXT)) == NULL))
    {
        goto cleanup; // not promoted yet
    }

    VMDIR_LOCK_MUTEX(bInLock, gVmdirKrbGlobals.pmutex);

    dwError = VmDirKrbSimpleDNToRealm( pAttrRootNamingCtx->vals, (PSTR*)(&gVmdirKrbGlobals.pszRealm));
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Set realm to (%s)", gVmdirKrbGlobals.pszRealm);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirKrbGlobals.pmutex);
    VmDirFreeEntryArrayContent(&entryArray);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirInitBackend(
    VOID
    )
{
    DWORD   dwError = 0;
    PVDIR_BACKEND_INTERFACE pBE = NULL;
    PVMDIR_MUTEX    pSchemaModMutex = NULL;
    BOOLEAN bInitializeEntries = FALSE;

    dwError = VmDirBackendConfig();
    BAIL_ON_VMDIR_ERROR(dwError);

    pBE = VmDirBackendSelect(NULL);
    assert(pBE);

    dwError = pBE->pfnBEInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    /*
     * Attribute indices are configured by attribute type entries.
     *
     * Note this implies that all index modification is schema modification.
     *
     * Concurrency control can be simplified by sharing mutex
     * between schema library and index library.
     */
    dwError = VmDirSchemaLibInit(&pSchemaModMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirIndexLibInit(pSchemaModMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLoadSchema(&bInitializeEntries);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLoadIndex();
    BAIL_ON_VMDIR_ERROR(dwError);

    // prepare USNList to guarantee safe USN for replication
    dwError = VmDirBackendInitUSNList(pBE);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bInitializeEntries)
    {
        dwError = _VmDirGenerateInvocationId(); // to be used in replication meta data for the entries created in
        BAIL_ON_VMDIR_ERROR(dwError);           // InitializeVmdirdSystemEntries()

        dwError = VmDirInitRaftPsState();
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = InitializeVmdirdSystemEntries();
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirSrvCreatePersistedDSERoot();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirInitBackend failed (%d)", dwError );
    goto cleanup;
}


/*
 * This function provides a mechanism to determine if lwraftd shutdown cleanly.
 * The logic is:
 * (1) At startup we check for a registry value called "DirtyShutdown". If
 * that value doesn't exist then this must be the first time we've started
 * up (so obviously we didn't experience a dirty shutdown previously).
 * (2) If the value exists then if its value is non-zero then we didn't
 * previously shutdown cleanly (because when we get to the end of our cleanup
 * code we'll set the value to zero explicitly before exiting).
 */
static
DWORD
VmDirCheckForDirtyShutdown(
    PBOOLEAN pbDirtyShutdown
    )
{
    DWORD dwDirtyShutdown = 0;
    DWORD dwError = 0;
    BOOLEAN bDirtyShutdown = FALSE;

    /*
     * Get the DirtyShutdown value (if it doesn't exist it's not dirty).
     */
    (VOID)VmDirGetRegKeyValueDword(
            VMDIR_CONFIG_PARAMETER_KEY_PATH,
            VMDIR_REG_KEY_DIRTY_SHUTDOWN,
            &dwDirtyShutdown,
            FALSE);
    bDirtyShutdown = !!dwDirtyShutdown;

    /*
     * Assume the worst and write out that we had a dirty shutdown. When we
     * cleanly shutdown we'll update this value.
     */
    dwError = VmDirSetRegKeyValueDword(
                VMDIR_CONFIG_PARAMETER_KEY_PATH,
                VMDIR_REG_KEY_DIRTY_SHUTDOWN,
                TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pbDirtyShutdown = bDirtyShutdown;

cleanup:
    return dwError;
error:
    goto cleanup;
}

/*
 * Initialize lwraftd components
 */
DWORD
VmDirInit(
    VOID
    )
{
    DWORD   dwError = 0;
    BOOLEAN bWriteInvocationId = FALSE;
    BOOLEAN bWaitTimeOut = FALSE;
    VMDIR_RUNMODE runMode = VmDirdGetRunMode();
    BOOLEAN bDirtyShutdown = FALSE;

    dwError = VmDirCheckForDirtyShutdown(&bDirtyShutdown);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = InitializeGlobalVars();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = InitializeServerStatusGlobals();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSuperLoggingInit(&gVmdirGlobals.pLogger);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMetricsInitialize();
    BAIL_ON_VMDIR_ERROR(dwError);

#ifndef _WIN32
    dwError = InitializeResouceLimit();
    BAIL_ON_VMDIR_ERROR(dwError);
#endif

    /* Override vmafd domain state to (1=infrastructure node) for gss plugin so that lwraftd
     * will use peer's identity in local ldap store for verifying peer's decrpc/srp binding.
     */
    if (setenv(VMDIR_ENV_OVERRIDE_AFD_DOMAIN_STATE, "1", 1) != 0)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirOpensslInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirPasswordSchemeInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMiddleLayerLibInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirPluginInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitBackend();
    BAIL_ON_VMDIR_ERROR(dwError);

    // load server globals before any write operations
    dwError = LoadServerGlobals(&bWriteInvocationId);
    if ( dwError == ERROR_BACKEND_ENTRY_NOTFOUND )
    {
        dwError = 0;    // vmdir not yet promoted
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirKrbInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirVmAclInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    if (runMode == VMDIR_RUNMODE_NORMAL)
    {
        dwError = VmDirRpcServerInit();
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirIpcServerInit();
        BAIL_ON_VMDIR_ERROR (dwError);

        dwError = VmDirReplicationLibInit();
        BAIL_ON_VMDIR_ERROR(dwError);

        if (gVmdirGlobals.bTrackLastLoginTime)
        {
            dwError = VmDirInitTrackLastLoginThread();
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    if (bWriteInvocationId) // Logic for backward compatibility. Needs to come after schema patch logic.
    {
        dwError = _VmDirWriteBackInvocationId();
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    //Will not free gVmdirGlobals.pPortListenSyncCounter since it maybe accessed when
    //  timeout occurred (e.g. waiting for promote) though there is a one time memory leak.
    dwError = VmDirAllocateSyncCounter( &gVmdirGlobals.pPortListenSyncCounter,
                                        2,      // ldap and ldaps - two ports
                                        SYNC_SIGNAL,
                                        5000);  // wait time 5 seconds
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!VmDirdGetRestoreMode())
    {
        dwError = VmDirInitConnAcceptThread();
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirRESTServerInit();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    //Wait only if there is not a vdcprome pending.
    dwError = VmDirSyncCounterWaitEvent(gVmdirGlobals.pPortListenSyncCounter, &bWaitTimeOut);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bWaitTimeOut)
    {
        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "%s: NOT all LDAP ports are ready for accepting services.", __func__);
    } else
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: all LDAP ports are ready for accepting services.", __func__);
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Config MaxLdapOpThrs (%d)", gVmdirGlobals.dwMaxFlowCtrlThr );

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s failed (%d)", __FUNCTION__, dwError );
    goto cleanup;
}

static
DWORD
_VmDirSrvCreatePersistedDSERoot(VOID)
{
    DWORD dwError = 0;
    PSTR ppszPersistedDSERootAttrs[] =
    {
            ATTR_OBJECT_CLASS,                  OC_DSE_ROOT,
            ATTR_OBJECT_CLASS,                  OC_TOP,
            ATTR_CN,                            PERSISTED_DSE_ROOT_NAMING_ATTR_VALUE,
            ATTR_SUPPORTED_LDAP_VERSION,        SUPPORTED_LDAP_VERSION,
            ATTR_INVOCATION_ID,                 gVmdirServerGlobals.invocationId.lberbv.bv_val,
            ATTR_SUPPORTED_CONTROL,             LDAP_CONTROL_SYNC,
            ATTR_SUPPORTED_CONTROL,             LDAP_CONTROL_SYNC_STATE,
            ATTR_SUPPORTED_CONTROL,             LDAP_CONTROL_SYNC_DONE,
            ATTR_SUPPORTED_CONTROL,             VDIR_LDAP_CONTROL_SHOW_DELETED_OBJECTS,
            ATTR_SUPPORTED_CONTROL,             LDAP_CONTROL_PAGEDRESULTS,
            ATTR_SERVER_VERSION,                VDIR_SERVER_VERSION,
            ATTR_PSC_VERSION,                   VDIR_PSC_VERSION,
            ATTR_SUPPORTED_SASL_MECHANISMS ,    SASL_MECH,
            NULL
    };
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEntryCreate(
                    pSchemaCtx,
                    ppszPersistedDSERootAttrs,
                    PERSISTED_DSE_ROOT_DN,
                    DSE_ROOT_ENTRY_ID);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirSrvCreatePersistedDSERoot failed (%d)", dwError );
    goto cleanup;
}

static
DWORD
InitializeServerStatusGlobals(
    VOID
    )
{
    DWORD   dwError = 0;

    gVmdirGlobals.iServerStartupTime = VmDirGetTimeInMilliSec();

    dwError = VmDirInitOPStatisticGlobals();
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "InitializeServerStatusGlobals failed (%d)", dwError );

    goto cleanup;
}

static
int
InitializeVmdirdSystemEntries(
    VOID)
{
    int     iError = 0;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;

    iError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(iError);

    iError = VmDirSchemaInitializeSubtree(pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(iError);

    iError = InitializeCFGEntries(pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(iError);

cleanup:
    VmDirSchemaCtxRelease(pSchemaCtx);
    return iError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
            "InitializeVmdirdSystemEntries failed (%d)", iError);
    goto cleanup;
}

static
DWORD
_VmDirGenerateInvocationId(VOID)
{
    DWORD           dwError = 0;
    uuid_t          guid = {0};
    VDIR_BERVALUE   bv = VDIR_BERVALUE_INIT;
    char            pszInvocationId[VMDIR_GUID_STR_LEN] = {0};
    PSTR            pszLocalErrMsg = NULL;

    if (VmDirUuidGenerate(&guid) != 0)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg, "_VmDirGenerateInvocationId: VmDirUuidGenerate() failed." );
    }
    if (VmDirUuidToStringLower( &guid, pszInvocationId, VMDIR_GUID_STR_LEN) != 0)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg, "_VmDirGenerateInvocationId: VmDirUuidToStringLower() failed." );
    }

    bv.lberbv.bv_val = pszInvocationId;
    bv.lberbv.bv_len = VmDirStringLenA( bv.lberbv.bv_val );
    if (VmDirBervalContentDup( &bv, &gVmdirServerGlobals.invocationId ) != 0)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg, "_VmDirGenerateInvocationId: VmDirBervalContentDup() failed." );
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Invocation ID: %s", gVmdirServerGlobals.invocationId.lberbv.bv_val);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, VDIR_SAFE_STRING(pszLocalErrMsg) );
    goto cleanup;
}

//
// LoadServerGlobals()
//
// Description:
//  - This function should always find DSE Root entry in the DB
//  - If DSE Root entry does NOT contain server object DN, that means the instance has NOT been promoted.
//  - If DSE Root entry does contain server object DN, there are 2 cases:
//    - It is the normal case i.e. the instance has been promoted and being restarted.
//    - There is an abnormal case when the DB was copied externally from a partner.
//      In this case DSE Root entry exists, and it contains partner's server object DN. In this case also instance has
//      NOT been promoted yet, and this is detected by the absence of the DC account password file (HACK to handle
//      the abnormal case).
//  - There is no special handling of any of these scenarios in the function, code just flows. Only in the last abnormal
//    case the new invocation ID needs to be generated here.


int
LoadServerGlobals(BOOLEAN *pbWriteInvocationId)
{
    DWORD               dwError = 0;
    VDIR_ENTRY          dseRoot = {0};
    VDIR_ENTRY          serverObj = {0};
    PVDIR_ATTRIBUTE     attr = NULL;
    VDIR_OPERATION      op = {0};
    VDIR_BERVALUE       bv = VDIR_BERVALUE_INIT;
    BOOLEAN             bHasTxn = FALSE;
    VDIR_BERVALUE       serverGuid = VDIR_BERVALUE_INIT;
    PSTR                pszLocalErrMsg = NULL;
    PSTR                pszDcAccountPwd = NULL;
    PSTR                pszServerName = NULL;
    DWORD               dwCurrentDfl = VDIR_DFL_DEFAULT;

    dwError = VmDirInitStackOperation( &op,
                                       VDIR_OPERATION_TYPE_INTERNAL,
                                       LDAP_REQ_SEARCH,
                                       NULL );
    BAIL_ON_VMDIR_ERROR(dwError);

    op.pBEIF = VmDirBackendSelect(PERSISTED_DSE_ROOT_DN);
    assert(op.pBEIF);

    dwError = op.pBEIF->pfnBETxnBegin( op.pBECtx, VDIR_BACKEND_TXN_READ);
    BAIL_ON_VMDIR_ERROR( dwError );
    bHasTxn = TRUE;

    dwError = op.pBEIF->pfnBEIdToEntry( op.pBECtx, op.pSchemaCtx, DSE_ROOT_ENTRY_ID, &dseRoot, VDIR_BACKEND_TXN_READ);
    BAIL_ON_VMDIR_ERROR( dwError );

    for (attr = dseRoot.attrs; attr; attr = attr->next)
    {
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_ROOT_DOMAIN_NAMING_CONTEXT, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.systemDomainDN ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed while copying system domain DN.");
            }
            if (VmDirNormalizeDN( &gVmdirServerGlobals.systemDomainDN, op.pSchemaCtx) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirNormalizeDN failed for system domain DN.");
            }

            // set DomainControllerGroupDN (NOTE, this is a hard code name, same as in instance.c)
            dwError = VmDirAllocateBerValueAVsnprintf(
                        &gVmdirServerGlobals.bvDCGroupDN,
                        "cn=%s,cn=%s,%s",
                        VMDIR_DC_GROUP_NAME,
                        VMDIR_BUILTIN_CONTAINER_NAME,
                        gVmdirServerGlobals.systemDomainDN.lberbv_val);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (VmDirNormalizeDN( &(gVmdirServerGlobals.bvDCGroupDN), op.pSchemaCtx) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirNormalizeDN failed for bvDCGroupDN.");
            }

            // set DCClientGroupDN (NOTE, this is a hard code name, same as in instance.c)
            dwError = VmDirAllocateBerValueAVsnprintf(
                        &gVmdirServerGlobals.bvDCClientGroupDN,
                        "cn=%s,cn=%s,%s",
                        VMDIR_DCCLIENT_GROUP_NAME,
                        VMDIR_BUILTIN_CONTAINER_NAME,
                        gVmdirServerGlobals.systemDomainDN.lberbv_val);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (VmDirNormalizeDN( &(gVmdirServerGlobals.bvDCClientGroupDN), op.pSchemaCtx) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirNormalizeDN failed for pszDCClientGroupDN.");
            }

            // set ServicesRootDN (NOTE, this is a hard code name, same as in instance.c)
            dwError = VmDirAllocateBerValueAVsnprintf(
                        &gVmdirServerGlobals.bvServicesRootDN,
                        "cn=%s,%s",
                        VMDIR_SERVICES_CONTAINER_NAME,
                        gVmdirServerGlobals.systemDomainDN.lberbv_val);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (VmDirNormalizeDN( &(gVmdirServerGlobals.bvServicesRootDN), op.pSchemaCtx) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirNormalizeDN failed for pszServicesRootDN.");
            }

            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_DEL_OBJS_CONTAINER, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.delObjsContainerDN ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed while copying deleted "
                                               "objects container DN.");
            }
            if (VmDirNormalizeDN( &gVmdirServerGlobals.delObjsContainerDN, op.pSchemaCtx) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirNormalizeDN failed for deleted objects container DN.");
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_SERVER_NAME, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.serverObjDN ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed while copying serverDN.");
            }
            if (VmDirNormalizeDN( &(gVmdirServerGlobals.serverObjDN), op.pSchemaCtx) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirNormalizeDN failed for server object DN.");
            }

            if (VmDirDnLastRDNToCn(attr->vals[0].lberbv_val, &pszServerName) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "%s: VmDirDnLastRDNToCn failed", __func__);
            }
            if (VmDirStringToBervalContent(pszServerName, &gVmdirServerGlobals.bvServerObjName) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "%s: VmDirStringToBervalContent failed", __func__);
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_DC_ACCOUNT_DN, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.dcAccountDN ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed while copying dcAccountDN.");
            }
            if (VmDirNormalizeDN( &(gVmdirServerGlobals.dcAccountDN), op.pSchemaCtx) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirNormalizeDN failed for dcAccountDN.");
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_DC_ACCOUNT_UPN, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.dcAccountUPN ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed while copying dcAccountUPN.");
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_INVOCATION_ID, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.invocationId ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed." );
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_DEFAULT_ADMIN_DN, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.bvDefaultAdminDN ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed." );
            }
            if (VmDirNormalizeDN( &(gVmdirServerGlobals.bvDefaultAdminDN), op.pSchemaCtx) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirNormalizeDN failed for bvDefaultAdminDN.");
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_SITE_NAME, FALSE) == 0)
        {
            dwError = VmDirAllocateStringA( attr->vals[0].lberbv.bv_val, &gVmdirServerGlobals.pszSiteName);
            BAIL_ON_VMDIR_ERROR(dwError);

            continue;
        }
    }

    if (gVmdirServerGlobals.serverObjDN.lberbv.bv_len == 0) // => not vdcpromed yet.
    {
        goto cleanup;
    }
    else
    { // Hack
        dwError = VmDirReadDCAccountPassword(&pszDcAccountPwd);
        if (dwError == ERROR_FILE_NOT_FOUND) // registry key not found => not vdcpromed yet.
        { // Replica server not configured/promoted yet.
          // This is the scenario where DB has been copied from the partner i.e. where gVmdirServerGlobals.serverObjDN
          // exists in the DSE Root entry exists but DC account password file does NOT exist.
          // (bit of an hack logic to detect this scenario). Generate invocation ID now.

            dwError = ERROR_BACKEND_ENTRY_NOTFOUND;

            // Server object will be updated with this invocation ID during vdcpromo. Also till
            // vdcpromo is done, values in gVmdirServerGlobals.serverObjDN, gVmdirServerGlobals.dcAccountDN, and
            // gVmdirServerGlobals.dcAccountUPN are that of this server's partner's and not it's own.
            if (_VmDirGenerateInvocationId() != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirGenerateInvocationId() failed.");
            }
        }
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    // Load Server object

    op.pBEIF = VmDirBackendSelect(gVmdirServerGlobals.serverObjDN.lberbv.bv_val);
    assert(op.pBEIF);

    if (VmDirNormalizeDN( &(gVmdirServerGlobals.serverObjDN), op.pSchemaCtx ) != 0)
    {
        dwError = VMDIR_ERROR_GENERIC;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                      "VmDirNormalizeDN for serverObjDN failed. ");
    }

    dwError = op.pBEIF->pfnBEDNToEntry(  op.pBECtx, op.pSchemaCtx, &(gVmdirServerGlobals.serverObjDN), &serverObj,
                                         VDIR_BACKEND_ENTRY_LOCK_READ );
    BAIL_ON_VMDIR_ERROR( dwError );

    bv.lberbv.bv_val = "";
    bv.lberbv.bv_len = 0;
    VmDirBervalContentDup( &bv, &gVmdirServerGlobals.utdVector );

    for (attr = serverObj.attrs; attr; attr = attr->next)
    {
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_SERVER_GUID, FALSE) == 0) // for data migration scenario
        {
            if (VmDirBervalContentDup( &attr->vals[0], &serverGuid ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed." );
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_INVOCATION_ID, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.invocationId ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed." );
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_REPL_INTERVAL, FALSE) == 0)
        {
            gVmdirServerGlobals.replInterval = atoi( attr->vals[0].lberbv.bv_val );
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_REPL_PAGE_SIZE, FALSE) == 0)
        {
            gVmdirServerGlobals.replPageSize = atoi( attr->vals[0].lberbv.bv_val );
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_UP_TO_DATE_VECTOR, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.utdVector ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed." );
            }
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "UpdateToDate Vector: (%s)", gVmdirServerGlobals.utdVector.lberbv_val);
            continue;
        }
    }
    if (gVmdirServerGlobals.invocationId.lberbv.bv_len == 0)
    {
        if (serverGuid.lberbv.bv_len == 0) // for data migration scenario: not even serverGuid is present.
        {
            dwError = VMDIR_ERROR_GENERIC;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                          "Server object entry does NOT contain invocationId attribute.");
        }
        if (VmDirBervalContentDup( &serverGuid, &gVmdirServerGlobals.invocationId ) != 0)
        {
            dwError = VMDIR_ERROR_GENERIC;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                          "BervalContentDup failed." );
        }
        *pbWriteInvocationId = TRUE;
    }
    else
    {
        *pbWriteInvocationId = FALSE;
    }

    dwError = op.pBEIF->pfnBETxnCommit( op.pBECtx );
    bHasTxn = FALSE;
    BAIL_ON_VMDIR_ERROR( dwError );

    // Set the domain functional level
    // TODO: update global when dfl is changed.
    dwError = VmDirSrvGetDomainFunctionalLevel(&dwCurrentDfl);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVmdirServerGlobals.dwDomainFunctionalLevel = dwCurrentDfl;

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Domain Functional Level (%d)",
                    gVmdirServerGlobals.dwDomainFunctionalLevel);

cleanup:

    VMDIR_SECURE_FREE_STRINGA(pszDcAccountPwd);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    VMDIR_SAFE_FREE_MEMORY(pszServerName);
    VmDirFreeEntryContent( &dseRoot );
    VmDirFreeEntryContent( &serverObj );
    VmDirFreeBervalContent(&serverGuid);

    VmDirFreeOperationContent(&op);

    return dwError;

error:

    if ( dwError != ERROR_BACKEND_ENTRY_NOTFOUND )
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "LoadServerGlobals: (%u)(%s)",
                         dwError, VDIR_SAFE_STRING(pszLocalErrMsg));
    }

    if (bHasTxn)
    {
        op.pBEIF->pfnBETxnAbort( op.pBECtx );
    }

    goto cleanup;
}

static
DWORD
_VmDirWriteBackInvocationId(VOID)
{
    DWORD                   dwError = LDAP_SUCCESS;
    PSTR                    pszLocalErrMsg = NULL;

    dwError = VmDirInternalEntryAttributeReplace( NULL, gVmdirServerGlobals.serverObjDN.bvnorm_val, ATTR_INVOCATION_ID,
                                                  &(gVmdirServerGlobals.invocationId));
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                "_VmDirWriteBackInvocationId: VmDirInternalEntryAttributeReplace failed. DN: %s, Error code: %d",
                gVmdirServerGlobals.serverObjDN.bvnorm_val, dwError );

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, VDIR_SAFE_STRING(pszLocalErrMsg) );
    goto cleanup;
}

/*
 * Create default config tree entries
 * 1. cn=config
 * Called only during the very first time server startup to give default
 *      content to config DIT entries.
 */
static
DWORD
InitializeCFGEntries(
    PVDIR_SCHEMA_CTX    pSchemaCtx)
{
    DWORD   dwError = 0;
    static PSTR ppszCFG_ROOT[] = VDIR_CFG_ROOT_ENTRY_INITIALIZER;

    dwError = VmDirSimpleEntryCreate(
            pSchemaCtx,
            ppszCFG_ROOT,
            CFG_ROOT_DN,
            CFG_ROOT_ENTRY_ID);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );
    goto cleanup;
}

/*
 * Set process resource limits
 */
static
DWORD
InitializeResouceLimit(
    VOID
    )
{
    DWORD           dwError = 0;
    BAIL_ON_VMDIR_ERROR(dwError);

#ifndef _WIN32
    struct rlimit   VMLimit = {0};

    // unlimited resource
    VMLimit.rlim_cur = RLIM_INFINITY;
    VMLimit.rlim_max = RLIM_INFINITY;

    if ( setrlimit(RLIMIT_AS, &VMLimit)     // virtual memory
         ||
         setrlimit(RLIMIT_CORE, &VMLimit)   // core file size
         ||
         setrlimit(RLIMIT_NPROC, &VMLimit)  // thread
       )
    {
        dwError = ERROR_INVALID_CONFIGURATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMLimit.rlim_cur = VDIR_OPEN_FILES_MAX;
    VMLimit.rlim_max = VDIR_OPEN_FILES_MAX;
    if (setrlimit(RLIMIT_NOFILE, &VMLimit)!=0)
    {
       //If VDIR_OPEN_FILES_MAX is too large to set, try to set soft limit to hard limit
       if (getrlimit(RLIMIT_NOFILE, &VMLimit)==0)
       {
            VMLimit.rlim_cur = VMLimit.rlim_max;
            if (setrlimit(RLIMIT_NOFILE, &VMLimit)!=0)
            {
                VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                    "Fail to increase RLIMIT_NOFILE - keep the default value.");
            }
       } else
       {
           VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                    "Fail to obtain current RLIMIT_NOFILE - keep the default value.");

       }
    }

#endif

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "InitializeResouceLimit failed (%d)(%d)", dwError, errno );

    goto cleanup;
}


static
DWORD
InitializeGlobalVars(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = VmDirAllocateMutex(&gVmdirRunmodeGlobals.pMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVmdirGlobals.mutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVmdirGlobals.replAgrsMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&gVmdirGlobals.replAgrsCondition);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVmdirGlobals.replCycleDoneMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&gVmdirGlobals.replCycleDoneCondition);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVmdirKrbGlobals.pmutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&gVmdirKrbGlobals.pcond);
    BAIL_ON_VMDIR_ERROR(dwError);

    // LDAP operation threads shutdown synchronization, shutdown continue
    // when count == 0 (i.e. all op thrs are done)
    dwError = VmDirAllocateSyncCounter( &gVmdirGlobals.pOperationThrSyncCounter,
                                        0,
                                        SYNC_SIGNAL,
                                        10000);  // wait time 10 seconds
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVmdirGlobals.pMutexIPCConnection);
    BAIL_ON_VMDIR_ERROR(dwError)

    dwError = VmDirAllocateMutex(&gVmdirGlobals.pFlowCtrlMutex);
    BAIL_ON_VMDIR_ERROR(dwError);


    if (gVmdirGlobals.bTrackLastLoginTime)
    {
        dwError = VmDirAllocateMutex(&gVmdirTrackLastLoginTime.pMutex);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateCondition(&gVmdirTrackLastLoginTime.pCond);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateTSStack(8, &(gVmdirTrackLastLoginTime.pTSStack));
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "InitializeGlobalVars failed (%d)", dwError );

    goto cleanup;
}

/*
 * Lookup servers topology internally first. Then one of the servers
 * will be used to query uptoupdate servers topology
 */
DWORD
VmDirGetHostsInternal(
    PSTR**  pppszServerInfo,
    size_t* pdwInfoCount
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PSTR    pszSearchBaseDN = NULL;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    PVDIR_ATTRIBUTE     pAttr = NULL;
    PSTR*   ppszServerInfo = NULL;

    dwError = VmDirAllocateStringPrintf(
            &pszSearchBaseDN,
            "cn=Sites,cn=Configuration,%s",
            gVmdirServerGlobals.systemDomainDN.bvnorm_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEqualFilterInternalSearch(
            pszSearchBaseDN,
            LDAP_SCOPE_SUBTREE,
            ATTR_OBJECT_CLASS,
            OC_DIR_SERVER,
            &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (entryArray.iSize == 0)
    {
        dwError = LDAP_NO_SUCH_OBJECT;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(PSTR) * (entryArray.iSize+1),
            (PVOID*)&ppszServerInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i=0; i<entryArray.iSize; i++)
    {
         pAttr = VmDirEntryFindAttribute(ATTR_CN, entryArray.pEntry+i);
         dwError = VmDirAllocateStringA(pAttr->vals[0].lberbv.bv_val, &ppszServerInfo[i]);
         BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pppszServerInfo = ppszServerInfo;
    *pdwInfoCount = entryArray.iSize;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszSearchBaseDN);
    VmDirFreeEntryArrayContent(&entryArray);
    return dwError;

error:
    VmDirFreeStrArray(ppszServerInfo);
    goto cleanup;
}

DWORD
VmDirAllocateBerValueAVsnprintf(
    PVDIR_BERVALUE pbvValue,
    PCSTR pszFormat,
    ...
    )
{
    DWORD dwError = 0;
    PSTR pszValue = NULL;
    va_list args;

    va_start(args, pszFormat);
    dwError = VmDirVsnprintf(&pszValue, pszFormat, args);
    va_end(args);

    BAIL_ON_VMDIR_ERROR(dwError);

    pbvValue->lberbv_val = pszValue;
    pbvValue->lberbv_len = VmDirStringLenA(pszValue);
    pbvValue->bOwnBvVal = TRUE;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszValue);
    goto cleanup;
}
