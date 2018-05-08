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

#include "includes.h"

#define MDB_ENV_MAX_DBS 100
#define MDB_ENV_MAPSIZE 107374182400 //100G

int
main(int argc, char* argv[])
{
    DWORD           dwError = 0;
    unsigned int    envFlags = 0;
    mdb_mode_t      mode = 0;
    BOOLEAN         bMdbWalEnable = TRUE;
    MDB_env*        mdbEnv = NULL;
#ifndef _WIN32
    const char     *dbHomeDir = LWRAFT_DB_DIR;
#else
    _TCHAR          dbHomeDir[MAX_PATH];
    dwError = VmDirMDBGetHomeDir(dbHomeDir);
    BAIL_ON_VMDIR_ERROR ( dwError );
#endif

    if (argc == 2)
    {
        dbHomeDir = argv[1];
    }

    dwError = mdb_env_create ( &mdbEnv );
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = mdb_env_set_maxdbs ( mdbEnv, MDB_ENV_MAX_DBS );
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = mdb_env_set_mapsize( mdbEnv, MDB_ENV_MAPSIZE );
    BAIL_ON_VMDIR_ERROR( dwError );
#ifdef MDB_NOTLS
    envFlags = MDB_NOTLS;
#endif

    if (bMdbWalEnable)
    {
        envFlags |= MDB_WAL;
    }

    mode = S_IRUSR | S_IWUSR;

    dwError = mdb_env_open ( mdbEnv, dbHomeDir, envFlags, mode );
    BAIL_ON_VMDIR_ERROR( dwError );

    mdb_env_sync(mdbEnv, 1);

cleanup:
    if (mdbEnv != NULL)
    {
        mdb_env_close(mdbEnv);
    }
    VmDirLog( LDAP_DEBUG_TRACE, "main: End" );
    return dwError;

error:
    VmDirLog( LDAP_DEBUG_ANY, "MDBInitializeDB failed with error code: %d, error string: %s", dwError, mdb_strerror(dwError) );
    goto cleanup;
}
