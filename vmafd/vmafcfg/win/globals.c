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

VMAF_CFG_PACKAGE gVmAfWinCfgApiTable =
{
        VMAFD_SF_INIT(.pfnOpenConnection,  &VmAfWinCfgOpenConnection),
        VMAFD_SF_INIT(.pfnOpenRootKey,     &VmAfWinCfgOpenRootKey),
        VMAFD_SF_INIT(.pfnOpenKey,         &VmAfWinCfgOpenKey),
        VMAFD_SF_INIT(.pfnCreateKey,       &VmAfWinCfgCreateKey),
        VMAFD_SF_INIT(.pfnDeleteKey,       &VmAfWinCfgDeleteKey),
        VMAFD_SF_INIT(.pfnEnumKeys,        &VmAfWinCfgEnumKeys),
        VMAFD_SF_INIT(.pfnReadStringValue, &VmAfWinCfgReadStringValue),
        VMAFD_SF_INIT(.pfnReadDWORDValue,  &VmAfWinCfgReadDWORDValue),
        VMAFD_SF_INIT(.pfnSetValue,        &VmAfWinCfgSetValue),
        VMAFD_SF_INIT(.pfnDeleteValue,     &VmAfWinCfgDeleteValue),
        VMAFD_SF_INIT(.pfnGetSecurity,     &VmAfWinCfgGetSecurity),
        VMAFD_SF_INIT(.pfnCloseKey,        &VmAfWinCfgCloseKey),
        VMAFD_SF_INIT(.pfnCloseConnection, &VmAfWinCfgCloseConnection)
};

PVMAF_CFG_PACKAGE gpVmAfWinCfgApiTable = &gVmAfWinCfgApiTable;
