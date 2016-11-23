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
* Module Name: VMDNS
*
* Filename: vmdnstypes.h
*
* Abstract:
*
* Common types definition
*
*/

#ifndef __VMDNSTYPES_H__
#define __VMDNSTYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32
#include <lw/types.h>
#endif

#ifdef _DCE_IDL_
    cpp_quote("#include <vmdnstypes.h>")
        cpp_quote("#if 0")
#endif

typedef
#ifdef _DCE_IDL_
    [ptr, string]
#endif
    char *PDNS_STRING;

typedef
#ifdef _DCE_IDL_
    [ptr, string]
#endif
    unsigned short *PWDNS_STRING;

#define VMDNS_RR_TYPE_NONE   0x0000
#define VMDNS_RR_TYPE_A      0x0001
#define VMDNS_RR_TYPE_NS     0x0002
#define VMDNS_RR_TYPE_MD     0x0003 // obsoleted by MX
#define VMDNS_RR_TYPE_MF     0x0004 // obsoleted by MX
#define VMDNS_RR_TYPE_CNAME  0x0005
#define VMDNS_RR_TYPE_SOA    0x0006
#define VMDNS_RR_TYPE_MB     0x0007 // experimental
#define VMDNS_RR_TYPE_MG     0x0008 // experimental
#define VMDNS_RR_TYPE_MR     0x0009 // experimental
#define VMDNS_RR_TYPE_NULL   0x000A // experimental
#define VMDNS_RR_TYPE_WKS    0x000B
#define VMDNS_RR_TYPE_PTR    0x000C
#define VMDNS_RR_TYPE_HINFO  0x000D
#define VMDNS_RR_TYPE_MINFO  0x000E
#define VMDNS_RR_TYPE_MX     0x000F
#define VMDNS_RR_TYPE_TXT    0x0010
#define VMDNS_RR_TYPE_RP     0x0011
#define VMDNS_RR_TYPE_AFSDB  0x0012
#define VMDNS_RR_TYPE_SIG    0x0018
#define VMDNS_RR_TYPE_AAAA   0x001C
#define VMDNS_RR_TYPE_LOC    0x001D
#define VMDNS_RR_TYPE_SRV    0x0021
#define VMDNS_RR_TYPE_CERT   0x0025
#define VMDNS_RR_TYPE_DS     0x002B
#define VMDNS_RR_TYPE_SSHFP  0x002C
#define VMDNS_RR_TYPE_IPSEC  0x002D
#define VMDNS_RR_TYPE_RRSIG  0x002E
#define VMDNS_RR_TYPE_DNSKEY 0x0030

#define VMDNS_RR_MTYPE_OPT    0x0029
#define VMDNS_RR_MTYPE_TKEY   0x00F9
#define VMDNS_RR_MTYPE_TSIG   0x00FA

#define VMDNS_RR_QTYPE_AXFR   0x00FC
#define VMDNS_RR_QTYPE_MAILB  0x00FD
#define VMDNS_RR_QTYPE_MAILA  0x00FE // obsoleted by RFC 973
#define VMDNS_RR_QTYPE_ANY    0x00FF

typedef UINT16  VMDNS_RR_TYPE;
typedef UINT16  VMDNS_CLASS;
typedef INT32   VMDNS_TTL;
typedef UINT16  VMDNS_RDLENGTH;
typedef DWORD   VMDNS_IP4_ADDRESS;
typedef DWORD   VMDNS_ZONE_FLAGS;
typedef DWORD   VMDNS_ZONE_TYPE;
typedef UINT16  VMDNS_SERVICE_TYPE;
typedef UINT16  VMDNS_SERVICE_PROTOCOL;

#define VMDNS_CLASS_IN       0x0001
#define VMDNS_CLASS_CS       0x0002 // obsolete
#define VMDNS_CLASS_CH       0x0003
#define VMDNS_CLASS_HS       0x0004
#define VMDNS_CLASS_NONE     0x00FE
#define VMDNS_CLASS_ANY      0x00FF

#define VMDNS_ZONE_TYPE_FORWARD 0
#define VMDNS_ZONE_TYPE_REVERSE 1

#define VMDNS_LABEL_LENGTH_MAX      63
#define VMDNS_NAME_LENGTH_MAX       255

#define DNS_ZONE_FLAG_AD_INTEGRATED 0x0001
#define DNS_ZONE_FLAG_READ_ONLY     0x0002

#define SERVICE_TYPE_NONE 0
#define DOMAIN_CONTROLLER 1
#define KERBEROS          2

#define SERVICE_PROTOCOL_NONE   0
#define SERVICE_PROTOCOL_TCP    1
#define SERVICE_PROTOCOL_UDP    2

typedef struct _VMDNS_BLOB
{
    UINT16      unSize;  // size in octects
    PBYTE       pData;
} VMDNS_BLOB, *PVMDNS_BLOB;

typedef struct _VMDNS_IP6_ADDRESS
{
    BYTE                    IP6Byte[16];
}VMDNS_IP6_ADDRESS, *PVMDNS_IP6_ADDRESS;

typedef struct _VMDNS_A_DATA
{
    VMDNS_IP4_ADDRESS       IpAddress;
} VMDNS_A_DATA, *PVMDNS_A_DATA;

typedef struct _VMDNS_PTR_DATAW
{
    PWDNS_STRING            pNameHost;
} VMDNS_PTR_DATAW, *PVMDNS_PTR_DATAW;

typedef struct _VMDNS_PTR_DATAA
{
    PDNS_STRING             pNameHost;
} VMDNS_PTR_DATAA, *PVMDNS_PTR_DATAA;

typedef struct _VMDNS_NS_DATAA
{
    PWDNS_STRING pwszHostname;
} VMDNS_NS_DATAW, *PVMDNS_NS_DATAW;

typedef struct _VMDNS_SOA_DATAW
{
    PWDNS_STRING            pNamePrimaryServer;
    PWDNS_STRING            pNameAdministrator;
    DWORD                   dwSerialNo;
    DWORD                   dwRefresh;
    DWORD                   dwRetry;
    DWORD                   dwExpire;
    DWORD                   dwDefaultTtl;
} VMDNS_SOA_DATAW, *PVMDNS_SOA_DATAW;

typedef struct VMDNS_SOA_DATAA
{
    PDNS_STRING             pNamePrimaryServer;
    PDNS_STRING             pNameAdministrator;
    DWORD                   dwSerialNo;
    DWORD                   dwRefresh;
    DWORD                   dwRetry;
    DWORD                   dwExpire;
    DWORD                   dwDefaultTtl;
} VMDNS_SOA_DATAA, *PVMDNS_SOA_DATAA;

typedef struct _VMDNS_MINFO_DATAW
{
    PWDNS_STRING            pNameMailbox;
    PWDNS_STRING            pNameErrorsMailbox;
} VMDNS_MINFO_DATAW, *PVMDNS_MINFO_DATAW;

typedef struct _VMDNS_MINFO_DATAA
{
    PDNS_STRING             pNameMailbox;
    PDNS_STRING             pNameErrorsMailbox;
} VMDNS_MINFO_DATAA, *PVMDNS_MINFO_DATAA;

typedef struct _VMDNS_MX_DATAW
{
    PWDNS_STRING            pNameExchange;
    WORD                    wPreference;
    WORD                    Pad;        // keep ptrs DWORD aligned
} VMDNS_MX_DATAW, *PVMDNS_MX_DATAW;

typedef struct _VMDNS_MX_DATAA
{
    PDNS_STRING             pNameExchange;
    WORD                    wPreference;
    WORD                    Pad;        // keep ptrs DWORD aligned
} VMDNS_MX_DATAA, *PVMDNS_MX_DATAA;

typedef struct _VMDNS_TXT_DATAW
{
    DWORD                   dwStringCount;
#ifdef _DCE_IDL
    [size_is(dwStringCount)] PWDNS_STRING pStringArray[];
#else
    PWDNS_STRING            pStringArray[1];
#endif
} VMDNS_TXT_DATAW, *PVMDNS_TXT_DATAW;

typedef struct _VMDNS_TXT_DATAA
{
    DWORD           dwStringCount;
#ifdef _DCE_IDL
    [size_is(dwStringCount)] PDNS_STRING  pStringArray[];
#else
    PDNS_STRING             pStringArray[1];
#endif
} VMDNS_TXT_DATAA, *PVMDNS_TXT_DATAA;

typedef struct _VMDNS_NULL_DATA
{
    DWORD                   dwByteCount;
#ifdef _DCE_IDL
    [size_is(dwByteCount)] BYTE Data[];
#else
    BYTE                    Data[1];
#endif
} VMDNS_NULL_DATA, *PVMDNS_NULL_DATA;

typedef struct _VMDNS_WKS_DATA
{
    VMDNS_IP4_ADDRESS       IpAddress;
    UCHAR                   chProtocol;
    BYTE                    BitMask[1];
}
VMDNS_WKS_DATA, *PVMDNS_WKS_DATA;

typedef struct _VMDNS_AAAA_DATA
{
    VMDNS_IP6_ADDRESS       Ip6Address;
}
VMDNS_AAAA_DATA, *PVMDNS_AAAA_DATA;

typedef struct _VMDNS_SIG_DATAW
{
    WORD                    wTypeCovered;
    BYTE                    chAlgorithm;
    BYTE                    chLabelCount;
    DWORD                   dwOriginalTtl;
    DWORD                   dwExpiration;
    DWORD                   dwTimeSigned;
    WORD                    wKeyTag;
    WORD                    wSignatureLength;
    PWDNS_STRING            pNameSigner;
#ifdef _DCE_IDL
    [size_is(wSignatureLength)] BYTE  Signature[];
#else
    BYTE            Signature[1];
#endif
} VMDNS_SIG_DATAW, *PVMDNS_SIG_DATAW, VMDNS_RRSIG_DATAW, *PVMDNS_RRSIG_DATAW;

typedef struct _VMDNS_SIG_DATAA
{
    WORD                    wTypeCovered;
    BYTE                    chAlgorithm;
    BYTE                    chLabelCount;
    DWORD                   dwOriginalTtl;
    DWORD                   dwExpiration;
    DWORD                   dwTimeSigned;
    WORD                    wKeyTag;
    WORD                    wSignatureLength;
    PDNS_STRING             pNameSigner;
#ifdef _DCE_IDL
    [size_is(wSignatureLength)] BYTE  Signature[];
#else
    BYTE                    Signature[1];
#endif
} VMDNS_SIG_DATAA, *PVMDNS_SIG_DATAA, VMDNS_RRSIG_DATAA, *PVMDNS_RRSIG_DATAA;

typedef struct _VMDNS_KEY_DATA
{
    WORD                    wFlags;
    BYTE                    chProtocol;
    BYTE                    chAlgorithm;
    WORD                    wKeyLength;
    WORD                    wPad;            // keep byte field aligned
#ifdef _DCE_IDL
    [size_is(wKeyLength)] BYTE Key[];
#else
    BYTE                    Key[1];
#endif
} VMDNS_KEY_DATA, *PVMDNS_KEY_DATA, VMDNS_DNSKEY_DATA, *PVMDNS_DNSKEY_DATA;

typedef struct _VMDNS_DHCID_DATA
{
    DWORD                   dwByteCount;
#ifdef _DCE_IDL
    [size_is(dwByteCount)] BYTE DHCID[];
#else
    BYTE                    DHCID[1];
#endif
} VMDNS_DHCID_DATA, *PVMDNS_DHCID_DATA;

typedef struct _VMDNS_NSEC_DATAW
{
#ifdef _DCE_IDL_
    [string]
#endif
    PWDNS_STRING            pNextDomainName;
    WORD                    wTypeBitMapsLength;
    WORD                    wPad;            // keep byte field aligned
#ifdef _DCE_IDL
    [size_is(wTypeBitMapsLength)] BYTE  TypeBitMaps[];
#else
    BYTE            TypeBitMaps[1];
#endif
} VMDNS_NSEC_DATAW, *PVMDNS_NSEC_DATAW;

typedef struct VMDNS_NSEC_DATAA
{
#ifdef _DCE_IDL_
    [string]
#endif
    PDNS_STRING             pNextDomainName;
    WORD                    wTypeBitMapsLength;
    WORD                    wPad;            // keep byte field aligned
#ifdef _DCE_IDL
    [size_is(wTypeBitMapsLength)] BYTE  TypeBitMaps[];
#else
    BYTE                    TypeBitMaps[1];
#endif
} VMDNS_NSEC_DATAA, *PVMDNS_NSEC_DATAA;

typedef struct _VMDNS_NSEC3_DATA
{
    BYTE                    chAlgorithm;
    BYTE                    bFlags;
    WORD                    wIterations;
    BYTE                    bSaltLength;
    BYTE                    bHashLength;
    WORD                    wTypeBitMapsLength;
#ifdef _DCE_IDL
    [size_is(bSaltLength + bHashLength + wTypeBitMapsLength)] BYTE  chData[];
#else
    BYTE                    chData[1];
#endif
} VMDNS_NSEC3_DATA, *PVMDNS_NSEC3_DATA;

typedef struct _VMDNS_NSEC3PARAM_DATA
{
    BYTE                    chAlgorithm;
    BYTE                    bFlags;
    WORD                    wIterations;
    BYTE                    bSaltLength;
    BYTE                    bPad[3];        // keep salt field aligned
#ifdef _DCE_IDL
    [size_is(bSaltLength)] BYTE  pbSalt[];
#else
    BYTE                    pbSalt[1];
#endif
}
VMDNS_NSEC3PARAM_DATA, *PVMDNS_NSEC3PARAM_DATA;

typedef struct _VMDNS_DS_DATA
{
    WORD                    wKeyTag;
    BYTE                    chAlgorithm;
    BYTE                    chDigestType;
    WORD                    wDigestLength;
    WORD                    wPad;            // keep byte field aligned
#ifdef _DCE_IDL
    [size_is(wDigestLength)] BYTE  Digest[];
#else
    BYTE                    Digest[1];
#endif
} VMDNS_DS_DATA, *PVMDNS_DS_DATA;

typedef struct
{
    WORD                    wDataLength;
    WORD                    wPad;            // keep byte field aligned
#ifdef _DCE_IDL
    [size_is(wDataLength)] BYTE Data[];
#else
    BYTE                    Data[1];
#endif
}
VMDNS_OPT_DATA, *PVMDNS_OPT_DATA;

typedef struct
{
    WORD                    wVersion;
    WORD                    wSize;
    WORD                    wHorPrec;
    WORD                    wVerPrec;
    DWORD                   dwLatitude;
    DWORD                   dwLongitude;
    DWORD                   dwAltitude;
}
VMDNS_LOC_DATA, *PVMDNS_LOC_DATA;

typedef struct
{
    PWDNS_STRING            pNameNext;
    WORD                    wNumTypes;
#ifdef _DCE_IDL
    [size_is(wNumTypes)] WORD wTypes[];
#else
    WORD                    wTypes[1];
#endif
}
VMDNS_NXT_DATAW, *PVMDNS_NXT_DATAW;

typedef struct
{
#ifdef _DCE_IDL_
    [string]
#endif
    PDNS_STRING             pNameNext;
    WORD                    wNumTypes;
#ifdef _DCE_IDL
    [size_is(wNumTypes)] WORD wTypes[];
#else
    WORD                    wTypes[1];
#endif
}
VMDNS_NXT_DATAA, *PVMDNS_NXT_DATAA;

// Service record format:
// _service._proto.name. TTL class SRV priority weight port target.
typedef struct
{
    PWDNS_STRING            pNameTarget;
    WORD                    wPriority;
    WORD                    wWeight;
    WORD                    wPort;
    WORD                    Pad;            // keep ptrs DWORD aligned
}
VMDNS_SRV_DATAW, *PVMDNS_SRV_DATAW;

typedef struct
{
    PDNS_STRING             pNameTarget;
    WORD                    wPriority;
    WORD                    wWeight;
    WORD                    wPort;
    WORD                    Pad;            // keep ptrs DWORD aligned
}
VMDNS_SRV_DATAA, *PVMDNS_SRV_DATAA;

typedef struct
{
    WORD                    wOrder;
    WORD                    wPreference;
    PWDNS_STRING            pFlags;
    PWDNS_STRING            pService;
    PWDNS_STRING            pRegularExpression;
    PWDNS_STRING            pReplacement;
}
VMDNS_NAPTR_DATAW, *PVMDNS_NAPTR_DATAW;

typedef struct
{
    WORD                    wOrder;
    WORD                    wPreference;
    PDNS_STRING             pFlags;
    PDNS_STRING             pService;
    PDNS_STRING             pRegularExpression;
    PDNS_STRING             pReplacement;
}
VMDNS_NAPTR_DATAA, *PVMDNS_NAPTR_DATAA;

typedef struct
{
    PWDNS_STRING            pNameAlgorithm;
    PVMDNS_BLOB             pKey;
    PVMDNS_BLOB             pOtherData;
    DWORD                   dwCreateTime;
    DWORD                   dwExpireTime;
    WORD                    wMode;
    WORD                    wError;
}
VMDNS_TKEY_DATAW, *PVMDNS_TKEY_DATAW;

typedef struct
{
    PDNS_STRING             pNameAlgorithm;
    PVMDNS_BLOB             pKey;
    PVMDNS_BLOB             pOtherData;
    DWORD                   dwCreateTime;
    DWORD                   dwExpireTime;
    WORD                    wMode;
    WORD                    wError;
}
VMDNS_TKEY_DATAA, *PVMDNS_TKEY_DATAA;

typedef struct
{
    PWDNS_STRING            pNameAlgorithm;
    PVMDNS_BLOB             pSignature;
    PVMDNS_BLOB             pOtherData;
    UINT64                  unCreateTime;
    WORD                    wFudgeTime;
    WORD                    wOriginalXid;
    WORD                    wError;
    PBYTE                   pRawTsigPtr;
}
VMDNS_TSIG_DATAW, *PVMDNS_TSIG_DATAW;

typedef struct
{
    PDNS_STRING             pNameAlgorithm;
    PVMDNS_BLOB             pSignature;
    PVMDNS_BLOB             pOtherData;
    UINT64                  unCreateTime;
    WORD                    wFudgeTime;
    WORD                    wOriginalXid;
    WORD                    wError;
    PBYTE                   pRawTsigPtr;
}
VMDNS_TSIG_DATAA, *PVMDNS_TSIG_DATAA;

typedef
#ifdef _DCE_IDL_
[switch_type(UINT16)]
#endif
union _VMDNS_RECORD_DATA
{
#ifdef _DCE_IDL_
    [case(VMDNS_RR_TYPE_A)]
#endif
    VMDNS_A_DATA            A;
#ifdef _DCE_IDL_
    [case(VMDNS_RR_TYPE_AAAA)]
#endif
    VMDNS_AAAA_DATA         AAAA;
#ifdef _DCE_IDL_
    [case(VMDNS_RR_TYPE_NS)]
#endif
    VMDNS_PTR_DATAA         NS;
#ifdef _DCE_IDL_
    [case(VMDNS_RR_TYPE_PTR)]
#endif
    VMDNS_PTR_DATAA         PTR;
#ifdef _DCE_IDL_
    [case(VMDNS_RR_TYPE_CNAME)]
#endif
    VMDNS_PTR_DATAA         CNAME;
#ifdef _DCE_IDL_
    [case(VMDNS_RR_TYPE_SOA)]
#endif
    VMDNS_SOA_DATAA         SOA;
#ifdef _DCE_IDL_
    [case(VMDNS_RR_TYPE_SRV)]
#endif
     VMDNS_SRV_DATAA        SRV;
#ifdef _DCE_IDL_
    [case(VMDNS_RR_MTYPE_TKEY)]
#endif
    VMDNS_TKEY_DATAA        TKEY;
#ifdef _DCE_IDL_
    [case(VMDNS_RR_MTYPE_TSIG)]
#endif
    VMDNS_TSIG_DATAA        TSIG;

} VMDNS_RECORD_DATA;

typedef VMDNS_RECORD_DATA* PVMDNS_RECORD_DATA;

typedef struct _VMDNS_RECORD
{
    PDNS_STRING             pszName; // Max length is 255 octets
    VMDNS_RR_TYPE           dwType;
    VMDNS_CLASS             iClass;
    VMDNS_TTL               dwTtl;
#ifdef _DCE_IDL_
    [switch_is(dwType)]
#endif
    VMDNS_RECORD_DATA       Data;
} VMDNS_RECORD, *PVMDNS_RECORD;

typedef struct _VMDNS_RECORD_ARRAY
{
    DWORD dwCount;
#ifdef _DCE_IDL_
    [size_is(dwCount)]
#endif
    PVMDNS_RECORD Records;
} VMDNS_RECORD_ARRAY, *PVMDNS_RECORD_ARRAY;

typedef struct _VMDNS_ZONE_INFO
{
    PDNS_STRING             pszName;                // Max length is 255 octets
    PDNS_STRING             pszPrimaryDnsSrvName;   // Primary DNS server name
    PDNS_STRING             pszRName;               // Mailbox of responsible person
    UINT32                  serial;                  // version of zone data.
    UINT32                  refreshInterval;         // Refresh interval
    UINT32                  retryInterval;           // Retry of refresh interval
    UINT32                  expire;                  // upper limit of being authoritative.
    UINT32                  minimum;                 // Minimum TTL
    VMDNS_ZONE_FLAGS        dwFlags;
    VMDNS_ZONE_TYPE         dwZoneType;
} VMDNS_ZONE_INFO, *PVMDNS_ZONE_INFO;

typedef struct _VMDNS_ZONE_INFO_ARRAY
{
    DWORD                   dwCount;
#ifdef _DCE_IDL_
    [size_is(dwCount)]
#endif
    PVMDNS_ZONE_INFO        ZoneInfos;
} VMDNS_ZONE_INFO_ARRAY, *PVMDNS_ZONE_INFO_ARRAY;

typedef struct _PVMDNS_FORWARDERS
{
    UINT32                  dwCount;
#ifdef _DCE_IDL_
    [size_is(dwCount)]
#endif
    PDNS_STRING*            ppszName;
} VMDNS_FORWARDERS, *PVMDNS_FORWARDERS;

typedef struct _VMDNS_IP4_ADDRESS_ARRAY
{
    DWORD dwCount;
#ifdef _DCE_IDL_
    [size_is(dwCount)]
#endif
    VMDNS_IP4_ADDRESS *Addrs;
} VMDNS_IP4_ADDRESS_ARRAY, *PVMDNS_IP4_ADDRESS_ARRAY;

typedef struct _VMDNS_IP6_ADDRESS_ARRAY
{
    DWORD dwCount;
#ifdef _DCE_IDL_
    [size_is(dwCount)]
#endif
    PVMDNS_IP6_ADDRESS Addrs;
} VMDNS_IP6_ADDRESS_ARRAY, *PVMDNS_IP6_ADDRESS_ARRAY;

typedef struct _VMDNS_INIT_INFO
{
#ifdef _DCE_IDL_
    [string]
#endif
    PSTR   pszDomain;
#ifdef _DCE_IDL_
    [string]
#endif
    PSTR   pszDcSrvName;
    VMDNS_IP4_ADDRESS_ARRAY     IpV4Addrs;
    VMDNS_IP6_ADDRESS_ARRAY     IpV6Addrs;
    WORD                        wPriority;
    WORD                        wWeight;
    WORD                        wPort;
    WORD                        wPad;
} VMDNS_INIT_INFO, *PVMDNS_INIT_INFO;

#ifdef _DCE_IDL_
    cpp_quote("#endif")
#endif

#ifdef __cplusplus
}
#endif

#endif /* __VMDNSTYPES_H__ */
