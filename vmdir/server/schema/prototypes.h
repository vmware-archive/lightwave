/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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

DWORD
VmDirSchemaCRNameToDescriptor(
    PVDIR_SCHEMA_CTX        pCtx,
    PCSTR                   pszName,
    PVDIR_SCHEMA_CR_DESC*   ppCRDesc
    );

BOOLEAN
VmDirSchemaIsAncestorOC(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_OC_DESC    pOCDesc,
    PVDIR_SCHEMA_OC_DESC    pAncestorOCDesc
    );

// check.c
DWORD
VmDirSchemaGetEntryStructureOCDesc(
    PVDIR_ENTRY             pEntry,
    PVDIR_SCHEMA_OC_DESC*   ppStructureOCDesc   // caller does not own *ppStructureOCDesc
    );

// idmap.c
DWORD
VmDirSchemaAttrIdMapInit(
    PVDIR_SCHEMA_ATTR_ID_MAP*   ppAttrIdMap
    );

DWORD
VmDirSchemaAttrIdMapReadDB(
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap
    );

DWORD
VmDirSchemaAttrIdMapUpdateDB(
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap
    );

DWORD
VmDirSchemaAttrIdMapGetAttrId(
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap,
    PSTR                        pszAttrName,
    USHORT*                     pusAttrId
    );

DWORD
VmDirSchemaAttrIdMapAddNewAttr(
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap,
    PSTR                        pszAttrName,
    USHORT                      usAttrId    // optional
    );

VOID
VmDirSchemaAttrIdMapRemoveAllPending(
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap
    );

VOID
VmDirFreeSchemaAttrIdMap(
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap
    );

// instance.c
DWORD
VmDirSchemaInstanceCreate(
    PVDIR_LDAP_SCHEMA           pRepository,
    PVDIR_SCHEMA_INSTANCE*      ppInstance
    );

DWORD
VmDirSchemaInstanceGetATDescByName(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PCSTR                   pszName,
    PVDIR_SCHEMA_AT_DESC*   ppATDesc
    );

DWORD
VmDirSchemaInstanceGetATDescById(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    USHORT                  usId,
    PVDIR_SCHEMA_AT_DESC*   ppATDesc
    );

DWORD
VmDirSchemaInstanceGetOCDescByName(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PCSTR                   pszName,
    PVDIR_SCHEMA_OC_DESC*   ppOCDesc
    );

DWORD
VmDirSchemaInstanceGetCRDescByName(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PCSTR                   pszName,
    PVDIR_SCHEMA_CR_DESC*   ppCRDesc
    );

DWORD
VmDirSchemaInstanceGetSRDescById(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PCSTR                   pszId,
    PVDIR_SCHEMA_SR_DESC*   ppSRDesc
    );

DWORD
VmDirSchemaInstanceGetNFDescByName(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PCSTR                   pszName,
    PVDIR_SCHEMA_NF_DESC*   ppNFDesc
    );

VOID
VmDirFreeSchemaInstance(
    PVDIR_SCHEMA_INSTANCE pSchema
    );

// libmain.c
DWORD
VmDirSchemaLibLoadBootstrapTable(
    VDIR_SCHEMA_BOOTSTRAP_TABLE bootstrapTable[]
    );

// matchingrule.c
DWORD
VdirMatchingRuleLoad(
    VOID
    );

PVDIR_MATCHING_RULE_DESC
VdirEqualityMRLookupBySyntaxOid(
    PCSTR    pszSyntaxOid
    );

PVDIR_MATCHING_RULE_DESC
VdirOrderingMRLookupBySyntaxOid(
    PCSTR    pszSyntaxOid
    );

PVDIR_MATCHING_RULE_DESC
VdirSubstrMRLookupBySyntaxOid(
    PCSTR    pszSyntaxOid
    );

// parse.c
DWORD
VmDirSchemaATDescCreate(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pLdapAt,
    PVDIR_SCHEMA_AT_DESC*       ppATDesc
    );

DWORD
VmDirSchemaOCDescCreate(
    PVDIR_LDAP_OBJECT_CLASS pLdapOc,
    PVDIR_SCHEMA_OC_DESC*   ppOCDesc
    );

DWORD
VmDirSchemaCRDescCreate(
    PVDIR_LDAP_CONTENT_RULE pLdapCr,
    PVDIR_SCHEMA_CR_DESC*   ppCRDesc
    );

DWORD
VmDirSchemaSRDescCreate(
    PVDIR_LDAP_STRUCTURE_RULE   pLdapSr,
    PVDIR_SCHEMA_SR_DESC*       ppSRDesc
    );

DWORD
VmDirSchemaNFDescCreate(
    PVDIR_LDAP_NAME_FORM    pLdapNf,
    PVDIR_SCHEMA_NF_DESC*   ppNFDesc
    );

DWORD
VmDirLdapAtParseVdirEntry(
    PVDIR_ENTRY                 pEntry,
    PVDIR_LDAP_ATTRIBUTE_TYPE*  ppAt
    );

DWORD
VmDirLdapOcParseVdirEntry(
    PVDIR_ENTRY                 pEntry,
    PVDIR_LDAP_OBJECT_CLASS*    ppOc
    );

DWORD
VmDirLdapCrParseVdirEntry(
    PVDIR_ENTRY                 pEntry,
    PVDIR_LDAP_CONTENT_RULE*    ppCr
    );

// replstatus.c
DWORD
VmDirSchemaReplStatusGlobalsInit(
    VOID
    );

VOID
VmDirSchemaReplStatusGlobalsShutdown(
    VOID
    );

DWORD
VmDirSchemaReplStatusEntriesInit(
    VOID
    );

VOID
VmDirSchemaReplStatusEntriesClear(
    VOID
    );

DWORD
VmDirSchemaReplStatusEntriesRefreshThread(
    PVOID   pArg
    );

DWORD
VmDirSchemaReplStateCreateFromReplAgr(
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr,
    PVDIR_SCHEMA_REPL_STATE*        ppReplStatus
    );

// syntax.c
DWORD
VdirSyntaxLoad(
    VOID
    );

DWORD
VdirSyntaxLookupByOid(
    PCSTR               pszOid,
    PVDIR_SYNTAX_DESC*  ppSyntax
    );

DWORD
VdirSyntaxGetDefinition(
    PSTR**    pppszOutStr,
    USHORT*   pusSize
    );

BOOLEAN
syntaxOID(
    PVDIR_BERVALUE pBerv
    );

// legacy/legacyload.c
DWORD
VmDirSchemaAttrIdMapLoadSubSchemaSubEntry(
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap,
    PVDIR_ENTRY                 pSchemaEntry
    );

DWORD
VmDirLdapSchemaLoadSubSchemaSubEntry(
    PVDIR_LDAP_SCHEMA   pLdapSchema,
    PVDIR_ENTRY         pSchemaEntry
    );

DWORD
VmDirLegacySchemaLoadSubSchemaSubEntry(
    PVDIR_LEGACY_SCHEMA pLegacySchema,
    PVDIR_ENTRY         pSchemaEntry
    );

// legacy/legacyutil.c
DWORD
VmDirUpdateSubSchemaSubEntry(
    PVDIR_LEGACY_SCHEMA_MOD pLegacySchemaMod
    );

#ifdef __cplusplus
}
#endif

#endif // _PROTOTYPES_H_

