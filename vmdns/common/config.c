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
VmDnsConfigOpenConnection(
    PVMDNS_CFG_CONNECTION* ppConnection
    )
{
    DWORD dwError = 0;
#ifndef _WIN32
    dwError = VmDnsPosixCfgOpenConnection(ppConnection);
#else
    dwError = VmDnsWinCfgOpenConnection(ppConnection);
#endif
    return dwError;
}

DWORD
VmDnsConfigOpenRootKey(
    PVMDNS_CFG_CONNECTION   pConnection,
    PCSTR                   pszKeyName,
    DWORD                   dwOptions,
    DWORD                   dwAccess,
    PVMDNS_CFG_KEY*         ppKey
    )
{
    DWORD dwError = 0;

#ifndef _WIN32
    dwError = VmDnsPosixCfgOpenRootKey(
                            pConnection,
                            pszKeyName,
                            dwOptions,
                            dwAccess,
                            ppKey);
#else
    dwError = VmDnsWinCfgOpenRootKey(
                            pConnection,
                            pszKeyName,
                            dwOptions,
                            dwAccess,
                            ppKey);
#endif

    return dwError;
}

DWORD
VmDnsConfigOpenKey(
    PVMDNS_CFG_CONNECTION   pConnection,
    PVMDNS_CFG_KEY          pKey,
    PCSTR                   pszSubKey,
    DWORD                   dwOptions,
    DWORD                   dwAccess,
    PVMDNS_CFG_KEY*         ppKey
    )
{
    DWORD dwError = 0;

#ifndef _WIN32
    dwError = VmDnsPosixCfgOpenKey(
                            pConnection,
                            pKey,
                            pszSubKey,
                            dwOptions,
                            dwAccess,
                            ppKey);
#else
    dwError = VmDnsWinCfgOpenKey(
                            pConnection,
                            pKey,
                            pszSubKey,
                            dwOptions,
                            dwAccess,
                            ppKey);
#endif

    return dwError;
}

DWORD
VmDnsConfigReadStringValue(
    PVMDNS_CFG_KEY      pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PSTR*               ppszValue
    )
{
    DWORD dwError = 0;

#ifndef _WIN32
    dwError = VmDnsPosixCfgReadStringValue(
                        pKey,
                        pszSubkey,
                        pszName,
                        ppszValue);
#else
    dwError = VmDnsWinCfgReadStringValue(
                        pKey,
                        pszSubkey,
                        pszName,
                        ppszValue);
#endif

    return dwError;
}

DWORD
VmDnsConfigReadDWORDValue(
    PVMDNS_CFG_KEY      pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PDWORD              pdwValue
    )
{
    DWORD dwError = 0;

#ifndef _WIN32
    dwError = VmDnsPosixCfgReadDWORDValue(
                        pKey,
                        pszSubkey,
                        pszName,
                        pdwValue);
#else
    dwError = VmDnsWinCfgReadDWORDValue(
                        pKey,
                        pszSubkey,
                        pszName,
                        pdwValue);
#endif

    return dwError;
}

VOID
VmDnsConfigCloseKey(
    PVMDNS_CFG_KEY pKey
    )
{
    if (pKey)
    {
#ifndef _WIN32
        VmDnsPosixCfgCloseKey(pKey);
#else
        VmDnsWinCfgCloseKey(pKey);
#endif
    }
}

VOID
VmDnsConfigCloseConnection(
    PVMDNS_CFG_CONNECTION pConnection
    )
{
    if (pConnection)
    {
#ifndef _WIN32
        VmDnsPosixCfgCloseConnection(pConnection);
#else
        VmDnsWinCfgCloseConnection(pConnection);
#endif
    }
}
