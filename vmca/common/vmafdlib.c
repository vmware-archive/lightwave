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


DWORD
VMCAOpenVmAfdClientLib(
    VMCA_LIB_HANDLE*   pplibHandle
    )
{
    DWORD   dwError = 0;
    CHAR    pszRegLibPath[VMCA_MAX_PATH_LEN] = {0};
    PSTR    pszVmafdLibPath = NULL;
    VMCA_LIB_HANDLE    plibHandle = NULL;

#ifdef _WIN32

    dwError = VMCAStringNCpyA(
            pszRegLibPath,
            VMCA_MAX_PATH_LEN,
            WIN_SYSTEM32_PATH,
            VMCA_MAX_PATH_LEN);
    BAIL_ON_VMCA_ERROR(dwError);

#elif LIGHTWAVE_BUILD

    dwError = VMCAStringNCpyA(
            pszRegLibPath,
            VMCA_MAX_PATH_LEN,
            VMCA_LIB_DIR,
            VMCA_MAX_PATH_LEN);
    BAIL_ON_VMCA_ERROR(dwError);

#else

    PSTR pszVmafdName = NULL;

    dwError = VMCAGetRegKeyValue(
            VMAFD_KEY_ROOT,
            VMAFD_LIB_KEY,
            pszRegLibPath,
            sizeof(pszRegLibPath) - 1);
    BAIL_ON_VMCA_ERROR(dwError);

    // find the first vmafd in path key "/usr/lib/vmware-vmafd/...."
    pszVmafdName = strstr(pszRegLibPath, VMAFD_NAME);

    dwError = pszVmafdName ? 0 : VMCA_ERROR_NO_FILE_OR_DIRECTORY;
    BAIL_ON_VMCA_ERROR(dwError);

    pszVmafdName[strlen(VMAFD_NAME)] = '\0';

#endif

    // construct full path to libvmafdclient
    dwError = VMCAAllocateStringPrintfA(
            &pszVmafdLibPath,
            "%s%s",
            pszRegLibPath,
            VMAFD_VECS_CLIENT_LIBRARY);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCALoadLibrary(pszVmafdLibPath, &plibHandle);
    BAIL_ON_VMCA_ERROR(dwError);

    *pplibHandle = plibHandle;

cleanup:
    VMCA_SAFE_FREE_MEMORY(pszVmafdLibPath);
    return dwError;

error:
    goto cleanup;
}
