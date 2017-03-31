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
 * Module Name: vdcrepadmin
 *
 * Filename: parseargs.c
 *
 * Abstract:
 *
 * vdcrepadmin argument parsing functions
 *
 */

#include "includes.h"

DWORD
VmDirParseArgs(
    int      argc,
    char*    argv[],
    PSTR*    ppszFeatureSet,
    PBOOLEAN pbTwoWayRepl,
    PSTR*    ppszSrcHostName,
    PSTR*    ppszSrcPort,
    PSTR*    ppszSrcUserName,
    PSTR*    ppszSrcPassword,
    PSTR*    ppszTgtHostName,
    PSTR*    ppszTgtPort,
    PSTR*    ppszEntryDn,
    PSTR*    ppszAttribute,
    PBOOLEAN pbVerbose
    )
{
    DWORD   dwError        = ERROR_SUCCESS;
    PSTR    pszFeatureSet  = NULL;
    PSTR    pszSrcHostName = NULL;
    PSTR    pszSrcPort     = DEFAULT_LDAPS_PORT_STR;
    PSTR    pszSrcUserName = NULL;
    PSTR    pszSrcPassword = NULL;
    PSTR    pszTgtHostName = NULL;
    PSTR    pszTgtPort     = DEFAULT_LDAPS_PORT_STR;
    PSTR    pszEntryDn     = NULL;
    PSTR    pszAttribute   = NULL;
    BOOLEAN bVerbose       = FALSE;
    BOOLEAN bTwoWayRepl    = FALSE;

#ifndef _WIN32
    int opt = 0;
#else
    int i=1;
    PSTR optarg = NULL;
#endif

    if (
           ppszFeatureSet  == NULL
        || ppszSrcHostName == NULL
        || ppszSrcPort     == NULL
        || ppszSrcUserName == NULL
        || ppszSrcPassword == NULL
        || ppszTgtHostName == NULL
        || ppszTgtPort     == NULL
        || pbVerbose       == NULL
        || pbTwoWayRepl    == NULL


        )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#ifndef _WIN32
    while ( (opt = getopt( argc, argv, VDCREPADMIN_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
            case VDCREPADMIN_OPTION_SOURCE_HOSTNAME:
                pszSrcHostName = optarg;
                break;

            case VDCREPADMIN_OPTION_SOURCE_PORT:
                pszSrcPort = optarg;
                break;

            case VDCREPADMIN_OPTION_SOURCE_USERNAME:
                pszSrcUserName = optarg;
                break;

            case VDCREPADMIN_OPTION_SOURCE_PASSWORD:
                pszSrcPassword = optarg;
                break;

            case VDCREPADMIN_OPTION_TARGET_HOSTNAME:
                pszTgtHostName = optarg;
                break;

            case VDCREPADMIN_OPTION_TARGET_PORT:
                pszTgtPort = optarg;
                break;

            case VDCREPADMIN_OPTION_VERBOSE:
                bVerbose = TRUE;
                break;

            case VDCREPADMIN_OPTION_TWO_WAY_REPL:
                bTwoWayRepl = TRUE;
                break;

            case VDCREPADMIN_OPTION_FEATURE_SET:
                pszFeatureSet = optarg;
                break;

            case VDCREPADMIN_OPTION_ENTRY_DN:
                pszEntryDn = optarg;
                break;

            case VDCREPADMIN_OPTION_ATTRIBUTE:
                pszAttribute = optarg;
                break;

            default:
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDIR_ERROR(dwError);
                break;
        }
    }
#else
    while (i < argc)
    {
        if (VmDirIsCmdLineOption(argv[i]) != FALSE)
        {
            if (VmDirStringCompareA(VDCREPADMIN_OPTION_SOURCE_HOSTNAME,
                                    argv[i],
                                    TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszSrcHostName);
            }
            else if (VmDirStringCompareA(VDCREPADMIN_OPTION_SOURCE_PORT,
                                         argv[i],
                                         TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszSrcPort);
            }
            else if (VmDirStringCompareA(VDCREPADMIN_OPTION_SOURCE_USERNAME,
                                         argv[i],
                                         TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszSrcUserName);
            }
            else if (VmDirStringCompareA(VDCREPADMIN_OPTION_SOURCE_PASSWORD,
                                         argv[i],
                                         TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszSrcPassword);
            }
            else if (VmDirStringCompareA(VDCREPADMIN_OPTION_TARGET_HOSTNAME,
                                         argv[i],
                                         TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszTgtHostName);
            }
            else if (VmDirStringCompareA(VDCREPADMIN_OPTION_TARGET_PORT,
                                         argv[i],
                                         TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszTgtPort);
            }
            else if (VmDirStringCompareA(VDCREPADMIN_OPTION_VERBOSE,
                                         argv[i],
                                         TRUE) == 0)
            {
                bVerbose = TRUE;
            }
            else if (VmDirStringCompareA(VDCREPADMIN_OPTION_TWO_WAY_REPL,
                                         argv[i],
                                         TRUE) == 0)
            {
                bTwoWayRepl = TRUE;
            }
            else if (VmDirStringCompareA(VDCREPADMIN_OPTION_FEATURE_SET,
                                         argv[i],
                                         TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszFeatureSet);
            }
            else if (VmDirStringCompareA(VDCREPADMIN_OPTION_ENTRY_DN,
                                         argv[i],
                                         TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszEntryDn);
            }
            else if (VmDirStringCompareA(VDCREPADMIN_OPTION_ATTRIBUTE,
                                         argv[i],
                                         TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszAttribute);
            }

        }
        i++;
    }
#endif

    if ( pszFeatureSet  == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_QUERY_IS_FIRST_CYCLE_DONE,
                                  pszFeatureSet,
                                  TRUE) == 0 )
    {
        if (
               pszSrcHostName == NULL
            || pszSrcPort     == NULL
            || pszSrcUserName == NULL
           )
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_SHOW_PARTNERS,
                                  pszFeatureSet,
                                  TRUE) == 0 )
    {
        if (
               pszSrcHostName == NULL
            || pszSrcPort     == NULL
            || pszSrcUserName == NULL
           )
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_SHOW_PARTNER_STATUS,
                                  pszFeatureSet,
                                  TRUE) == 0 )
    {
        if (
               pszSrcHostName == NULL
            || pszSrcPort     == NULL
            || pszSrcUserName == NULL
           )
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_SHOW_FEDERATION_STATUS,
                                  pszFeatureSet,
                                  TRUE) == 0 )
    {
        if (
               pszSrcHostName == NULL
            || pszSrcPort     == NULL
            || pszSrcUserName == NULL
           )
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_SHOW_SERVER_ATTRIBUTE,
                                  pszFeatureSet,
                                  TRUE) == 0 )
    {
        if (
               pszSrcHostName == NULL
            || pszSrcPort     == NULL
            || pszSrcUserName == NULL
            )
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_CREATE_AGREEMENT,
                                  pszFeatureSet,
                                  TRUE) == 0 )
    {
        if (
               pszSrcHostName == NULL
            || pszSrcPort     == NULL
            || pszSrcUserName == NULL
            || pszTgtHostName == NULL
            || pszTgtPort     == NULL
           )
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_REMOVE_AGREEMENT,
                                  pszFeatureSet,
                                  TRUE) == 0 )
    {
        if (
               pszSrcHostName == NULL
            || pszSrcPort     == NULL
            || pszSrcUserName == NULL
            || pszTgtHostName == NULL
            || pszTgtPort     == NULL
           )
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_DUMMY_DOMAIN_WRITE,
                                  pszFeatureSet,
                                  TRUE) == 0 )
    {
        if (
               pszSrcHostName == NULL
            || pszSrcPort     == NULL
            || pszSrcUserName == NULL
           )
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_SHOW_ATTRIBUTE_METADATA,
                                  pszFeatureSet,
                                  TRUE) == 0 )
    {
        if (
               pszSrcHostName == NULL
            || pszSrcPort     == NULL
            || pszSrcUserName == NULL
            || pszEntryDn     == NULL
           )
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszFeatureSet  = pszFeatureSet;
    *ppszSrcHostName = pszSrcHostName;
    *ppszSrcPort     = pszSrcPort;
    *ppszSrcUserName = pszSrcUserName;
    *ppszSrcPassword = pszSrcPassword;
    *ppszTgtHostName = pszTgtHostName;
    *ppszTgtPort     = pszTgtPort;
    *ppszEntryDn     = pszEntryDn;
    *ppszAttribute   = pszAttribute;
    *pbVerbose       = bVerbose;
    *pbTwoWayRepl    = bTwoWayRepl;

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
ShowUsage(
    VOID
    )
{
    printf(
        "Usage: vdcrepadmin -f showpartners\n"
        "                   -h <source_hostname> [-p <source_portnumber>]\n"
        "                   -u <source_username> [-w <source_password>]\n"
        "       vdcrepadmin -f showpartnerstatus\n"
        "                   -h <source_hostname> [-p <source_portnumber>]\n"
        "                   -u <source_username> [-w <source_password>]\n"
        "       vdcrepadmin -f showfederationstatus\n"
        "                   -h <source_hostname> [-p <source_portnumber>]\n"
        "                   -u <source_username> [-w <source_password>]\n"
        "       vdcrepadmin -f showservers\n"
        "                   -h <source_hostname> [-p <source_portnumber>]\n"
        "                   -u <source_username> [-w <source_password>]\n"
        "       vdcrepadmin -f createagreement [-2]\n"
        "                   -h <source_hostname> [-p <source_portnumber>]\n"
        "                   -u <source_username> [-w <source_password>]\n"
        "                   -H <target_hostname> [-P <target_portnumber>]\n"
        "       Note: if create only one-way replication agreement (source->target),\n"
        "             the entry is created on the target.\n"
        "       vdcrepadmin -f removeagreement [-2]\n"
        "                   -h <source_hostname> [-p <source_portnumber>]\n"
        "                   -u <source_username> [-w <source_password>]\n"
        "                   -H <target_hostname> [-P <target_portnumber>]\n"
        "       Note: if remove only one-way replication agreement (source->target),\n"
        "             the entry is removed from the target.\n"
        "       vdcrepadmin -f isfirstcycledone\n"
        "                   -h <source_hostname> [-p <source_portnumber>]\n"
        "                   -u <source_username> [-w <source_password>]\n"
        "       vdcrepadmin -f dummydomainwrite\n"
        "                   -h <fully-qualified-domain-name>\n"
        "                   -u <admin UPN, e.g. Administrator@vsphere.local>\n"
        "                   [-w <password>]\n"
        "       vdcrepadmin -f showattributemetadata\n"
        "                   -e <entry_dn> [-a <attribute>]\n"
        "                   -h <source_hostname> [-p <source_portnumber>]\n"
        "                   -u <source_username> [-w <source_password>]\n"
      );
}
