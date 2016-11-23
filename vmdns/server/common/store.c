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
VmDnsStoreInitialize(
    )
{
    DWORD dwError = 0;
    dwError = VmDnsCreateInitZoneContainer();
    return dwError;
}

VOID
VmDnsStoreCleanup(
    )
{
}

DWORD
VmDnsStoreCreateZone(
    PVMDNS_ZONE_INFO    pZoneInfo
    )
{
    DWORD dwError = 0;
    dwError = VmDnsDirCreateZone(pZoneInfo);
    return dwError;
}

DWORD
VmDnsStoreUpdateZone(
    PVMDNS_ZONE_INFO    pZoneInfo
    )
{
    DWORD dwError = 0;
    dwError = VmDnsDirUpdateZone(pZoneInfo);
    return dwError;
}

DWORD
VmDnsStoreDeleteZone(
    PCSTR               pszZoneName
    )
{
    DWORD dwError = 0;
    dwError = VmDnsDirDeleteZone(pszZoneName);
    return dwError;
}

DWORD
VmDnsStoreListZones(
    PSTR**              pppszZones,
    PDWORD              pdwCount
    )
{
    DWORD dwError = 0;
    dwError = VmDnsDirListZones(pppszZones, pdwCount);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}


DWORD
VmDnsStoreListRecords(
    PCSTR               pszZone,
    PVMDNS_RECORD_LIST  *ppRecordArray
    )
{
    DWORD dwError = 0;

    dwError = VmDnsDirListRecords(pszZone, ppRecordArray);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsStoreAddZoneRecord(
    PCSTR               pszZoneName,
    PVMDNS_RECORD       pRecord
    )
{
    DWORD dwError = 0;
    dwError = VmDnsDirAddZoneRecord(pszZoneName, pRecord);

    return dwError;
}

DWORD
VmDnsStoreDeleteZoneRecord(
    PCSTR               pszZoneName,
    PVMDNS_RECORD       pRecord
    )
{
    DWORD dwError = 0;
    dwError = VmDnsDirDeleteZoneRecord(pszZoneName, pRecord);
    return dwError;
}

DWORD
VmDnsStoreGetRecords(
    PCSTR               pszZone,
    PCSTR               pszName,
    PVMDNS_RECORD_LIST *ppRecordArray
    )
{
    DWORD dwError = 0;
    dwError = VmDnsDirGetRecords(
                        pszZone,
                        pszName,
                        ppRecordArray);

    /* Translating LDAP error code to MS error code for use in serviceapi */
    dwError = (dwError == LDAP_NO_SUCH_OBJECT) ? ERROR_NOT_FOUND : dwError;

    return dwError;
}
DWORD
VmDnsStoreSaveForwarders(
    DWORD               dwCount,
    PSTR*               ppszForwarders
    )
{
    DWORD dwError = 0;
    dwError = VmDnsDirSaveForwarders(dwCount, ppszForwarders);
    return dwError;
}

DWORD
VmDnsStoreGetForwarders(
    PDWORD              pdwCount,
    PSTR**              pppszForwarders
    )
{
    DWORD dwError = 0;
    dwError = VmDnsDirLoadForwarders(pdwCount, pppszForwarders);
    return dwError;
}

DWORD
VmDnsStoreGetReplicationStatus(
    PDWORD             pdwLastChangedUSN
    )
{
    DWORD dwError = 0;
    dwError = VmDnsDirGetReplicationStatus(pdwLastChangedUSN);
    return dwError;
}

DWORD
VmDnsStoreSyncDeleted(
    DWORD                        dwLastChangedUSN,
    DWORD                        dwNewUSN,
    LPVMDNS_ADD_REMOVE_ZONE_PROC LpRemoveZoneProc,
    PVOID                        pData
    )
{
    DWORD dwError = 0;
    dwError = VmDnsDirSyncDeleted(
                    dwLastChangedUSN,
                    dwNewUSN,
                    LpRemoveZoneProc,
                    pData
                    );
    return dwError;
}

DWORD
VmDnsStoreSyncNewObjects(
    DWORD                        dwLastChangedUSN,
    DWORD                        dwNewUSN,
    LPVMDNS_ADD_REMOVE_ZONE_PROC LpSyncZoneProc,
    LPVMDNS_PURGE_RECORD_PROC    LpPurgeRecordProc,
    PVOID                        pData
    )
{
    DWORD dwError = 0;
    dwError = VmDnsDirSyncNewObjects(
                        dwLastChangedUSN,
                        dwNewUSN,
                        LpSyncZoneProc,
                        LpPurgeRecordProc,
                        pData
                        );
    return dwError;
}

