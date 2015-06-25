/*
 * Copyright (c) VmDnsare Inc.  All rights Reserved.
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

    if (IsNullOrEmptyString(pContext->pszZone))
    {
        fprintf(stderr, "Error: DNS Zone is not specified\n");

        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (IsNullOrEmptyString(pContext->pszNSHost))
    {
        fprintf(stderr, "Error: Primary Nameserver host is not specified\n");

        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (IsNullOrEmptyString(pContext->pszNSIp))
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

    zoneInfo.pszName                = pContext->pszZone;
    zoneInfo.pszPrimaryDnsSrvName   = pContext->pszNSHost;
    zoneInfo.pszRName               = pszMboxDomain ? pszMboxDomain : pContext->pszMboxDomain;
    zoneInfo.serial                 = 1;
    zoneInfo.refreshInterval        = 3200;
    zoneInfo.retryInterval          = 3200;
    zoneInfo.expire                 = 0;           // upper limit of being authoritative.
    zoneInfo.minimum                = pContext->record.dwTtl;          // Minimum TTL
    zoneInfo.dwFlags                = 0;
    zoneInfo.dwZoneType             = 0;

    dwError = VmDnsCreateZoneA(pContext->pServerContext, &zoneInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

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
        printf("Type:               %u\n", pZoneInfo->dwZoneType);
        printf("Serial:             %u\n", pZoneInfo->serial);
        printf("Flags:              %u\n", pZoneInfo->dwFlags);
        printf("Primar DNS server:  %s\n", pZoneInfo->pszPrimaryDnsSrvName);
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

    if (pContext->record.dwType == VMDNS_RR_TYPE_SOA)
    {
        fprintf(stdout, "SOA record cannot be deleted.\n");
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCliValidateAndCompleteRecord(pContext);
    BAIL_ON_VMDNS_ERROR(dwError);

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
        }
    }

error:
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

    pContext->record.iClass = VMDNS_CLASS_IN;
    pContext->record.dwTtl = 3600;

    dwError = VmDnsCliValidateRecordInput(pContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    switch (pContext->record.dwType)
    {
        case VMDNS_RR_TYPE_A:
            break;

        case VMDNS_RR_TYPE_SRV:

            dwError = VmDnsAllocateStringPrintfA(&pContext->record.pszName,
                                                "%s.%s",
                                                pContext->pszService,
                                                pContext->pszProtocol);
            BAIL_ON_VMDNS_ERROR(dwError);

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
    PSTR* ppszForwarders = NULL;
    DWORD dwCount = 0;
    DWORD iForwarder = 0;

    dwError = VmDnsGetForwardersA(pContext->pServerContext, &ppszForwarders, &dwCount);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (dwCount > 0)
    {
        fprintf(stdout, "Forwarders:\n");

        for (; iForwarder < dwCount; iForwarder++)
        {
            fprintf(stdout, "%s\n", ppszForwarders[iForwarder]);
        }
    }

error:

    if (ppszForwarders)
    {
        VmDnsFreeStringCountedArrayA(ppszForwarders, dwCount);
    }

    return dwError;
}

