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


#ifndef _WIN32
static const struct option g_long_options[] =
{
        {SLCLI_LONG_OPTION_NETWORK_ADDRESS, required_argument,  0,  SLCLI_OPTION_NETWORK_ADDRESS},
        {SLCLI_LONG_OPTION_DOMAIN,          required_argument,  0,  SLCLI_OPTION_DOMAIN},
        {SLCLI_LONG_OPTION_USERNAME,        required_argument,  0,  SLCLI_OPTION_USERNAME},
        {SLCLI_LONG_OPTION_PASSWORD,        required_argument,  0,  SLCLI_OPTION_PASSWORD},
        {SLCLI_LONG_OPTION_ENABLE,          no_argument,        0,  SLCLI_OPTION_ENABLE},
        {SLCLI_LONG_OPTION_IS_ENABLED,      no_argument,        0,  SLCLI_OPTION_IS_ENABLED},
        {SLCLI_LONG_OPTION_DISABLE,         no_argument,        0,  SLCLI_OPTION_DISABLE},
        {SLCLI_LONG_OPTION_SET_SIZE,        required_argument,  0,  SLCLI_OPTION_SET_SIZE},
        {SLCLI_LONG_OPTION_GET_SIZE,        no_argument,        0,  SLCLI_OPTION_GET_SIZE},
        {SLCLI_LONG_OPTION_RETRIEVE,        no_argument,        0,  SLCLI_OPTION_RETRIEVE},
        {SLCLI_LONG_OPTION_FLUSH,           no_argument,        0,  SLCLI_OPTION_FLUSH},
        {SLCLI_LONG_OPTION_AGGREGATE,       no_argument,        0,  SLCLI_OPTION_AGGREGATE},
        {SLCLI_LONG_COLUMN_LOGIN_DN,        no_argument,        0,  SLCLI_COLUMN_LOGIN_DN},
        {SLCLI_LONG_COLUMN_IP,              no_argument,        0,  SLCLI_COLUMN_IP},
        {SLCLI_LONG_COLUMN_PORT,            no_argument,        0,  SLCLI_COLUMN_PORT},
        {SLCLI_LONG_COLUMN_OPERATION,       no_argument,        0,  SLCLI_COLUMN_OPERATION},
        {SLCLI_LONG_COLUMN_STRING,          no_argument,        0,  SLCLI_COLUMN_STRING},
        {SLCLI_LONG_COLUMN_ERROR_CODE,      no_argument,        0,  SLCLI_COLUMN_ERROR_CODE},
        {SLCLI_LONG_COLUMN_TIME,            no_argument,        0,  SLCLI_COLUMN_TIME},
        {0, 0, 0, 0}
};
#endif


DWORD
VmAfdParseBaseArgs(
        int   argc,
        char* argv[],
        PSTR* ppszNetworkAddress,
        PSTR* ppszDomain,
        PSTR* ppszUserName,
        PSTR* ppszPassword
        )
{
    DWORD dwError = ERROR_SUCCESS;

#ifndef _WIN32
    int opt = 0;
    int opt_idx = 0;
#else
    int i = 1;
#endif

    PSTR pszNetworkAddress = NULL;
    PSTR pszDomain = NULL;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;

    if(argc <= 1)
    {
        //printf("%s\n", "VmAfdParseBaseArgs::ERROR_INVALID_PARAMETER. Mandatory parameters are not provided.");
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (ppszNetworkAddress == NULL
            || ppszDomain == NULL
            || ppszUserName == NULL
            || ppszPassword == NULL)
    {
        //printf("%s\n", "VmAfdParseBaseArgs::ERROR_INVALID_PARAMETER");
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

#ifndef _WIN32
    optind = 1;
    while ((opt = getopt_long(argc, argv, SLCLI_OPTIONS, g_long_options, &opt_idx)) != -1)
    {
        switch (opt)
        {
        case SLCLI_OPTION_NETWORK_ADDRESS:
            pszNetworkAddress = optarg;
            break;

        case SLCLI_OPTION_DOMAIN:
            pszDomain = optarg;
            break;

        case SLCLI_OPTION_USERNAME:
            pszUserName = optarg;
            break;

        case SLCLI_OPTION_PASSWORD:
            pszPassword = optarg;
            break;

        case '?':
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
            break;

        default:
            break;
        }
    }
#else
    while (i < argc)
    {
        if (VmAfdIsCmdLineOption(argv[i]))
        {
            if (VmAfdStringCompareA(SLCLI_LONG_OPTION_NETWORK_ADDRESS, argv[i], TRUE) == 0
                    || VmAfdStringCompareA(SLCLI_OPTION_NETWORK_ADDRESS, argv[i], TRUE) == 0)
            {
                VmAfdGetCmdLineOption(argc, argv, &i, &pszNetworkAddress);
            }
            else if (VmAfdStringCompareA(SLCLI_LONG_OPTION_DOMAIN, argv[i], TRUE) == 0
                        || VmAfdStringCompareA(SLCLI_OPTION_DOMAIN, argv[i], TRUE) == 0)
            {
                VmAfdGetCmdLineOption(argc, argv, &i, &pszDomain);
            }
            else if (VmAfdStringCompareA(SLCLI_LONG_OPTION_USERNAME, argv[i], TRUE) == 0
                        || VmAfdStringCompareA(SLCLI_OPTION_USERNAME, argv[i], TRUE) == 0)
            {
                VmAfdGetCmdLineOption(argc, argv, &i, &pszUserName);
            }
            else if (VmAfdStringCompareA(SLCLI_LONG_OPTION_PASSWORD, argv[i], TRUE) == 0
                        || VmAfdStringCompareA(SLCLI_OPTION_PASSWORD, argv[i], TRUE) == 0)
            {
                VmAfdGetCmdLineOption(argc, argv, &i, &pszPassword);
            }
        }
        i++;
    }
#endif

    *ppszNetworkAddress = pszNetworkAddress;
    *ppszDomain = pszDomain;
    *ppszUserName = pszUserName;
    *ppszPassword = pszPassword;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmAfdParseOperationArgs(
        int      argc,
        char*    argv[],
        PBOOLEAN pbEnable,
        PBOOLEAN pbIsEnabled,
        PBOOLEAN pbDisable,
        PBOOLEAN pbSetSize,
        PBOOLEAN pbGetSize,
        PBOOLEAN pbRetrieve,
        PBOOLEAN pbFlush,
        PBOOLEAN pbAggregate
        )
{
    DWORD dwError = ERROR_SUCCESS;
#ifndef _WIN32
    int opt = 0;
    int opt_idx = 0;
#else
    int i = 1;
#endif

    BOOLEAN bEnable     = FALSE;
    BOOLEAN bIsEnabled  = FALSE;
    BOOLEAN bDisable    = FALSE;
    BOOLEAN bSetSize    = FALSE;
    BOOLEAN bGetSize    = FALSE;
    BOOLEAN bRetrieve   = FALSE;
    BOOLEAN bFlush      = FALSE;
    BOOLEAN bAggregate  = FALSE;

    if (pbEnable == NULL
            || pbIsEnabled == NULL
            || pbDisable == NULL
            || pbSetSize == NULL
            || pbGetSize == NULL
            || pbRetrieve == NULL
            || pbFlush == NULL
            || pbAggregate == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

#ifndef _WIN32
    optind = 1;
    while ((opt = getopt_long(argc, argv, SLCLI_OPTIONS, g_long_options, &opt_idx)) != -1)
    {
        switch (opt)
        {
        case SLCLI_OPTION_ENABLE:
            bEnable = TRUE;
            break;

        case SLCLI_OPTION_IS_ENABLED:
            bIsEnabled = TRUE;
            break;

        case SLCLI_OPTION_DISABLE:
            bDisable = TRUE;
            break;

        case SLCLI_OPTION_SET_SIZE:
            bSetSize = TRUE;
            break;

        case SLCLI_OPTION_GET_SIZE:
            bGetSize = TRUE;
            break;

        case SLCLI_OPTION_RETRIEVE:
            bRetrieve = TRUE;
            break;

        case SLCLI_OPTION_FLUSH:
            bFlush = TRUE;
            break;

        case SLCLI_OPTION_AGGREGATE:
            bAggregate = TRUE;
            break;

        case '?':
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
            break;

        default:
            break;
        }
    }
#else
    while (i < argc)
    {
        printf("\narg %d %s\n", i, argv[i]);

        if (VmAfdIsCmdLineOption(argv[i]))
        {
            if (VmAfdStringCompareA(SLCLI_LONG_OPTION_ENABLE, argv[i], TRUE) == 0
                    || VmAfdStringCompareA(SLCLI_OPTION_ENABLE, argv[i], TRUE) == 0)
            {
                bEnable = TRUE;
            }
            else if (VmAfdStringCompareA(SLCLI_LONG_OPTION_IS_ENABLED, argv[i], TRUE) == 0
                        || VmAfdStringCompareA(SLCLI_OPTION_IS_ENABLED, argv[i], TRUE) == 0)
            {
                bIsEnabled = TRUE;
            }
            else if (VmAfdStringCompareA(SLCLI_LONG_OPTION_DISABLE, argv[i], TRUE) == 0
                        || VmAfdStringCompareA(SLCLI_OPTION_DISABLE, argv[i], TRUE) == 0)
            {
                bDisable = TRUE;
            }
            else if (VmAfdStringCompareA(SLCLI_LONG_OPTION_SET_SIZE, argv[i], TRUE) == 0
                        || VmAfdStringCompareA(SLCLI_OPTION_SET_SIZE, argv[i], TRUE) == 0)
            {
                bSetSize = TRUE;
            }
            else if (VmAfdStringCompareA(SLCLI_LONG_OPTION_GET_SIZE, argv[i], TRUE) == 0
                        || VmAfdStringCompareA(SLCLI_OPTION_GET_SIZE, argv[i], TRUE) == 0)
            {
                bGetSize = TRUE;
            }
            else if (VmAfdStringCompareA(SLCLI_LONG_OPTION_RETRIEVE, argv[i], TRUE) == 0
                        || VmAfdStringCompareA(SLCLI_OPTION_RETRIEVE, argv[i], TRUE) == 0)
            {
                bRetrieve = TRUE;
            }
            else if (VmAfdStringCompareA(SLCLI_LONG_OPTION_FLUSH, argv[i], TRUE) == 0
                        || VmAfdStringCompareA(SLCLI_OPTION_FLUSH, argv[i], TRUE) == 0)
            {
                bFlush = TRUE;
            }
            else if (VmAfdStringCompareA(SLCLI_LONG_OPTION_AGGREGATE, argv[i], TRUE) == 0
                        || VmAfdStringCompareA(SLCLI_OPTION_AGGREGATE, argv[i], TRUE) == 0)
            {
                bAggregate = TRUE;
            }
        }
        i++;
    }
#endif

    *pbEnable       = bEnable;
    *pbIsEnabled    = bIsEnabled;
    *pbDisable      = bDisable;
    *pbSetSize      = bSetSize;
    *pbGetSize      = bGetSize;
    *pbRetrieve     = bRetrieve;
    *pbFlush        = bFlush;
    *pbAggregate    = bAggregate;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmAfdParseSetSizeArgs(
        int     argc,
        char*   argv[],
        PDWORD  pdwSize
        )
{
    DWORD dwError = ERROR_SUCCESS;
#ifndef _WIN32
    int opt = 0;
    int opt_idx = 0;
#else
    int i = 1;
    PSTR pszSize = NULL;
#endif

    DWORD dwSize = 0;

    if (pdwSize == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

#ifndef _WIN32
    optind = 1;
    while ((opt = getopt_long(argc, argv, SLCLI_OPTIONS, g_long_options, &opt_idx)) != -1)
    {
        switch (opt)
        {
        case SLCLI_OPTION_SET_SIZE:
            dwSize = (DWORD)VmAfdStringToLA(optarg, NULL, 10);
            break;

        case '?':
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
            break;

        default:
            break;
        }
    }
#else
    while (i < argc)
    {
        if (VmAfdIsCmdLineOption(argv[i]))
        {
            if (VmAfdStringCompareA(SLCLI_LONG_OPTION_SET_SIZE, argv[i], TRUE) == 0
                    || VmAfdStringCompareA(SLCLI_OPTION_SET_SIZE, argv[i], TRUE) == 0)
            {
                VmAfdGetCmdLineOption(argc, argv, &i, &pszSize);
                dwSize = (DWORD)VmAfdStringToLA(pszSize, NULL, 10);
            }
        }
        i++;
    }
#endif

    *pdwSize = dwSize;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmAfdParseAggregateArgs(
        int      argc,
        char*    argv[],
        PBOOLEAN pbLoginDN,
        PBOOLEAN pbIP,
        PBOOLEAN pbPort,
        PBOOLEAN pbOperation,
        PBOOLEAN pbString,
        PBOOLEAN pbErrorCode,
        PBOOLEAN pbTime
        )
{
    DWORD dwError = ERROR_SUCCESS;
#ifndef _WIN32
    int opt = 0;
    int opt_idx = 0;
#else
    int i = 1;
#endif

    BOOLEAN bLoginDN = FALSE;
    BOOLEAN bIP = FALSE;
    BOOLEAN bPort = FALSE;
    BOOLEAN bOperation = FALSE;
    BOOLEAN bString = FALSE;
    BOOLEAN bErrorCode = FALSE;
    BOOLEAN bTime = FALSE;

    if (pbLoginDN == NULL
            || pbIP == NULL
            || pbPort == NULL
            || pbOperation == NULL
            || pbString == NULL
            || pbErrorCode == NULL
            || pbTime == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

#ifndef _WIN32
    optind = 1;
    while ((opt = getopt_long(argc, argv, SLCLI_OPTIONS, g_long_options, &opt_idx)) != -1)
    {
        switch (opt)
        {
        case SLCLI_COLUMN_LOGIN_DN:
            bLoginDN = TRUE;
            break;

        case SLCLI_COLUMN_IP:
            bIP = TRUE;
            break;

        case SLCLI_COLUMN_PORT:
            bPort = TRUE;
            break;

        case SLCLI_COLUMN_OPERATION:
            bOperation = TRUE;
            break;

        case SLCLI_COLUMN_STRING:
            bString = TRUE;
            break;

        case SLCLI_COLUMN_ERROR_CODE:
            bErrorCode = TRUE;
            break;

        case SLCLI_COLUMN_TIME:
            bTime = TRUE;
            break;

        default:
            break;
        }
    }
#else
    while (i < argc)
    {
        if (VmAfdIsCmdLineOption(argv[i]))
        {
            if (VmAfdStringCompareA(SLCLI_LONG_COLUMN_LOGIN_DN, argv[i], TRUE) == 0
                    || VmAfdStringCompareA(SLCLI_COLUMN_LOGIN_DN, argv[i], TRUE) == 0)
            {
                bLoginDN = TRUE;
            }
            else if (VmAfdStringCompareA(SLCLI_LONG_COLUMN_IP, argv[i], TRUE) == 0
                        || VmAfdStringCompareA(SLCLI_COLUMN_IP, argv[i], TRUE) == 0)
            {
                bIP = TRUE;
            }
            else if (VmAfdStringCompareA(SLCLI_LONG_COLUMN_PORT, argv[i], TRUE) == 0
                        || VmAfdStringCompareA(SLCLI_COLUMN_PORT, argv[i], TRUE) == 0)
            {
                bPort = TRUE;
            }
            else if (VmAfdStringCompareA(SLCLI_LONG_COLUMN_OPERATION, argv[i], TRUE) == 0
                        || VmAfdStringCompareA(SLCLI_COLUMN_OPERATION, argv[i], TRUE) == 0)
            {
                bOperation = TRUE;
            }
            else if (VmAfdStringCompareA(SLCLI_LONG_COLUMN_STRING, argv[i], TRUE) == 0
                        || VmAfdStringCompareA(SLCLI_COLUMN_STRING, argv[i], TRUE) == 0)
            {
                bString = TRUE;
            }
            else if (VmAfdStringCompareA(SLCLI_LONG_COLUMN_ERROR_CODE, argv[i], TRUE) == 0
                        || VmAfdStringCompareA(SLCLI_COLUMN_ERROR_CODE, argv[i], TRUE) == 0)
            {
                bErrorCode = TRUE;
            }
            else if (VmAfdStringCompareA(SLCLI_LONG_COLUMN_TIME, argv[i], TRUE) == 0
                        || VmAfdStringCompareA(SLCLI_COLUMN_TIME, argv[i], TRUE) == 0)
            {
                bTime = TRUE;
            }
        }
        i++;
    }
#endif

    *pbLoginDN = bLoginDN;
    *pbIP = bIP;
    *pbPort = bPort;
    *pbOperation = bOperation;
    *pbString = bString;
    *pbErrorCode = bErrorCode;
    *pbTime = bTime;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmAfdValidateBaseArgs(
        PSTR pszNetworkAddress,
        PSTR pszDomain,
        PSTR pszUserName,
        PSTR pszPassword
        )
{
    DWORD dwError = 0;

    if (!pszNetworkAddress || !pszDomain || !pszUserName || !pszPassword)
    {
        if (!pszNetworkAddress)
        {
            printf("Missing option: Network address\n");
            dwError = ERROR_INVALID_PARAMETER;
        }
        if (!pszDomain)
        {
            printf("Missing option: domain\n");
            dwError = ERROR_INVALID_PARAMETER;
        }
        if (!pszUserName)
        {
            printf("Missing option: user name\n");
            dwError = ERROR_INVALID_PARAMETER;
        }
        if (!pszPassword)
        {
            printf("Missing option: password\n");
            dwError = ERROR_INVALID_PARAMETER;
        }
    }

    return dwError;
}

DWORD
VmAfdValidateOperationArgs(
        BOOLEAN bEnable,
        BOOLEAN bIsEnabled,
        BOOLEAN bDisable,
        BOOLEAN bSetSize,
        BOOLEAN bGetSize,
        BOOLEAN bRetrieve,
        BOOLEAN bFlush,
        BOOLEAN bAggregate
        )
{
    DWORD dwError = 0;
    DWORD dwOpCnt = 0;

    if (bEnable)
    {
        dwOpCnt++;
    }
    if (bIsEnabled)
    {
        dwOpCnt++;
    }
    if (bDisable)
    {
        dwOpCnt++;
    }
    if (bSetSize)
    {
        dwOpCnt++;
    }
    if (bGetSize)
    {
        dwOpCnt++;
    }
    if (bRetrieve)
    {
        dwOpCnt++;
    }
    if (bFlush)
    {
        dwOpCnt++;
    }
    if (bAggregate)
    {
        dwOpCnt++;
    }

    if (dwOpCnt == 0)
    {
        printf( "No operation were chosen\n");
        dwError = ERROR_INVALID_PARAMETER;
    }
    else if (dwOpCnt > 1)
    {
        printf( "More than one operation were chosen, "
                "You can choose only one operation\n");
        dwError = ERROR_INVALID_PARAMETER;
    }

    return dwError;
}

DWORD
VmAfdValidateSetSizeArgs(
        DWORD dwSize
        )
{
    DWORD dwError = 0;

    if (dwSize == 0)
    {
        printf("Enter valid buffer size to set\n");
        dwError = ERROR_INVALID_PARAMETER;
    }

    return dwError;
}


VOID
ShowUsage(
        VOID
        )
{
    printf( "Usage: \n"
            "SLCLI [-h <network_address> -d <domain> -u <username> -w <password>] ...\n"
            "\n"
            "   [--enable|-E] Enables the node collecting data\n"
            "       SLCLI --enable\n"
            "                 [-h <network_address>\n"
            "                  -d <domain>\n"
            "                  -u <username>\n"
            "                  -w <password>]\n"
            "\n"
            "   [--isenabled|-I] Checks whether the node is already collecting data\n"
            "       SLCLI --isenabled\n"
            "                 [-h <network_address>\n"
            "                  -d <domain>\n"
            "                  -u <username>\n"
            "                  -w <password>]\n"
            "\n"
            "   [--disable|-D] Disables the node collecting data\n"
            "       SLCLI --disable\n"
            "                 [-h <network_address>\n"
            "                  -d <domain>\n"
            "                  -u <username>\n"
            "                  -w <password>]\n"
            "\n"
            "   [--setsize|-S] Configures how much data the node should keep in its buffer\n"
            "       SLCLI --setsize <capacity>\n"
            "                 [-h <network_address>\n"
            "                  -d <domain>\n"
            "                  -u <username>\n"
            "                  -w <password>]\n"
            "\n"
            "   [--getsize|-G] Gets how much data the node is configured to keep in its buffer\n"
            "       SLCLI --getsize\n"
            "                 [-h <network_address>\n"
            "                  -d <domain>\n"
            "                  -u <username>\n"
            "                  -w <password>]\n"
            "\n"
            "   [--retrieve|-R] Retrieve collected data from the node and prints in terminal\n"
            "       SLCLI --retrieve\n"
            "                 [-h <network_address>\n"
            "                  -d <domain>\n"
            "                  -u <username>\n"
            "                  -w <password>]\n"
            "\n"
            "   [--flush|-F] Flushes the node's buffer up to the offset which already has been read\n"
            "       SLCLI --flush\n"
            "                 [-h <network_address>\n"
            "                  -d <domain>\n"
            "                  -u <username>\n"
            "                  -w <password>]\n"
            "\n"
//            "   [--aggregate|-A] Aggregates superlog data and displays by columns: (l) loginDN\n"
//            "                                                                      (i) ip\n"
//            "                                                                      (p) port\n"
//            "                                                                      (o) operation\n"
//            "                                                                      (s) string\n"
//            "                                                                      (e) errorcode\n"
//            "                                                                      (t) time\n"
//            "       SLCLI --aggregate [-l -i -p -o -s -e -t]\n"
//            "                 [-h <network_address>\n"
//            "                  -d <domain>\n"
//            "                  -u <username>\n"
//            "                  -w <password>]\n"
    );
}
