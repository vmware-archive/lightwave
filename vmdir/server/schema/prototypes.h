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
 * Module Name: Directory Schema
 *
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * Function prototypes
 *
 */

#ifndef _PROTOTYPES_H_
#define _PROTOTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

// api.c
DWORD
VdirSchemaCtxAcquireInLock(
    BOOLEAN    bHasLock,    // TRUE if owns gVdirSchemaGlobals.mutex already
    PVDIR_SCHEMA_CTX* ppSchemaCtx
    );

// attr2idmap.c
DWORD
VdirSchemaAttrToIdMapInit(
    PVDIR_SCHEMA_INSTANCE    pSchema,
    PVDIR_ENTRY              pEntry
    );

// check.c
DWORD
VmDirSchemaGetEntryStructureOCDesc(
    PVDIR_ENTRY             pEntry,
    PVDIR_SCHEMA_OC_DESC*   ppStructureOCDesc   // caller does not own *ppStructureOCDesc
    );

// init.c
DWORD
VdirSchemaInstanceAllocate(
    PVDIR_SCHEMA_INSTANCE* ppSchema,
    USHORT   dwATSize,
    USHORT   dwOCSize,
    USHORT   dwContentSize,
    USHORT   dwStructureSize,
    USHORT   dwNameformSize
    );

VOID
VdirSchemaInstanceFree(
    PVDIR_SCHEMA_INSTANCE pSchema
    );

DWORD
UnitTestSchemaInstanceInit(
    PSTR*    ppszDescs,
    DWORD    dwDescsSize,
    PVDIR_SCHEMA_INSTANCE*    ppSchema
    );

DWORD
VdirSchemaInstanceInitViaEntry(
    PVDIR_ENTRY                  pEntry,
    PVDIR_SCHEMA_INSTANCE*  ppSchema
    );

DWORD
VmDirSchemaInitalizeFileToEntry(
    PCSTR           pszSchemaFilePath,
    PVDIR_ENTRY*    ppEntry
    );

// load.c
DWORD
VmDirSchemaLoadInstance(
    PVDIR_SCHEMA_INSTANCE    pSchema
    );

int
VdirSchemaPCRNameCmp(
    const void *p1,
    const void *p2
    );

int
VdirSchemaPATNameCmp(
    const void *p1,
    const void *p2
    );

int
VdirSchemaPOCNameCmp(
    const void *p1,
    const void *p2
    );

/*  defined in schema.h already
DWORD
VmDirSchemaNameToOCDesc(
    PVDIR_SCHEMA_OC_COLLECTION  pOCCollection,
    PCSTR                       pszOCName,
    PVDIR_SCHEMA_OC_DESC*       ppOCDesc
    );

DWORD
VmDirSchemaNameToATDesc(
    PVDIR_SCHEMA_AT_COLLECTION  pATCollection,
    PCSTR                       pszATName,
    PVDIR_SCHEMA_AT_DESC*       ppATDesc
    );
*/

DWORD
VmDirSchemaCRNameToDescriptor(
    PVDIR_SCHEMA_CTX        pCtx,
    PCSTR                   pszName,
    PVDIR_SCHEMA_CR_DESC*   ppCRDesc
    );

// matchingrule.c
DWORD
VdirMatchingRuleLoad(
    VOID
    );

PVDIR_MATCHING_RULE_DESC
VdirMatchingRuleLookupByName(
    PCSTR    pszName
    );

DWORD
VdirMatchingRuleGetDefinition(
    PSTR**    pppszOutStr,
    USHORT*   pdwSize
    );

// parse.c
VOID
VmDirSchemaATDescContentFree(
    PVDIR_SCHEMA_AT_DESC pATDesc
    );

VOID
VmDirSchemaOCDescContentFree(
    PVDIR_SCHEMA_OC_DESC pOCDesc
    );

VOID
VmDirSchemaContentDescContentFree(
    PVDIR_SCHEMA_CR_DESC pContentDesc
    );

VOID
VmDirSchemaStructureDescContentFree(
    PVDIR_SCHEMA_SR_DESC pStructureDesc
    );

VOID
VmDirSchemaNameformDescContentFree(
    PVDIR_SCHEMA_NF_DESC pNameformDesc
    );

DWORD
VmDirSchemaParseStrToATDesc(
    const char* pszStr,
    PVDIR_SCHEMA_AT_DESC pATDesc
    );

DWORD
VmDirSchemaParseStrToOCDesc(
    const char*             pszStr,
    PVDIR_SCHEMA_OC_DESC    pOCDesc
    );

DWORD
VmDirSchemaParseStrToContentDesc(
    const char*             pStr,
    PVDIR_SCHEMA_CR_DESC    pConetentDesc
    );

DWORD
VmDirSchemaParseStrToStructureDesc(
    const char*            pStr,
    PVDIR_SCHEMA_SR_DESC   pStructureDesc
    );

DWORD
VmDirSchemaParseStrToNameformDesc(
    const char*                 pStr,
    PVDIR_SCHEMA_NF_DESC  pNameformDesc
    );

// syntax.c
DWORD
VdirSyntaxLoad(
    VOID
    );

PVDIR_SYNTAX_DESC
VdirSyntaxLookupByOid(
    PCSTR    pszOid
    );

DWORD
VdirSyntaxGetDefinition(
    PSTR**    pppszOutStr,
    USHORT*   pdwSize
    );

BOOLEAN
syntaxOID(
    PVDIR_BERVALUE pBerv
    );

// verify.c
BOOLEAN
VmDirSchemaVerifyIntegrity(
    PVDIR_SCHEMA_INSTANCE    pSchema
    );

DWORD
VmDirSchemaInstancePatchCheck(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PVDIR_SCHEMA_INSTANCE   pInstance,
    PBOOLEAN                pbCompatible,
    PBOOLEAN                pbNeedPatch
    );

VOID
VdirSchemaVerifyATDescPrint(
    PVDIR_SCHEMA_AT_DESC pATDesc,
    PSTR*                ppszOut
    );

VOID
VdirSchemaVerifyOCDescPrint(
    PVDIR_SCHEMA_OC_DESC pOCDesc,
    PSTR*                ppszOut
    );

// adcompatibleschema.c
DWORD
VdirSchemaADCompatibleSetup(
    PVDIR_SCHEMA_INSTANCE   pSchema
    );

#ifdef __cplusplus
}
#endif

#endif // _PROTOTYPES_H_

