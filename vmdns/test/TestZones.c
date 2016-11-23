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

#include "CuTest.h"
#include "../server/common/includes.h"
#include "vmdnscommon.h"
#include "semaphore.h"

extern int  vmdns_syslog_level;

#define DEFAULT_EXPIRE  (60*60*24*30*6)
#define DEFAULT_TTL     (60*30)
#define DEFAULT_SERIAL  1
#define DEFAULT_REFRESH (60*60)
#define DEFAULT_RETRY   (60*10)

static
PVMDNS_RECORD
CreateAddressRecord(
    PCSTR   pszName,
    DWORD   address,
    PCSTR   pszZone
);

static
PVMDNS_RECORD
CreateAAAARecord(
    PCSTR   pszName,
    BYTE    bytes[16],
    PCSTR   pszZone
);

static
PVMDNS_RECORD
CreateServiceRecord(
    PSTR                    pszDomain,
    VMDNS_SERVICE_TYPE      service,
    VMDNS_SERVICE_PROTOCOL  protocol,
    PSTR                    pszTarget,
    UINT16                  port,
    UINT16                  priority,
    UINT16                  weight
);

static
PVMDNS_RECORD
CreateNSRecord(
    PSTR    pszName,
    PSTR    pszTarget,
    UINT16  port,
    UINT16  priority,
    UINT16  weight
);

static
void
VerifyTestZone(
    CuTest* tc,
    PSTR pszZoneName
    );

void
ListRecords()
{
    PVMDNS_RECORD_ARRAY pra = NULL;
    VmDnsZoneListRecord("vsphere.local", &pra);
    VMDNS_FREE_RECORD_ARRAY(pra);
}

void TestZoneInit(CuTest* tc)
{
    vmdns_syslog_level = VMDNS_LOG_LEVEL_DEBUG;

    VmDnsCoreInit(FALSE);
    CuAssertTrue(tc, gpDNSDriverGlobals->pZoneList != NULL);
    VmDnsSetState(VMDNS_READY);
}

void TestZoneCreate(CuTest* tc)
{
    DWORD dwError = 0;
    VMDNS_ZONE_INFO zi = {0};
    PVMDNS_RECORD_ARRAY pRecords = NULL;
    PVMDNS_ZONE_INFO_ARRAY pZoneArray = NULL;

    zi.pszName ="vsphere.local";
    zi.pszPrimaryDnsSrvName = "dns.vsphere.local";
    zi.pszRName = "admin@vsphere.local";
    zi.expire = DEFAULT_EXPIRE;
    zi.minimum = DEFAULT_TTL;
    zi.serial = DEFAULT_SERIAL;
    zi.retryInterval = DEFAULT_RETRY;
    zi.refreshInterval = DEFAULT_REFRESH;

    dwError = VmDnsZoneCreate(NULL, &zi, FALSE);
    CuAssert(tc, "Zones create should succeed.", !dwError);

    dwError = VmDnsZoneQuery(zi.pszName, VMDNS_SOA_RECORD_NAME, VMDNS_RR_TYPE_SOA, &pRecords);
    CuAssert(tc, "Zones query of SOA record should succeed.", !dwError);
    CuAssert(tc, "SOA record should not be null.", pRecords != NULL);
    CuAssert(tc, "SOA record array size should be 1.", pRecords->dwCount == 1);
    CuAssert(tc, "SOA record type check should pass.", pRecords->Records[0].dwType == VMDNS_RR_TYPE_SOA);
    CuAssert(tc, "SOA record name check should pass.",
        !VmDnsStringCompareA(pRecords->Records[0].pszName, VMDNS_SOA_RECORD_NAME, TRUE));
    CuAssert(tc, "SOA record admin check should pass.",
        !VmDnsStringCompareA(pRecords->Records[0].Data.SOA.pNameAdministrator,
                            zi.pszRName, TRUE));
    CuAssert(tc, "SOA record ttl check should pass.",
        pRecords->Records[0].Data.SOA.dwDefaultTtl == DEFAULT_TTL);
    VMDNS_FREE_RECORD_ARRAY(pRecords);

    dwError = VmDnsZoneList(&pZoneArray);
    CuAssert(tc, "Listing zone should succeed.", !dwError);
    CuAssert(tc, "One and only one zone is expected.", pZoneArray->dwCount == 1);
    CuAssert(tc, "Zone name should be older one.",
        VmDnsStringCompareA(pZoneArray->ZoneInfos[0].pszName, zi.pszName, FALSE) == 0);
    VMDNS_FREE_ZONE_INFO_ARRAY(pZoneArray);

    dwError = VmDnsZoneCreate(NULL, &zi, FALSE);
    CuAssert(tc, "Creating a zone with a name conflict should fail.", dwError);

    VerifyTestZone(tc, zi.pszName);
    ListRecords();
}

void TestZoneUpdate(CuTest* tc)
{
    DWORD dwError = 0;
    VMDNS_ZONE_INFO zi = {0};
    PVMDNS_RECORD_ARRAY pRecords = NULL;

    zi.pszName = "vsphere.local";
    zi.pszPrimaryDnsSrvName = "dns2.vsphere.local";
    zi.pszRName = "admin2@vsphere.local";
    zi.expire = DEFAULT_EXPIRE / 2;
    zi.minimum = DEFAULT_TTL / 2;
    zi.serial = DEFAULT_SERIAL + 1;
    zi.retryInterval = DEFAULT_RETRY / 2;
    zi.refreshInterval = DEFAULT_REFRESH / 2;

    dwError = VmDnsZoneUpdate(NULL, &zi, FALSE);
    CuAssert(tc, "Zones update should always succeed.", !dwError);

    dwError = VmDnsZoneQuery(zi.pszName, VMDNS_SOA_RECORD_NAME, VMDNS_RR_TYPE_SOA, &pRecords);
    CuAssert(tc, "Zones query of SOA record should succeed.", !dwError);

    CuAssert(tc, "SOA record should not be null.", pRecords != NULL);
    CuAssert(tc, "SOA record array size should be 1.", pRecords->dwCount == 1);
    CuAssert(tc, "SOA record type check should pass.", pRecords->Records[0].dwType == VMDNS_RR_TYPE_SOA);
    CuAssert(tc, "SOA record name check should pass.",
        !VmDnsStringCompareA(pRecords->Records[0].pszName, VMDNS_SOA_RECORD_NAME, TRUE));
    CuAssert(tc, "SOA record admin check should pass.",
        !VmDnsStringCompareA(pRecords->Records[0].Data.SOA.pNameAdministrator,
                            zi.pszRName, TRUE));
    CuAssert(tc, "SOA record ttl check should pass.",
        pRecords->Records[0].Data.SOA.dwDefaultTtl == zi.minimum);
    VMDNS_FREE_RECORD_ARRAY(pRecords);

    zi.pszPrimaryDnsSrvName = "dns1.vsphere.local";
    dwError = VmDnsZoneUpdate(NULL, &zi, FALSE);
    CuAssert(tc, "Zones primary server name restore should succeed.", !dwError);

    VerifyTestZone(tc, "vsphere.local");
    ListRecords();
}

void TestAddressRecord(CuTest* tc)
{
    DWORD dwError = 0;
    PVMDNS_RECORD pRecord = NULL;
    PVMDNS_RECORD_ARRAY    pRecords = NULL;
    PSTR pcszHostName = "host1";
    PSTR pszZone = "vsphere.local";
    PBYTE pSerializedBytes = NULL;
    DWORD dwSerializationSize = 0;
    BYTE bytes[16] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};

    VerifyTestZone(tc, "vsphere.local");

    pRecord = CreateAddressRecord(pcszHostName, 11111, pszZone);
    dwError = VmDnsZoneAddRecord(NULL, pszZone, pRecord, FALSE);
    CuAssert(tc, "Adding address record should succeed.", !dwError);
    VMDNS_FREE_RECORD(pRecord);

    dwError = VmDnsZoneQuery(pszZone, pcszHostName, VMDNS_RR_TYPE_A, &pRecords);
    CuAssert(tc, "Querying the added record should succeed.", !dwError);
    CuAssert(tc, "Address record count from query should be 1.", pRecords->dwCount == 1);
    VMDNS_FREE_RECORD_ARRAY(pRecords);

    pRecord = CreateAddressRecord(pcszHostName, 22222, pszZone);
    dwError = VmDnsZoneAddRecord(NULL, pszZone, pRecord, FALSE);
    CuAssert(tc, "Adding 2nd address record should succeed.", !dwError);

    dwError = VmDnsSerializeDnsRecord(pRecord, &pSerializedBytes, &dwSerializationSize, TRUE);
    CuAssert(tc, "Serialization Address record should succeed.", !dwError);
    VMDNS_FREE_RECORD(pRecord);

    dwError = VmDnsDeserializeDnsRecord(pSerializedBytes,dwSerializationSize,&pRecord, TRUE);
    CuAssert(tc, "Deserialization of Address record should succeed.", !dwError);

    VMDNS_SAFE_FREE_MEMORY(pSerializedBytes);
    VMDNS_FREE_RECORD(pRecord);

    dwError = VmDnsZoneQuery(pszZone, pcszHostName, VMDNS_RR_TYPE_A, &pRecords);
    CuAssert(tc, "Querying the added records should succeed.", !dwError);
    CuAssert(tc, "Address record count from query should be 2.", pRecords->dwCount == 2);

    pRecord = CreateAAAARecord(pcszHostName, bytes, pszZone);
    dwError = VmDnsZoneAddRecord(NULL, pszZone, pRecord, FALSE);
    CuAssert(tc, "Adding ipv6 address record should succeed.", !dwError);
    VMDNS_FREE_RECORD(pRecord);

    dwError = VmDnsZoneQuery(pszZone, pcszHostName, VMDNS_RR_TYPE_A, &pRecords);
    CuAssert(tc, "Querying the added records should succeed.", !dwError);
    CuAssert(tc, "IPv4 address record count from query should be 2.", pRecords->dwCount == 2);

    dwError = VmDnsZoneDeleteRecord(NULL, pszZone, &pRecords->Records[1], FALSE);
    CuAssert(tc, "Deleting the second address record should succeed.", !dwError);
    VMDNS_FREE_RECORD_ARRAY(pRecords);

    dwError = VmDnsZoneQuery(pszZone, pcszHostName, VMDNS_RR_TYPE_AAAA, &pRecords);
    CuAssert(tc, "Querying the added records should succeed.", !dwError);
    CuAssert(tc, "IPV6 address record count from query should be 1.", pRecords->dwCount == 1);
    VMDNS_FREE_RECORD_ARRAY(pRecords);

    dwError = VmDnsZoneQuery(pszZone, pcszHostName, VMDNS_RR_TYPE_A, &pRecords);
    CuAssert(tc, "Querying the added records should succeed.", !dwError);
    CuAssert(tc, "Address record count from query should be 1.", pRecords->dwCount == 1);
    VMDNS_FREE_RECORD_ARRAY(pRecords);

    dwError = VmDnsZoneFindAndDeleteRecords(NULL, pszZone, pcszHostName, FALSE);
    CuAssert(tc, "Deleting the remaining address record should succeed.", !dwError);

    dwError = VmDnsZoneQuery(pszZone, pcszHostName, VMDNS_RR_TYPE_A, &pRecords);
    CuAssert(tc, "Address records should no longer exist after deletion.", dwError == ERROR_NOT_FOUND);

    pRecord = CreateAAAARecord(pcszHostName, bytes, pszZone);
    dwError = VmDnsZoneAddRecord(NULL, pszZone, pRecord, FALSE);
    CuAssert(tc, "Adding ipv6 address record should succeed.", !dwError);
    VMDNS_FREE_RECORD(pRecord);

    dwError = VmDnsZoneQuery(pszZone, pcszHostName, VMDNS_RR_TYPE_AAAA, &pRecords);
    CuAssert(tc, "Querying the added record should succeed.", !dwError);
    CuAssert(tc, "Address record count from query should be 1.", pRecords->dwCount == 1);
    VMDNS_FREE_RECORD_ARRAY(pRecords);

    bytes[1] = 2;
    pRecord = CreateAAAARecord(pcszHostName, bytes, pszZone);
    dwError = VmDnsZoneAddRecord(NULL, pszZone, pRecord, FALSE);
    CuAssert(tc, "Adding 2nd ipv6 address record should succeed.", !dwError);

    // TODO: delete AAAA records

    VerifyTestZone(tc, pszZone);
    ListRecords();
}

void TestSrvRecord(CuTest* tc)
{
    DWORD dwError = 0;
    PVMDNS_RECORD pRecord = NULL;
    PVMDNS_RECORD pDeserializedRecord = NULL;
    PSTR pcszDomain = "vsphere.local";
    PSTR pszZone = pcszDomain;
    PSTR pszTarget1 = "dc1.vsphere.local";
    PSTR pszTarget2 = "dc2.vsphere.local";
    PVMDNS_RECORD_ARRAY    pRecords = NULL;
    PBYTE pSerializedBytes = NULL;
    DWORD dwSerializationSize = 0;

    VerifyTestZone(tc, "vsphere.local");

    pRecord = CreateServiceRecord(pcszDomain, DOMAIN_CONTROLLER, SERVICE_PROTOCOL_TCP, pszTarget1, 2015, 1, 3);
    dwError = VmDnsZoneAddRecord(NULL, pszZone, pRecord, FALSE);
    CuAssert(tc, "Adding service record should succeed.", !dwError);

    dwError = VmDnsZoneQuery(pszZone, pcszDomain, VMDNS_RR_TYPE_SRV, &pRecords);
    CuAssert(tc, "Querying service record should succeed after successfully add.", !dwError);
    VMDNS_FREE_RECORD_ARRAY(pRecords);

    dwError = VmDnsSerializeDnsRecord(pRecord,&pSerializedBytes, &dwSerializationSize, TRUE);
    CuAssert(tc, "Serialization of NS record should succeed.",!dwError);

    dwError = VmDnsDeserializeDnsRecord(pSerializedBytes,dwSerializationSize,&pDeserializedRecord, TRUE);
    CuAssert(tc, "Deserialization of Address record should succeed.", !dwError);

    CuAssert(tc, "Deserialized record should be the same as the original one",
                VmDnsCompareRecord(pRecord, pDeserializedRecord));

    VMDNS_SAFE_FREE_MEMORY(pSerializedBytes);
    VMDNS_FREE_RECORD(pRecord);
    VMDNS_FREE_RECORD(pDeserializedRecord);

    pRecord = CreateServiceRecord(pcszDomain, DOMAIN_CONTROLLER, SERVICE_PROTOCOL_TCP, pszTarget2, 2015, 1, 3);
    dwError = VmDnsZoneAddRecord(NULL, pszZone, pRecord, FALSE);
    CuAssert(tc, "Adding second service record should succeed.", !dwError);
    VMDNS_FREE_RECORD(pRecord);

    dwError = VmDnsZoneQuery(pszZone, pcszDomain, VMDNS_RR_TYPE_SRV, &pRecords);
    CuAssert(tc, "Querying service record should succeed.", !dwError);
    CuAssert(tc, "Querying service record should return 2 records.", pRecords->dwCount == 2);
    VMDNS_FREE_RECORD_ARRAY(pRecords);

    dwError = VmDnsZoneFindAndDeleteRecords(NULL, pszZone, pcszDomain, FALSE);
    CuAssert(tc, "Deleting the service record should succeed.", !dwError);

    dwError = VmDnsZoneQuery(pszZone, pcszDomain, VMDNS_RR_TYPE_SRV, &pRecords);
    CuAssert(tc, "Service record should no longer exist after deletion.", dwError == ERROR_NOT_FOUND);

    VerifyTestZone(tc, pszZone);
    ListRecords();
}

void TestNSRecord(CuTest* tc)
{
    DWORD dwError = 0;
    PVMDNS_RECORD pRecord = NULL;
    PSTR pszZone = "vsphere.local";
    PSTR pcszName1 = "external";
    PSTR pcszName2 = "local";
    PSTR pcszName3 = "vsphere.local";
    PSTR pszTarget = "dns.vsphere.local";
    UINT16 port = 53, priority = 1, weight = 1;

    PBYTE pSerializedBytes = NULL;
    DWORD dwSerializationSize = 0;
    PVMDNS_RECORD_ARRAY    pRecords = NULL;

    VerifyTestZone(tc, "vsphere.local");

    pRecord = CreateNSRecord(pcszName1, pszTarget, port, priority, weight);
    dwError = VmDnsZoneAddRecord(NULL, pszZone, pRecord, FALSE);
    CuAssert(tc, "Adding NS record should succeed.", !dwError);
    VMDNS_FREE_RECORD(pRecord);

    dwError = VmDnsZoneQuery(pszZone, pcszName1, VMDNS_RR_TYPE_NS, &pRecords);
    CuAssert(tc, "Querying NS record should succeed.", !dwError);
    CuAssert(tc, "One NS record should be found.", pRecords->dwCount == 1);
    VMDNS_FREE_RECORD_ARRAY(pRecords);

    pRecord = CreateNSRecord(pcszName2, pszTarget, port, priority, weight);
    dwError = VmDnsZoneAddRecord(NULL, pszZone, pRecord, FALSE);
    CuAssert(tc, "Adding NS record should succeed.", !dwError);
    VMDNS_FREE_RECORD(pRecord);

    dwError = VmDnsZoneQuery(pszZone, pcszName2, VMDNS_RR_TYPE_NS, &pRecords);
    CuAssert(tc, "Querying NS record should succeed.", !dwError);
    CuAssert(tc, "One NS record should be found.", pRecords->dwCount == 1);
    VMDNS_FREE_RECORD_ARRAY(pRecords);

    pRecord = CreateNSRecord(pcszName3, pszTarget, port, priority, weight);
    dwError = VmDnsZoneAddRecord(NULL, pszZone, pRecord, FALSE);
    CuAssert(tc, "Adding NS record should succeed.", !dwError);

    dwError = VmDnsSerializeDnsRecord(pRecord,&pSerializedBytes, &dwSerializationSize, TRUE);
    CuAssert(tc, "Serialization of NS record should succeed.",!dwError);
    VMDNS_FREE_RECORD(pRecord);

    dwError = VmDnsDeserializeDnsRecord(pSerializedBytes,dwSerializationSize,&pRecord, TRUE);
    CuAssert(tc, "Deserialization of NS record should succeed.", !dwError);

    VMDNS_SAFE_FREE_MEMORY(pSerializedBytes);
    VMDNS_FREE_RECORD(pRecord);

    dwError = VmDnsZoneQuery(pszZone, pcszName3, VMDNS_RR_TYPE_NS, &pRecords);
    CuAssert(tc, "Querying NS record should succeed.", !dwError);
    CuAssert(tc, "One NS record should be found.", pRecords->dwCount == 1);
    VMDNS_FREE_RECORD_ARRAY(pRecords);

    dwError = VmDnsZoneFindAndDeleteRecords(NULL, pszZone, pcszName1, FALSE);
    CuAssert(tc, "Deleting the service record should succeed.", !dwError);

    dwError = VmDnsZoneQuery(pszZone, pcszName1, VMDNS_RR_TYPE_NS, &pRecords);
    CuAssert(tc, "Querying deleted NS record should return ERROR_NOT_FOUND.", dwError == ERROR_NOT_FOUND);

    dwError = VmDnsZoneFindAndDeleteRecords(NULL, pszZone, pcszName2, FALSE);
    CuAssert(tc, "Deleting the service record should succeed.", !dwError);

    dwError = VmDnsZoneQuery(pszZone, pcszName2, VMDNS_RR_TYPE_NS, &pRecords);
    CuAssert(tc, "Querying deleted NS record should return ERROR_NOT_FOUND.", dwError == ERROR_NOT_FOUND);

    dwError = VmDnsZoneFindAndDeleteRecords(NULL, pszZone, pcszName3, FALSE);
    CuAssert(tc, "Deleting the service record should succeed.", !dwError);

    dwError = VmDnsZoneQuery(pszZone, pcszName3, VMDNS_RR_TYPE_NS, &pRecords);
    CuAssert(tc, "Querying deleted NS record should return ERROR_NOT_FOUND.", dwError == ERROR_NOT_FOUND);

    VerifyTestZone(tc, pszZone);
    ListRecords();
}

void TestSoaRecord(CuTest* tc)
{
    DWORD dwError = 0;
    VMDNS_ZONE_INFO zi = {0};
    PVMDNS_RECORD pSoa = NULL;
    PVMDNS_RECORD pRecord = NULL;
    PSTR  pcszDomain = "vsphere.local";
    PSTR  pcszOldServerName = "dns1.vsphere.local";
    PSTR  pcszNewServerName = "dns2.vsphere.local";
    PVMDNS_ZONE_INFO_ARRAY pZoneArray = NULL;
    PBYTE pSerializedBytes = NULL;
    DWORD dwSerializationSize = 0;

    zi.pszName = pcszDomain;
    zi.pszPrimaryDnsSrvName = pcszOldServerName;
    zi.pszRName = "admin@vsphere.local";
    zi.expire = DEFAULT_EXPIRE;
    zi.minimum = DEFAULT_TTL;
    zi.serial = DEFAULT_SERIAL;
    zi.retryInterval = DEFAULT_RETRY;
    zi.refreshInterval = DEFAULT_REFRESH;

    VerifyTestZone(tc, "vsphere.local");

    dwError = VmDnsCreateSoaRecord(&zi, &pSoa);
    CuAssert(tc, "Creating SOA record should succeed.", !dwError);

    dwError = VmDnsSerializeDnsRecord(pSoa,&pSerializedBytes, &dwSerializationSize, TRUE);
    CuAssert(tc, "Serialization of SOA record should succeed.",!dwError);

    dwError = VmDnsDeserializeDnsRecord(pSerializedBytes,dwSerializationSize,&pRecord, TRUE);
    CuAssert(tc, "Deserialization of SOA record should succeed.", !dwError);

    VMDNS_SAFE_FREE_MEMORY(pSerializedBytes);
    VMDNS_FREE_RECORD(pRecord);
    VMDNS_FREE_RECORD(pSoa);

    dwError = VmDnsZoneList(&pZoneArray);
    CuAssert(tc, "Listing zone should succeed.", !dwError);
    CuAssert(tc, "One and only one zone is expected.", pZoneArray->dwCount == 1);
    CuAssert(tc, "Zone primary server name should be older one.",
        VmDnsStringCompareA(pZoneArray->ZoneInfos[0].pszPrimaryDnsSrvName, pcszOldServerName, FALSE) == 0);
    VMDNS_FREE_ZONE_INFO_ARRAY(pZoneArray);

    ListRecords();

    // Update with new zone info
    zi.pszPrimaryDnsSrvName = pcszNewServerName;
    dwError = VmDnsCreateSoaRecord(&zi, &pSoa);
    CuAssert(tc, "Creating SOA record should succeed.", !dwError);
    dwError = VmDnsZoneFindAndDeleteRecords(NULL, pcszDomain, VMDNS_SOA_RECORD_NAME, FALSE);
    CuAssert(tc, "Deleting SOA record should succeed.", !dwError);
    dwError = VmDnsZoneAddRecord(NULL, pcszDomain, pSoa, FALSE);
    CuAssert(tc, "Updating SOA record should succeed.", !dwError);
    VMDNS_FREE_RECORD(pSoa);

    ListRecords();
    dwError = VmDnsZoneList(&pZoneArray);
    CuAssert(tc, "Listing zone should succeed.", !dwError);
    CuAssert(tc, "One and only one zone is expected.", pZoneArray->dwCount == 1);
    CuAssert(tc, "Zone primary server name should be newer one.",
        VmDnsStringCompareA(pZoneArray->ZoneInfos[0].pszPrimaryDnsSrvName, pcszNewServerName, FALSE) == 0);
    VMDNS_FREE_ZONE_INFO_ARRAY(pZoneArray);

    ListRecords();
    // Restore original zone info
    zi.pszPrimaryDnsSrvName = pcszOldServerName;
    dwError = VmDnsCreateSoaRecord(&zi, &pSoa);
    CuAssert(tc, "Creating SOA record should succeed.", !dwError);
    dwError = VmDnsZoneFindAndDeleteRecords(NULL, pcszDomain, VMDNS_SOA_RECORD_NAME, FALSE);
    CuAssert(tc, "Deleting SOA record should succeed.", !dwError);
    ListRecords();
    dwError = VmDnsZoneAddRecord(NULL, pcszDomain, pSoa, FALSE);
    CuAssert(tc, "Updating SOA record should succeed.", !dwError);
    VMDNS_FREE_RECORD(pSoa);

    ListRecords();
    dwError = VmDnsZoneList(&pZoneArray);
    CuAssert(tc, "Listing zone should succeed.", !dwError);
    CuAssert(tc, "One and only one zone is expected.", pZoneArray->dwCount == 1);
    CuAssert(tc, "Zone primary server name should be older one.",
        VmDnsStringCompareA(pZoneArray->ZoneInfos[0].pszPrimaryDnsSrvName, pcszOldServerName, FALSE) == 0);
    VMDNS_FREE_ZONE_INFO_ARRAY(pZoneArray);

    VerifyTestZone(tc, pcszDomain);
    ListRecords();
}

void TestNameEntry(CuTest* tc)
{
    DWORD dwError = ERROR_SUCCESS;
    PCSTR pszZone = "vsphere.local";
    PCSTR pszHost = "dc1.vsphere.local";
    DWORD address = 11111;
    PVMDNS_RECORD pRecord1 = NULL;
    PVMDNS_RECORD pRecord2 = NULL;
    PVMDNS_NAME_ENTRY pNameEntry = NULL;
    PVMDNS_RECORD_ARRAY pRecords = NULL;

    dwError = VmDnsCreateNameEntry(pszZone, pszHost, &pNameEntry);
    CuAssert(tc, "Creating name entry should succeed.", !dwError);

    pRecord1 = CreateAddressRecord(pszHost, address, pszZone);
    CuAssert(tc, "Creating address record should succeed.", !!pRecord1);

    dwError = VmDnsNameEntryAddRecord(pNameEntry, pRecord1, FALSE);
    CuAssert(tc, "Adding address record to name entry should succeed.", !dwError);

    CuAssert(tc, "Record count in name entry should be 1.",
        VmDnsNameEntryGetRecordCount(pNameEntry) == 1);

    dwError = VmDnsNameEntryListRecord(pNameEntry, &pRecords, VMDNS_RR_TYPE_A);
    CuAssert(tc, "Listing record should succeed.", !dwError);
    CuAssert(tc, "One record is exptected.", pRecords->dwCount == 1);
    CuAssert(tc, "Record should be the same as added.",
            VmDnsCompareRecord(pRecord1, &pRecords->Records[0]));
    VMDNS_FREE_RECORD_ARRAY(pRecords);

    pRecord2 = CreateAddressRecord(pszHost, 22222, pszZone);
    CuAssert(tc, "Creating address record 2 should succeed.", !dwError);

    dwError = VmDnsNameEntryAddRecord(pNameEntry, pRecord2, FALSE);
    CuAssert(tc, "Adding second address record to name entry should succeed.", !dwError);

    CuAssert(tc, "Record count in name entry should be 2.",
        VmDnsNameEntryGetRecordCount(pNameEntry) == 2);

    dwError = VmDnsNameEntryDeleteRecord(pNameEntry, pRecord1, FALSE);
    CuAssert(tc, "Deleting address record should succeed.", !dwError);

    CuAssert(tc, "Record count in name entry should be 1.",
        VmDnsNameEntryGetRecordCount(pNameEntry) == 1);

    dwError = VmDnsNameEntryDeleteRecord(pNameEntry, pRecord2, FALSE);
    CuAssert(tc, "Deleting second address record should succeed.", !dwError);

    CuAssert(tc, "Record count in name entry should be 0.",
        VmDnsNameEntryGetRecordCount(pNameEntry) == 0);

    dwError = VmDnsNameEntryAddRecord(pNameEntry, pRecord1, FALSE);
    CuAssert(tc, "Adding address record to name entry should succeed.", !dwError);

    dwError = VmDnsDeleteNameEntry(pNameEntry, FALSE);
    CuAssert(tc, "Deleting name entry should succeed.", !dwError);

    VMDNS_FREE_RECORD(pRecord1);
    VMDNS_FREE_RECORD(pRecord2);
}

void TestZoneCleanup(CuTest* tc)
{
    VmDnsCoreCleanup();
}

void TestServerInitialize(CuTest* tc)
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR pszZone = "vsphere.local";
    PSTR pszHost = "dc1.vsphere.local";
    VMDNS_IP6_ADDRESS      ipV6Addresses[] = {
                                                { { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
                                                { { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 } }
                                             };
    VMDNS_IP4_ADDRESS      ipV4Addresses[] = { 0xAC031F43 };
    VMDNS_INIT_INFO        initInfo = { 0 };

    initInfo.IpV4Addrs.Addrs = ipV4Addresses;
    initInfo.IpV6Addrs.Addrs = ipV6Addresses;
    initInfo.IpV4Addrs.dwCount = 1;
    initInfo.IpV6Addrs.dwCount = 2;
    initInfo.pszDcSrvName = pszHost;
    initInfo.pszDomain = pszZone;
    initInfo.wPort = VMDNS_DEFAULT_LDAP_PORT;

    VmDnsCoreInit(FALSE);

    dwError = VmDnsInitialize(NULL, &initInfo);
    CuAssert(tc, "DnsInit should succeed.", !dwError);

    VmDnsSetState(VMDNS_READY);

    dwError = VmDnsUninitialize(NULL, &initInfo);
    CuAssert(tc, "DnsInit should succeed.", !dwError);

    VmDnsCoreCleanup();
}

CuSuite* CuGetZoneSuite(void)
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, TestZoneInit);
    SUITE_ADD_TEST(suite, TestZoneCreate);
    SUITE_ADD_TEST(suite, TestZoneUpdate);
    SUITE_ADD_TEST(suite, TestAddressRecord);
    SUITE_ADD_TEST(suite, TestSrvRecord);
    SUITE_ADD_TEST(suite, TestNSRecord);
    SUITE_ADD_TEST(suite, TestSoaRecord);
    SUITE_ADD_TEST(suite, TestNameEntry);
    SUITE_ADD_TEST(suite, TestZoneCleanup);
    SUITE_ADD_TEST(suite, TestServerInitialize);

    return suite;
}

static
PVMDNS_RECORD
CreateAddressRecord(
    PCSTR   pszName,
    DWORD   address,
    PCSTR   pszZone
)
{
    PVMDNS_RECORD pRecord = NULL;
    PSTR          pszHostName = NULL;

    VmDnsAllocateStringA(pszName, &pszHostName);
    VmDnsAllocateMemory(sizeof(VMDNS_RECORD), (void**)&pRecord);
    VmDnsTrimDomainNameSuffix(pszHostName, pszZone);
    VmDnsAllocateStringA(pszHostName, &pRecord->pszName);
    pRecord->Data.A.IpAddress = address;
    pRecord->dwType = VMDNS_RR_TYPE_A;
    pRecord->iClass = VMDNS_CLASS_IN;

    return pRecord;
}

static
PVMDNS_RECORD
CreateAAAARecord(
    PCSTR   pszName,
    BYTE    bytes[16],
    PCSTR   pszZone
)
{
    PVMDNS_RECORD pRecord = NULL;
    PSTR          pszHostName = NULL;

    VmDnsAllocateStringA(pszName, &pszHostName);
    VmDnsAllocateMemory(sizeof(VMDNS_RECORD), (void**)&pRecord);
    VmDnsTrimDomainNameSuffix(pszHostName, pszZone);
    VmDnsAllocateStringA(pszHostName, &pRecord->pszName);
    memcpy(pRecord->Data.AAAA.Ip6Address.IP6Byte, bytes, sizeof(bytes));
    pRecord->dwType = VMDNS_RR_TYPE_AAAA;
    pRecord->iClass = VMDNS_CLASS_IN;

    return pRecord;
}

static
PVMDNS_RECORD
CreateServiceRecord(
    PSTR                    pszDomain,
    VMDNS_SERVICE_TYPE      service,
    VMDNS_SERVICE_PROTOCOL  protocol,
    PSTR                    pszTarget,
    UINT16                  port,
    UINT16                  priority,
    UINT16                  weight
)
{
    PVMDNS_RECORD pRecord = NULL;


    VmDnsAllocateMemory(sizeof(VMDNS_RECORD), (void**)&pRecord);
    VmDnsAllocateStringA(pszTarget, &pRecord->Data.SRV.pNameTarget);
    VmDnsAllocateStringA(pszDomain, &pRecord->pszName);
    pRecord->Data.SRV.wPort = port;
    pRecord->Data.SRV.wPriority = priority;
    pRecord->Data.SRV.wWeight = weight;
    pRecord->dwType = VMDNS_RR_TYPE_SRV;
    pRecord->iClass = VMDNS_CLASS_IN;

    return pRecord;
}

static
PVMDNS_RECORD
CreateNSRecord(
    PSTR    pszName,
    PSTR    pszTarget,
    UINT16  port,
    UINT16  priority,
    UINT16  weight
)
{
    PVMDNS_RECORD pRecord = NULL;

    VmDnsAllocateMemory(sizeof(VMDNS_RECORD), (void**)&pRecord);
    VmDnsAllocateStringA(pszName, &pRecord->pszName);
    VmDnsAllocateStringA(pszTarget, &pRecord->Data.NS.pNameHost);
    pRecord->dwType = VMDNS_RR_TYPE_NS;
    pRecord->iClass = VMDNS_CLASS_IN;

    return pRecord;
}

static
void
VerifyTestZone(
    CuTest* tc,
    PSTR pszZoneName
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD_ARRAY pRecords = NULL;
    PVMDNS_ZONE pZone = NULL;

    dwError = VmDnsZoneFindByName(pszZoneName, &pZone);
    CuAssert(tc, "Find test zone by name should succeed.", !dwError);

    dwError = VmDnsZoneQuery(pZone->pszName, VMDNS_SOA_RECORD_NAME, VMDNS_RR_TYPE_SOA, &pRecords);
    CuAssert(tc, "Zones query of SOA record should succeed.", !dwError);
    CuAssert(tc, "Zones query of SOA record should return one record.",
        pRecords->dwCount == 1);

    printf("%s SOA name: %s\n", __FUNCTION__, pRecords->Records[0].pszName);
    CuAssert(tc, "SOA record name should be @.",
        VmDnsStringCompareA(pRecords->Records[0].pszName, VMDNS_SOA_RECORD_NAME, FALSE) == 0);

    VMDNS_FREE_RECORD_ARRAY(pRecords);
}
