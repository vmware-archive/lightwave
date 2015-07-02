/*
 * Copyright (c) VmDnsare Inc.  All rights Reserved.
 */

#include "includes.h"

int  vmdns_syslog_level = VMDNS_LOG_LEVEL_INFO;
int  vmdns_syslog = 0;
int  vmdns_debug = 0;

static
DWORD
ParseArgs(
    int                  argc,
    char*                argv[],
    PVM_DNS_CLI_CONTEXT* ppContext
    );

static
DWORD
ParseArgsAddZone(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsDelZone(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsAddRecord(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsDelRecord(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsQueryRecords(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsListRecords(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsForwardersList(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext
    );

static
void
ShowUsage(
    VOID
    );

static
DWORD
VmDnsCliGetRecordData_A(
    PCSTR           pszIPAddress,
    PVMDNS_A_DATA   pData
    );

int main(int argc, char* argv[])
{
    DWORD dwError = 0;
    PVM_DNS_CLI_CONTEXT pContext = NULL;

#ifdef _WIN32
    WSADATA wsaData;
    dwError = WSAStartup(MAKEWORD(2, 2), &wsaData);
    BAIL_ON_VMDNS_ERROR(dwError);
#endif

    dwError = ParseArgs(argc, argv, &pContext);
    if (dwError == ERROR_INVALID_PARAMETER)
    {
        ShowUsage();
    }
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCliExecute(pContext);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    if (pContext)
    {
        VmDnsCliFreeContext(pContext);
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return dwError;

error:
    fprintf(
        stderr,
        "vmdns-cli failed, error %u\n",
        dwError);
    goto cleanup;
}


static
DWORD
ParseArgs(
    int                  argc,
    char*                argv[],
    PVM_DNS_CLI_CONTEXT* ppContext
    )
{
    DWORD dwError = 0;
    PVM_DNS_CLI_CONTEXT pContext = NULL;
    DWORD iArg = 0;
    DWORD dwArgsLeft = argc;
    PSTR  pszArg = NULL;
    PVMDNS_SERVER_CONTEXT pServerContext = NULL;

    if (!argc || !argv)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    iArg++; // skip first argument
    dwArgsLeft--;

    if (!dwArgsLeft)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pszArg = argv[iArg++];
    dwArgsLeft--;

    dwError = VmDnsAllocateMemory(sizeof(*pContext), (PVOID*)&pContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    pContext->record.dwType = VMDNS_RR_TYPE_NONE;

    if (!strcmp(pszArg, "help"))
    {
        dwError = ERROR_INVALID_PARAMETER;
    }
    else if (!strcmp(pszArg, "add-zone"))
    {
        pContext->action = VM_DNS_ACTION_ADD_ZONE;

        dwError = ParseArgsAddZone(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "del-zone"))
    {
        pContext->action = VM_DNS_ACTION_DEL_ZONE;

        dwError = ParseArgsDelZone(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "list-zones"))
    {
        pContext->action = VM_DNS_ACTION_LIST_ZONES;
    }
    else if (!strcmp(pszArg, "add-record"))
    {
        pContext->action = VM_DNS_ACTION_ADD_RECORD;

        dwError = ParseArgsAddRecord(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "del-record"))
    {
        pContext->action = VM_DNS_ACTION_DEL_RECORD;

        dwError = ParseArgsDelRecord(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "list-records"))
    {
        pContext->action = VM_DNS_ACTION_LIST_RECORDS;

        dwError = ParseArgsListRecords(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "query-record"))
    {
        pContext->action = VM_DNS_ACTION_QUERY_RECORD;

        dwError = ParseArgsQueryRecords(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "add-forwarder"))
    {
        pContext->action = VM_DNS_ACTION_ADD_FORWARDER;

        dwError = ParseArgsForwardersList(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "del-forwarder"))
    {
        pContext->action = VM_DNS_ACTION_DEL_FORWARDER;

        dwError = ParseArgsForwardersList(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "list-forwarders"))
    {
        pContext->action = VM_DNS_ACTION_LIST_FORWARDERS;
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
    }
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsOpenServerA("localhost", NULL, NULL, NULL, 0, NULL, &pServerContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    pContext->pServerContext = pServerContext;
    pServerContext = NULL;

    *ppContext = pContext;

cleanup:

    return dwError;

error:

    *ppContext = NULL;

    if (pServerContext)
    {
        VmDnsCloseServer(pServerContext);
    }

    if (pContext)
    {
        VmDnsCliFreeContext(pContext);
    }

    goto cleanup;
}

static
DWORD
ParseArgsAddZone(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_ADD_ZONE_OPEN = 0,
        PARSE_MODE_ADD_ZONE_NS_HOST,
        PARSE_MODE_ADD_ZONE_NS_IP,
        PARSE_MODE_ADD_ZONE_MBOX_DOMAIN
    } PARSE_MODE_ADD_ZONE;
    PARSE_MODE_ADD_ZONE parseMode = PARSE_MODE_ADD_ZONE_OPEN;
    int iArg = 0;

    if (!argc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    for (; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];

        switch (parseMode)
        {
            case PARSE_MODE_ADD_ZONE_OPEN:

                if (!strcmp(pszArg, "--ns-host"))
                {
                    parseMode = PARSE_MODE_ADD_ZONE_NS_HOST;
                }
                else if (!strcmp(pszArg, "--ns-ip"))
                {
                    parseMode = PARSE_MODE_ADD_ZONE_NS_IP;
                }
                else if (!strcmp(pszArg, "--mbox-domain"))
                {
                    parseMode = PARSE_MODE_ADD_ZONE_MBOX_DOMAIN;
                }
                else
                {
                    if (pContext->pszZone)
                    {
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VMDNS_ERROR(dwError);
                    }

                    dwError = VmDnsAllocateStringA(pszArg, &pContext->pszZone);
                    BAIL_ON_VMDNS_ERROR(dwError);
                }

                break;

            case PARSE_MODE_ADD_ZONE_NS_HOST:

                if (pContext->pszNSHost)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMDNS_ERROR(dwError);
                }

                dwError = VmDnsAllocateStringA(pszArg, &pContext->pszNSHost);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_ZONE_OPEN;

                break;

            case PARSE_MODE_ADD_ZONE_NS_IP:

                if (pContext->pszNSIp)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMDNS_ERROR(dwError);
                }

                dwError = VmDnsAllocateStringA(pszArg, &pContext->pszNSIp);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_ZONE_OPEN;

                break;

            case PARSE_MODE_ADD_ZONE_MBOX_DOMAIN:

                if (pContext->pszMboxDomain)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMDNS_ERROR(dwError);
                }

                dwError = VmDnsAllocateStringA(pszArg, &pContext->pszMboxDomain);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_ZONE_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMDNS_ERROR(dwError);

                break;
        }
    }

error:

    return dwError;
}

static
DWORD
ParseArgsDelZone(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!argc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringA(argv[0], &pContext->pszZone);
    BAIL_ON_VMDNS_ERROR(dwError);

error:

    return dwError;
}

static
DWORD
ParseArgsAddRecord(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_ADD_RECORD_OPEN = 0,
        PARSE_MODE_ADD_RECORD_ZONE,
        PARSE_MODE_ADD_RECORD_TYPE,
        PARSE_MODE_ADD_RECORD_TTL,

        // SRV
        PARSE_MODE_ADD_RECORD_SRV_TARGET,
        PARSE_MODE_ADD_RECORD_SRV_SERVICE,
        PARSE_MODE_ADD_RECORD_SRV_PROTOCOL,
        PARSE_MODE_ADD_RECORD_SRV_PRIORITY,
        PARSE_MODE_ADD_RECORD_SRV_WEIGHT,
        PARSE_MODE_ADD_RECORD_SRV_PORT,

        // A
        PARSE_MODE_ADD_RECORD_A_HOSTNAME,
        PARSE_MODE_ADD_RECORD_A_IP
    } PARSE_MODE_ADD_RECORD;
    PARSE_MODE_ADD_RECORD parseMode = PARSE_MODE_ADD_RECORD_OPEN;
    int iArg = 0;

    if (!argc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pContext->record.dwTtl = -1;

    for (; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];

        switch (parseMode)
        {
            case PARSE_MODE_ADD_RECORD_OPEN:

                if (!strcmp(pszArg, "--zone"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_ZONE;
                }
                else if (!strcmp(pszArg, "--type"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_TYPE;
                }
                else if (!strcmp(pszArg, "--ttl"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_TTL;
                }

                // SRV

                else if (!strcmp(pszArg, "--target"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_SRV_TARGET;
                }
                else if (!strcmp(pszArg, "--service"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_SRV_SERVICE;
                }
                else if (!strcmp(pszArg, "--protocol"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_SRV_PROTOCOL;
                }
                else if (!strcmp(pszArg, "--priority"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_SRV_PRIORITY;
                }
                else if (!strcmp(pszArg, "--weight"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_SRV_WEIGHT;
                }
                else if (!strcmp(pszArg, "--port"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_SRV_PORT;
                }

                // A

                else if (!strcmp(pszArg, "--hostname"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_A_HOSTNAME;
                }
                else if (!strcmp(pszArg, "--ip"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_A_IP;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMDNS_ERROR(dwError);
                }

                break;

            case PARSE_MODE_ADD_RECORD_SRV_TARGET:

                dwError = VmDnsAllocateStringA(pszArg,
                                        &pContext->record.Data.SRV.pNameTarget);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_SRV_SERVICE:

                dwError = VmDnsParseServiceType(pszArg, NULL, &pContext->pszService);

                if (dwError)
                {
                    fprintf(stdout, "Invalid service type: %s\n", pszArg);
                }

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_SRV_PROTOCOL:

                dwError = VmDnsParseServiceProtocol(pszArg, NULL, &pContext->pszProtocol);

                if (dwError)
                {
                    fprintf(stdout, "Invalid service protocol: %s\n", pszArg);
                }

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_SRV_PRIORITY:

                pContext->record.Data.SRV.wPriority = (WORD)atoi(pszArg);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_SRV_WEIGHT:

                pContext->record.Data.SRV.wWeight = (WORD)atoi(pszArg);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_SRV_PORT:

                pContext->record.Data.SRV.wPort = (WORD)atoi(pszArg);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_A_IP:

                dwError = VmDnsCliGetRecordData_A(pszArg, &pContext->record.Data.A);
                BAIL_ON_VMDNS_ERROR(dwError);
                break;

            case PARSE_MODE_ADD_RECORD_A_HOSTNAME:

                dwError = VmDnsAllocateStringA(pszArg, &pContext->record.pszName);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_ZONE:

                if (pContext->pszZone)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMDNS_ERROR(dwError);
                }

                dwError = VmDnsAllocateStringA(pszArg, &pContext->pszZone);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_TYPE:

                dwError = VmDnsParseRecordType(pszArg, &pContext->record.dwType);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_TTL:

                pContext->record.dwTtl = atoi(pszArg);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMDNS_ERROR(dwError);

                break;
        }
    }

error:

    return dwError;
}

static
DWORD
ParseArgsDelRecord(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_DEL_RECORD_OPEN = 0,
        PARSE_MODE_DEL_RECORD_ZONE,
        PARSE_MODE_DEL_RECORD_TYPE,
        // SRV
        PARSE_MODE_ADD_RECORD_SRV_TARGET,
        PARSE_MODE_ADD_RECORD_SRV_SERVICE,
        PARSE_MODE_ADD_RECORD_SRV_PROTOCOL,
        PARSE_MODE_ADD_RECORD_SRV_PRIORITY,
        PARSE_MODE_ADD_RECORD_SRV_WEIGHT,
        PARSE_MODE_ADD_RECORD_SRV_PORT,
        // A
        PARSE_MODE_ADD_RECORD_A_HOSTNAME,
        PARSE_MODE_ADD_RECORD_A_IP
    } PARSE_MODE_DEL_RECORD;
    PARSE_MODE_DEL_RECORD parseMode = PARSE_MODE_DEL_RECORD_OPEN;
    int iArg = 0;

    if (!argc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    for (; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];

        switch (parseMode)
        {
            case PARSE_MODE_DEL_RECORD_OPEN:

                if (!strcmp(pszArg, "--zone"))
                {
                    parseMode = PARSE_MODE_DEL_RECORD_ZONE;
                }
                else if (!strcmp(pszArg, "--type"))
                {
                    parseMode = PARSE_MODE_DEL_RECORD_TYPE;
                }
                // SRV

                else if (!strcmp(pszArg, "--target"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_SRV_TARGET;
                }
                else if (!strcmp(pszArg, "--service"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_SRV_SERVICE;
                }
                else if (!strcmp(pszArg, "--protocol"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_SRV_PROTOCOL;
                }

                // A

                else if (!strcmp(pszArg, "--hostname"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_A_HOSTNAME;
                }
                else if (!strcmp(pszArg, "--ip"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_A_IP;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMDNS_ERROR(dwError);
                }

                break;

            case PARSE_MODE_DEL_RECORD_ZONE:

                if (pContext->pszZone)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMDNS_ERROR(dwError);
                }

                dwError = VmDnsAllocateStringA(pszArg, &pContext->pszZone);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_TYPE:

                dwError = VmDnsParseRecordType(pszArg, &pContext->record.dwType);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_SRV_TARGET:

                dwError = VmDnsAllocateStringA(pszArg,
                                        &pContext->record.Data.SRV.pNameTarget);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_SRV_SERVICE:

                dwError = VmDnsParseServiceType(pszArg, NULL, &pContext->pszService);

                if (dwError)
                {
                    fprintf(stdout, "Invalid service type: %s\n", pszArg);
                }

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_SRV_PROTOCOL:

                dwError = VmDnsParseServiceProtocol(pszArg, NULL, &pContext->pszProtocol);

                if (dwError)
                {
                    fprintf(stdout, "Invalid service protocol: %s\n", pszArg);
                }

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_A_IP:

                dwError = VmDnsCliGetRecordData_A(pszArg, &pContext->record.Data.A);
                BAIL_ON_VMDNS_ERROR(dwError);
                break;

            case PARSE_MODE_ADD_RECORD_A_HOSTNAME:

                dwError = VmDnsAllocateStringA(pszArg, &pContext->record.pszName);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMDNS_ERROR(dwError);

                break;
        }
    }

error:

    return dwError;
}

static
DWORD
ParseArgsQueryRecords(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!argc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    // TODO

error:

    return dwError;
}

static
DWORD
ParseArgsListRecords(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!argc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    typedef enum
    {
        PARSE_MODE_LIST_RECORD_OPEN = 0,
        PARSE_MODE_LIST_RECORD_ZONE
    } PARSE_MODE_LIST_RECORD;
    PARSE_MODE_LIST_RECORD parseMode = PARSE_MODE_LIST_RECORD_OPEN;
    int iArg = 0;

    if (!argc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    for (; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];

        switch (parseMode)
        {
            case PARSE_MODE_LIST_RECORD_OPEN:

                if (!strcmp(pszArg, "--zone"))
                {
                    parseMode = PARSE_MODE_LIST_RECORD_ZONE;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMDNS_ERROR(dwError);
                }

                break;

            case PARSE_MODE_LIST_RECORD_ZONE:

                if (pContext->pszZone)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMDNS_ERROR(dwError);
                }

                dwError = VmDnsAllocateStringA(pszArg, &pContext->pszZone);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_LIST_RECORD_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMDNS_ERROR(dwError);

                break;
        }
    }

error:

    return dwError;
}

static
DWORD
ParseArgsForwardersList(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext
    )
{
    DWORD  dwError = 0;
    PSTR  pszForwarder = NULL;

    if (!argc || pContext->pszForwarder)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringA(argv[0], &pszForwarder);
    BAIL_ON_VMDNS_ERROR(dwError);


    pContext->pszForwarder = pszForwarder;

cleanup:

    return dwError;

error:

    if (pszForwarder)
    {
        VmDnsFreeStringA(pszForwarder);
    }

    goto cleanup;
}

static
void
ShowUsage(
    VOID
    )
{
    fprintf(
        stdout,
        "Usage: vmdns-cli { arguments }\n\n"
        "Arguments:\n\n"
        "\tadd-zone --ns-host <hostname> --ns-ip <ip address> --mbox-domain <mbox-domain> <zone name>\n"
        "\tdel-zone <zone name>\n"
        "\tlist-zones\n"
        "\tadd-record --zone <zone name> --type <record type> [<key> <value>]...\n"
        "                   <key> <value> pair for SRV:\n"
        "                           --service <ldap|...>\n"
        "                           --protocol <tcp|udp>\n"
        "                           --target   <hostname>\n"
        "                           --priority <priority>\n"
        "                           --weight   <weight>\n"
        "                           --port     <port>\n"
        "                   <key> <value> pair for A:\n"
        "                           --hostname <hostname>\n"
        "                           --ip       <ipaddress>\n"
        "\tdel-record --zone <zone name> --type <record type> [<key> <value>]...\n"        "                           ip       <ip>\n"
        "                   <key> <value> pair for SRV:\n"
        "                           --service <ldap>\n"
        "                           --protocol <tcp|udp>\n"
        "                           --target   <hostname>\n"
        "                   <key> <value> pair for A:\n"
        "                           --hostname <hostname>\n"
        "                           --ip       <ipaddress>\n"
        "\tlist-records --zone <zone name>\n"
        "\tquery-record --zone <zone name> --type <record type> <key>\n"
        "\tadd-forwarder <forwarder ip>\n"
        "\tdel-forwarder <forwarder ip>\n"
        "\tlist-forwarders\n"
        "\thelp\n");
}

static
DWORD
VmDnsCliGetRecordData_A(
    PCSTR           pszIPAddress,
    PVMDNS_A_DATA   pData
    )
{
    DWORD dwError = 0;
    struct addrinfo *result = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pszIPAddress, dwError);

    if (getaddrinfo(pszIPAddress, NULL, NULL, &result))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (result->ai_family != AF_INET)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(sizeof(*pData), (PVOID*)&pData);
    BAIL_ON_VMDNS_ERROR(dwError);

    memcpy(&pData->IpAddress, &((struct sockaddr_in *)result->ai_addr)->sin_addr, sizeof(pData->IpAddress));

cleanup:

    if (result)
    {
        freeaddrinfo(result);
    }
    return dwError;

error:

    goto cleanup;
}

