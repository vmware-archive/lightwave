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

VOID
VmKdcFreeChecksum(
    PVMKDC_CHECKSUM pChecksum)
{
    if (pChecksum)
    {
        VMKDC_SAFE_FREE_DATA(pChecksum->data);
        VMKDC_SAFE_FREE_MEMORY(pChecksum);
    }
}

DWORD
VmKdcMakeChecksum(
    VMKDC_CKSUMTYPE type,
    PUCHAR contents,
    DWORD length,
    PVMKDC_CHECKSUM *ppRetChecksum)
{
    DWORD dwError = 0;
    PVMKDC_CHECKSUM pChecksum = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_CHECKSUM), (PVOID*)&pChecksum);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateData(contents, length, &pChecksum->data);
    BAIL_ON_VMKDC_ERROR(dwError);

    pChecksum->type = type;

    /* TBD - calculate checksum */

    *ppRetChecksum = pChecksum;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_CHECKSUM(pChecksum);
    }

    return dwError;
}
