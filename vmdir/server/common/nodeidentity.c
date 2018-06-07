/*
 * Copyright © 218 VMware, Inc.  All Rights Reserved.
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
VmDirUpdateMaxServerId(
    PVDIR_SCHEMA_CTX    pSchemaCtx
    );

DWORD
VmDirSrvCreateServerObj(
    PVDIR_SCHEMA_CTX pSchemaCtx)
{
    DWORD dwError = 0;
    PSTR  ppszServerObjAttrs[] =
    {
            ATTR_OBJECT_CLASS,                  OC_DIR_SERVER,
            ATTR_OBJECT_CLASS,                  OC_TOP,
            ATTR_CN,                            NULL,
            ATTR_SERVER_ID,                     NULL,
            ATTR_INVOCATION_ID,                 NULL,
            ATTR_REPL_INTERVAL,                 NULL,
            ATTR_REPL_PAGE_SIZE,                NULL,
            NULL
    };

#define HOST_NAME_VAL_IND       5
#define SERVER_ID_VAL_IND       7
#define INVOCATION_ID_VAL_IND   9
#define REPL_INTERVAL_VAL_IND   11
#define REPL_PAGE_SIZE_VAL_IND  13

    char            pszHostName[VMDIR_MAX_HOSTNAME_LEN] = {0};
    CHAR            pszServerId[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    VDIR_BERVALUE   serversContainerDN = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE   siteContainerDN = VDIR_BERVALUE_INIT;
    PCSTR           pszServersContainerName = VMDIR_SERVERS_CONTAINER_NAME;

    // Try to create Site-Name and Servers container, ignore if already exists.

    dwError = VmDirGetParentDN( &(gVmdirServerGlobals.serverObjDN), &serversContainerDN );
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = VmDirGetParentDN( &(serversContainerDN), &siteContainerDN );
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = VmDirSrvCreateContainer( pSchemaCtx, siteContainerDN.lberbv.bv_val, gVmdirServerGlobals.pszSiteName );
    switch (dwError)
    {
        case VMDIR_SUCCESS:

            assert(gVmdirServerGlobals.bvDCClientGroupDN.lberbv_val);

            // allow DCClients group to read site container (to get siteGUID during client join)
            dwError = VmDirAppendAllowAceForDn(
                        siteContainerDN.lberbv.bv_val,
                        gVmdirServerGlobals.bvDCClientGroupDN.lberbv_val,
                        VMDIR_RIGHT_DS_READ_PROP);
            BAIL_ON_VMDIR_ERROR(dwError);

            // Create Servers container
            dwError = VmDirSrvCreateContainer( pSchemaCtx, serversContainerDN.lberbv.bv_val, pszServersContainerName );
            BAIL_ON_VMDIR_ERROR(dwError);
            break;

        case VMDIR_ERROR_ENTRY_ALREADY_EXIST:
        case VMDIR_ERROR_BACKEND_ENTRY_EXISTS:
            break;

        default:
            BAIL_ON_VMDIR_ERROR(dwError);
            break;
    }

    // vdcpromo sets this key.
    dwError = VmDirGetRegKeyValue( VMDIR_CONFIG_PARAMETER_KEY_PATH,
                                   VMDIR_REG_KEY_DC_ACCOUNT,
                                   pszHostName,
                                   sizeof(pszHostName)-1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA( pszServerId, sizeof(pszServerId), "%d", gVmdirServerGlobals.serverId );
    BAIL_ON_VMDIR_ERROR(dwError);

    ppszServerObjAttrs[HOST_NAME_VAL_IND] = pszHostName;
    ppszServerObjAttrs[SERVER_ID_VAL_IND] = pszServerId;
    ppszServerObjAttrs[INVOCATION_ID_VAL_IND] = gVmdirServerGlobals.invocationId.lberbv.bv_val;
    ppszServerObjAttrs[REPL_INTERVAL_VAL_IND] = VMDIR_DEFAULT_REPL_INTERVAL;
    ppszServerObjAttrs[REPL_PAGE_SIZE_VAL_IND] = VMDIR_DEFAULT_REPL_PAGE_SIZE;

    dwError = VmDirSimpleEntryCreate(pSchemaCtx, ppszServerObjAttrs, gVmdirServerGlobals.serverObjDN.lberbv.bv_val, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    // update ATTR_MAX_SERVER_ID attribute of the system domain object.
    dwError = VmDirUpdateMaxServerId(pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Server ID (%d), InvocationID (%s)",
                                        gVmdirServerGlobals.serverId,
                                        gVmdirServerGlobals.invocationId.lberbv_val);

cleanup:
    VmDirFreeBervalContent( &serversContainerDN );
    VmDirFreeBervalContent( &siteContainerDN );
    return dwError;

error:
    goto cleanup;
}

// Update global server ID, so next call to VmDirSrvCreateServerObj will have new value
DWORD
VmDirSetGlobalServerId(
    VOID
    )
{
    DWORD           dwError = 0;
    PVDIR_ENTRY     pEntry = NULL;
    PVDIR_ATTRIBUTE pMaxServerId = NULL;

    dwError = VmDirSimpleDNToEntry(gVmdirServerGlobals.systemDomainDN.bvnorm_val, &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMaxServerId = VmDirEntryFindAttribute(ATTR_MAX_SERVER_ID, pEntry);
    assert( pMaxServerId != NULL );

    gVmdirServerGlobals.serverId = atoi(pMaxServerId->vals[0].lberbv.bv_val) + 1;

cleanup:
    if (pEntry)
    {
        VmDirFreeEntry(pEntry);
    }
    return dwError;

error:
    goto cleanup;
}

// Update vmwMaxServerId attribute in the system domain object.
static
DWORD
VmDirUpdateMaxServerId(
    PVDIR_SCHEMA_CTX    pSchemaCtx
    )
{
    DWORD           dwError = 0;
    VDIR_BERVALUE   berMaxServerId = VDIR_BERVALUE_INIT;
    CHAR            pszMaxServerId[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};

    dwError = VmDirStringPrintFA( pszMaxServerId, sizeof(pszMaxServerId), "%d", gVmdirServerGlobals.serverId );
    BAIL_ON_VMDIR_ERROR(dwError);

    berMaxServerId.lberbv.bv_val = pszMaxServerId;
    berMaxServerId.lberbv.bv_len = VmDirStringLenA(pszMaxServerId);

    dwError = VmDirInternalEntryAttributeReplace( pSchemaCtx, BERVAL_NORM_VAL(gVmdirServerGlobals.systemDomainDN),
                                                  ATTR_MAX_SERVER_ID, &berMaxServerId );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}
