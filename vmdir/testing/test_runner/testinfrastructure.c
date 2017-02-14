/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
_VmDirEnumerateTests(
    PCSTR pszDirectoryName,
    PVMDIR_STRING_LIST pStringList
    )
{
    DWORD dwError = 0;
    DIR *d = NULL;
    struct dirent *dir = NULL;
    PSTR pszFilePath = NULL;

    d = opendir(pszDirectoryName);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (VmDirStringEndsWith(dir->d_name, ".so", FALSE))
            {
                dwError = VmDirAllocateStringPrintf(
                            &pszFilePath,
                            "%s/%s",
                            pszDirectoryName,
                            dir->d_name);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirStringListAdd(pStringList, pszFilePath);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        closedir(d);
    }
    else if (errno == ENOTDIR)
    {
        dwError = VmDirStringListAddStrClone(pszDirectoryName, pStringList);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}
