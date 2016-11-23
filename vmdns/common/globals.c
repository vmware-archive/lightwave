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

VMDNS_RECORD_TYPE_NAME_MAP gRecordTypeMap[] =
{
    { VMDNS_RR_TYPE_SOA,    "SOA"   },
    { VMDNS_RR_TYPE_SRV,    "SRV"   },
    { VMDNS_RR_TYPE_A,      "A"     },
    { VMDNS_RR_TYPE_AAAA,   "AAAA"  },
    { VMDNS_RR_TYPE_NS,     "NS"    },
    { VMDNS_RR_TYPE_CNAME,  "CNAME" },
    { VMDNS_RR_TYPE_PTR,    "PTR"   },
    { VMDNS_RR_MTYPE_TKEY,  "TKEY"  },
    { VMDNS_RR_MTYPE_TSIG,  "TSIG"  }
};
DWORD gRecordTypeMapSize = sizeof(gRecordTypeMap) / sizeof(VMDNS_RECORD_TYPE_NAME_MAP);

VMDNS_SERVICE_TYPE_NAME_MAP gServiceNameMap[] =
{
    { DOMAIN_CONTROLLER, "_ldap",       "ldap" },
    { KERBEROS,          "_kerberos",   "kerberos" }
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
    {VMDNS_RR_TYPE_SOA,     VmDnsCompareSoaRecord,
                            VmDnsMatchSoaRecord,
                            VmDnsClearSoaRecord,
                            VmDnsRpcClearSoaRecord,
                            VmDnsValidateSoaRecord,
                            VmDnsDuplicateSoaRecord,
                            VmDnsRpcDuplicateSoaRecord,
                            VmDnsCopySoaRecord,
                            VmDnsRpcCopySoaRecord,
                            VmDnsSoaRecordToString,
                            VmDnsSoaRecordGetCN,
                            VmDnsSoaGetRDataLength,
                            VmDnsSerializeDnsSoaRecord,
                            VmDnsDeserializeDnsSoaRecord},
    {VMDNS_RR_TYPE_SRV,     VmDnsCompareSrvRecord,
                            VmDnsMatchSrvRecord,
                            VmDnsClearSrvRecord,
                            VmDnsRpcClearSrvRecord,
                            VmDnsValidateSrvRecord,
                            VmDnsDuplicateSrvRecord,
                            VmDnsRpcDuplicateSrvRecord,
                            VmDnsCopySrvRecord,
                            VmDnsRpcCopySrvRecord,
                            VmDnsSrvRecordToString,
                            VmDnsSrvRecordGetCN,
                            VmDnsSrvGetRDataLength,
                            VmDnsSerializeDnsSrvRecord,
                            VmDnsDeserializeDnsSrvRecord},
    {VMDNS_RR_TYPE_CNAME,   VmDnsCompareCNameRecord,
                            VmDnsCompareCNameRecord,
                            VmDnsClearCNameRecord,
                            VmDnsRpcClearCNameRecord,
                            VmDnsValidateCNameRecord,
                            VmDnsDuplicateCNameRecord,
                            VmDnsRpcDuplicateCNameRecord,
                            VmDnsCopyCNameRecord,
                            VmDnsRpcCopyCNameRecord,
                            VmDnsCNameRecordToString,
                            VmDnsCNameRecordGetCN,
                            VmDnsCNameRecordGetRDataLength,
                            VmDnsSerializeDnsCNameRecord,
                            VmDnsDeserializeDnsCNameRecord},
    {VMDNS_RR_TYPE_NS,      VmDnsCompareNSRecord,
                            VmDnsCompareNSRecord,
                            VmDnsClearNSRecord,
                            VmDnsRpcClearNSRecord,
                            VmDnsValidateNSRecord,
                            VmDnsDuplicateNSRecord,
                            VmDnsRpcDuplicateNSRecord,
                            VmDnsCopyNSRecord,
                            VmDnsRpcCopyNSRecord,
                            VmDnsNSRecordToString,
                            VmDnsNSRecordGetCN,
                            VmDnsNSRecordGetRDataLength,
                            VmDnsSerializeDnsNSRecord,
                            VmDnsDeserializeDnsNSRecord},
    {VMDNS_RR_TYPE_PTR,     VmDnsComparePtrRecord,
                            VmDnsComparePtrRecord,
                            VmDnsClearPtrRecord,
                            VmDnsRpcClearPtrRecord,
                            VmDnsValidatePtrRecord,
                            VmDnsDuplicatePtrRecord,
                            VmDnsRpcDuplicatePtrRecord,
                            VmDnsCopyPtrRecord,
                            VmDnsRpcCopyPtrRecord,
                            VmDnsPtrRecordToString,
                            VmDnsPtrRecordGetCN,
                            VmDnsPtrRecordGetRDataLength,
                            VmDnsSerializeDnsPtrRecord,
                            VmDnsDeserializeDnsPtrRecord},
    {VMDNS_RR_TYPE_A,       VmDnsCompareAddressRecord,
                            VmDnsCompareAddressRecord,
                            VmDnsClearAddressRecord,
                            VmDnsRpcClearAddressRecord,
                            VmDnsValidateAddressRecord,
                            VmDnsDuplicateAddressRecord,
                            VmDnsRpcDuplicateAddressRecord,
                            VmDnsCopyAddressRecord,
                            VmDnsRpcCopyAddressRecord,
                            VmDnsAddressRecordToString,
                            VmDnsAddressRecordGetCN,
                            VmDnsAddressRecordGetRDataLength,
                            VmDnsSerializeDnsAddressRecord,
                            VmDnsDeserializeDnsAddressRecord},
    {VMDNS_RR_TYPE_AAAA,    VmDnsCompareIp6AddressRecord,
                            VmDnsCompareIp6AddressRecord,
                            VmDnsClearIp6AddressRecord,
                            VmDnsRpcClearIp6AddressRecord,
                            VmDnsValidateIp6AddressRecord,
                            VmDnsDuplicateIp6AddressRecord,
                            VmDnsRpcDuplicateIp6AddressRecord,
                            VmDnsCopyIp6AddressRecord,
                            VmDnsRpcCopyIp6AddressRecord,
                            VmDnsIp6AddressRecordToString,
                            VmDnsIp6AddressRecordGetCN,
                            VmDnsIp6AddressRecordGetRDataLength,
                            VmDnsSerializeDnsIp6AddressRecord,
                            VmDnsDeserializeDnsIp6AddressRecord},
    {VMDNS_RR_MTYPE_TKEY,   VmDnsCompareTkeyRecord,
                            VmDnsCompareTkeyRecord,
                            VmDnsClearTkeyRecord,
                            VmDnsRpcClearTkeyRecord,
                            VmDnsValidateTkeyRecord,
                            VmDnsDuplicateTkeyRecord,
                            VmDnsRpcDuplicateTkeyRecord,
                            VmDnsCopyTkeyRecord,
                            VmDnsRpcCopyTkeyRecord,
                            VmDnsTkeyRecordToString,
                            VmDnsTkeyRecordGetCN,
                            VmDnsTkeyRecordGetRDataLength,
                            VmDnsSerializeDnsTkeyRecord,
                            VmDnsDeserializeDnsTkeyRecord},
    {VMDNS_RR_MTYPE_TSIG,   VmDnsCompareTsigRecord,
                            VmDnsCompareTsigRecord,
                            VmDnsClearTsigRecord,
                            VmDnsRpcClearTsigRecord,
                            VmDnsValidateTsigRecord,
                            VmDnsDuplicateTsigRecord,
                            VmDnsRpcDuplicateTsigRecord,
                            VmDnsCopyTsigRecord,
                            VmDnsRpcCopyTsigRecord,
                            VmDnsTsigRecordToString,
                            VmDnsTsigRecordGetCN,
                            VmDnsTsigRecordGetRDataLength,
                            VmDnsSerializeDnsTsigRecord,
                            VmDnsDeserializeDnsTsigRecord}
};
DWORD gRecordMethodMapSize = sizeof(gRecordMethods) / sizeof(VMDNS_RECORD_METHODS);
