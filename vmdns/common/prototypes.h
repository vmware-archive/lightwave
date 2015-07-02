/*
 * Copyright (C) 2011 VMware, Inc. All rights reserved.
 *
 * Module   : prototypes.h
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            Common Utilities (Client & Server)
 *
 */

/* threading.c */

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
VmDnsCompareRecordCommon(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    );

DWORD
VmDnsGetDomainNameLength(
    PSTR pszDomainName,
    PUINT16 puSize
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

DWORD VmDnsDuplicateSoaRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD VmDnsRpcDuplicateSoaRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD VmDnsCopySoaRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD VmDnsRpcCopySoaRecord(
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

DWORD VmDnsDuplicateSrvRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD VmDnsRpcDuplicateSrvRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD VmDnsCopySrvRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD VmDnsRpcCopySrvRecord(
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

DWORD VmDnsDuplicateNSRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD VmDnsRpcDuplicateNSRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD VmDnsCopyNSRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD VmDnsRpcCopyNSRecord(
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
VmDnsSerializeDnsNSRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );
DWORD
VmDnsDeserializeDnsNSRecord(
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

DWORD VmDnsDuplicateAddressRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD VmDnsRpcDuplicateAddressRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    );

DWORD VmDnsCopyAddressRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD VmDnsRpcCopyAddressRecord(
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
VmDnsSerializeDnsAddressRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsDeserializeDnsAddressRecord(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA pData
    );

