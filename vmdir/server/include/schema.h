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
 * Filename: vdirschema.h
 *
 * Abstract:
 *
 * schema api
 *
 */

#ifndef __VIDRSCHEMA_H__
#define __VIDRSCHEMA_H__

#include <ldap_schema.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    VDIR_SCHEMA_MATCH_EQUAL = 0,
    VDIR_SCHEMA_MATCH_GE,
    VDIR_SCHEMA_MATCH_LE,
    VDIR_SCHEMA_MATCH_SUBSTR_INIT,
    VDIR_SCHEMA_MATCH_SUBSTR_ANY,
    VDIR_SCHEMA_MATCH_SUBSTR_FINAL

} VDIR_SCHEMA_MATCH_TYPE;

typedef struct _VDIR_MATCHING_RULE_DESC*    PVDIR_MATCHING_RULE_DESC;
typedef struct _VDIR_SYNTAX_DESC*           PVDIR_SYNTAX_DESC;

typedef struct _VDIR_SCHEMA_AT_DESC
{
    PVDIR_LDAP_ATTRIBUTE_TYPE   pLdapAt;

    USHORT      usAttrID;     // internal id stored in database

    PSTR        pszName;
    PSTR        pszOid;
    PSTR        pszSyntaxOid;

    BOOLEAN     bSingleValue;
    BOOLEAN     bCollective;
    BOOLEAN     bNoUserModifiable;
    BOOLEAN     bObsolete;

    VDIR_LDAP_ATTRIBUTE_TYPE_USAGE  usage;

    // index configuration attributes
    DWORD       dwSearchFlags;
    PSTR*       ppszUniqueScopes;

    PVDIR_SYNTAX_DESC           pSyntax;
    PVDIR_MATCHING_RULE_DESC    pEqualityMR;
    PVDIR_MATCHING_RULE_DESC    pOrderingMR;
    PVDIR_MATCHING_RULE_DESC    pSubStringMR;

} VDIR_SCHEMA_AT_DESC;

typedef struct _VDIR_SCHEMA_OC_DESC
{
    PVDIR_LDAP_OBJECT_CLASS pLdapOc;

    PSTR        pszName;
    PSTR        pszOid;
    PSTR        pszSup;
    PSTR*       ppszMustATs;    // ends with NULL PSTR
    PSTR*       ppszMayATs;     // ends with NULL PSTR

    BOOLEAN     bObsolete;

    VDIR_LDAP_OBJECT_CLASS_TYPE type;

} VDIR_SCHEMA_OC_DESC, *PVDIR_SCHEMA_OC_DESC;

typedef DWORD (*PFN_VDIR_NORMALIZE_FUNCTION)(PVDIR_BERVALUE pBerv);

typedef BOOLEAN (*PFN_VDIR_COMPARE_FUNCTION)(VDIR_SCHEMA_MATCH_TYPE type, PVDIR_BERVALUE pBerv1, PVDIR_BERVALUE pBerv2);

typedef struct _VDIR_MATCHING_RULE_DESC
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    PSTR                        pszSyntaxOid;
    PVDIR_SYNTAX_DESC           pSyntax;
    PFN_VDIR_NORMALIZE_FUNCTION pNormalizeFunc;
    PFN_VDIR_COMPARE_FUNCTION   pCompareFunc;

} VDIR_MATCHING_RULE_DESC;

///////////////////////////////////////////////////////////////////////////////
// Schema library initialize / update / shutdown
///////////////////////////////////////////////////////////////////////////////
/*
 * Initializes schema library
 */
DWORD
VmDirSchemaLibInit(
    PVMDIR_MUTEX*   ppModMutex
    );

/*
 * Reads schema file and loads definitions into schema library.
 * The function will fail if the definitions from the file are
 * not compatible with the current definitions in the library.
 *
 * Should be called once during server startup / schema patch.
 *
 * New schema changes from this function is not effective until
 * VmDirSchemaLibUpdate() is called.
 */
DWORD
VmDirSchemaLibPrepareUpdateViaFile(
    PCSTR   pszSchemaFilePath
    );

/*
 * Reads schema objects entries from data store and loads them
 * into schema library.
 *
 * Should be called once during server startup.
 *
 * New schema changes from this function is not effective until
 * VmDirSchemaLibUpdate() is called.
 */
DWORD
VmDirSchemaLibPrepareUpdateViaEntries(
    PVDIR_ENTRY_ARRAY   pAtEntries,
    PVDIR_ENTRY_ARRAY   pOcEntries
    );

/*
 * Before an operation modifies an schema object, makes sure
 * the change is valid.
 *
 * Should be called when pOperation is on a schema object.
 *
 * Not needed to be called if pOperation is internal.
 * Schema objects are only modified internally during bootstrap/patch,
 * and the cache is already complete at this stage of bootstrap/patch.
 *
 * New schema changes from this function is not effective until
 * VmDirSchemaLibUpdate() is called.
 */
DWORD
VmDirSchemaLibPrepareUpdateViaModify(
    PVDIR_OPERATION      pOperation,
    PVDIR_ENTRY          pSchemaEntry
    );

/*
 * Will update live schema context with prepared updates from
 * above three functions.
 */
DWORD
VmDirSchemaLibUpdate(
    DWORD   dwPriorResult
    );

/*
 * If schema is modified concurrently, use the following mutex functions
 * to protect schema from race condition.
 */
DWORD
VmDirSchemaModMutexAcquire(
    PVDIR_OPERATION pOperation
    );

DWORD
VmDirSchemaModMutexRelease(
    PVDIR_OPERATION pOperation
    );

/*
 * Shutdown schema library
 */
void
VmDirSchemaLibShutdown(
    void
    );

///////////////////////////////////////////////////////////////////////////////
// Schema storage read and write
///////////////////////////////////////////////////////////////////////////////
DWORD
VmDirReadSchemaObjects(
    PVDIR_ENTRY_ARRAY*  ppAtEntries,
    PVDIR_ENTRY_ARRAY*  ppOcEntries
    );

DWORD
VmDirPatchLocalSchemaObjects(
    PVDIR_SCHEMA_CTX    pOldCtx,
    PVDIR_SCHEMA_CTX    pNewCtx
    );

DWORD
VmDirWriteSchemaObjects(
    VOID
    );

///////////////////////////////////////////////////////////////////////////////
// Schema head query
///////////////////////////////////////////////////////////////////////////////
/*
 * Build subschema subentry from schema cache
 */
DWORD
VmDirSubSchemaSubEntry(
    PVDIR_ENTRY*    ppEntry
    );

///////////////////////////////////////////////////////////////////////////////
// Schema legacy support for 6.0u3 and 6.5
///////////////////////////////////////////////////////////////////////////////
/*
 * Auxiliary function to tune schema library compatible with legacy data
 */
DWORD
VmDirSchemaLibInitLegacy(
    VOID
    );

/*
 * Reads subschema subentry from legacy data store and loads it
 * into schema library.
 *
 * Should be called once and only once in the node's lifetime when
 * 1) join to legacy partner.
 * 2) upgrade a legacy node.
 *
 * New schema changes from this function is not effective until
 * VmDirSchemaLibUpdate() is called.
 */
DWORD
VmDirSchemaLibPrepareUpdateViaSubSchemaSubEntry(
    PVDIR_ENTRY pSchemaEntry
    );

DWORD
VmDirReadSubSchemaSubEntry(
    PVDIR_ENTRY*    ppSubSchemaSubEntry
    );

DWORD
VmDirPatchLocalSubSchemaSubEntry(
    VOID
    );

///////////////////////////////////////////////////////////////////////////////
// Schema context
///////////////////////////////////////////////////////////////////////////////
/*
 * Caller acquire schema context
 */
DWORD
VmDirSchemaCtxAcquire(
    PVDIR_SCHEMA_CTX* ppSchemaCtx
    );

PVDIR_SCHEMA_CTX
VmDirSchemaCtxClone(
    PVDIR_SCHEMA_CTX    pOrgCtx
    );

/*
 * Caller release schema context
 */
void
VmDirSchemaCtxRelease(
    PVDIR_SCHEMA_CTX
    );

/*
 * Get context error message.
 */
PCSTR
VmDirSchemaCtxGetErrorMsg(
    PVDIR_SCHEMA_CTX
    );

/*
 * Get context error code.
 */
DWORD
VmDirSchemaCtxGetErrorCode(
    PVDIR_SCHEMA_CTX
    );

BOOLEAN
VmDirSchemaCtxIsBootStrap(
    PVDIR_SCHEMA_CTX    pCtx
    );

///////////////////////////////////////////////////////////////////////////////
// Schema lookup
///////////////////////////////////////////////////////////////////////////////

DWORD
VmDirSchemaAttrNameToDescriptor(
    PVDIR_SCHEMA_CTX        pCtx,
    PCSTR                   pszName,
    PVDIR_SCHEMA_AT_DESC*   ppATDesc
    );

DWORD
VmDirSchemaOCNameToDescriptor(
    PVDIR_SCHEMA_CTX        pCtx,
    PCSTR                   pszName,
    PVDIR_SCHEMA_OC_DESC*   ppOCDesc
    );

PVDIR_SCHEMA_AT_DESC
VmDirSchemaAttrNameToDesc(
    PVDIR_SCHEMA_CTX    pCtx,
    PCSTR               pszName
    );

PVDIR_SCHEMA_AT_DESC
VmDirSchemaAttrIdToDesc(
    PVDIR_SCHEMA_CTX    pCtx,
    USHORT              usId
    );

USHORT
VmDirSchemaAttrNameToId(
    PVDIR_SCHEMA_CTX    pCtx,
    PCSTR               pszName
    );

PCSTR
VmDirSchemaAttrIdToName(
    PVDIR_SCHEMA_CTX    pCtx,
    USHORT              usId
    );

DWORD
VmDirSchemaAttrList(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_AT_DESC**  pppATDescList
    );

DWORD
VmDirSchemaClassGetAllMayAttrs(
    PVDIR_SCHEMA_CTX        pCtx,           // IN
    PVDIR_SCHEMA_OC_DESC    pOCDesc,        // IN
    PLW_HASHMAP             pAllMayAttrMap  // IN
    );

DWORD
VmDirSchemaClassGetAllMustAttrs(
    PVDIR_SCHEMA_CTX        pCtx,           // IN
    PVDIR_SCHEMA_OC_DESC    pOCDesc,        // IN
    PLW_HASHMAP             pAllMustAttrMap // IN
    );

BOOLEAN
VmDirSchemaSyntaxIsNumeric(
    PCSTR   pszSyntaxOid
    );

BOOLEAN
VmDirSchemaAttrIsNumeric(
    PVDIR_SCHEMA_AT_DESC    pATDesc
    );

BOOLEAN
VmDirSchemaAttrIsOctetString(
    PVDIR_SCHEMA_AT_DESC    pATDesc
    );

PVDIR_ENTRY
VmDirSchemaAcquireAndOwnStartupEntry(
    VOID
    );

// check if pszName is a structure objectclass name
BOOLEAN
VmDirSchemaIsStructureOC(
    PVDIR_SCHEMA_CTX    pCtx,
    PCSTR               pszName
    );

// check if pszName == entry strcuture objectclass name
BOOLEAN
VmDirSchemaIsNameEntryLeafStructureOC(
    PVDIR_ENTRY     pEntry,
    PCSTR           pszName
    );

///////////////////////////////////////////////////////////////////////////////
// Schema enforcement
///////////////////////////////////////////////////////////////////////////////
/*
 * Entry schema check
 */
DWORD
VmDirSchemaCheck(
    PVDIR_ENTRY     pEntry
    );

/*
 * Entry schema structure rule check
 */
DWORD
VmDirSchemaCheckDITStructure(
    PVDIR_SCHEMA_CTX    pCtx,
    PVDIR_ENTRY         pParentEntry,
    PVDIR_ENTRY         pEntry
    );

/*
 * Berval syntax check and optionally value normalization
 */
DWORD
VmDirSchemaBervalSyntaxCheck(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_AT_DESC    pATDesc,
    PVDIR_BERVALUE          pBerv
    );

/*
 * Berval value normalization
 * pBerv->bvnorm_val and len will be set if normalize yield different value;
 * otherwise, pBerv->bvnorm_val = pBerv->bv_val;
 *            pBerv->bvnorm_len = pBerv->bv_len;
 */
DWORD
VmDirSchemaBervalNormalize(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_AT_DESC    pATDesc,
    PVDIR_BERVALUE          pBerv
    );

DWORD
VmDirNormalizeDNWrapper(
    PVDIR_BERVALUE      pBerv
    );

DWORD
VmDirNormalizeDN(
    PVDIR_BERVALUE      pBerv,
    PVDIR_SCHEMA_CTX    pSchemaCtx
    );

/*
 * Berval value comparison
 */
BOOLEAN
VmDirSchemaBervalCompare(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_AT_DESC    pATDesc,
    VDIR_SCHEMA_MATCH_TYPE  matchType,
    PVDIR_BERVALUE          pNormBervAssert,
    PVDIR_BERVALUE          pNormBerv
    );

/*
 * Set Attribute.pATDesc
 */
DWORD
VmDirSchemaCheckSetAttrDesc(
    PVDIR_SCHEMA_CTX    pCtx,
    PVDIR_ENTRY         pEntry
    );

/*
 * Check if ctx contains live/global schema instance
 */
BOOLEAN
VmDirIsLiveSchemaCtx(
    PVDIR_SCHEMA_CTX    pCtx
    );

DWORD
VmDirSchemaGetEntryStructureOCDesc(
    PVDIR_ENTRY             pEntry,
    PVDIR_SCHEMA_OC_DESC*   ppStructureOCDesc       // caller does not own *ppStructureOCDesc
    );

#ifdef __cplusplus
}
#endif

#endif /* __VIDRSCHEMA_H__ */
