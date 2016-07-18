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
*
* Module Name:  store.c
*
* Abstract: VMware Domain Name Service.
*
* Storage interface - currently only ldap implementation
*/

#include "includes.h"

DWORD
VmDnsStoreCreateZone(
    PVMDNS_ZONE_INFO    pZoneInfo
    )
{
    DWORD dwError = 0;
    if (gpDNSDriverGlobals->bUseDirectoryStore)
    {
        dwError = VmDnsDirCreateZone(pZoneInfo);
    }

    return dwError;
}

DWORD
VmDnsStoreUpdateZone(
    PVMDNS_ZONE_INFO    pZoneInfo
    )
{
    DWORD dwError = 0;
    if (gpDNSDriverGlobals->bUseDirectoryStore)
    {
        dwError = VmDnsDirUpdateZone(pZoneInfo);
    }

    return dwError;
}

DWORD
VmDnsStoreDeleteZone(
    PCSTR               pszZoneName
    )
{
    DWORD dwError = 0;
    if (gpDNSDriverGlobals->bUseDirectoryStore)
    {
        dwError = VmDnsDirDeleteZone(pszZoneName);
    }

    return dwError;
}

DWORD
VmDnsStoreAddZoneRecord(
    PCSTR               pszZoneName,
    PVMDNS_RECORD       pRecord
    )
{
    DWORD dwError = 0;
    if (gpDNSDriverGlobals->bUseDirectoryStore)
    {
        dwError = VmDnsDirAddZoneRecord(pszZoneName, pRecord);
    }

    return dwError;
}

DWORD
VmDnsStoreDeleteZoneRecord(
    PCSTR               pszZoneName,
    PVMDNS_RECORD       pRecord
    )
{
    DWORD dwError = 0;
    if (gpDNSDriverGlobals->bUseDirectoryStore)
    {
        dwError = VmDnsDirDeleteZoneRecord(pszZoneName, pRecord);
    }

    return dwError;
}

DWORD
VmDnsStoreSaveForwarders(
    DWORD               dwCount,
    PSTR*               ppszForwarders
    )
{
    DWORD dwError = 0;
    if (gpDNSDriverGlobals->bUseDirectoryStore)
    {
        dwError = VmDnsDirSaveForwarders(dwCount, ppszForwarders);
    }

    return dwError;
}
