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

static
DWORD
VmDnsCliCreateZone(
    PVM_DNS_CLI_CONTEXT pContext
    );

static
DWORD
VmDnsCliListZone(
    PVM_DNS_CLI_CONTEXT pContext
    );

static
DWORD
VmDnsCliListRecord(
    PVM_DNS_CLI_CONTEXT pContext
    );

static
DWORD
VmDnsCliQueryRecord(
    PVM_DNS_CLI_CONTEXT pContext
    );

static
DWORD
VmDnsCliAddRecord(
    PVM_DNS_CLI_CONTEXT pContext
    );

static
DWORD
VmDnsCliDelRecord(
    PVM_DNS_CLI_CONTEXT pContext
    );

static
VOID
VmDnsCliPrintZone(
    PVMDNS_ZONE_INFO pZoneInfo
    );

static
VOID
VmDnsCliPrintZoneArray(
    PVMDNS_ZONE_INFO_ARRAY pZoneInfoArray
    );

static
DWORD
VmDnsCliValidateAndCompleteRecord(
    PVM_DNS_CLI_CONTEXT pContext
    );

static
DWORD
VmDnsCliValidateRecordInput(
    PVM_DNS_CLI_CONTEXT pContext
    );

static
DWORD
VmDnsCliListForwarders(
    PVM_DNS_CLI_CONTEXT pContext
    );

DWORD
VmDnsCliExecute(
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    switch (pContext->action)
    {
        case VM_DNS_ACTION_ADD_ZONE:

            dwError = VmDnsCliCreateZone(pContext);

            break;

        case VM_DNS_ACTION_DEL_ZONE:

            dwError = VmDnsDeleteZoneA(pContext->pServerContext, pContext->pszZone);

            break;

        case VM_DNS_ACTION_LIST_ZONES:
            dwError = VmDnsCliListZone(pContext);

            break;

        case VM_DNS_ACTION_ADD_RECORD:

            dwError = VmDnsCliAddRecord(pContext);

            break;

        case VM_DNS_ACTION_DEL_RECORD:

            dwError = VmDnsCliDelRecord(pContext);

            break;

        case VM_DNS_ACTION_LIST_RECORDS:

            dwError = VmDnsCliListRecord(pContext);

            break;

        case VM_DNS_ACTION_QUERY_RECORD:

            dwError = VmDnsCliQueryRecord(pContext);

            break;

        case VM_DNS_ACTION_ADD_FORWARDER:

            dwError = VmDnsAddForwarderA(
                            pContext->pServerContext,
                            pContext->pszForwarder);

            break;

        case VM_DNS_ACTION_LIST_FORWARDERS:

            dwError = VmDnsCliListForwarders(pContext);

            break;

        case VM_DNS_ACTION_DEL_FORWARDER:

            dwError = VmDnsDeleteForwarderA(
                            pContext->pServerContext,
                            pContext->pszForwarder);

            break;

        default:

            dwError = ERROR_INVALID_PARAMETER;

            break;
    }

    return dwError;
}

static
DWORD
VmDnsCliCreateZone(
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    PSTR  pszMboxDomain         = NULL;
    VMDNS_ZONE_INFO zoneInfo = { 0 };
    VMDNS_RECORD nsRecord = {0};
    VMDNS_RECORD addrRecord = {0};
    PSTR pszTargetFQDN = NULL;
    int ret = 0;
    int af = AF_INET;
    unsigned char buf[sizeof(struct in6_addr)];

    if (IsNullOrEmptyString(pContext->pszZone))
    {
        fprintf(stderr, "Error: DNS Zone is not specified\n");

        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (IsNullOrEmptyString(pContext->pszNSHost)
       || VmDnsCheckIfIPV4AddressA(pContext->pszNSHost)
       || VmDnsCheckIfIPV6AddressA(pContext->pszNSHost))
    {
        fprintf(stderr, "Error: Primary Nameserver host is not specified or the format is invalid\n");

        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (pContext->dwZoneType == VMDNS_ZONE_TYPE_FORWARD &&
        IsNullOrEmptyString(pContext->pszNSIp))
    {
        fprintf(stderr, "Error: Primary Nameserver IP Address is not specified\n");

        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (IsNullOrEmptyString(pContext->pszMboxDomain))
    {
        dwError = VmDnsAllocateStringA("hostmaster", &pszMboxDomain);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsMakeFQDN(pContext->pszNSHost,
                           pContext->pszZone,
                           &pszTargetFQDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    zoneInfo.pszName                = pContext->pszZone;
    zoneInfo.pszPrimaryDnsSrvName   = pszTargetFQDN;
    zoneInfo.pszRName               = pszMboxDomain ? pszMboxDomain : pContext->pszMboxDomain;
    zoneInfo.serial                 = 1;
    zoneInfo.refreshInterval        = VMDNS_DEFAULT_REFRESH_INTERVAL;
    zoneInfo.retryInterval          = VMDNS_DEFAULT_RETRY_INTERVAL;
    zoneInfo.expire                 = VMDNS_DEFAULT_EXPIRE;
    zoneInfo.minimum                = VMDNS_DEFAULT_TTL;
    zoneInfo.dwFlags                = 0;
    zoneInfo.dwZoneType             = VmDnsIsReverseZoneName(zoneInfo.pszName) ?
                                        VMDNS_ZONE_TYPE_REVERSE : VMDNS_ZONE_TYPE_FORWARD;

    dwError = VmDnsCreateZoneA(pContext->pServerContext, &zoneInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!IsNullOrEmptyString(pContext->pszNSHost))
    {
        nsRecord.pszName = pContext->pszZone;
        nsRecord.Data.NS.pNameHost = pszTargetFQDN;
        nsRecord.iClass   = VMDNS_CLASS_IN;
        nsRecord.pszName = pContext->pszZone;
        nsRecord.dwType    = VMDNS_RR_TYPE_NS;
        nsRecord.dwTtl     = pContext->record.dwTtl;

        dwError = VmDnsAddRecordA(
                    pContext->pServerContext,
                    pContext->pszZone,
                    &nsRecord);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pContext->pszNSHost) &&
        !IsNullOrEmptyString(pContext->pszNSIp) &&
        pContext->dwZoneType == VMDNS_ZONE_TYPE_FORWARD)

    {
        addrRecord.iClass   = VMDNS_CLASS_IN;
        addrRecord.pszName  = pszTargetFQDN;
        addrRecord.dwType   = VMDNS_RR_TYPE_A;
        addrRecord.dwTtl    = pContext->record.dwTtl;

        if (VmDnsStringChrA(pContext->pszNSIp, ':'))
        {
            af = AF_INET6;
        }

        ret = inet_pton(af, pContext->pszNSIp, buf);
        if (ret <= 0)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        if (af == AF_INET)
        {
            addrRecord.Data.A.IpAddress =
                   (VMDNS_IP4_ADDRESS)ntohl(((struct in_addr *)buf)->s_addr);
        }
        else if (af == AF_INET6)
        {
            addrRecord.dwType = VMDNS_RR_TYPE_AAAA;
            dwError = VmDnsCopyMemory(
                addrRecord.Data.AAAA.Ip6Address.IP6Byte,
                sizeof(addrRecord.Data.AAAA.Ip6Address.IP6Byte),
#ifdef _WIN32
                ((struct in6_addr*)buf)->u.Byte,
#else
                ((struct in6_addr*)buf)->s6_addr,
#endif
                sizeof(addrRecord.Data.AAAA.Ip6Address.IP6Byte));
            BAIL_ON_VMDNS_ERROR(dwError);
        }
        else
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        dwError = VmDnsAddRecordA(
                    pContext->pServerContext,
                    pContext->pszZone,
                    &addrRecord);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    if (pszMboxDomain)
    {
        VmDnsFreeStringA(pszMboxDomain);
    }

    return dwError;

error:

    goto cleanup;
}

static
VOID
VmDnsCliPrintZone(
    PVMDNS_ZONE_INFO pZoneInfo
    )
{
    if (pZoneInfo)
    {
        printf("Name:               %s\n", pZoneInfo->pszName);
        printf("Type:               %s\n",
                    VmDnsIsReverseZoneName(pZoneInfo->pszName) ?
                        "Reverse" : "Forward");
        printf("Serial:             %u\n", pZoneInfo->serial);
        printf("Primary DNS server: %s\n", pZoneInfo->pszPrimaryDnsSrvName);
        printf("Administrator:      %s\n", pZoneInfo->pszRName);
        printf("Expiration:         %u\n", pZoneInfo->expire);
        printf("Minimum TTL:        %u\n", pZoneInfo->minimum);
        printf("Refresh interval:   %u\n", pZoneInfo->refreshInterval);
        printf("Retry interval:     %u\n", pZoneInfo->retryInterval);
    }
}

static
VOID
VmDnsCliPrintZoneArray(
    PVMDNS_ZONE_INFO_ARRAY pZoneInfoArray
    )
{
    if (pZoneInfoArray)
    {
        DWORD idx = 0;
        if (pZoneInfoArray)
        {
            printf("Total number of zones: %u\n\n", pZoneInfoArray->dwCount);
            for (; idx < pZoneInfoArray->dwCount; ++idx)
            {
                printf("Zone # %u:\n", idx + 1);
                VmDnsCliPrintZone(&pZoneInfoArray->ZoneInfos[idx]);
                if (idx <pZoneInfoArray->dwCount - 1)
                {
                    printf("\n");
                }
            }
        }
    }
}

static
VOID
VmDnsCliPrintRecord(
    PVMDNS_RECORD pRecord
    )
{
    PSTR pszRecord = NULL;
    if (pRecord)
    {
        if (!VmDnsRecordToString(pRecord, &pszRecord))
        {
            printf("%s\n", pszRecord);
            VMDNS_SAFE_FREE_STRINGA(pszRecord);
        }
    }
}

static
VOID
VmDnsCliPrintRecordArray(
    PVMDNS_RECORD_ARRAY pRecordArray
    )
{
    if (pRecordArray)
    {
        DWORD idx = 0;
        if (pRecordArray)
        {
            printf("Total number of record: %u\n\n", pRecordArray->dwCount);
            for (; idx < pRecordArray->dwCount; ++idx)
            {
                printf("Record # %u:\n", idx + 1);
                VmDnsCliPrintRecord(&pRecordArray->Records[idx]);
                if (idx <pRecordArray->dwCount - 1)
                {
                    printf("\n");
                }
            }
        }
    }
}

static
DWORD
VmDnsCliListZone(
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    PVMDNS_ZONE_INFO_ARRAY pZoneInfos = NULL;

    dwError = VmDnsListZoneA(pContext->pServerContext, &pZoneInfos);
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsCliPrintZoneArray(pZoneInfos);

cleanup:
    VmDnsFreeZoneInfoArray(pZoneInfos);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDnsCliListRecord(
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD_ARRAY pRecordArray = NULL;

    dwError = VmDnsListRecordsA(pContext->pServerContext,
                                pContext->pszZone,
                                &pRecordArray);
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsCliPrintRecordArray(pRecordArray);

cleanup:
    VmDnsFreeRecordArray(pRecordArray);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDnsCliQueryRecord(
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD_ARRAY pRecordArray = NULL;

    if (IsNullOrEmptyString(pContext->pszZone) ||
        IsNullOrEmptyString(pContext->record.pszName) ||
        pContext->record.dwType == VMDNS_RR_TYPE_NONE)
    {
        fprintf(
            stderr,
            "Zone, record name and type are required for querying record.\n");
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsQueryRecordsA(pContext->pServerContext,
                                pContext->pszZone,
                                pContext->record.pszName,
                                pContext->record.dwType,
                                0, // Options - Reserved
                                &pRecordArray);
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsCliPrintRecordArray(pRecordArray);

cleanup:
    VmDnsFreeRecordArray(pRecordArray);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDnsCliAddRecord(
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    pContext->record.iClass = VMDNS_CLASS_IN;
    dwError = VmDnsCliValidateAndCompleteRecord(pContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAddRecordA(pContext->pServerContext,
                            pContext->pszZone,
                            &pContext->record);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDnsCliDelRecord(
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    DWORD idx = 0;
    PVMDNS_RECORD_ARRAY pRecordArray = NULL;
    BOOL bFound = FALSE;
    PSTR pszTargetFQDN = NULL;

    if (pContext->record.dwType == VMDNS_RR_TYPE_SOA)
    {
        fprintf(stdout, "SOA record cannot be deleted.\n");
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCliValidateAndCompleteRecord(pContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pContext->record.dwType == VMDNS_RR_TYPE_SRV)
    {
        dwError = VmDnsMakeFQDN(pContext->record.Data.SRV.pNameTarget,
                        pContext->pszZone,
                        &pszTargetFQDN);
        BAIL_ON_VMDNS_ERROR(dwError);

        if (pszTargetFQDN)
        {
            VMDNS_SAFE_FREE_STRINGA(pContext->record.Data.SRV.pNameTarget);
            pContext->record.Data.SRV.pNameTarget = pszTargetFQDN;
            pszTargetFQDN = NULL;
        }
    }
    if (pContext->record.dwType == VMDNS_RR_TYPE_NS)
    {
        dwError = VmDnsMakeFQDN(pContext->record.Data.NS.pNameHost,
                                pContext->pszZone,
                                &pszTargetFQDN);
        BAIL_ON_VMDNS_ERROR(dwError);
        if (pszTargetFQDN)
        {
            VMDNS_SAFE_FREE_STRINGA(pContext->record.Data.NS.pNameHost);
            pContext->record.Data.NS.pNameHost = pszTargetFQDN;
            pszTargetFQDN = NULL;
        }
        DWORD dwNameLen = strlen(pContext->record.pszName);
        if (pContext->record.pszName[dwNameLen -1] != '.')
        {
            VmDnsAllocateStringPrintfA(&pszTargetFQDN,
                                       "%s.",
                                       pContext->record.pszName);
            VMDNS_SAFE_FREE_STRINGA(pContext->record.pszName);
            pContext->record.pszName = pszTargetFQDN;
            pszTargetFQDN = NULL;
        }
    }
    else
    {
        VmDnsTrimDomainNameSuffix(pContext->record.pszName, pContext->pszZone);

        dwError = VmDnsMakeFQDN(
                    pContext->record.pszName,
                    pContext->pszZone,
                    &pszTargetFQDN);
        BAIL_ON_VMDNS_ERROR(dwError);

        if (pszTargetFQDN)
        {
            VMDNS_SAFE_FREE_STRINGA(pContext->record.pszName);
            pContext->record.pszName = pszTargetFQDN;
            pszTargetFQDN = NULL;
        }
    }
    dwError = VmDnsQueryRecordsA(
                    pContext->pServerContext,
                    pContext->pszZone,
                    pContext->record.pszName,
                    pContext->record.dwType,
                    0,
                    &pRecordArray);
    BAIL_ON_VMDNS_ERROR(dwError);
    for (; idx < pRecordArray->dwCount; ++idx)
    {

        if (VmDnsMatchRecord(&pRecordArray->Records[idx],
                             &pContext->record))
        {
            dwError = VmDnsDeleteRecordA(
                            pContext->pServerContext,
                            pContext->pszZone,
                            &pRecordArray->Records[idx]
                            );
            BAIL_ON_VMDNS_ERROR(dwError);
            bFound = TRUE;
        }
    }

error:
    if (!bFound)
    {
        fprintf(stderr, "Error: no matching record found.\n");
    }
    VMDNS_SAFE_FREE_STRINGA(pszTargetFQDN);
    VmDnsFreeRecordArray(pRecordArray);
    return dwError;
}

static
DWORD
VmDnsCliValidateAndCompleteRecord(
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    PSTR pszTargetFQDN = NULL;

    pContext->record.iClass = VMDNS_CLASS_IN;
    pContext->record.dwTtl = VMDNS_DEFAULT_TTL;

    dwError = VmDnsCliValidateRecordInput(pContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    switch (pContext->record.dwType)
    {
        case VMDNS_RR_TYPE_A:
        case VMDNS_RR_TYPE_AAAA:

            dwError = VmDnsMakeFQDN(
                        pContext->record.pszName,
                        pContext->pszZone,
                        &pszTargetFQDN);
            BAIL_ON_VMDNS_ERROR(dwError);

            if (pszTargetFQDN)
            {
                VMDNS_SAFE_FREE_STRINGA(pContext->record.pszName);
                pContext->record.pszName = pszTargetFQDN;
                pszTargetFQDN = NULL;
            }
            break;

        case VMDNS_RR_TYPE_SRV:
            dwError = VmDnsAllocateStringPrintfA(&pContext->record.pszName,
                                                "%s.%s.%s",
                                                pContext->pszService,
                                                pContext->pszProtocol,
                                                pContext->pszZone);
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsMakeFQDN(pContext->record.Data.SRV.pNameTarget,
                        pContext->pszZone,
                        &pszTargetFQDN);
            BAIL_ON_VMDNS_ERROR(dwError);

            if (pszTargetFQDN)
            {
                VMDNS_SAFE_FREE_STRINGA(pContext->record.Data.SRV.pNameTarget);
                pContext->record.Data.SRV.pNameTarget = pszTargetFQDN;
                pszTargetFQDN = NULL;
            }
            break;

        case VMDNS_RR_TYPE_NS:
            dwError = VmDnsMakeFQDN(pContext->record.Data.NS.pNameHost,
                                    pContext->pszZone,
                                    &pszTargetFQDN);
            BAIL_ON_VMDNS_ERROR(dwError);

            if (pszTargetFQDN)
            {
                VMDNS_SAFE_FREE_STRINGA(pContext->record.Data.NS.pNameHost);
                pContext->record.Data.NS.pNameHost = pszTargetFQDN;
                pszTargetFQDN = NULL;
            }
            DWORD dwNameLen = strlen(pContext->record.pszName);
            if (pContext->record.pszName[dwNameLen -1] != '.')
            {
                dwError = VmDnsAllocateStringPrintfA(&pszTargetFQDN,
                                                    "%s.",
                                                    pContext->record.pszName);
                BAIL_ON_VMDNS_ERROR(dwError);
                if (pszTargetFQDN)
                {
                     VMDNS_SAFE_FREE_STRINGA(pContext->record.pszName);
                     pContext->record.pszName = pszTargetFQDN;
                     pszTargetFQDN = NULL;
                }
            }
            break;

        case VMDNS_RR_TYPE_PTR:
            if (!VmDnsStringStrA(pContext->record.pszName, pContext->pszZone))
            {
                fprintf(stderr,
                    "Error: IP address doesn't belong to "
                    "the given reverse zone.\n");
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDNS_ERROR(dwError);
            }
            dwError = VmDnsMakeFQDN(
                            pContext->record.pszName,
                            pContext->pszZone,
                            &pszTargetFQDN);
            BAIL_ON_VMDNS_ERROR(dwError);

            if (pszTargetFQDN)
            {
                VMDNS_SAFE_FREE_STRINGA(pContext->record.pszName);
                pContext->record.pszName = pszTargetFQDN;
                pszTargetFQDN = NULL;
            }
            break;

        case VMDNS_RR_TYPE_CNAME:
            dwError = VmDnsMakeFQDN(pContext->record.Data.CNAME.pNameHost,
                                    pContext->pszZone,
                                    &pszTargetFQDN);
            BAIL_ON_VMDNS_ERROR(dwError);
            if (pszTargetFQDN)
            {
                VMDNS_SAFE_FREE_STRINGA(pContext->record.Data.CNAME.pNameHost);
                pContext->record.Data.NS.pNameHost = pszTargetFQDN;
                pszTargetFQDN = NULL;
            }

            dwError = VmDnsMakeFQDN(
                        pContext->record.pszName,
                        pContext->pszZone,
                        &pszTargetFQDN);
            BAIL_ON_VMDNS_ERROR(dwError);

            if (pszTargetFQDN)
            {
                VMDNS_SAFE_FREE_STRINGA(pContext->record.pszName);
                pContext->record.pszName = pszTargetFQDN;
                pszTargetFQDN = NULL;
            }

            break;

        default:

            dwError = ERROR_NOT_SUPPORTED;

            break;
    }

    if (IsNullOrEmptyString(pContext->record.pszName))
    {
        fprintf(stderr, "Error: DNS Record name is not specified\n");

        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDnsCliValidateRecordInput(
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pContext->pszZone))
    {
        fprintf(stderr, "Error: DNS Zone is not specified\n");

        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (pContext->record.dwType == VMDNS_RR_TYPE_NONE)
    {
        fprintf(stderr, "Error: DNS Record type is not specified\n");

        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (pContext->record.dwType == VMDNS_RR_TYPE_SRV)
    {
        if (!pContext->record.Data.SRV.pNameTarget)
        {
            fprintf(stderr, "Error: service target is not specified\n");
            dwError = ERROR_INVALID_PARAMETER;
        }
        if (!pContext->pszService)
        {
            fprintf(stderr, "Error: service type is not specified\n");
            dwError = ERROR_INVALID_PARAMETER;
        }
        if (!pContext->pszProtocol)
        {
            fprintf(stderr, "Error: service protocol is not specified\n");
            dwError = ERROR_INVALID_PARAMETER;
        }
        BAIL_ON_VMDNS_ERROR(dwError);
    }

error:
    return dwError;
}


static
DWORD
VmDnsCliListForwarders(
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    PVMDNS_FORWARDERS pForwarders = NULL;
    DWORD iForwarder = 0;

    dwError = VmDnsGetForwardersA(pContext->pServerContext, &pForwarders);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pForwarders->dwCount > 0)
    {
        fprintf(stdout, "Forwarders:\n");

        for (; iForwarder < pForwarders->dwCount; iForwarder++)
        {
            fprintf(stdout, "%s\n", pForwarders->ppszName[iForwarder]);
        }
    }

error:

    if (pForwarders)
    {
        VmDnsFreeForwarders(pForwarders);
    }

    return dwError;
}

