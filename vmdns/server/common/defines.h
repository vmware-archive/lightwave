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

#define VMW_DNS_DEFAULT_THREAD_COUNT (1)

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
    VM_DNS_OPCODE_QUERY = 0,
    VM_DNS_OPCODE_IQUERY = 1,
    VM_DNS_OPCODE_STATUS = 2,
    VM_DNS_OPCODE_UPDATE = 5
} VM_DNS_OPCODE;

typedef enum
{
    VM_DNS_RCODE_NOERROR = 0,
    VM_DNS_RCODE_FORMAT_ERROR = 1,
    VM_DNS_RCODE_SERVER_FAILURE = 2,
    VM_DNS_RCODE_NAME_ERROR = 3,
    VM_DNS_RCODE_NOT_IMPLEMENTED = 4,
    VM_DNS_RCODE_REFUSED = 5
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

#ifndef VMDNS_FORM_HEADER
#define VMDNS_FORM_HEADER(pVmDnsHeader) \
    (pVmDnsHeader->codes.QR & 0x1) << 15 | \
    (pVmDnsHeader->codes.opcode & 0xf) << 10 | \
    (pVmDnsHeader->codes.AA & 0x1) << 10 | \
    (pVmDnsHeader->codes.TC & 0x1) << 9 | \
    (pVmDnsHeader->codes.RD & 0x1) << 8 | \
    (pVmDnsHeader->codes.RA & 0x1) << 7 | \
    (pVmDnsHeader->codes.RCODE & 0xf);
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* DEFINES_H */

