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
 * Module   : prototypes.h
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            Common Utilities (Client & Server)
 *
 */

DWORD
VmDnsInitializeMutexContent(
    PVMDNS_MUTEX            pMutex
);

VOID
VmDnsFreeMutexContent(
    PVMDNS_MUTEX            pMutex
);

DWORD
VmDnsInitializeConditionContent(
    PVMDNS_COND             pCondition
);

VOID
VmDnsFreeConditionContent(
    PVMDNS_COND             pCondition
);

/* config.c */
#ifdef _WIN32
DWORD
VmDnsWinCfgOpenConnection(
    PVMDNS_CFG_CONNECTION*  ppConnection
    );

DWORD
VmDnsWinCfgOpenRootKey(
    PVMDNS_CFG_CONNECTION   pConnection,
    PCSTR                   pszKeyName,
    DWORD                   dwOptions,
    DWORD                   dwAccess,
    PVMDNS_CFG_KEY*         ppKey
    );

DWORD
VmDnsWinCfgOpenKey(
    PVMDNS_CFG_CONNECTION   pConnection,
    PVMDNS_CFG_KEY          pKey,
    PCSTR                   pszSubKey,
    DWORD                   dwOptions,
    DWORD                   dwAccess,
    PVMDNS_CFG_KEY*         ppKey
    );

DWORD
VmDnsWinCfgReadStringValue(
    PVMDNS_CFG_KEY          pKey,
    PCSTR                   pszSubkey,
    PCSTR                   pszName,
    PSTR*                   ppszValue
    );

DWORD
VmDnsWinCfgReadDWORDValue(
    PVMDNS_CFG_KEY          pKey,
    PCSTR                   pszSubkey,
    PCSTR                   pszName,
    PDWORD                  pdwValue
    );

VOID
VmDnsWinCfgCloseKey(
    PVMDNS_CFG_KEY          pKey
    );

VOID
VmDnsWinCfgCloseConnection(
    PVMDNS_CFG_CONNECTION   pConnection
    );

PVMDNS_CFG_CONNECTION
VmDnsWinCfgAcquireConnection(
    PVMDNS_CFG_CONNECTION   pConnection
    );

VOID
VmDnsWinCfgFreeConnection(
    PVMDNS_CFG_CONNECTION   pConnection
    );

#else

DWORD
VmDnsPosixCfgOpenConnection(
    PVMDNS_CFG_CONNECTION* ppConnection
    );

DWORD
VmDnsPosixCfgOpenRootKey(
    PVMDNS_CFG_CONNECTION   pConnection,
    PCSTR                   pszKeyName,
    DWORD                   dwOptions,
    DWORD                   dwAccess,
    PVMDNS_CFG_KEY*         ppKey
    );

DWORD
VmDnsPosixCfgOpenKey(
    PVMDNS_CFG_CONNECTION   pConnection,
    PVMDNS_CFG_KEY          pKey,
    PCSTR                   pszSubKey,
    DWORD                   dwOptions,
    DWORD                   dwAccess,
    PVMDNS_CFG_KEY*         ppKey
    );

DWORD
VmDnsPosixCfgReadStringValue(
    PVMDNS_CFG_KEY  pKey,
    PCSTR           pszSubkey,
    PCSTR           pszName,
    PSTR*           ppszValue
    );

DWORD
VmDnsPosixCfgReadDWORDValue(
    PVMDNS_CFG_KEY  pKey,
    PCSTR           pszSubkey,
    PCSTR           pszName,
    PDWORD          pdwValue
    );

VOID
VmDnsPosixCfgCloseKey(
    PVMDNS_CFG_KEY pKey
    );

VOID
VmDnsPosixCfgCloseConnection(
    PVMDNS_CFG_CONNECTION pConnection
    );

#endif

/* records */

BOOLEAN
VmDnsFindRecordMethods(
    PVMDNS_RECORD   pRecord,
    DWORD           *pIdx
    );

BOOLEAN
VmDnsMatchRecordCommon(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    );

BOOLEAN
VmDnsCompareRecordCommon(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    );

BOOLEAN
VmDnsValidateRecordCommon(
    PVMDNS_RECORD   pRecord1
    );

DWORD
VmDnsGetDomainNameLength(
    PSTR pszDomainName,
    PUINT16 puSize,
    BOOL bTokenizeDomainName
    );

DWORD
VmDnsRecordTypeToString(
    VMDNS_RR_TYPE       type,
    PCSTR*              ppszName
    );

DWORD
VmDnsServiceTypeToString(
    VMDNS_SERVICE_TYPE  type,
    PCSTR*              ppszName
    );

DWORD
VmDnsProtocolToString(
    VMDNS_SERVICE_PROTOCOL  protocol,
    PCSTR*                  ppszName
    );

// SOA
BOOLEAN
VmDnsCompareSoaRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    );

BOOLEAN
VmDnsMatchSoaRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    );

VOID
VmDnsClearSoaRecord(
    PVMDNS_RECORD       pRecord
    );

VOID
VmDnsRpcClearSoaRecord(
    PVMDNS_RECORD       pRecord
    );

BOOLEAN
VmDnsValidateSoaRecord(
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsDuplicateSoaRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsRpcDuplicateSoaRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsCopySoaRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsRpcCopySoaRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsSoaRecordToString(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsSoaRecordGetCN(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsSoaGetRDataLength(
    VMDNS_RECORD_DATA   Data,
    PUINT16             puRDataLength,
    BOOL                bTokenizeDomainName
    );

DWORD
VmDnsSerializeDnsSoaRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsDeserializeDnsSoaRecord(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA pData
    );

// SRV
BOOLEAN
VmDnsCompareSrvRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    );

BOOLEAN
VmDnsMatchSrvRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    );

VOID
VmDnsClearSrvRecord(
    PVMDNS_RECORD       pRecord
    );

VOID
VmDnsRpcClearSrvRecord(
    PVMDNS_RECORD       pRecord
    );

BOOLEAN
VmDnsValidateSrvRecord(
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsDuplicateSrvRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsRpcDuplicateSrvRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsCopySrvRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsRpcCopySrvRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsSrvRecordToString(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsSrvRecordGetCN(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsSrvGetRDataLength(
    VMDNS_RECORD_DATA   Data,
    PUINT16             puRDataLength,
    BOOL                bTokenizeDomainName
    );

DWORD
VmDnsSerializeDnsSrvRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsDeserializeDnsSrvRecord(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA pData
    );

// NS
BOOLEAN
VmDnsCompareNSRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    );

VOID
VmDnsClearNSRecord(
    PVMDNS_RECORD       pRecord
    );

VOID
VmDnsRpcClearNSRecord(
    PVMDNS_RECORD       pRecord
    );

BOOLEAN
VmDnsValidateNSRecord(
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsDuplicateNSRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsRpcDuplicateNSRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsCopyNSRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsRpcCopyNSRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsNSRecordToString(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsNSRecordGetCN(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsNSRecordGetRDataLength(
    VMDNS_RECORD_DATA   Data,
    PUINT16             puRDataLength,
    BOOL                bTokenizeDomainName
    );

DWORD
VmDnsSerializeDnsNSRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsDeserializeDnsNSRecord(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA pData
    );

// PTR
BOOLEAN
VmDnsComparePtrRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    );

VOID
VmDnsClearPtrRecord(
    PVMDNS_RECORD       pRecord
    );

VOID
VmDnsRpcClearPtrRecord(
    PVMDNS_RECORD       pRecord
    );

BOOLEAN
VmDnsValidatePtrRecord(
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsDuplicatePtrRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsRpcDuplicatePtrRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsCopyPtrRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsRpcCopyPtrRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsPtrRecordToString(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsPtrRecordGetCN(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsPtrRecordGetRDataLength(
    VMDNS_RECORD_DATA   Data,
    PUINT16             puRDataLength,
    BOOL                bTokenizeDomainName
    );

DWORD
VmDnsSerializeDnsPtrRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsDeserializeDnsPtrRecord(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA pData
    );

// CNAME
BOOLEAN
VmDnsCompareCNameRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    );

VOID
VmDnsClearCNameRecord(
    PVMDNS_RECORD       pRecord
    );

VOID
VmDnsRpcClearCNameRecord(
    PVMDNS_RECORD       pRecord
    );

BOOLEAN
VmDnsValidateCNameRecord(
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsDuplicateCNameRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsRpcDuplicateCNameRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsCopyCNameRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsRpcCopyCNameRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsCNameRecordToString(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsCNameRecordGetCN(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsCNameRecordGetRDataLength(
    VMDNS_RECORD_DATA   Data,
    PUINT16             puRDataLength,
    BOOL                bTokenizeDomainName
    );

DWORD
VmDnsSerializeDnsCNameRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsDeserializeDnsCNameRecord(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA pData
    );

// IPV4 Address
BOOLEAN
VmDnsCompareAddressRecord(
    PVMDNS_RECORD       pRecord1,
    PVMDNS_RECORD       pRecord2
    );

VOID
VmDnsClearAddressRecord(
    PVMDNS_RECORD       pRecord
    );

VOID
VmDnsRpcClearAddressRecord(
    PVMDNS_RECORD       pRecord
    );

BOOLEAN
VmDnsValidateAddressRecord(
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsDuplicateAddressRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsRpcDuplicateAddressRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsCopyAddressRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsRpcCopyAddressRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsAddressRecordToString(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsAddressRecordGetCN(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsAddressRecordGetRDataLength(
    VMDNS_RECORD_DATA   Data,
    PUINT16             puRDataLength,
    BOOL                bTokenizeDomainName
    );

DWORD
VmDnsSerializeDnsAddressRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsDeserializeDnsAddressRecord(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA pData
    );

// IPV6 Address
BOOLEAN
VmDnsCompareIp6AddressRecord(
    PVMDNS_RECORD       pRecord1,
    PVMDNS_RECORD       pRecord2
    );

VOID
VmDnsClearIp6AddressRecord(
    PVMDNS_RECORD       pRecord
    );

VOID
VmDnsRpcClearIp6AddressRecord(
    PVMDNS_RECORD       pRecord
    );

BOOLEAN
VmDnsValidateIp6AddressRecord(
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsDuplicateIp6AddressRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsRpcDuplicateIp6AddressRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsCopyIp6AddressRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsRpcCopyIp6AddressRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsIp6AddressRecordToString(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsIp6AddressRecordGetCN(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsIp6AddressRecordGetRDataLength(
    VMDNS_RECORD_DATA   Data,
    PUINT16             puRDataLength,
    BOOL                bTokenizeDomainName
    );

DWORD
VmDnsSerializeDnsIp6AddressRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsDeserializeDnsIp6AddressRecord(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA pData
    );

// TKEY
BOOLEAN
VmDnsCompareTkeyRecord(
    PVMDNS_RECORD       pRecord1,
    PVMDNS_RECORD       pRecord2
    );

VOID
VmDnsClearTkeyRecord(
    PVMDNS_RECORD       pRecord
    );

VOID
VmDnsRpcClearTkeyRecord(
    PVMDNS_RECORD       pRecord
    );

BOOLEAN
VmDnsValidateTkeyRecord(
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsDuplicateTkeyRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsRpcDuplicateTkeyRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsCopyTkeyRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsRpcCopyTkeyRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsTkeyRecordToString(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsTkeyRecordGetCN(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsTkeyRecordGetRDataLength(
    VMDNS_RECORD_DATA   Data,
    PUINT16             puRDataLength,
    BOOL                bTokenizeDomainName
    );

DWORD
VmDnsSerializeDnsTkeyRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsDeserializeDnsTkeyRecord(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA pData
    );

// TSIG
BOOLEAN
VmDnsCompareTsigRecord(
    PVMDNS_RECORD       pRecord1,
    PVMDNS_RECORD       pRecord2
    );

VOID
VmDnsClearTsigRecord(
    PVMDNS_RECORD       pRecord
    );

VOID
VmDnsRpcClearTsigRecord(
    PVMDNS_RECORD       pRecord
    );

BOOLEAN
VmDnsValidateTsigRecord(
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsDuplicateTsigRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsRpcDuplicateTsigRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD
VmDnsCopyTsigRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsRpcCopyTsigRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsTsigRecordToString(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsTsigRecordGetCN(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    );

DWORD
VmDnsTsigRecordGetRDataLength(
    VMDNS_RECORD_DATA   Data,
    PUINT16             puRDataLength,
    BOOL                bTokenizeDomainName
    );

DWORD
VmDnsSerializeDnsTsigRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsDeserializeDnsTsigRecord(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA pData
    );


