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
 * BDB configuration are done in two stages -
 * 1. startup stage for fix DB (i.e. non configurable one like ENTRY)
 *    in InitBdbConfig()
 * 2. cn=indices stage to open all index DB
 *    in InitializeBDBIndexDB
 *
 */
DWORD
InitBdbConfig()
{
    DWORD   dwError = 0;

   VmDirLog( LDAP_DEBUG_TRACE, "InitBdbConfig: Begin" );

   // Initialize entry database
   dwError = VmDirAllocateMemory(
           sizeof(VDIR_CFG_BDB_DATAFILE_DESC) * 1,
           (PVOID)&gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles);
   BAIL_ON_VMDIR_ERROR(dwError);

   gVdirBdbGlobals.bdbEntryDB.usNumDataFiles = 1;

   gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles[0].bIsUnique = TRUE;

   dwError = VmDirAllocateStringA(
           VMDIR_ENTRY_DB,
           &gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles[0].pszDBName);
   BAIL_ON_VMDIR_ERROR(dwError);

   dwError = VmDirAllocateStringA(
           VMDIR_DB_FILE_NAME,
           &gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles[0].pszDBFile);
   BAIL_ON_VMDIR_ERROR(dwError);

   gVdirBdbGlobals.bdbEntryDB.btKeyCmpFcn = NULL;

   // Set hard limit of MAX index attribute
   //TODO, could make this configurable
   gVdirBdbGlobals.bdbIndexDBs.usMaxSize = BDB_MAX_INDEX_ATTRIBUTE;

   dwError = VmDirAllocateMemory(
           sizeof(VDIR_BDB_INDEX_DATABASE) * BDB_MAX_INDEX_ATTRIBUTE,
           (PVOID)&gVdirBdbGlobals.bdbIndexDBs.pIndexDBs);
   BAIL_ON_VMDIR_ERROR(dwError);

   VmDirLog( LDAP_DEBUG_TRACE, "InitBdbConfig: End" );

cleanup:

    return dwError;

error:

    goto cleanup;
}
