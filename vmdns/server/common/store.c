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
    UINT64 startTime = 0;
    UINT64 endTime = 0;

    startTime = VmDnsGetTimeInMilliSec();

    dwError = VmDnsDirAddZoneRecord(pszZoneName, pRecord);

    endTime = VmDnsGetTimeInMilliSec();
    VmMetricsHistogramUpdate(
            gVmDnsHistogramMetrics[STORE_UPDATE_DURATION],
            VDNS_RESPONSE_TIME(endTime - startTime)
            );

    return dwError;
}

DWORD
VmDnsStoreDeleteZoneRecord(
    PCSTR               pszZoneName,
    PVMDNS_RECORD       pRecord
    )
{
    DWORD dwError = 0;
    UINT64 startTime = 0;
    UINT64 endTime = 0;

    startTime = VmDnsGetTimeInMilliSec();

    dwError = VmDnsDirDeleteZoneRecord(pszZoneName, pRecord);

    endTime = VmDnsGetTimeInMilliSec();
    VmMetricsHistogramUpdate(
            gVmDnsHistogramMetrics[STORE_UPDATE_DURATION],
            VDNS_RESPONSE_TIME(endTime - startTime)
            );

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
    UINT64 startTime = 0;
    UINT64 endTime = 0;

    startTime = VmDnsGetTimeInMilliSec();

    dwError = VmDnsDirGetRecords(
                        pszZone,
                        pszName,
                        ppRecordArray);

    /* Translating LDAP error code to MS error code for use in serviceapi */
    dwError = (dwError == LDAP_NO_SUCH_OBJECT) ? ERROR_NOT_FOUND : dwError;

    endTime = VmDnsGetTimeInMilliSec();
    VmMetricsHistogramUpdate(
            gVmDnsHistogramMetrics[STORE_QUERY_DURATION],
            VDNS_RESPONSE_TIME(endTime - startTime)
            );

    return dwError;
}

DWORD
VmDnsStoreSaveForwarders(
    PCSTR               pszZone,
    DWORD               dwCount,
    PSTR*               ppszForwarders
    )
{
    DWORD dwError = 0;

    if (pszZone)
    {
        dwError = VmDnsDirSaveForwarders(
                        pszZone,
                        dwCount,
                        (PSTR*)ppszForwarders);
    }
    else
    {
        dwError = VmDnsRegSaveForwarders(
                        dwCount,
                        (PCSTR*)ppszForwarders);
    }

    return dwError;
}

DWORD
VmDnsStoreGetForwarders(
    PCSTR               pszZone,
    PDWORD              pdwCount,
    PSTR**              pppszForwarders
    )
{
    DWORD dwError = 0;

    if (pszZone)
    {
        dwError = VmDnsDirLoadForwarders(
                        pszZone,
                        pdwCount,
                        pppszForwarders);
    }
    else
    {
        dwError = VmDnsRegLoadForwarders(
                        pdwCount,
                        pppszForwarders);
    }

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
    LPVMDNS_ADD_REMOVE_ZONE_PROC LpRemoveZoneProc,
    PVOID                        pData
    )
{
    DWORD dwError = 0;
    dwError = VmDnsDirSyncDeleted(
                    dwLastChangedUSN,
                    LpRemoveZoneProc,
                    pData
                    );
    return dwError;
}

DWORD
VmDnsStoreSyncNewObjects(
    DWORD                        dwLastChangedUSN,
    LPVMDNS_ADD_REMOVE_ZONE_PROC LpSyncZoneProc,
    LPVMDNS_PURGE_RECORD_PROC    LpPurgeRecordProc,
    PVOID                        pData
    )
{
    DWORD dwError = 0;
    dwError = VmDnsDirSyncNewObjects(
                        dwLastChangedUSN,
                        LpSyncZoneProc,
                        LpPurgeRecordProc,
                        pData
                        );
    return dwError;
}

DWORD
VmDnsStoreGetProperties(
    PCSTR               pszZone,
    PVMDNS_PROPERTY_LIST *ppPropertyArray
    )
{
    DWORD dwError = 0;

    dwError = VmDnsDirGetProperties(
                        pszZone,
                        ppPropertyArray);

    /* Translating LDAP error code to MS error code for use in serviceapi */
    dwError = (dwError == LDAP_NO_SUCH_OBJECT) ? ERROR_NOT_FOUND : dwError;

    return dwError;
}
