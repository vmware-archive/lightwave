//
// Created by sakhardandea on 6/13/17.
//

/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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
#include<stdio.h>
#include "CuTest.h"
#include "../server/common/includes.h"
#include "../server/common/prototypes.h"

extern int vmdns_syslog_level;

#define DEFAULT_EXPIRE  (60*60*24*30*6)
#define DEFAULT_TTL     (60*30)
#define DEFAULT_SERIAL  1
#define DEFAULT_REFRESH (60*60)
#define DEFAULT_RETRY   (60*10)

static
PVMDNS_RECORD
CreatePtrRecord(
        PCSTR pszIpAddress,
        PCSTR pszHostname,
        PCSTR pszZone
)
{
    PVMDNS_RECORD pRecord = NULL;
    PSTR         pszPtrName = NULL;

    VmDnsGeneratePtrNameFromIp(pszIpAddress,&pszPtrName);
    VmDnsAllocateMemory(sizeof(VMDNS_RECORD), (void**)&pRecord);
    VmDnsAllocateStringA(pszPtrName, &pRecord->pszName);
    VmDnsAllocateStringPrintfA(&pRecord->Data.PTR.pNameHost,"%s.",pszHostname);
    pRecord->dwType = VMDNS_RR_TYPE_PTR;
    pRecord->iClass = VMDNS_CLASS_IN;
    pRecord->dwTtl = VMDNS_DEFAULT_TTL;

    return pRecord;
}

void TestPtrRecord(CuTest *tc) {
    DWORD dwError = 0;
    PVMDNS_RECORD pRecord;
    PSTR pszIpAddress = "1.2.3.4";
    PSTR pszHostname = "test";
    PSTR pszZone = "in-addr.arpa.";
    PVMDNS_RECORD_LIST pRecordList = NULL;

    pRecord = CreatePtrRecord(pszIpAddress, pszHostname, pszZone);

    dwError = VmDnsStoreAddZoneRecord(pszZone, pRecord);
    CuAssert(tc, "Adding Record should succeed", !dwError);

    dwError = VmDnsStoreGetRecords(pszZone, pRecord->pszName, &pRecordList);
    CuAssert(tc, "Querying Records should succeed", !dwError);
    CuAssert(tc, "Ipv4 pointer record count should be 1", pRecordList->dwCurrentSize==1);

    dwError = VmDnsStoreDeleteZoneRecord(pszZone,pRecord);
    CuAssert(tc, "Deleting Records should succeed", !dwError);

    PSTR pszPtrName = NULL;
    VmDnsGeneratePtrNameFromIp(pszIpAddress,&pszPtrName);

    VMDNS_FREE_RECORD(pRecord);
    VmDnsRecordListRelease(pRecordList);
    pRecordList = NULL;

    dwError = VmDnsStoreGetRecords(pszZone, pszPtrName, &pRecordList);
    CuAssert(tc, "Querying Records should fail", dwError);

    pszIpAddress = "::1:2:3";
    pszZone = "ip6.arpa.";
    pRecordList = NULL;

    pRecord = CreatePtrRecord(pszIpAddress, pszHostname, pszZone);

    dwError = VmDnsStoreAddZoneRecord(pszZone, pRecord);
    CuAssert(tc, "Adding Record should succeed", !dwError);

    dwError = VmDnsStoreGetRecords(pszZone, pRecord->pszName, &pRecordList);
    CuAssert(tc, "Querying Records should succeed", !dwError);
    CuAssert(tc, "Ipv4 pointer record count should be 1", pRecordList->dwCurrentSize==1);

    dwError = VmDnsStoreDeleteZoneRecord(pszZone,pRecord);
    CuAssert(tc, "Deleting Records should succeed", !dwError);

    pszPtrName = NULL;
    VmDnsGeneratePtrNameFromIp(pszIpAddress,&pszPtrName);

    VMDNS_FREE_RECORD(pRecord);
    VmDnsRecordListRelease(pRecordList);
    pRecordList = NULL;

    dwError = VmDnsStoreGetRecords(pszZone, pszPtrName, &pRecordList);
    CuAssert(tc, "Querying Records should fail", dwError);
    }

CuSuite *CuGetPtrSuite(void) {
    CuSuite *suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, TestPtrRecord);

    return suite;
}


