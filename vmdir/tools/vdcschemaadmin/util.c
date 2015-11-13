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
 * Module Name: vdcschemaadmin
 *
 * Filename: util.c
 *
 * Abstract:
 *
 *
 * vdcschemaadmin util  functions
 *
 */


#include "includes.h"

DWORD
VMDirCheckSchemaEquality (
    PSTR     pszBaseHostName ,
    PSTR     pszUPN ,
    PSTR     pszPassword
    )
{
    DWORD                dwError      = 0;
    DWORD                dwInfoCount  = 0;
    PVMDIR_SCHEMA_DIFF   pSchemaDiff  = NULL ;

    if ( !pszBaseHostName ||  !pszUPN ||  !pszPassword )
    {
        dwError = ERROR_INVALID_PARAMETER ;
        BAIL_ON_VMDIR_ERROR (dwError) ;
    }


    dwError = VmDirCompareSchema(
            pszBaseHostName ,
            pszUPN ,
            pszPassword ,
            &pSchemaDiff ,
            &dwInfoCount
            );

    if (dwError == ERROR_SCHEMA_MISMATCH )
    {

        VmDirPrintSchemaDiff(
                pSchemaDiff ,
                dwInfoCount
                );

    }

    BAIL_ON_VMDIR_ERROR (dwError);
    VMDIR_LOG_INFO (VMDIR_LOG_MASK_ALL, "VMDirCheckSchemaEquality function execution completed succesfully ") ;

cleanup:
    if (pSchemaDiff != NULL )
    {
        VmDirSchemaDiffFree (pSchemaDiff, dwInfoCount);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VMDirCheckSchemaEquality failed. Error[%d] ",  dwError);
    goto cleanup;

}

VOID
VmDirPrintSchemaDiff (
    PVMDIR_SCHEMA_DIFF  pSchemaDiff ,
    DWORD               dwInfoCount
    )
{
    DWORD         i = 0 ;
    DWORD         j = 0 ;
    DWORD   dwError = 0 ;
    BOOLEAN bIsAllNodesOnline = TRUE;
    BOOLEAN bIsAllInSync = TRUE;

    if (pSchemaDiff == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER ;
        BAIL_ON_VMDIR_ERROR (dwError) ;
    }

    printf("\nSummary Report:\n\n");
    printf("Reference Host: %s\n", pSchemaDiff[0].pszBaseHostName);

    for (i=0; i< dwInfoCount ; i++ )
    {
        if (pSchemaDiff[i].bIsServerDown)
        {
            bIsAllNodesOnline = FALSE;
        }

        if ( pSchemaDiff[i].bIsMetadataVersionOutofSync ||
             (pSchemaDiff[i].dwBaseHostDiffCount != 0 || pSchemaDiff[i].dwPartnerHostDiffCount != 0)
           )
        {
            bIsAllInSync = FALSE;
        }

        printf("Partner   Host: %s is %s. Its metadata version is %s in sync.\n",
               pSchemaDiff[i].pszPartnerHostName,
               pSchemaDiff[i].bIsServerDown ? "down":"alive",
               pSchemaDiff[i].bIsMetadataVersionOutofSync ? " NOT ":"");

     }

    if ( bIsAllInSync == FALSE )
    {
        printf("\n\nWARNING, Schema in federation is out of sync.\n");
    }

    if ( bIsAllNodesOnline == FALSE )
    {
        printf("\n\nAt least one node in the federation is not reachable.  ");
        printf("Please start all nodes and rerun again.\n");
    }

    printf("\n\nDetail Report:\n\n");
    for (i=0; i< dwInfoCount ; i++ )
    {
        printf("\n\n------------------------------------------------------------------------------------------\n");
        printf("\n Diff between Base Host %s AND Partner Host %s \n",pSchemaDiff[i].pszBaseHostName, pSchemaDiff[i].pszPartnerHostName);

        if (pSchemaDiff[i].bIsServerDown == TRUE )
        {
            printf("\n Partner host %s is down. Can't compare schema \n", pSchemaDiff[i].pszPartnerHostName );
            continue ;
        }

        if (pSchemaDiff[i].dwBaseHostDiffCount == 0 )
        {
            printf("\n No Attributes Missing from Base Host \n");
        }
        else
        {
            printf("\n Base Host Missing following values \n ");
            for(j=0; j< pSchemaDiff[i].dwBaseHostDiffCount; j++ )
            {
                printf("\n %s \n", pSchemaDiff[i].baseHostDiffList[j]) ;
            }
        }

        if (pSchemaDiff[i].dwPartnerHostDiffCount == 0 )
        {
            printf(" \n No Attributes Missing from Partner Host \n");
        }
        else
        {
            printf("\n Partner Host Missing following values \n ");
            for(j=0; j< pSchemaDiff[i].dwPartnerHostDiffCount; j++ )
            {
                printf("\n %s \n", pSchemaDiff[i].partnerHostDiffList[j]) ;
            }
        }

        if (pSchemaDiff[i].bIsMetadataVersionOutofSync)
        {
            printf("\n WARNING !!!!! Metadata Version out of sync \n %s\n\n", pSchemaDiff[i].pszMetadataVerison);
        }

    }

    VMDIR_LOG_INFO (VMDIR_LOG_MASK_ALL, "VmDirPrintSchemaDiff function execution completed succesfully ") ;

cleanup:
    return ;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirPrintSchemaDiff failed. Error[%d] ",  dwError);
    goto cleanup;
}
