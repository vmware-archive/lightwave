#include "CuTest.h"
#include "../server/common/includes.h"
#include "vmdnscommon.h"

extern int  vmdns_syslog_level;

#define DEFAULT_EXPIRE  (60*60*24*30*6)
#define DEFAULT_TTL     (60*30)
#define DEFAULT_SERIAL  1
#define DEFAULT_REFRESH (60*60)
#define DEFAULT_RETRY   (60*10)

static
DWORD
CachePurgeProc(
    PVMDNS_NAME_ENTRY pNameEntry,
    PVMDNS_ZONE_OBJECT pZoneObject
    )
{
    return 0;
}

void
TestLruInitialize(
    CuTest* tc
    )
{
    DWORD dwError = 0;
    PVMDNS_LRU_LIST pList = NULL;
    VMDNS_ZONE_INFO zi = {0};
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;

    zi.pszName ="lw.local";
    zi.pszPrimaryDnsSrvName = "dns.lw.local";
    zi.pszRName = "admin@lw.local";
    zi.expire = DEFAULT_EXPIRE;
    zi.minimum = DEFAULT_TTL;
    zi.serial = DEFAULT_SERIAL;
    zi.retryInterval = DEFAULT_RETRY;
    zi.refreshInterval = DEFAULT_REFRESH;

    dwError = VmDnsZoneCreate(&zi, &pZoneObject);
    CuAssert(tc, "Zones create should succeed.", !dwError);

    dwError = VmDnsLruInitialize(pZoneObject, CachePurgeProc, &pList);
    CuAssert(tc, "VmDnsLruInitialize: Error code check.", !dwError);
    CuAssert(tc, "VmDnsLruInitialize: Error code check.", !!pList);
    CuAssert(tc, "VmDnsLruInitialize: Zero entries.", pList->dwCurrentCount == 0);
    CuAssert(tc, "VmDnsLruInitialize: Lower threshold set.", pList->dwLowerThreshold != 0);
    CuAssert(tc, "VmDnsLruInitialize: Upper threshold set.", pList->dwUpperThreshold != 0);
    CuAssert(tc, "VmDnsLruInitialize: Max size set.", pList->dwMaxCount != 0);
    VmDnsLruFree(pList);
    CuAssert(tc, "VmDnsLruFree succeeded.", TRUE);
}

void
TestLruAddRemoveList(
    CuTest* tc
    )
{
    DWORD dwError = 0;
    VMDNS_NAME_ENTRY ne1 = { 1, { NULL, NULL }, "lw.local", NULL };
    PVMDNS_LRU_LIST pList = NULL;
    VMDNS_ZONE_INFO zi = {0};
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;

    zi.pszName ="lw.local";
    zi.pszPrimaryDnsSrvName = "dns.lw.local";
    zi.pszRName = "admin@lw.local";
    zi.expire = DEFAULT_EXPIRE;
    zi.minimum = DEFAULT_TTL;
    zi.serial = DEFAULT_SERIAL;
    zi.retryInterval = DEFAULT_RETRY;
    zi.refreshInterval = DEFAULT_REFRESH;

    dwError = VmDnsZoneCreate(&zi, &pZoneObject);
    CuAssert(tc, "Zones create should succeed.", !dwError);

    dwError = VmDnsLruInitialize(pZoneObject, CachePurgeProc, &pList);
    CuAssert(tc, "VmDnsLruInitialize: Allocation check.", !!pList);

    dwError = VmDnsLruAddNameEntry(pList, &ne1);
    CuAssert(tc, "VmDnsLruInitialize: Allocation check.", !dwError);
    CuAssert(tc, "VmDnsLruInitialize: Size incremented.", pList->dwCurrentCount == 1);
    dwError = VmDnsLruRemoveNameEntry(pList, &ne1);
    CuAssert(tc, "VmDnsLruInitialize: Allocation check.", !dwError);
    CuAssert(tc, "VmDnsLruInitialize: Size incremented.", pList->dwCurrentCount == 0);

    VmDnsLruFree(pList);
    CuAssert(tc, "VmDnsLruFree succeeded.", TRUE);
}

CuSuite* CuGetLruSuite(void)
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, TestLruInitialize);
    SUITE_ADD_TEST(suite, TestLruAddRemoveList);

    return suite;
}
