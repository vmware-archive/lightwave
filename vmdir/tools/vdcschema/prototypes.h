/*
 * Copyright © 2016-2017 VMware, Inc.  All Rights Reserved.
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

// conn.c
DWORD
VdcSchemaConnInit(
    PVDC_SCHEMA_CONN*   ppConn
    );

DWORD
VdcSchemaConnValidateAndSetDefault(
    PVDC_SCHEMA_CONN    pConn
    );

DWORD
VdcSchemaConnOpen(
    PVDC_SCHEMA_CONN    pConn
    );

VOID
VdcSchemaFreeConn(
    PVDC_SCHEMA_CONN    pConn
    );

// operations.c
DWORD
VdcSchemaOpParamInit(
    PVDC_SCHEMA_OP_PARAM*   ppOpParam
    );

DWORD
VdcSchemaOpParamValidate(
    PVDC_SCHEMA_OP_PARAM    pOpParam
    );

DWORD
VdcSchemaOpGetSupportedSyntaxes(
    PVDC_SCHEMA_CONN        pConn,
    PVDC_SCHEMA_OP_PARAM    pOpParam
    );

DWORD
VdcSchemaOpPatchSchemaDefs(
    PVDC_SCHEMA_CONN        pConn,
    PVDC_SCHEMA_OP_PARAM    pOpParam
    );

DWORD
VdcSchemaOpGetSchemaReplStatus(
    PVDC_SCHEMA_CONN        pConn,
    PVDC_SCHEMA_OP_PARAM    pOpParam
    );

VOID
VdcSchemaFreeOpParam(
    PVDC_SCHEMA_OP_PARAM    pOpParam
    );

// parseargs.c
DWORD
VdcSchemaParseArgs(
    int                     argc,
    char*                   argv[],
    PVDC_SCHEMA_CONN*       ppConn,
    PVDC_SCHEMA_OP_PARAM*   ppOpParam
    );

VOID
VdcSchemaShowUsage(
    VOID
    );

// syntax.c
DWORD
VdcSchemaGetSyntaxMap(
    PVDC_SCHEMA_CONN    pConn,
    PLW_HASHMAP*        ppSyntaxMap
    );

DWORD
VmDirSchemaValidateSyntaxes(
    PVDIR_LDAP_SCHEMA       pSchema,
    PVDIR_LDAP_SCHEMA_DIFF  pSchemaDiff,
    PVDC_SCHEMA_CONN        pConn
    );

// util.c
DWORD
VdcSchemaReadPassword(
    PSTR*   ppszPassword
    );

VOID
VmDirSchemaPrintSyntaxMap(
    PLW_HASHMAP pSyntaxMap
    );

VOID
VmDirSchemaPrintDiff(
    PVDIR_LDAP_SCHEMA_DIFF  pSchemaDiff
    );

DWORD
VdcSchemaRefreshSchemaReplStatusEntries(
    PVDC_SCHEMA_CONN    pConn
    );

DWORD
VdcSchemaWaitForSchemaReplStatusEntries(
    PVDC_SCHEMA_CONN        pConn,
    PVDC_SCHEMA_OP_PARAM    pOpParam
    );

DWORD
VdcSchemaGetSchemaReplStatusEntries(
    PVDC_SCHEMA_CONN            pConn,
    PVDIR_SCHEMA_REPL_STATE**   pppReplStates
    );

DWORD
VdcSchemaPrintSchemaReplStatusEntry(
    PVDIR_SCHEMA_REPL_STATE pReplState
    );
