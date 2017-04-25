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


#ifndef DEFINES_H
#define	DEFINES_H

#ifdef	__cplusplus
extern "C" {
#endif

#define VMW_DNS_PORT (53)

#define VMW_DNS_DEFAULT_LISTENER_QUEUE_COUNT (5)

#define VMW_DNS_DEFAULT_THREAD_COUNT (4)

#define ATTR_KRB_UPN  "userPrincipalName"
#define ATTR_MEMBEROF "memberOf"

#define VMDNS_REG_KEY_PORT          "Port"
#define VMDIR_REG_KEY_DC_ACCOUNT_DN "dcAccountDN"
#define VMDIR_REG_KEY_DC_PASSWORD   "dcAccountPassword"
#define VMDIR_REG_KEY_DC_ACCOUNT    "dcAccount"
#define VMAFD_REG_KEY_DOMAIN_NAME   "DomainName"
#define VMAFD_REG_KEY_PNID          "PNID"

#define VMDNS_DOMAINDNSZONES_NAME       "DomainDnsZones"

#define VMDNS_LDAP_SEARCH_TIMEOUT_SECS  (15)

#define VMDNS_LDAP_OC_TOP               "top"
#define VMDNS_LDAP_OC_DOMAIN            "domain"
#define VMDNS_LDAP_OC_DOMAINDNS         "domainDNS"
#define VMDNS_LDAP_OC_DNSZONE           "dnsZone"
#define VMDNS_LDAP_OC_DNSNODE           "dnsNode"
#define VMDNS_LDAP_OC_VMWDNSCONFIG      "vmwDNSConfig"

#define VMDNS_LDAP_ATTR_DC              "dc"
#define VMDNS_LDAP_ATTR_FORWARDERS      "vmwDNSForwarders"
#define VMDNS_LDAP_ATTR_NAME            "name"
#define VMDNS_LDAP_ATTR_DNS_RECORD      "dnsRecord"
#define VMDNS_LDAP_ATTR_OBJECTCLASS     "objectclass"
#define VMDNS_LDAP_ATTR_USNCHANGED      "USNChanged"
#define VMDNS_LDAP_ATTR_DNSANY          "dns*"
#define VMDNS_LDAP_ATTR_DNSBASEDN       "dc=DomainDnsZones,dc=vsphere,dc=local"
#define VMDNS_LDAP_ATTR_RUNTIMESTATUS   "vmwServerRunTimeStatus"
#define VMDNS_LDAP_ATTR_USN             "USN: "

#define VMDNS_LDAP_DELETE_CONTROL       "1.2.840.113556.1.4.417"
#define VMDNS_LDAP_DELETE_BASEDN        "cn=Deleted Objects,dc=vsphere,dc=local"
#define VMDNS_LDAP_DELETE_DELIMITER     "#"

#define VMDNS_REPL_BASEDN               "cn=replicationstatus"
#define VMDNS_REPL_FILTER               "objectclass=*"

#define VMDNS_LRU_SIZE                  (10000)
#define VMDNS_LRU_PURGE                 (10) //out of 100
#define VMDNS_LRU_PURGEFAST             (20) //out of 100
#define VMDNS_LRU_UPPERTHRES            (80) //out of 100
#define VMDNS_LRU_LOWERTHRES            (60) //out of 100
#define VMDNS_SEC_ALG_NAME "gss-tsig"
#define VMDNS_SEC_DEFAULT_FUDGE_TIME 300
#define VMDNS_FORWARDER_TIMEOUT 5

#ifndef _WIN32
#define VMDIR_CONFIG_PARAMETER_KEY_PATH "Services\\vmdir"
#define VMAFD_CONFIG_PARAMETER_KEY_PATH "Services\\vmafd\\Parameters"
#define VMDNS_CONFIG_PARAMETER_KEY_PATH "Services\\vmdns\\Parameters"
#else
#define VMDIR_CONFIG_PARAMETER_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\VMWareDirectoryService"
#define VMAFD_CONFIG_PARAMETER_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\VMWareAfdService\\Parameters"
#define VMDNS_CONFIG_PARAMETER_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\VMwareDNSService\\Parameters"
#endif


typedef enum
{
    VM_DNS_LDAP_ATTR_TYPE_UNKNOWN = 0,
    VM_DNS_LDAP_ATTR_TYPE_STRING,
    VM_DNS_LDAP_ATTR_TYPE_MULTISTRING
} VM_DNS_LDAP_ATTR_TYPE;

typedef enum
{
    VM_DNS_GSS_CTX_UNINITIALIZED = 0,
    VM_DNS_GSS_CTX_NEGOTIATING,
    VM_DNS_GSS_CTX_ESTABLISHED
} VM_DNS_GSS_CTX_STATE;

typedef enum
{
    VM_DNS_OPCODE_QUERY = 0,
    VM_DNS_OPCODE_IQUERY = 1,
    VM_DNS_OPCODE_STATUS = 2,
    VM_DNS_OPCODE_NOTIFY = 4,
    VM_DNS_OPCODE_UPDATE = 5
} VM_DNS_OPCODE;

typedef enum
{
    VM_DNS_TKEY_MODE_SERVER_ASSIGNMENT = 1,
    VM_DNS_TKEY_MODE_DIFFIE_HELLMAN = 2,
    VM_DNS_TKEY_MODE_GSS_API = 3,
    VM_DNS_TKEY_MODE_RESOLVER_ASSIGNMENT = 4,
    VM_DNS_TKEY_MODE_KEY_DELETION = 5
} VM_DNS_TKEY_MODE;

typedef enum
{
    VM_DNS_RCODE_NOERROR = 0,
    VM_DNS_RCODE_FORMAT_ERROR = 1,
    VM_DNS_RCODE_SERVER_FAILURE = 2,
    VM_DNS_RCODE_NAME_ERROR = 3,
    VM_DNS_RCODE_NOT_IMPLEMENTED = 4,
    VM_DNS_RCODE_REFUSED = 5,
    VM_DNS_RCODE_YXDOMAIN = 6,
    VM_DNS_RCODE_YXRRSET = 7,
    VM_DNS_RCODE_NXRRSET = 8,
    VM_DNS_RCODE_NOTAUTH = 9,
    VM_DNS_RCODE_NOTZONE = 10,
    VM_DNS_RCODE_BADSIG = 16,
    VM_DNS_RCODE_BADKEY = 17,
    VM_DNS_RCODE_BADTIME = 18,
    VM_DNS_RCODE_BADMODE = 19,
    VM_DNS_RCODE_BADNAME = 20,
    VM_DNS_RCODE_BADALG = 21
} VM_DNS_RCODE;

typedef enum
{
    VM_DNS_QUERY_OP_QUESTION = 0,
    VM_DNS_QUERY_OP_RESPONSE = 1
} VM_DNS_QUERY_OP;


#ifndef PopEntryList
#define PopEntryList(ListHead) \
    (ListHead)->Next;\
        {\
        PSINGLE_LIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Next;\
        if (FirstEntry != NULL) {     \
            (ListHead)->Next = FirstEntry->Next;\
                }                             \
        }
#endif

#ifndef PushEntryList
#define PushEntryList(ListHead,Entry) \
    (Entry)->Next = (ListHead)->Next; \
    (ListHead)->Next = (Entry)
#endif

#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (PCHAR)(address) - \
                                                  (ULONG_PTR)(&((type *)0)->field)))
#endif

#ifndef InitializeListHead

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink; \
{RemoveEntryList((ListHead)->Flink)}

#define RemoveTailList(ListHead) \
    (ListHead)->Blink; \
{RemoveEntryList((ListHead)->Blink)}

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink; \
    PLIST_ENTRY _EX_Flink; \
    _EX_Flink = (Entry)->Flink; \
    _EX_Blink = (Entry)->Blink; \
    _EX_Blink->Flink = _EX_Flink; \
    _EX_Flink->Blink = _EX_Blink; \
    }

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink; \
    PLIST_ENTRY _EX_ListHead; \
    _EX_ListHead = (ListHead); \
    _EX_Blink = _EX_ListHead->Blink; \
    (Entry)->Flink = _EX_ListHead; \
    (Entry)->Blink = _EX_Blink; \
    _EX_Blink->Flink = (Entry); \
    _EX_ListHead->Blink = (Entry); \
    }

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink; \
    PLIST_ENTRY _EX_ListHead; \
    _EX_ListHead = (ListHead); \
    _EX_Flink = _EX_ListHead->Flink; \
    (Entry)->Flink = _EX_Flink; \
    (Entry)->Blink = _EX_ListHead; \
    _EX_Flink->Blink = (Entry); \
    _EX_ListHead->Flink = (Entry); \
    }

#endif

#ifndef VMDNS_GET_BITS
#define VMDNS_GET_BITS(Data,Start,End) \
    ((((1 << (End-Start+1))-1) << Start) & \
    Data) >> Start
#endif

#ifndef VMDNS_FORM_HEADER
#define VMDNS_FORM_HEADER(pVmDnsHeader) \
    (pVmDnsHeader->codes.QR & 0x01) << 0x0f | \
    (pVmDnsHeader->codes.opcode & 0x0f) << 0x0b | \
    (pVmDnsHeader->codes.AA & 0x01) << 0x0a | \
    (pVmDnsHeader->codes.TC & 0x01) << 0x09 | \
    (pVmDnsHeader->codes.RD & 0x01) << 0x08 | \
    (pVmDnsHeader->codes.RA & 0x01) << 0x07 | \
    (pVmDnsHeader->codes.Z & 0x07) << 0x04 | \
    (pVmDnsHeader->codes.RCODE & 0x0f);
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* DEFINES_H */
