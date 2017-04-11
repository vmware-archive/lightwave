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

#define VMDNS_LOCALHOST     "localhost"
#define VMDNS_MAX_PWD_LEN   128

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
    PVM_DNS_CLI_CONTEXT pContext,
    PSTR*               ppszUserName,
    PSTR*               ppszDomain,
    PSTR*               ppszPassword
    );

static
DWORD
ParseArgsDelZone(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext,
    PSTR*               ppszUserName,
    PSTR*               ppszDomain,
    PSTR*               ppszPassword
    );

static
DWORD
ParseArgsAddRecord(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext,
    PSTR*               ppszUserName,
    PSTR*               ppszDomain,
    PSTR*               ppszPassword
    );

static
DWORD
ParseArgsDelRecord(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext,
    PSTR*               ppszUserName,
    PSTR*               ppszDomain,
    PSTR*               ppszPassword
    );

static
DWORD
ParseArgsQueryRecords(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext,
    PSTR*               ppszUserName,
    PSTR*               ppszDomain,
    PSTR*               ppszPassword
    );

static
DWORD
ParseArgsListRecords(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext,
    PSTR*               ppszUserName,
    PSTR*               ppszDomain,
    PSTR*               ppszPassword
    );

static
DWORD
ParseArgsForwardersList(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext,
    PSTR*               ppszUserName,
    PSTR*               ppszDomain,
    PSTR*               ppszPassword,
    BOOL                bWithArgs
    );

static
DWORD
ParseArgsListZones(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext,
    PSTR*               ppszUserName,
    PSTR*               ppszDomain,
    PSTR*               ppszPassword
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

static
DWORD
VmDnsCliGetRecordData_AAAA(
    PCSTR               pszIP6Address,
    PVMDNS_AAAA_DATA    pData
    );

static
DWORD
VmDnsCopyStringArg(
    PCSTR               pszArg,
    PSTR*               ppszDest
    );

static
BOOL
VerifyRemoteConnectionArgs(
    PSTR* ppszServer,
    PSTR pszUserName,
    PSTR pszDomain,
    PSTR* ppszPassword
    );

static
DWORD
VmDnsMakeZoneFQDN(
    PCSTR psszZoneName,
    PSTR* ppszZoneFqdn
    );

static
DWORD
VmDnsSetDefaultParams(
    PSTR* ppszServer,
    PSTR* ppszUserName
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
    PSTR pszUserName = NULL;
    PSTR pszDomain = NULL;
    PSTR pszPassword = NULL;

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
        ShowUsage();
    }
    else if (!strcmp(pszArg, "add-zone"))
    {
        pContext->action = VM_DNS_ACTION_ADD_ZONE;

        dwError = ParseArgsAddZone(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext,
                        &pszUserName,
                        &pszDomain,
                        &pszPassword);
    }
    else if (!strcmp(pszArg, "del-zone"))
    {
        pContext->action = VM_DNS_ACTION_DEL_ZONE;

        dwError = ParseArgsDelZone(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext,
                        &pszUserName,
                        &pszDomain,
                        &pszPassword);
    }
    else if (!strcmp(pszArg, "list-zone"))
    {
        pContext->action = VM_DNS_ACTION_LIST_ZONES;

        dwError = ParseArgsListZones(
            dwArgsLeft,
            dwArgsLeft > 0 ? &argv[iArg] : NULL,
            pContext,
            &pszUserName,
            &pszDomain,
            &pszPassword);
    }
    else if (!strcmp(pszArg, "add-record"))
    {
        pContext->action = VM_DNS_ACTION_ADD_RECORD;

        dwError = ParseArgsAddRecord(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext,
                        &pszUserName,
                        &pszDomain,
                        &pszPassword);
    }
    else if (!strcmp(pszArg, "del-record"))
    {
        pContext->action = VM_DNS_ACTION_DEL_RECORD;

        dwError = ParseArgsDelRecord(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext,
                        &pszUserName,
                        &pszDomain,
                        &pszPassword);
    }
    else if (!strcmp(pszArg, "list-record"))
    {
        pContext->action = VM_DNS_ACTION_LIST_RECORDS;

        dwError = ParseArgsListRecords(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext,
                        &pszUserName,
                        &pszDomain,
                        &pszPassword);
    }
    else if (!strcmp(pszArg, "query-record"))
    {
        pContext->action = VM_DNS_ACTION_QUERY_RECORD;

        dwError = ParseArgsQueryRecords(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext,
                        &pszUserName,
                        &pszDomain,
                        &pszPassword);
    }
    else if (!strcmp(pszArg, "add-forwarder"))
    {
        pContext->action = VM_DNS_ACTION_ADD_FORWARDER;

        dwError = ParseArgsForwardersList(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext,
                        &pszUserName,
                        &pszDomain,
                        &pszPassword,
                        TRUE);
    }
    else if (!strcmp(pszArg, "del-forwarder"))
    {
        pContext->action = VM_DNS_ACTION_DEL_FORWARDER;

        dwError = ParseArgsForwardersList(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext,
                        &pszUserName,
                        &pszDomain,
                        &pszPassword,
                        TRUE);
    }
    else if (!strcmp(pszArg, "list-forwarders"))
    {
        pContext->action = VM_DNS_ACTION_LIST_FORWARDERS;

        dwError = ParseArgsForwardersList(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext,
                        &pszUserName,
                        &pszDomain,
                        &pszPassword,
                        FALSE);
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
    }
    BAIL_ON_VMDNS_ERROR(dwError);
    // set defaults if not set
    dwError = VmDnsSetDefaultParams(&pContext->pszServer,
                                    &pszUserName
                                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VerifyRemoteConnectionArgs(
                            &pContext->pszServer,
                            pszUserName,
                            pszDomain,
                            &pszPassword
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsOpenServerA(pContext->pszServer,
                                pszUserName,
                                pszDomain,
                                pszPassword,
                                0,
                                NULL,
                                &pServerContext);

    BAIL_ON_VMDNS_ERROR(dwError);

    pContext->pServerContext = pServerContext;
    pServerContext = NULL;

    *ppContext = pContext;

cleanup:

    VmDnsFreeMemory(pszUserName);
    VmDnsFreeMemory(pszDomain);
    VmDnsFreeMemory(pszPassword);

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
    PVM_DNS_CLI_CONTEXT pContext,
    PSTR*               ppszUserName,
    PSTR*               ppszDomain,
    PSTR*               ppszPassword
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_ADD_ZONE_OPEN = 0,
        PARSE_MODE_ADD_ZONE_TYPE,
        PARSE_MODE_ADD_ZONE_NS_HOST,
        PARSE_MODE_ADD_ZONE_NS_IP,
        PARSE_MODE_ADD_ZONE_MBOX_DOMAIN,
        PARSE_MODE_ADD_ZONE_USERNAME,
        PARSE_MODE_ADD_ZONE_DOMAIN,
        PARSE_MODE_ADD_ZONE_PASSWORD,
        PARSE_MODE_ADD_ZONE_SERVER
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
            else if (!strcmp(pszArg, "--type"))
            {
                parseMode = PARSE_MODE_ADD_ZONE_TYPE;
            }
            else if (!strcmp(pszArg, "--ns-ip"))
            {
                parseMode = PARSE_MODE_ADD_ZONE_NS_IP;
            }
            else if (!strcmp(pszArg, "--admin-email"))
            {
                parseMode = PARSE_MODE_ADD_ZONE_MBOX_DOMAIN;
            }
            else if (!strcmp(pszArg, "--username"))
            {
                parseMode = PARSE_MODE_ADD_ZONE_USERNAME;
            }
            else if (!strcmp(pszArg, "--domain"))
            {
                parseMode = PARSE_MODE_ADD_ZONE_DOMAIN;
            }
            else if (!strcmp(pszArg, "--password"))
            {
                parseMode = PARSE_MODE_ADD_ZONE_PASSWORD;
            }
            else if (!strcmp(pszArg, "--server"))
            {
                parseMode = PARSE_MODE_ADD_ZONE_SERVER;
            }
            else
            {
                if (pContext->pszZone)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMDNS_ERROR(dwError);
                }

                dwError = VmDnsMakeZoneFQDN(pszArg, &pContext->pszZone);
                BAIL_ON_VMDNS_ERROR(dwError);
            }

            break;

        case PARSE_MODE_ADD_ZONE_TYPE:

            if (VmDnsStringCompareA(pszArg, "reverse", FALSE) == 0)
            {
                pContext->dwZoneType = VMDNS_ZONE_TYPE_REVERSE;
            }
            else if (VmDnsStringCompareA(pszArg, "forward", FALSE) == 0)
            {
                pContext->dwZoneType = VMDNS_ZONE_TYPE_FORWARD;
            }
            else
            {
                fprintf(stdout, "Invalid zone type %s.\n", pszArg);
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDNS_ERROR(dwError);
            }

            parseMode = PARSE_MODE_ADD_ZONE_OPEN;

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

            if (!VmDnsCheckIfIPV4AddressA(pContext->pszNSIp) && !VmDnsCheckIfIPV6AddressA(pContext->pszNSIp))
            {
                fprintf(stdout, "NS-IP parameter value should be valid IP (IPV4 or IPV6) address.");
                dwError = ERROR_INVALID_ADDRESS;
                BAIL_ON_VMDNS_ERROR(dwError);
            }

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

            case PARSE_MODE_ADD_ZONE_USERNAME:

                dwError = VmDnsCopyStringArg(pszArg, ppszUserName);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_ZONE_OPEN;

                break;

            case PARSE_MODE_ADD_ZONE_DOMAIN:

                dwError = VmDnsCopyStringArg(pszArg, ppszDomain);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_ZONE_OPEN;

                break;

            case PARSE_MODE_ADD_ZONE_PASSWORD:

                dwError = VmDnsCopyStringArg(pszArg, ppszPassword);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_ZONE_OPEN;

                break;

            case PARSE_MODE_ADD_ZONE_SERVER:

                dwError = VmDnsCopyStringArg(pszArg, &pContext->pszServer);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_ZONE_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMDNS_ERROR(dwError);

                break;
        }
    }

    if (pContext->dwZoneType == VMDNS_ZONE_TYPE_REVERSE)
    {
        dwError = VmDnsGenerateReversZoneNameFromNetworkId(
                    pContext->pszZone,
                    &pContext->pszZone);
        if (dwError)
        {
            fprintf(
                stdout,
                "Failed to generate reverse zone name, %u.\n",
                dwError);
        }
        BAIL_ON_VMDNS_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsDelZone(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext,
    PSTR*               ppszUserName,
    PSTR*               ppszDomain,
    PSTR*               ppszPassword
    )
{
    DWORD dwError = 0;
    PSTR pszArg;
    DWORD iArg = 0;

    if (!argc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    typedef enum
    {
        PARSE_MODE_DELZONE_OPEN = 0,
        PARSE_MODE_DELZONE_USERNAME,
        PARSE_MODE_DELZONE_DOMAIN,
        PARSE_MODE_DELZONE_PASSWORD,
        PARSE_MODE_DELZONE_SERVER
    } PARSE_MODE_DELZONE;
    PARSE_MODE_DELZONE parseMode = PARSE_MODE_DELZONE_OPEN;

    for (; iArg < argc; iArg++)
    {
        pszArg = argv[iArg];

        switch (parseMode)
        {
        case PARSE_MODE_DELZONE_OPEN:
            if (!strcmp(pszArg, "--username"))
            {
                parseMode = PARSE_MODE_DELZONE_USERNAME;
            }
            else if (!strcmp(pszArg, "--domain"))
            {
                parseMode = PARSE_MODE_DELZONE_DOMAIN;
            }
            else if (!strcmp(pszArg, "--password"))
            {
                parseMode = PARSE_MODE_DELZONE_PASSWORD;
            }
            else if (!strcmp(pszArg, "--server"))
            {
                parseMode = PARSE_MODE_DELZONE_SERVER;
            }
            else
            {
                if (pContext->pszZone)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMDNS_ERROR(dwError);
                }

                dwError = VmDnsMakeZoneFQDN(pszArg, &pContext->pszZone);
                BAIL_ON_VMDNS_ERROR(dwError);
            }

            break;

        case PARSE_MODE_DELZONE_USERNAME:

            dwError = VmDnsCopyStringArg(pszArg, ppszUserName);
            BAIL_ON_VMDNS_ERROR(dwError);

            parseMode = PARSE_MODE_DELZONE_OPEN;

            break;

        case PARSE_MODE_DELZONE_DOMAIN:

            dwError = VmDnsCopyStringArg(pszArg, ppszDomain);
            BAIL_ON_VMDNS_ERROR(dwError);

            parseMode = PARSE_MODE_DELZONE_OPEN;

            break;

        case PARSE_MODE_DELZONE_PASSWORD:

            dwError = VmDnsCopyStringArg(pszArg, ppszPassword);
            BAIL_ON_VMDNS_ERROR(dwError);

            parseMode = PARSE_MODE_DELZONE_OPEN;

            break;

        case PARSE_MODE_DELZONE_SERVER:

            dwError = VmDnsCopyStringArg(pszArg, &pContext->pszServer);
            BAIL_ON_VMDNS_ERROR(dwError);

            parseMode = PARSE_MODE_DELZONE_OPEN;

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
ParseArgsAddRecord(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext,
    PSTR*               ppszUserName,
    PSTR*               ppszDomain,
    PSTR*               ppszPassword
    )
{
    DWORD dwError = 0;

    typedef enum
    {
        PARSE_MODE_ADD_RECORD_OPEN = 0,
        PARSE_MODE_ADD_RECORD_ZONE,
        PARSE_MODE_ADD_RECORD_TYPE,
        PARSE_MODE_ADD_RECORD_TTL,
        PARSE_MODE_ADD_RECORD_USERNAME,
        PARSE_MODE_ADD_RECORD_DOMAIN,
        PARSE_MODE_ADD_RECORD_PASSWORD,
        PARSE_MODE_ADD_RECORD_SERVER,

        // SRV
        PARSE_MODE_ADD_RECORD_SRV_TARGET,
        PARSE_MODE_ADD_RECORD_SRV_SERVICE,
        PARSE_MODE_ADD_RECORD_SRV_PROTOCOL,
        PARSE_MODE_ADD_RECORD_SRV_PRIORITY,
        PARSE_MODE_ADD_RECORD_SRV_WEIGHT,
        PARSE_MODE_ADD_RECORD_SRV_PORT,

        // A && AAAA
        PARSE_MODE_ADD_RECORD_A_HOSTNAME,
        PARSE_MODE_ADD_RECORD_A_IP,
        PARSE_MODE_ADD_RECORD_AAAA_IP,

        // NS
        PARSE_MODE_ADD_RECORD_NS_DOMAIN,
        PARSE_MODE_ADD_RECORD_NS_HOSTNAME,

        // PTR
        PARSE_MODE_ADD_RECORD_PTR_IP,
        PARSE_MODE_ADD_RECORD_PTR_HOSTNAME,

        // CNAME
        PARSE_MODE_ADD_RECORD_CNAME_NAME,
        PARSE_MODE_ADD_RECORD_CNAME_HOSTNAME
    } PARSE_MODE_ADD_RECORD;
    PARSE_MODE_ADD_RECORD parseMode = PARSE_MODE_ADD_RECORD_OPEN;
    int iArg = 0;

    if (!argc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pContext->record.dwTtl = VMDNS_DEFAULT_TTL;

    // First get record type
    pContext->record.dwType = VMDNS_RR_TYPE_NONE;
    for (; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];

        switch (parseMode)
        {
            case PARSE_MODE_ADD_RECORD_OPEN:

                if (!strcmp(pszArg, "--type"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_TYPE;
                }

                break;

            case PARSE_MODE_ADD_RECORD_TYPE:

                dwError = VmDnsParseRecordType(pszArg, &pContext->record.dwType);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            default:

                break;
        }
    }

    if (pContext->record.dwType == VMDNS_RR_TYPE_NONE)
    {
        dwError = ERROR_INVALID_PARAMETER;
        fprintf(stdout, "Record type is not specified.\n");
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    for (iArg = 0; iArg < argc; iArg++)
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

                else if (!strcmp(pszArg, "--hostname"))
                {
                    if (pContext->record.dwType == VMDNS_RR_TYPE_A ||
                        pContext->record.dwType == VMDNS_RR_TYPE_AAAA)
                    {
                        parseMode = PARSE_MODE_ADD_RECORD_A_HOSTNAME;
                    }
                    else if (pContext->record.dwType == VMDNS_RR_TYPE_NS)
                    {
                        parseMode = PARSE_MODE_ADD_RECORD_NS_HOSTNAME;
                    }
                    else if (pContext->record.dwType == VMDNS_RR_TYPE_PTR)
                    {
                        parseMode = PARSE_MODE_ADD_RECORD_PTR_HOSTNAME;
                    }
                    else if (pContext->record.dwType == VMDNS_RR_TYPE_CNAME)
                    {
                        parseMode = PARSE_MODE_ADD_RECORD_CNAME_HOSTNAME;
                    }
                    else
                    {
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VMDNS_ERROR(dwError);
                    }
                }

                else if (!strcmp(pszArg, "--ip"))
                {
                    if (pContext->record.dwType == VMDNS_RR_TYPE_A)
                    {
                        parseMode = PARSE_MODE_ADD_RECORD_A_IP;
                    }
                    else if (pContext->record.dwType == VMDNS_RR_TYPE_PTR)
                    {
                        parseMode = PARSE_MODE_ADD_RECORD_PTR_IP;
                    }
                    else
                    {
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VMDNS_ERROR(dwError);
                    }
                }
                else if (!strcmp(pszArg, "--ip6"))
                {
                    if (pContext->record.dwType == VMDNS_RR_TYPE_AAAA)
                    {
                        parseMode = PARSE_MODE_ADD_RECORD_AAAA_IP;
                    }
                    else if (pContext->record.dwType == VMDNS_RR_TYPE_PTR)
                    {
                        parseMode = PARSE_MODE_ADD_RECORD_PTR_IP;
                    }
                    else
                    {
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VMDNS_ERROR(dwError);
                    }
                }
                else if (!strcmp(pszArg, "--ns-domain"))
                {
                    if (pContext->record.dwType == VMDNS_RR_TYPE_NS)
                    {
                        parseMode = PARSE_MODE_ADD_RECORD_NS_DOMAIN;
                    }
                    else
                    {
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VMDNS_ERROR(dwError);
                    }
                }
                else if (!strcmp(pszArg, "--name"))
                {
                    if (pContext->record.dwType == VMDNS_RR_TYPE_CNAME)
                    {
                        parseMode = PARSE_MODE_ADD_RECORD_CNAME_NAME;
                    }
                    else
                    {
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VMDNS_ERROR(dwError);
                    }
                }
                else if (!strcmp(pszArg, "--username"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_USERNAME;
                }
                else if (!strcmp(pszArg, "--domain"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_DOMAIN;
                }
                else if (!strcmp(pszArg, "--password"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_PASSWORD;
                }
                else if (!strcmp(pszArg, "--server"))
                {
                    parseMode = PARSE_MODE_ADD_RECORD_SERVER;
                }
                else
                {
                    fprintf(stdout, "Unrecognized option: %s\n", pszArg);
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

                if (!IsNullOrEmptyString(pszArg))
                {
                    if (pszArg[0] == '_')
                    {
                        dwError = VmDnsAllocateStringA(pszArg, &pContext->pszService);
                    }
                    else
                    {
                        dwError = VmDnsAllocateStringPrintfA(
                                    &pContext->pszService,
                                    "_%s",
                                    pszArg);
                    }
                }
                BAIL_ON_VMDNS_ERROR(dwError);

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

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_AAAA_IP:

                dwError = VmDnsCliGetRecordData_AAAA(pszArg, &pContext->record.Data.AAAA);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

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

                dwError = VmDnsMakeZoneFQDN(pszArg, &pContext->pszZone);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_TYPE:

                // Type has already been parsed above
                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_TTL:

                pContext->record.dwTtl = atoi(pszArg);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_USERNAME:

                dwError = VmDnsCopyStringArg(pszArg, ppszUserName);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_DOMAIN:

                dwError = VmDnsCopyStringArg(pszArg, ppszDomain);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_PASSWORD:

                dwError = VmDnsCopyStringArg(pszArg, ppszPassword);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;
            case PARSE_MODE_ADD_RECORD_SERVER:

                dwError = VmDnsCopyStringArg(pszArg, &pContext->pszServer);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_NS_HOSTNAME:

                dwError = VmDnsAllocateStringA(
                                pszArg,
                                &pContext->record.Data.NS.pNameHost);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_NS_DOMAIN:

                dwError = VmDnsAllocateStringA(
                                pszArg,
                                &pContext->record.pszName);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_PTR_IP:

                dwError = VmDnsGeneratePtrNameFromIp(
                                pszArg,
                                NULL,
                                &pContext->record.pszName);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_PTR_HOSTNAME:

                dwError = VmDnsAllocateStringA(
                                pszArg,
                                &pContext->record.Data.PTR.pNameHost);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_CNAME_NAME:

                dwError = VmDnsAllocateStringA(
                                pszArg,
                                &pContext->record.pszName);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_ADD_RECORD_OPEN;

                break;

            case PARSE_MODE_ADD_RECORD_CNAME_HOSTNAME:

                dwError = VmDnsAllocateStringA(
                                pszArg,
                                &pContext->record.Data.CNAME.pNameHost);
                BAIL_ON_VMDNS_ERROR(dwError);

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
VmDnsCopyStringArg(
    PCSTR               pszArg,
    PSTR*               ppszDest
)
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    dwError = VmDnsAllocateStringA(pszArg, ppszDest);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}


static
DWORD
ParseArgsDelRecord(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext,
    PSTR* ppszUserName,
    PSTR* ppszDomain,
    PSTR* ppszPassword
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_DEL_RECORD_OPEN = 0,
        PARSE_MODE_DEL_RECORD_ZONE,
        PARSE_MODE_DEL_RECORD_TYPE,
        PARSE_MODE_DEL_RECORD_USERNAME,
        PARSE_MODE_DEL_RECORD_DOMAIN,
        PARSE_MODE_DEL_RECORD_PASSWORD,
        PARSE_MODE_DEL_RECORD_SERVER,

        // SRV
        PARSE_MODE_DEL_RECORD_SRV_TARGET,
        PARSE_MODE_DEL_RECORD_SRV_SERVICE,
        PARSE_MODE_DEL_RECORD_SRV_PROTOCOL,
        PARSE_MODE_DEL_RECORD_SRV_PRIORITY,
        PARSE_MODE_DEL_RECORD_SRV_WEIGHT,
        PARSE_MODE_DEL_RECORD_SRV_PORT,

        // A && AAAA
        PARSE_MODE_DEL_RECORD_A_HOSTNAME,
        PARSE_MODE_DEL_RECORD_A_IP,
        PARSE_MODE_DEL_RECORD_AAAA_IP,

        // NS
        PARSE_MODE_DEL_RECORD_NS_DOMAIN,
        PARSE_MODE_DEL_RECORD_NS_HOSTNAME,

        // PTR
        PARSE_MODE_DEL_RECORD_PTR_IP,
        PARSE_MODE_DEL_RECORD_PTR_HOSTNAME,

        // CNAME
        PARSE_MODE_DEL_RECORD_CNAME_NAME,
        PARSE_MODE_DEL_RECORD_CNAME_HOSTNAME
    } PARSE_MODE_DEL_RECORD;
    PARSE_MODE_DEL_RECORD parseMode = PARSE_MODE_DEL_RECORD_OPEN;
    int iArg = 0;

    if (!argc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pContext->record.dwTtl = VMDNS_DEFAULT_TTL;

    // First get record type
    pContext->record.dwType = VMDNS_RR_TYPE_NONE;
    for (; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];

        switch (parseMode)
        {
            case PARSE_MODE_DEL_RECORD_OPEN:

                if (!strcmp(pszArg, "--type"))
                {
                    parseMode = PARSE_MODE_DEL_RECORD_TYPE;
                }
                else if (!strcmp(pszArg, "--username"))
                {
                    parseMode = PARSE_MODE_DEL_RECORD_USERNAME;
                }
                else if (!strcmp(pszArg, "--domain"))
                {
                    parseMode = PARSE_MODE_DEL_RECORD_DOMAIN;
                }
                else if (!strcmp(pszArg, "--password"))
                {
                    parseMode = PARSE_MODE_DEL_RECORD_PASSWORD;
                }
                else if (!strcmp(pszArg, "--server"))
                {
                    parseMode = PARSE_MODE_DEL_RECORD_SERVER;
                }

                break;

            case PARSE_MODE_DEL_RECORD_TYPE:

                dwError = VmDnsParseRecordType(pszArg, &pContext->record.dwType);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_USERNAME:

                dwError = VmDnsCopyStringArg(pszArg, ppszUserName);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_PASSWORD:

                dwError = VmDnsCopyStringArg(pszArg, ppszPassword);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_SERVER:

                dwError = VmDnsCopyStringArg(pszArg, &pContext->pszServer);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_DOMAIN:

                dwError = VmDnsCopyStringArg(pszArg, ppszDomain);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            default:

                break;
        }
    }

    for (iArg = 0; iArg < argc; iArg++)
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
                    parseMode = PARSE_MODE_DEL_RECORD_SRV_TARGET;
                }
                else if (!strcmp(pszArg, "--service"))
                {
                    parseMode = PARSE_MODE_DEL_RECORD_SRV_SERVICE;
                }
                else if (!strcmp(pszArg, "--protocol"))
                {
                    parseMode = PARSE_MODE_DEL_RECORD_SRV_PROTOCOL;
                }

                // A

                else if (!strcmp(pszArg, "--hostname"))
                {
                    if (pContext->record.dwType == VMDNS_RR_TYPE_A ||
                        pContext->record.dwType == VMDNS_RR_TYPE_AAAA)
                    {
                        parseMode = PARSE_MODE_DEL_RECORD_A_HOSTNAME;
                    }
                    else if (pContext->record.dwType == VMDNS_RR_TYPE_NS)
                    {
                        parseMode = PARSE_MODE_DEL_RECORD_NS_HOSTNAME;
                    }
                    else if (pContext->record.dwType == VMDNS_RR_TYPE_PTR)
                    {
                        parseMode = PARSE_MODE_DEL_RECORD_PTR_HOSTNAME;
                    }
                    else if (pContext->record.dwType == VMDNS_RR_TYPE_CNAME)
                    {
                        parseMode = PARSE_MODE_DEL_RECORD_CNAME_HOSTNAME;
                    }
                    else
                    {
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VMDNS_ERROR(dwError);
                    }
                }

                else if (!strcmp(pszArg, "--ip"))
                {
                    if (pContext->record.dwType == VMDNS_RR_TYPE_A ||
                        pContext->record.dwType == VMDNS_RR_TYPE_AAAA)
                    {
                        parseMode = PARSE_MODE_DEL_RECORD_A_IP;
                    }
                    else if (pContext->record.dwType == VMDNS_RR_TYPE_PTR)
                    {
                        parseMode = PARSE_MODE_DEL_RECORD_PTR_IP;
                    }
                    else
                    {
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VMDNS_ERROR(dwError);
                    }
                }
                else if (!strcmp(pszArg, "--ip6"))
                {
                    parseMode = PARSE_MODE_DEL_RECORD_AAAA_IP;
                }
                else if (!strcmp(pszArg, "--ns-domain"))
                {
                    parseMode = PARSE_MODE_DEL_RECORD_NS_DOMAIN;
                }
                else if (!strcmp(pszArg, "--name"))
                {
                    if (pContext->record.dwType == VMDNS_RR_TYPE_CNAME)
                    {
                        parseMode = PARSE_MODE_DEL_RECORD_CNAME_NAME;
                    }
                    else
                    {
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VMDNS_ERROR(dwError);
                    }
                }
                else if (!strcmp(pszArg, "--username"))
                {
                    parseMode = PARSE_MODE_DEL_RECORD_USERNAME;
                }
                else if (!strcmp(pszArg, "--domain"))
                {
                    parseMode = PARSE_MODE_DEL_RECORD_DOMAIN;
                }
                else if (!strcmp(pszArg, "--password"))
                {
                    parseMode = PARSE_MODE_DEL_RECORD_PASSWORD;
                }
                else if (!strcmp(pszArg, "--server"))
                {
                    parseMode = PARSE_MODE_DEL_RECORD_SERVER;
                }
                else
                {
                    fprintf(stdout, "Unrecognized option: %s\n", pszArg);
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMDNS_ERROR(dwError);
                }

                break;

                break;

            case PARSE_MODE_DEL_RECORD_ZONE:

                if (pContext->pszZone)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMDNS_ERROR(dwError);
                }

                dwError = VmDnsMakeZoneFQDN(pszArg, &pContext->pszZone);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_TYPE:

                dwError = VmDnsParseRecordType(pszArg, &pContext->record.dwType);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_SRV_TARGET:

                dwError = VmDnsAllocateStringA(pszArg,
                                        &pContext->record.Data.SRV.pNameTarget);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_SRV_SERVICE:

                if (!IsNullOrEmptyString(pszArg))
                {
                    if (pszArg[0] == '_')
                    {
                        dwError = VmDnsAllocateStringA(pszArg, &pContext->pszService);
                    }
                    else
                    {
                        dwError = VmDnsAllocateStringPrintfA(
                                    &pContext->pszService,
                                    "_%s",
                                    pszArg);
                    }
                }
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_SRV_PROTOCOL:

                dwError = VmDnsParseServiceProtocol(pszArg, NULL, &pContext->pszProtocol);

                if (dwError)
                {
                    fprintf(stdout, "Invalid service protocol: %s\n", pszArg);
                }

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_A_IP:

                dwError = VmDnsCliGetRecordData_A(pszArg, &pContext->record.Data.A);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                BAIL_ON_VMDNS_ERROR(dwError);
                break;

            case PARSE_MODE_DEL_RECORD_AAAA_IP:

                dwError = VmDnsCliGetRecordData_AAAA(pszArg, &pContext->record.Data.AAAA);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                BAIL_ON_VMDNS_ERROR(dwError);
                break;

            case PARSE_MODE_DEL_RECORD_A_HOSTNAME:

                dwError = VmDnsAllocateStringA(pszArg, &pContext->record.pszName);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_NS_HOSTNAME:

                dwError = VmDnsAllocateStringA(
                                pszArg,
                                &pContext->record.Data.NS.pNameHost);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_NS_DOMAIN:

                dwError = VmDnsAllocateStringA(
                                pszArg,
                                &pContext->record.pszName);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_PTR_IP:

                dwError = VmDnsGeneratePtrNameFromIp(
                                pszArg,
                                NULL,
                                &pContext->record.pszName);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_PTR_HOSTNAME:

                dwError = VmDnsAllocateStringA(
                                pszArg,
                                &pContext->record.Data.PTR.pNameHost);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_CNAME_NAME:

                dwError = VmDnsAllocateStringA(
                                pszArg,
                                &pContext->record.pszName);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_CNAME_HOSTNAME:

                dwError = VmDnsAllocateStringA(
                                pszArg,
                                &pContext->record.Data.CNAME.pNameHost);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_USERNAME:

                dwError = VmDnsCopyStringArg(pszArg, ppszUserName);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_PASSWORD:

                dwError = VmDnsCopyStringArg(pszArg, ppszPassword);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_SERVER:

                dwError = VmDnsCopyStringArg(pszArg, &pContext->pszServer);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_DEL_RECORD_OPEN;

                break;

            case PARSE_MODE_DEL_RECORD_DOMAIN:

                dwError = VmDnsCopyStringArg(pszArg, ppszDomain);
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
    PVM_DNS_CLI_CONTEXT pContext,
    PSTR* ppszUserName,
    PSTR* ppszDomain,
    PSTR* ppszPassword
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_QUERY_RECORD_OPEN = 0,
        PARSE_MODE_QUERY_RECORD_ZONE,
        PARSE_MODE_QUERY_RECORD_TYPE,
        PARSE_MODE_QUERY_RECORD_NAME,
        PARSE_MODE_QUERY_RECORD_USERNAME,
        PARSE_MODE_QUERY_RECORD_DOMAIN,
        PARSE_MODE_QUERY_RECORD_PASSWORD,
        PARSE_MODE_QUERY_RECORD_SERVER,

    } PARSE_MODE_QUERY_RECORD;
    PARSE_MODE_QUERY_RECORD parseMode = PARSE_MODE_QUERY_RECORD_OPEN;
    int iArg = 0;

    if (!argc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pContext->record.dwTtl = VMDNS_DEFAULT_TTL;

    pContext->record.dwType = VMDNS_RR_TYPE_NONE;
    for (; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];

        switch (parseMode)
        {
            case PARSE_MODE_QUERY_RECORD_OPEN:

                if (!strcmp(pszArg, "--zone"))
                {
                    parseMode = PARSE_MODE_QUERY_RECORD_ZONE;
                }
                else if (!strcmp(pszArg, "--type"))
                {
                    parseMode = PARSE_MODE_QUERY_RECORD_TYPE;
                }
                else if (!strcmp(pszArg, "--name"))
                {
                    parseMode = PARSE_MODE_QUERY_RECORD_NAME;
                }
                else if (!strcmp(pszArg, "--username"))
                {
                    parseMode = PARSE_MODE_QUERY_RECORD_USERNAME;
                }
                else if (!strcmp(pszArg, "--domain"))
                {
                    parseMode = PARSE_MODE_QUERY_RECORD_DOMAIN;
                }
                else if (!strcmp(pszArg, "--password"))
                {
                    parseMode = PARSE_MODE_QUERY_RECORD_PASSWORD;
                }
                else if (!strcmp(pszArg, "--server"))
                {
                    parseMode = PARSE_MODE_QUERY_RECORD_SERVER;
                }

                break;

            case PARSE_MODE_QUERY_RECORD_ZONE:

                dwError = VmDnsMakeZoneFQDN(pszArg, &pContext->pszZone);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_QUERY_RECORD_OPEN;

                break;

            case PARSE_MODE_QUERY_RECORD_TYPE:

                dwError = VmDnsParseRecordType(pszArg, &pContext->record.dwType);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_QUERY_RECORD_OPEN;

                break;

            case PARSE_MODE_QUERY_RECORD_NAME:

                dwError = VmDnsCopyStringArg(pszArg, &pContext->record.pszName);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_QUERY_RECORD_OPEN;

                break;

            case PARSE_MODE_QUERY_RECORD_USERNAME:

                dwError = VmDnsCopyStringArg(pszArg, ppszUserName);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_QUERY_RECORD_OPEN;

                break;

            case PARSE_MODE_QUERY_RECORD_PASSWORD:

                dwError = VmDnsCopyStringArg(pszArg, ppszPassword);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_QUERY_RECORD_OPEN;

                break;

            case PARSE_MODE_QUERY_RECORD_SERVER:

                dwError = VmDnsCopyStringArg(pszArg, &pContext->pszServer);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_QUERY_RECORD_OPEN;

                break;

            case PARSE_MODE_QUERY_RECORD_DOMAIN:

                dwError = VmDnsCopyStringArg(pszArg, ppszDomain);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_QUERY_RECORD_OPEN;

                break;

            default:

                break;
        }
    }

error:

    return dwError;
}

static
DWORD
ParseArgsListRecords(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext,
    PSTR* ppszUserName,
    PSTR* ppszDomain,
    PSTR* ppszPassword
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
        PARSE_MODE_LIST_RECORD_ZONE,
        PARSE_MODE_LIST_RECORD_USERNAME,
        PARSE_MODE_LIST_RECORD_PASSWORD,
        PARSE_MODE_LIST_RECORD_DOMAIN,
        PARSE_MODE_LIST_RECORD_SERVER
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
                else if (!strcmp(pszArg, "--username"))
                {
                    parseMode = PARSE_MODE_LIST_RECORD_USERNAME;
                }
                else if (!strcmp(pszArg, "--domain"))
                {
                    parseMode = PARSE_MODE_LIST_RECORD_DOMAIN;
                }
                else if (!strcmp(pszArg, "--password"))
                {
                    parseMode = PARSE_MODE_LIST_RECORD_PASSWORD;
                }
                else if (!strcmp(pszArg, "--server"))
                {
                    parseMode = PARSE_MODE_LIST_RECORD_SERVER;
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

                dwError = VmDnsMakeZoneFQDN(pszArg, &pContext->pszZone);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_LIST_RECORD_OPEN;

                break;

            case PARSE_MODE_LIST_RECORD_USERNAME:

                dwError = VmDnsCopyStringArg(pszArg, ppszUserName);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_LIST_RECORD_OPEN;

                break;

            case PARSE_MODE_LIST_RECORD_DOMAIN:

                dwError = VmDnsCopyStringArg(pszArg, ppszDomain);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_LIST_RECORD_OPEN;

                break;

            case PARSE_MODE_LIST_RECORD_PASSWORD:

                dwError = VmDnsCopyStringArg(pszArg, ppszPassword);
                BAIL_ON_VMDNS_ERROR(dwError);

                parseMode = PARSE_MODE_LIST_RECORD_OPEN;

                break;

            case PARSE_MODE_LIST_RECORD_SERVER:

                dwError = VmDnsCopyStringArg(pszArg, &pContext->pszServer);
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
    PVM_DNS_CLI_CONTEXT pContext,
    PSTR* ppszUserName,
    PSTR* ppszDomain,
    PSTR* ppszPassword,
    BOOL bWithArgs
    )
{
    DWORD  dwError = 0;
    PSTR  pszForwarder = NULL;
    DWORD iArg = 0;
    PSTR pszArg;

    if (bWithArgs)
    {
        if (!argc || pContext->pszForwarder)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }
    else
    {
        if (!argc)
        {
            dwError = VmDnsCopyStringArg(VMDNS_LOCALHOST, &pContext->pszServer);
            return dwError;
        }
    }

    typedef enum
    {
        PARSE_MODE_FORWARDER_OPEN = 0,
        PARSE_MODE_FORWARDER_USERNAME,
        PARSE_MODE_FORWARDER_DOMAIN,
        PARSE_MODE_FORWARDER_PASSWORD,
        PARSE_MODE_FORWARDER_SERVER
    } PARSE_MODE_FORWARDER;
    PARSE_MODE_FORWARDER parseMode = PARSE_MODE_FORWARDER_OPEN;

    for (; iArg < argc; iArg++)
    {
        pszArg = argv[iArg];

        switch (parseMode)
        {
        case PARSE_MODE_FORWARDER_OPEN:

            if (!strcmp(pszArg, "--username"))
            {
                parseMode = PARSE_MODE_FORWARDER_USERNAME;
            }
            else if (!strcmp(pszArg, "--domain"))
            {
                parseMode = PARSE_MODE_FORWARDER_DOMAIN;
            }
            else if (!strcmp(pszArg, "--password"))
            {
                parseMode = PARSE_MODE_FORWARDER_PASSWORD;
            }
            else if (!strcmp(pszArg, "--server"))
            {
                parseMode = PARSE_MODE_FORWARDER_SERVER;
            }
            else
            {
                dwError = VmDnsAllocateStringA(pszArg, &pszForwarder);
                BAIL_ON_VMDNS_ERROR(dwError);

                pContext->pszForwarder = pszForwarder;
            }

            break;

        case PARSE_MODE_FORWARDER_USERNAME:

            dwError = VmDnsCopyStringArg(pszArg, ppszUserName);
            BAIL_ON_VMDNS_ERROR(dwError);

            parseMode = PARSE_MODE_FORWARDER_OPEN;

            break;

        case PARSE_MODE_FORWARDER_DOMAIN:

            dwError = VmDnsCopyStringArg(pszArg, ppszDomain);
            BAIL_ON_VMDNS_ERROR(dwError);

            parseMode = PARSE_MODE_FORWARDER_OPEN;

            break;

        case PARSE_MODE_FORWARDER_PASSWORD:

            dwError = VmDnsCopyStringArg(pszArg, ppszPassword);
            BAIL_ON_VMDNS_ERROR(dwError);

            parseMode = PARSE_MODE_FORWARDER_OPEN;

            break;

        case PARSE_MODE_FORWARDER_SERVER:

            dwError = VmDnsCopyStringArg(pszArg, &pContext->pszServer);
            BAIL_ON_VMDNS_ERROR(dwError);

            parseMode = PARSE_MODE_FORWARDER_OPEN;

            break;

        default:

            dwError = ERROR_INTERNAL_ERROR;
            BAIL_ON_VMDNS_ERROR(dwError);

            break;
        }

    }


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
        "\tadd-zone <zone name | network id>\n"
        "\t\t--ns-host <hostname>\n"
        "\t\t--ns-ip <ip address>\n"
        "\t\t[--admin-email <admin-email>]\n"
        "\t\t[--type <forward>]\n"
        /*"\t\t[--type <forward|reverse>]\n"*/
        "\t\t--server <server>\n"
        "\t\t--username <user>\n"
        "\t\t--domain <domain>\n"
        "\t\t--password <pwd>\n"
        /*"\t\tfor reverse lookup zone, pass network id instead of zone name. Zone name will be generated based on network id.\n"
        "\t\tfor example, 10.118.1.0/24 where 24 is the network id length in bits.\n"
        "\t\tfor ipv4, network id length can be 8, 16 or 24.\n"
        "\t\tfor ipv6, network id length must be more than 0 and less than 128 and must be dividable by 8.\n"
        "\t\tip address part must be full ip address.\n"
        "\t\t--ns-ip isn't needed for reverse zone.\n"*/
        "\tExample: add-zone zone1 --ns-host ns1 --ns-ip 1.10.0.192 --type forward\n\n"
        "\tdel-zone <zone name>\n"
        "\t\t--server <server>\n"
        "\t\t--username <user>\n"
        "\t\t--domain <domain>\n"
        "\t\t--password <pwd>\n"
        "\tExample: del-zone zone1\n\n"
        "\tlist-zone\n"
        "\t\t--server <server>\n"
        "\t\t--username <user>\n"
        "\t\t--domain <domain>\n"
        "\t\t--password <pwd>\n\n"
        "\tadd-record --zone <zone name>\n"
        "\t\t--type <record type>\n"
        "\t\t[<key> <value>]...\n"
        "\t\t--server <server>\n"
        "\t\t--username <user>\n"
        "\t\t--domain <domain>\n"
        "\t\t--password <pwd>\n"
        "\t\t<key> <value> pair for SRV:\n"
        "\t\t\t--service <service-type>\n"
        "\t\t\tService type examples: ldap, kerberos or any other services\n"
        "\t\t\t--protocol <tcp|udp>\n"
        "\t\t\t--target <hostname>\n"
        "\t\t\t--priority <priority>\n"
        "\t\t\t--weight <weight>\n"
        "\t\t\t--port <port>\n"
        "\t\t<key> <value> pair for A:\n"
        "\t\t\t--hostname <hostname>\n"
        "\t\t\t--ip <ip-address>\n"
        "\t\t<key> <value> pair for AAAA:\n"
        "\t\t\t--hostname <hostname>\n"
        "\t\t\t--ip6 <ip6address>\n"
        "\t\t<key> <value> pair for NS:\n"
        "\t\t\t--ns-domain   <domain>\n"
        "\t\t\t--hostname <hostname>\n"
/*      "\t\t<key> <value> pair for PTR:\n"
        "\t\t\t--<ip|ip6> <address>\n"
        "\t\t\t--hostname <hostname>\n" */
        "\t\t<key> <value> pair for CNAME:\n"
        "\t\t\t--<name> <name>\n"
        "\t\t\t--hostname <hostname>\n"
        "\tExample: add-record --zone zone1 --type A --hostname test-host --ip 1.10.0.124\n\n"
        "\tdel-record --zone <zone name>\n"
        "\t\t--type <record type>\n"
        "\t\t[<key> <value>]...\n"
        "\t\t--server <server>\n"
        "\t\t--username <user>\n"
        "\t\t--domain <domain>\n"
        "\t\t--password <pwd>\n"
        "\t\t<key> <value> pair for SRV:\n"
        "\t\t\t--service <service-type>\n"
        "\t\t\tService type examples: ldap, kerberos or any other services\n"
        "\t\t\t--protocol <tcp|udp>\n"
        "\t\t\t--target <hostname>\n"
        "\t\t<key> <value> pair for A:\n"
        "\t\t\t--hostname <hostname>\n"
        "\t\t\t--ip <ip-address>\n"
        "\t\t<key> <value> pair for AAAA:\n"
        "\t\t\t--hostname <hostname>\n"
        "\t\t\t--ip6 <ip6-address>\n"
        "\t\t<key> <value> pair for NS:\n"
        "\t\t\t--ns-domain <domain>\n"
        "\t\t\t--hostname <hostname>\n"
/*        "\t\t<key> <value> pair for PTR:\n"
        "\t\t\t--<ip|ip6> <address>\n"
        "\t\t\t--hostname <hostname>\n" */
        "\t\t<key> <value> pair for CNAME:\n"
        "\t\t\t--name <name>\n"
        "\t\t\t--hostname <hostname>\n"
        "\tlist-record --zone <zone name>\n"
        "\t\t[--server <server>]\n"
        "\t\t[--username <user>]\n"
        "\t\t[--domain <domain>]\n"
        "\t\t[--password <pwd>]\n\n"
        "\tquery-record --zone <zone name>\n"
        "\t\t--type <record type>\n"
        "\t\t--name <record name>\n"
        "\t\t<key> <value>\n"
        "\t\t--server <server>\n"
        "\t\t--username <user>\n"
        "\t\t--domain <domain>\n"
        "\t\t--password <pwd>\n\n"
        "\tadd-forwarder <forwarder ip>\n"
        "\t\t--server <server>\n"
        "\t\t--username <user>\n"
        "\t\t--domain <domain>\n"
        "\t\t--password <pwd>\n\n"
        "\tdel-forwarder <forwarder ip>\n"
        "\t\t--server <server>\n"
        "\t\t--username <user>\n"
        "\t\t--domain <domain>\n"
        "\t\t--password <pwd>\n\n"
        "\tlist-forwarders\n"
        "\t\t--server <server>\n"
        "\t\t--username <user>\n"
        "\t\t--domain <domain>\n"
        "\t\t--password <pwd>\n\n"
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
    VMDNS_IP4_ADDRESS ip4 = 0;
    int ret = 0;
    unsigned char buf[sizeof(struct in_addr)];

    BAIL_ON_VMDNS_EMPTY_STRING(pszIPAddress, dwError);

    ret = inet_pton(AF_INET, pszIPAddress, buf);
    if (ret <= 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    ip4 = (VMDNS_IP4_ADDRESS)ntohl(((struct in_addr *)buf)->s_addr);

    dwError = VmDnsCopyMemory(
                  &pData->IpAddress,
                  sizeof(pData->IpAddress),
                  &ip4,
                  sizeof(pData->IpAddress)
                  );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsCliGetRecordData_AAAA(
    PCSTR               pszIP6Address,
    PVMDNS_AAAA_DATA    pData
    )
{
    DWORD dwError = 0;
    struct addrinfo *pAddrInfo = NULL;
    int ret = 0;
    unsigned char buf[sizeof(struct in6_addr)];

    BAIL_ON_VMDNS_EMPTY_STRING(pszIP6Address, dwError);

    ret = inet_pton(AF_INET6, pszIP6Address, buf);
    if (ret <= 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCopyMemory(
                    pData->Ip6Address.IP6Byte,
                    sizeof(pData->Ip6Address.IP6Byte),
#ifdef _WIN32
                    ((struct in6_addr*)buf)->u.Byte,
#else
                    ((struct in6_addr*)buf)->s6_addr,
#endif
                    sizeof(pData->Ip6Address.IP6Byte));
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    if (pAddrInfo)
    {
        freeaddrinfo(pAddrInfo);
    }
    return dwError;

error:

    goto cleanup;
}


static
DWORD
ParseArgsListZones(
    int                 argc,
    char*               argv[],
    PVM_DNS_CLI_CONTEXT pContext,
    PSTR*               ppszUserName,
    PSTR*               ppszDomain,
    PSTR*               ppszPassword
    )
{
    DWORD dwError = 0;

    if (!argc)
    {
        dwError = VmDnsCopyStringArg(VMDNS_LOCALHOST, &pContext->pszServer);
        BAIL_ON_VMDNS_ERROR(dwError);
        return dwError;
    }

    typedef enum
    {
        PARSE_MODE_LIST_ZONE_OPEN = 0,
        PARSE_MODE_LIST_ZONE_SERVER,
        PARSE_MODE_LIST_ZONE_USERNAME,
        PARSE_MODE_LIST_ZONE_PASSWORD,
        PARSE_MODE_LIST_ZONE_DOMAIN,
    } PARSE_MODE_LIST_ZONE;
    PARSE_MODE_LIST_ZONE parseMode = PARSE_MODE_LIST_ZONE_OPEN;
    int iArg = 0;

    for (; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];

        switch (parseMode)
        {
        case PARSE_MODE_LIST_ZONE_OPEN:

            if (!strcmp(pszArg, "--username"))
            {
                parseMode = PARSE_MODE_LIST_ZONE_USERNAME;
            }
            else if (!strcmp(pszArg, "--domain"))
            {
                parseMode = PARSE_MODE_LIST_ZONE_DOMAIN;
            }
            else if (!strcmp(pszArg, "--password"))
            {
                parseMode = PARSE_MODE_LIST_ZONE_PASSWORD;
            }
            else if (!strcmp(pszArg, "--server"))
            {
                parseMode = PARSE_MODE_LIST_ZONE_SERVER;
            }
            else
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDNS_ERROR(dwError);
            }

            break;

        case PARSE_MODE_LIST_ZONE_USERNAME:

            dwError = VmDnsCopyStringArg(pszArg, ppszUserName);
            BAIL_ON_VMDNS_ERROR(dwError);

            parseMode = PARSE_MODE_LIST_ZONE_OPEN;

            break;

        case PARSE_MODE_LIST_ZONE_DOMAIN:

            dwError = VmDnsCopyStringArg(pszArg, ppszDomain);
            BAIL_ON_VMDNS_ERROR(dwError);

            parseMode = PARSE_MODE_LIST_ZONE_OPEN;

            break;

        case PARSE_MODE_LIST_ZONE_PASSWORD:

            dwError = VmDnsCopyStringArg(pszArg, ppszPassword);
            BAIL_ON_VMDNS_ERROR(dwError);

            parseMode = PARSE_MODE_LIST_ZONE_OPEN;

            break;

        case PARSE_MODE_LIST_ZONE_SERVER:

            dwError = VmDnsCopyStringArg(pszArg, &pContext->pszServer);
            BAIL_ON_VMDNS_ERROR(dwError);

            parseMode = PARSE_MODE_LIST_ZONE_OPEN;

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
VerifyRemoteConnectionArgs(
    PSTR* ppszServer,
    PSTR pszUserName,
    PSTR pszDomain,
    PSTR* ppszPassword
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR  pszPasswordBuf = NULL;
    PSTR  pszPassword = NULL;

    if (!IsNullOrEmptyString(*ppszServer)
            && !IsNullOrEmptyString(pszUserName)
            && !IsNullOrEmptyString(*ppszPassword)
            && !IsNullOrEmptyString(pszDomain))
    {
        dwError = ERROR_SUCCESS;
    }
    else if (IsNullOrEmptyString(*ppszServer))
    {
        dwError = VmDnsCopyStringArg(VMDNS_LOCALHOST, ppszServer);
    }
    else if (!IsNullOrEmptyString(*ppszServer)
                && (VmDnsStringCompareA(*ppszServer, VMDNS_LOCALHOST, FALSE) != 0)
                && (IsNullOrEmptyString(pszUserName)
                || IsNullOrEmptyString(*ppszPassword)
                || IsNullOrEmptyString(pszDomain)))
    {
        fprintf(
            stdout,
            "Remote server requires authentication. Please, specify username, password and domain.\n");

        dwError = ERROR_INVALID_PARAMETER;
    }

    if (!IsNullOrEmptyString(pszUserName) && IsNullOrEmptyString(*ppszPassword))
    {
        dwError = VmDnsAllocateMemory(VMDNS_MAX_PWD_LEN + 1, (PVOID *)&pszPasswordBuf);
        BAIL_ON_VMDNS_ERROR(dwError);

        VmDnsReadString("password: ", pszPasswordBuf, VMDNS_MAX_PWD_LEN + 1, TRUE);

        dwError = VmDnsAllocateStringA(pszPasswordBuf, &pszPassword);
        BAIL_ON_VMDNS_ERROR(dwError);

        *ppszPassword = pszPassword;
    }

cleanup:
    return dwError;

error:
    VMDNS_SAFE_FREE_STRINGA(pszPassword);
    goto cleanup;
}


static
DWORD
VmDnsSetDefaultParams(
    PSTR* ppszServer,
    PSTR* ppszUserName
    )
{
    DWORD dwError = 0;
    PSTR pszServerName = NULL;
    PSTR pszUser = NULL;

    if ((*ppszServer) == NULL )
    {
        dwError = VmDnsAllocateStringA("localhost", &pszServerName);
        BAIL_ON_VMDNS_ERROR(dwError);

    }

    if ((*ppszUserName) == NULL )

    {
        dwError = VmDnsAllocateStringA("Administrator", &pszUser );
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (pszServerName != NULL)
    {
        *ppszServer = pszServerName;
    }
    if (pszUser != NULL)
    {
        *ppszUserName = pszUser;
    }
cleanup:
    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsMakeZoneFQDN(
    PCSTR pszZoneName,
    PSTR* ppszZoneFqdn
    )
{
    DWORD dwError = 0;
    size_t len = 0;
    PSTR q = NULL;
    PCSTR p = pszZoneName;
    PSTR pszZoneFqdn = NULL;

    if (IsNullOrEmptyString(pszZoneName) ||
        ppszZoneFqdn == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    len = strlen(pszZoneName);

    dwError = VmDnsAllocateMemory(len + 2, (PVOID*)&pszZoneFqdn);
    BAIL_ON_VMDNS_ERROR(dwError);

    q = pszZoneFqdn;
    while (*p)
    {
        *q++ = *p++;
    }

    if (*(q-1) != '.')
    {
        *q++ = '.';
        *q = 0;
    }

    *ppszZoneFqdn = pszZoneFqdn;

cleanup:
    return dwError;

error :
    VMDNS_SAFE_FREE_MEMORY(pszZoneFqdn);
    goto cleanup;
}
