/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

#include "includes.h"

VMDNS_RECORD_TYPE_NAME_MAP gRecordTypeMap[] =
{
    { VMDNS_RR_TYPE_SOA,    "SOA"   },
    { VMDNS_RR_TYPE_SRV,    "SRV"   },
    { VMDNS_RR_TYPE_A,      "A"     },
    { VMDNS_RR_TYPE_AAAA,   "AAAA"  },
    { VMDNS_RR_TYPE_NS,     "NS"    },
    { VMDNS_RR_TYPE_CNAME,  "CNAME" },
    { VMDNS_RR_TYPE_PTR,    "PTR"   }
};
DWORD gRecordTypeMapSize = sizeof(gRecordTypeMap) / sizeof(VMDNS_RECORD_TYPE_NAME_MAP);

VMDNS_SERVICE_TYPE_NAME_MAP gServiceNameMap[] =
{
    { DOMAIN_CONTROLLER, "_ldap",   "ldap" }
};
DWORD gServiceNameMapSize = sizeof(gServiceNameMap) / sizeof(VMDNS_SERVICE_TYPE_NAME_MAP);

VMDNS_SERVICE_PROTOCOL_NAME_MAP gProtocolNameMap[] =
{
    { SERVICE_PROTOCOL_TCP, "_tcp", "tcp" },
    { SERVICE_PROTOCOL_UDP, "_udp", "udp" }
};
DWORD gProtocolNameMapSize = sizeof(gProtocolNameMap) / sizeof(VMDNS_SERVICE_PROTOCOL_NAME_MAP);

VMDNS_RECORD_METHODS gRecordMethods[] =
{
    {VMDNS_RR_TYPE_SOA, VmDnsCompareSoaRecord,
                        VmDnsMatchSoaRecord,
                        VmDnsClearSoaRecord,
                        VmDnsRpcClearSoaRecord,
                        VmDnsDuplicateSoaRecord,
                        VmDnsRpcDuplicateSoaRecord,
                        VmDnsCopySoaRecord,
                        VmDnsRpcCopySoaRecord,
                        VmDnsSoaRecordToString,
                        VmDnsSoaRecordGetCN,
                        VmDnsSerializeDnsSoaRecord,
                        VmDnsDeserializeDnsSoaRecord},
    {VMDNS_RR_TYPE_SRV, VmDnsCompareSrvRecord,
                        VmDnsMatchSrvRecord,
                        VmDnsClearSrvRecord,
                        VmDnsRpcClearSrvRecord,
                        VmDnsDuplicateSrvRecord,
                        VmDnsRpcDuplicateSrvRecord,
                        VmDnsCopySrvRecord,
                        VmDnsRpcCopySrvRecord,
                        VmDnsSrvRecordToString,
                        VmDnsSrvRecordGetCN,
                        VmDnsSerializeDnsSrvRecord,
                        VmDnsDeserializeDnsSrvRecord},
    {VMDNS_RR_TYPE_NS,  VmDnsCompareNSRecord,
                        VmDnsCompareNSRecord,
                        VmDnsClearNSRecord,
                        VmDnsRpcClearNSRecord,
                        VmDnsDuplicateNSRecord,
                        VmDnsRpcDuplicateNSRecord,
                        VmDnsCopyNSRecord,
                        VmDnsRpcCopyNSRecord,
                        VmDnsNSRecordToString,
                        VmDnsNSRecordGetCN,
                        VmDnsSerializeDnsNSRecord,
                        VmDnsDeserializeDnsNSRecord},
    {VMDNS_RR_TYPE_A,   VmDnsCompareAddressRecord,
                        VmDnsCompareAddressRecord,
                        VmDnsClearAddressRecord,
                        VmDnsRpcClearAddressRecord,
                        VmDnsDuplicateAddressRecord,
                        VmDnsRpcDuplicateAddressRecord,
                        VmDnsCopyAddressRecord,
                        VmDnsRpcCopyAddressRecord,
                        VmDnsAddressRecordToString,
                        VmDnsAddressRecordGetCN,
                        VmDnsSerializeDnsAddressRecord,
                        VmDnsDeserializeDnsAddressRecord}
};
DWORD gRecordMethodMapSize = sizeof(gRecordMethods) / sizeof(VMDNS_RECORD_METHODS);
