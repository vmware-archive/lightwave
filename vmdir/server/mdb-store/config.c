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

/*
 * MDB configuration are done in two stages -
 * 1. startup stage for fix DB (i.e. non configurable one like ENTRY)
 *    in VmDirMDBInitializeDB()
 * 2. schema initialization stage to open all custom index DB
 *    using VmDirMDBIndexOpen()
 */
DWORD
MDBInitConfig()
{
    DWORD   dwError = 0;

   VmDirLog( LDAP_DEBUG_TRACE, "InitMdbConfig: Begin" );

   // Initialize entry database
   dwError = VmDirAllocateMemory( sizeof(VDIR_CFG_MDB_DATAFILE_DESC) * 1,
                                  (PVOID)&gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles);
   BAIL_ON_VMDIR_ERROR(dwError);

   gVdirMdbGlobals.mdbEntryDB.usNumDataFiles = 1;

   gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles[0].bIsUnique = TRUE;

   dwError = VmDirAllocateStringA( VMDIR_ENTRY_DB,
                                   &gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles[0].pszDBName);
   BAIL_ON_VMDIR_ERROR(dwError);

   dwError = VmDirAllocateStringA( VMDIR_DB_FILE_NAME,
                                   &gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles[0].pszDBFile);
   BAIL_ON_VMDIR_ERROR(dwError);

   gVdirMdbGlobals.mdbEntryDB.btKeyCmpFcn = NULL;

   VmDirLog( LDAP_DEBUG_TRACE, "InitMdbConfig: End" );

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirMDBConfigureFsync(
    BOOLEAN bFsyncOn
    )
{
    DWORD   dwError = 0;

    dwError = mdb_env_set_flags(gVdirMdbGlobals.mdbEnv, MDB_NOSYNC, !bFsyncOn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_env_sync(gVdirMdbGlobals.mdbEnv, 1);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, mdb error (%d)", __FUNCTION__, dwError );

    VMDIR_SET_BACKEND_ERROR(dwError);
    goto cleanup;
}
