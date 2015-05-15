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

typedef enum
{
    VDIR_ATTRIBUTETYPE_USER_APPLICATIONS = 0,
    VDIR_ATTRIBUTETYPE_DIRECTORY_OPERATION,
    VDIR_ATTRIBUTETYPE_DISTRIBUTED_OPERATION,
    VDIR_ATTRIBUTETYPE_DSA_OPERATION,
    VDIR_ATTRIBUTETYPE_VMDIR_EXTENSION

} VDIR_ATTRIBUTETYPE_USAGE;

typedef struct _VDIR_MATCHING_RULE_DESC*    PVDIR_MATCHING_RULE_DESC;
typedef struct _VDIR_SYNTAX_DESC*           PVDIR_SYNTAX_DESC;

typedef struct _VDIR_SCHEMA_AT_DESC
{
    PVDIR_SCHEMA_AT_DESC    pSup;

    PSTR        pszDefinition;  // original definition used to create this descriptor
    USHORT      usAttrID;     // internal id stored in database
    UINT        uiMaxSize;    // optional attribute length constrain

    PSTR        pszName;
    PSTR        pszOid;
    PSTR        pszDesc;
    PSTR        pszSup;
    PSTR*       ppszAliases;        // ends with NULL PSTR
    PSTR        pszSyntaxName;
    PSTR        pszEqualityMRName;
    PSTR        pszOrderingMRName;
    PSTR        pszSubStringMRName;

    BOOLEAN     bSingleValue;
    BOOLEAN     bCollective;
    BOOLEAN     bNoUserModifiable;
    BOOLEAN     bObsolete;

    VDIR_ATTRIBUTETYPE_USAGE    usage;

    PVDIR_SYNTAX_DESC           pSyntax;
    PVDIR_MATCHING_RULE_DESC    pEqualityMR;
    PVDIR_MATCHING_RULE_DESC    pOrderingMR;
    PVDIR_MATCHING_RULE_DESC    pSubStringMR;

    // Entry to simulate AD AttributeSchema
    PVDIR_ENTRY    pADAttributeSchemaEntry;

} VDIR_SCHEMA_AT_DESC;

typedef DWORD (*PFN_VDIR_NORMALIZE_FUNCTION)(PVDIR_BERVALUE pBerv);

typedef BOOLEAN (*PFN_VDIR_COMPARE_FUNCTION)(VDIR_SCHEMA_MATCH_TYPE type, PVDIR_BERVALUE pBerv1, PVDIR_BERVALUE pBerv2);

typedef struct _VDIR_MATCHING_RULE_DESC
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    PSTR                        pszName;
    PSTR                        pszOid;
    PSTR                        pszSyntaxOid;
    PVDIR_SYNTAX_DESC           pSyntax;
    PFN_VDIR_NORMALIZE_FUNCTION pNormalizeFunc;
    PFN_VDIR_COMPARE_FUNCTION   pCompareFunc;

} VDIR_MATCHING_RULE_DESC;

///////////////////////////////////////////////////////////////////////////////
// Schema library initialize / shutdown
// Schema cache instantiation
///////////////////////////////////////////////////////////////////////////////
/*
 * Initialize schema library
 */
DWORD
VmDirSchemaLibInit(
    VOID
    );

/*
 * Should call initialize function once during server startup
 * (i.e. force load schema from a file - this is potentially dangerous
 *  operation as it could cause internal id to attribute mapping corruption.
 *  Should use in extreme caution.)
 *  The only valid use case is - some how schema entry gone bad or
 *  can not be read from data store.  In this case, we can use this feature
 *  to force load a copy of schema dump file from the last schema modification.
 */
DWORD
VmDirSchemaInitializeViaFile(
    PCSTR pszSchemaFilePath
    );

/*
 * Should call initialize function once during server startup
 * (i.e. read schema entry from data store and pass it here)
 */
DWORD
VmDirSchemaInitializeViaEntry(
    PVDIR_ENTRY    pEntry
    );

/*
 * Shutdown schema library
 */
void
VmDirSchemaLibShutdown(
    void
    );

/*
 * convert schema file + current custom schema definitions into an entry
 */
DWORD
VmDirSchemaPatchFileToEntry(
    PCSTR           pszSchemaFilePath,
    PVDIR_ENTRY     pEntry
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
// Schema cache modification
///////////////////////////////////////////////////////////////////////////////
/*
 * Before modify schema cache, make sure new schema is valid.
 */
DWORD
VmDirSchemaCacheModifyPrepare(
    PVDIR_OPERATION      pOperation,
    PVDIR_MODIFICATION   pMods,
    PVDIR_ENTRY          pEntry
    );

/*
 * Commit schema modification into cache.
 * Do we need two calls here to modify schema?
 */
VOID
VmDirSchemaCacheModifyCommit(
    PVDIR_ENTRY  pEntry
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

BOOLEAN
VmDirSchemaAttrHasIntegerMatchingRule(
    PVDIR_SCHEMA_CTX    pCtx,
    PCSTR               pszName
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
    PVDIR_ENTRY           pEntry
    );

/*
 * Entry schema structure rule check
 */
DWORD
VmDirSchemaCheckDITStructure(
    PVDIR_SCHEMA_CTX pCtx,
    PVDIR_ENTRY           pParentEntry,
    PVDIR_ENTRY           pEntry
    );

/*
 * Berval syntax check and optionally value normalization
 */
DWORD
VmDirSchemaBervalSyntaxCheck(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_AT_DESC    pATDesc,
    PVDIR_BERVALUE                 pBerv
    );
/*
 *

DWORD
VmDirSchemaBervalArraySyntaxCheck(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_AT_DESC    pATDesc,
    DWORD                   dwNumBervs,
    PBERVAL*                ppBervs
    );
 */
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
    PVDIR_BERVALUE                 pBerv
    );

DWORD
VmDirNormalizeDNWrapper(
    PVDIR_BERVALUE             pBerv
    );

DWORD
VmDirNormalizeDN(
    PVDIR_BERVALUE             pBerv,
    PVDIR_SCHEMA_CTX    pSchemaCtx
    );
/*
 *

DWORD
VmdirSchemaBervalArrayNormalize(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_AT_DESC    pATDesc,
    DWORD                   dwNumBervs,
    PBERVAL*                ppBervs
    );
 */
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

DWORD
VmDirSchemaGetComputedAttribute(
    PCSTR               pszComputedAttrName,
    PVDIR_ENTRY         pEntry,
    PVDIR_ATTRIBUTE*    ppOutAttr
    );

/*
 * Set Attribute.pATDesc
 */
DWORD
VmDirSchemaCheckSetAttrDesc(
    PVDIR_SCHEMA_CTX pCtx,
    PVDIR_ENTRY           pEntry
    );

///////////////////////////////////////////////////////////////////////////////
// AD compatible schema search
///////////////////////////////////////////////////////////////////////////////
/*
 * caller does NOT own *ppEntry.  it is a cached entry in schema.
 */
DWORD
VmDirADCompatibleSearchClassSchema(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PCSTR               pszName,
    PVDIR_ENTRY*        ppEntry
    );

/*
 * caller does NOT own *ppEntry.  it is a cached entry in schema.
 */
DWORD
VmDirADCompatibleSearchAttributeSchema(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PCSTR               pszName,
    PVDIR_ENTRY*        ppEntry
    );

/*
 * send all AD compatible attributeschema entries back.
 */
DWORD
VmDirADCompatibleSendAllAttributeSchema(
    PVDIR_OPERATION     pOp
    );

#ifdef __cplusplus
}
#endif

#endif /* __VIDRSCHEMA_H__ */


