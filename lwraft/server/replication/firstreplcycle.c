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
 * Module Name: Replication
 *
 * Filename: firstreplcycle.c
 *
 * Abstract: First replication cycle being implemented by copying the DB from partner, and "patching" it.
 *
 */

#include "includes.h"

static
int
_VmDirSwapDBInternal(
    PCSTR   pszdbHomeDir,   // e.g. /var/lib/vmware/post
    PCSTR   pszSwapDir      // e.g. /var/lib/vmware/lightwave_tmp
    );

int
VmDirFirstReplicationCycle(
    PSTR pszHostname
    )
{
    int     retVal = LDAP_SUCCESS;
    PSTR    pszLocalErrorMsg = NULL;

    VmDirBkgdThreadShutdown();

    VmDirMetricsShutdown();

    //Shutdown local database
    VmDirShutdownDB();

    retVal = VmDirCopyRemoteDB(pszHostname);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal,
            pszLocalErrorMsg,
            "VmDirFirstReplicationCycle: _VmDirGetRemoteDBsUsingRPC() call failed with error: %d",
            retVal);

    retVal = VmDirSwapDBs();
    BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal,
            pszLocalErrorMsg,
            "VmDirFirstReplicationCycle: _VmDirSwapDB() call failed, error: %d.",
            retVal);

    retVal = VmDirMetricsInitialize();
    BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal,
            pszLocalErrorMsg,
            "VmDirFirstReplicationCycle: VmDirMetricsInitialize call failed, error: %d.",
            retVal);

    retVal = VmDirBkgdThreadInitialize();
    BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal,
            pszLocalErrorMsg,
            "VmDirFirstReplicationCycle: VmDirBkgdThreadInitialize call failed, error: %d.",
            retVal);

    VMDIR_LOG_INFO(
            VMDIR_LOG_MASK_ALL,
            "Remote DB copied from %s, and swapped successfully",
            pszHostname);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s",
            VDIR_SAFE_STRING(pszLocalErrorMsg));

    goto cleanup;
}

/**
 * @ VmDirShutdownDB()
 * shutdown the current backend
 * @return VOID
 */
VOID
VmDirShutdownDB(
    VOID
    )
{
    VmDirdStateSet(VMDIRD_STATE_SHUTDOWN);

    // in DR case, stop listening thread.
    // in Join case, listening thread is not in listen mode yet.
    VmDirShutdownConnAcceptThread();

    VmDirSchemaLibShutdown();

    VmDirIndexLibShutdown();

    VmDirShutdownAndFreeAllBackends();
}

int
VmDirSwapDBs(
    VOID
    )
{
    DWORD retVal = 0;

    retVal = _VmDirSwapDBInternal(LWRAFT_DB_DIR, LIGHTWAVE_TMP_DIR);
    BAIL_ON_VMDIR_ERROR(retVal);

    VmDirdStateSet(VMDIRD_STATE_STARTUP);

    retVal = VmDirInitBackend();
    BAIL_ON_VMDIR_ERROR(retVal);

    VmDirdStateSet(VMDIRD_STATE_NORMAL);

error:
    return retVal;
}

static
int
_VmDirSwapDBInternal(
    PCSTR   pszdbHomeDir,   // e.g. /var/lib/vmware/post
    PCSTR   pszSwapDir      // e.g. /var/lib/vmware/lightwave_tmp
    )
{
    int     retVal = 0;
    int     errorCode = 0;
    CHAR    cmdBuf[VMDIR_MAX_FILE_NAME_LEN] = {0};
    CHAR    partnerDBdBuf[VMDIR_MAX_FILE_NAME_LEN] = {0};
    CHAR    swapDirBuf[VMDIR_MAX_FILE_NAME_LEN] = {0};

    // /var/lib/vmware/lightwave_tmp/partner
    retVal = VmDirStringPrintFA(swapDirBuf, VMDIR_MAX_FILE_NAME_LEN,
        "%s/%s", pszSwapDir, LOCAL_PARTNER_DIR);
    BAIL_ON_VMDIR_ERROR(retVal);

    // /var/lib/vmware/post/partner
    retVal = VmDirStringPrintFA(partnerDBdBuf, VMDIR_MAX_FILE_NAME_LEN,
        "%s/%s", pszdbHomeDir, LOCAL_PARTNER_DIR);
    BAIL_ON_VMDIR_ERROR(retVal);

    // rm -rf /var/lib/post/lightwave_tmp/partner
    retVal = VmDirStringPrintFA(cmdBuf, VMDIR_MAX_FILE_NAME_LEN,
        "rm -rf %s", swapDirBuf);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirRun(cmdBuf);
    BAIL_ON_VMDIR_ERROR(retVal);

    // mv /var/lib/vmware/post/partner /var/lib/vmware/lightwave_tmp/partner
    retVal = VmDirStringPrintFA(cmdBuf, VMDIR_MAX_FILE_NAME_LEN,
        "mv %s %s", partnerDBdBuf, swapDirBuf);
    BAIL_ON_VMDIR_ERROR(retVal);
    retVal = VmDirRun(cmdBuf);
    BAIL_ON_VMDIR_ERROR(retVal);

    // rm -rf /var/lib/vmware/post/*
    retVal = VmDirStringPrintFA(cmdBuf, VMDIR_MAX_FILE_NAME_LEN,
        "rm -rf %s/*", pszdbHomeDir);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirRun(cmdBuf);
    BAIL_ON_VMDIR_ERROR(retVal);

    // mv /var/lib/vmware/lightwave_tmp/partner/* /var/lib/vmware/post/
    retVal = VmDirStringPrintFA(cmdBuf, VMDIR_MAX_FILE_NAME_LEN,
        "mv %s/* %s/", swapDirBuf, pszdbHomeDir);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirRun(cmdBuf);
    BAIL_ON_VMDIR_ERROR(retVal);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s, DB Swapped", __FUNCTION__);

cleanup:

    return retVal;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "Error %d, errno %d", retVal, errorCode);
    goto cleanup;
}
