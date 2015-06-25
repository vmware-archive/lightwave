/*
* Copyright (c) VMware Inc. 2015  All rights Reserved.
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
    return VmDnsDirCreateZone(pZoneInfo);
}

DWORD
VmDnsStoreUpdateZone(
    PVMDNS_ZONE_INFO    pZoneInfo
    )
{
    return VmDnsDirUpdateZone(pZoneInfo);
}

DWORD
VmDnsStoreDeleteZone(
    PCSTR               pszZoneName
    )
{
    return VmDnsDirDeleteZone(pszZoneName);
}

DWORD
VmDnsStoreAddZoneRecord(
    PCSTR               pszZoneName,
    PVMDNS_RECORD       pRecord
    )
{
    return VmDnsDirAddZoneRecord(pszZoneName, pRecord);
}

DWORD
VmDnsStoreDeleteZoneRecord(
    PCSTR               pszZoneName,
    PVMDNS_RECORD       pRecord
    )
{
    return VmDnsDirDeleteZoneRecord(pszZoneName, pRecord);
}

DWORD
VmDnsStoreSaveForwarders(
    DWORD               dwCount,
    PSTR*               ppszForwarders
    )
{
    return VmDnsDirSaveForwarders(dwCount, ppszForwarders);
}
