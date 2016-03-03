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
 *
 * Module   : prototypes.h
 *
 * Abstract :
 *
 *            VMware Directory Service
 *
 *            vdcschemaadmin
 *
 */

/*prototypes.h*/

DWORD
VmDirParseArgs(
    int      argc,
    char*    argv[],
    PSTR*    ppszUPN,
    PSTR*    ppszPartnerHostName,
    PSTR*    ppszVersionHostName,
    PSTR*    ppszUpgradeHostName,
    PSTR*    ppszSchemaFile,
    BOOLEAN* pDryRun,
    PSTR*    ppszPartnerCurrPassword
    );

VOID
ShowUsage(
    VOID
    );


DWORD
VMDirCheckSchemaEquality (
    PSTR     pszBaseHostName ,
    PSTR     pszUPN ,
    PSTR     pszPassword
    );

VOID
VmDirPrintSchemaDiff (
    PVMDIR_SCHEMA_DIFF  pSchemaDiff ,
    DWORD               dwInfoCount
    );

VOID
PrintDownHosts(
    PVMDIR_SCHEMA_DIFF   pSchemaDiff,
    DWORD                dwInfoCount
    );

