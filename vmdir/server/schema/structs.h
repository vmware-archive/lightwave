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
 * Filename: structs.h
 *
 * Abstract:
 *
 * Directory schema module
 *
 * Private Structures
 *
 */

typedef enum
{
    SCHEMA_DESC_SORT_BY_NAME = 0,
    SCHEMA_DESC_SORT_BY_ID,
    SCHEMA_DESC_SORT_BY_OID

} VDIR_SCHEMA_DESC_SORT_BY ;

typedef enum
{
    VDIR_SYNTAX_HUMAN_READABLE_YES = 0,
    VDIR_SYNTAX_HUMAN_READABLE_NO

} VDIR_SYNTAX_READABLE_FLAG;

typedef BOOLEAN (*VDIR_SYNTAX_VALIDATION_FUNCTION)(PVDIR_BERVALUE pBerv);

typedef struct _VDIR_SYNTAX_DESC
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    PSTR    pszName;
    PSTR    pszOid;
    VDIR_SYNTAX_READABLE_FLAG        readableFlag;
    VDIR_SYNTAX_VALIDATION_FUNCTION  pValidateFunc;

} VDIR_SYNTAX_DESC;

typedef struct _VDIR_SCHEMA_AT_COLLECION
{
    USHORT    usNumATs;
    USHORT    usNextId;

    // pATSortName[dwNumATs] owns its content (sort by pszName)
    PVDIR_SCHEMA_AT_DESC    pATSortName;
    // size = usNextId, which is > usNumAtTs.  (sort by usIdMap)
    PVDIR_SCHEMA_AT_DESC*    ppATSortIdMap;

} VDIR_SCHEMA_AT_COLLECTION, *PVDIR_SCHEMA_AT_COLLECTION;

typedef enum
{
    VDIR_OC_ABSTRACT = 0,
    VDIR_OC_STRUCTURAL,
    VDIR_OC_AUXILIARY
} VDIR_SCHEMA_OBJECTCLASS_TYPE;

typedef enum
{
    VDIR_OC_ATTR_MUST = 0,
    VDIR_OC_ATTR_MAY
} VDIR_SCHEMA_OC_ATTRS_TYPE;

typedef struct _VDIR_SCHEMA_OC_DESC
{
    VDIR_SCHEMA_OBJECTCLASS_TYPE type;

    PSTR        pszDefinition;  // original definition used to create this descriptor
    USHORT      usNumSupOCs;
    PSTR*       ppszSupOCs;     // ends with NULL PSTR
    PSTR        pszName;
    PSTR        pszOid;
    PSTR        pszDesc;
    USHORT      usNumMustATs;
    PSTR*       ppszMustATs;    // ends with NULL PSTR
    USHORT      usNumMayATs;
    PSTR*       ppszMayATs;     // ends with NULL PSTR
    USHORT      usNumAuxOCs;
    PSTR*       ppszAuxOCs;     // ends with NULL PSTR

    BOOLEAN     bObsolete;

    // all super objectclass
    PVDIR_SCHEMA_OC_DESC*    ppSupOCs;
    PVDIR_SCHEMA_OC_DESC     pStructSupOC;

    // defined in this objectclass
    PVDIR_SCHEMA_AT_DESC*    ppMustATs;
    PVDIR_SCHEMA_AT_DESC*    ppMayATs;

    // defined in this + all super classes + content rule
    USHORT                   usNumAllMustATs;
    PVDIR_SCHEMA_AT_DESC*    ppAllMustATs;
    USHORT                   usNumAllMayATs;
    PVDIR_SCHEMA_AT_DESC*    ppAllMayATs;
    USHORT                   usNumAllNotATs;
    PVDIR_SCHEMA_AT_DESC*    ppAllNotATs;   //TODO, not supported yet.

    // DITConentRule: allowed auxiliary oc array
    struct _VDIR_SCHEMA_OC_DESC**    ppAllowedAuxOCs; // ends with NULL

    // DITStructureRule: allowed parent/child structure oc
    BOOLEAN     bAllowedParentRoot;
    PSTR*       ppszAllowedParentOCs;       // ends with NULL PSTR
    PSTR*       ppszAllowedChildOCs;        // ends with NULL PSTR

    // Nameform: allowed MUST and MAY RDNs
    PSTR*       ppszMustRDNs;
    USHORT      usNumMustRDNs;
    PSTR*       ppszMayRDNs;
    SHORT       usNumMayRDNs;

    // Entry to simulate AD ClassSchema
    PVDIR_ENTRY    pADClassSchemaEntry;

} VDIR_SCHEMA_OC_DESC;

typedef struct _VDIR_SCHEMA_OC_COLLECTION
{
    USHORT      usNumOCs;

    // pOCSortName[dwNumOCs] owns its content. (sort by pszName)
    PVDIR_SCHEMA_OC_DESC    pOCSortName;

} VDIR_SCHEMA_OC_COLLECTION, *PVDIR_SCHEMA_OC_COLLECTION;

typedef struct _VDIR_SCHEMA_CR_DESC
{
    PSTR        pszDefinition;  // original definition used to create this descriptor
    PSTR        pszName;
    PSTR        pszOid;
    PSTR        pszDesc;
    USHORT      usNumAuxOCs;
    PSTR*       ppszAuxOCs;     // ends with NULL PSTR
    USHORT      usNumMustATs;
    PSTR*       ppszMustATs;    // ends with NULL PSTR
    USHORT      usNumMayATs;
    PSTR*       ppszMayATs;     // ends with NULL PSTR
    USHORT      usNumNotATs;
    PSTR*       ppszNotATs;     // ends with NULL PSTR

    BOOLEAN     bObsolete;

} VDIR_SCHEMA_CR_DESC, *PVDIR_SCHEMA_CR_DESC;

typedef struct _VDIR_SCHEMA_CR_COLLECTION
{
    USHORT      usNumContents;

    // pContentSortName[dwNumConetentss] owns its content. (sort by pszName)
    PVDIR_SCHEMA_CR_DESC   pContentSortName;

} VDIR_SCHEMA_CR_COLLECTION, *PVDIR_SCHEMA_CR_COLLECTION;

typedef struct _VDIR_SCHEMA_SR_DESC
{
    PSTR        pszRuleID;
    PSTR        pszName;
    PSTR        pszOid;
    PSTR        pszDesc;
    PSTR        pszNameform;    // nameform of this rule.
    USHORT      usNumSupRulesID;
    PSTR*       ppszSupRulesID; // Sup rules in RuleID
    BOOLEAN     bObsolete;

} VDIR_SCHEMA_SR_DESC, *PVDIR_SCHEMA_SR_DESC;

typedef struct _VDIR_SCHEMA_SR_COLLECTION
{
    USHORT      usNumStructures;

    // pStructureSortRuleID[dwNumConetentss] owns its content. (sort by pszRuleID)
    PVDIR_SCHEMA_SR_DESC   pStructureSortRuleID;

} VDIR_SCHEMA_SR_COLLECTION, *PVDIR_SCHEMA_SR_COLLECTION;

/*
 * NameFormDescription = "(" whsp
          numericoid whsp  ; NameForm identifier
          [ "NAME" qdescrs ]
          [ "DESC" qdstring ]
          [ "OBSOLETE" whsp ]
          "OC" woid         ; Structural ObjectClass
          "MUST" oids       ; AttributeTypes
          [ "MAY" oids ]    ; AttributeTypes
          whsp ")"
 */
typedef struct _VDIR_SCHEMA_NF_DESC
{
    PSTR        pszOid;
    PSTR        pszName;
    PSTR        pszDesc;
    BOOLEAN     bObsolete;
    PSTR        pszStructOC;        // structural object class name of this rule.
    USHORT      usNumMustATs;
    PSTR*       ppszMustATs;
    USHORT      usNumMayATs;
    PSTR*       ppszMayATs;

} VDIR_SCHEMA_NF_DESC, *PVDIR_SCHEMA_NF_DESC;

typedef struct _VDIR_SCHEMA_NF_COLLECTION
{
    USHORT                  usNumNameForms;

    // pNameForms[usNumNameForms] owns its content. (sort by pszName)
    PVDIR_SCHEMA_NF_DESC    pNameFormSortName;

} VDIR_SCHEMA_NF_COLLECTION, *PVDIR_SCHEMA_NF_COLLECTION;

typedef struct _VDIR_SCHEMA_INSTANCE
{
    // lock to protect usRefCount
    PVMDIR_MUTEX mutex;
    // live schema has at least (1 + usNumSelfRef) count
    USHORT      usRefCount;
    // when usRefCount == usNumSelfRef, we can free this instance
    USHORT      usNumSelfRef;

    BOOLEAN     bIsBootStrapSchema;

    VDIR_SCHEMA_AT_COLLECTION        ats;               // attribute types
    VDIR_SCHEMA_OC_COLLECTION        ocs;               // object classes
    VDIR_SCHEMA_CR_COLLECTION        contentRules;      // content rules
    VDIR_SCHEMA_SR_COLLECTION        structureRules;    // structure rules
    VDIR_SCHEMA_NF_COLLECTION        nameForms;         // nameforms

} VDIR_SCHEMA_INSTANCE, *PVDIR_SCHEMA_INSTANCE;

typedef struct _VDIR_SCHEMA_CTX
{
    PSTR    pszErrorMsg;
    DWORD   dwErrorCode;
    PVDIR_SCHEMA_INSTANCE pSchema;

} VDIR_SCHEMA_CTX;

typedef struct _VDIR_SCHEMA_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    PVMDIR_MUTEX mutex;

    PSTR        pszDN;

    // a self reference to active pSchema
    PVDIR_SCHEMA_CTX      pCtx;
    PVDIR_SCHEMA_INSTANCE pSchema;  // active pSchema

    // has pending modified schema
    BOOLEAN                 bHasPendingChange;

    // entry created from file to load schema when server start up for the first time
    PVDIR_ENTRY             pLoadFromFileEntry;

} VDIR_SCHEMA_GLOBALS, *PVDIR_SCHEMA_GLOBALS;


typedef struct _VDIR_SYNTAX_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    USHORT               usSize;
    PVDIR_SYNTAX_DESC    pSyntax;

} VDIR_SYNTAX_GLOBALS, *PVDIR_SYNTAX_GLOBALS;

typedef struct _VDIR_MATCHING_RULE_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    USHORT                      usSize;
    PVDIR_MATCHING_RULE_DESC    pMatchingRule;

} VDIR_MATCHING_RULE_GLOBALS, *PVDIR_MATCHING_RULE_GLOBALS;

typedef struct _VDIR_SCHEMA_BOOTSTRAP_TABLE
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    USHORT   usAttrID;
    PCSTR    pszDesc;

} VDIR_SCHEMA_BOOTSTRAP_TABLE, *PVDIR_SCHEMA_BOOTSTRAP_TABLE;

typedef struct _VDIR_SCHEMA_FILL_ALL_ATS
{
    PVDIR_SCHEMA_AT_DESC**    pppAllATs;
    USHORT    usSize;
    USHORT    usCnt;

} VDIR_SCHEMA_FILL_ALL_ATS, *PVDIR_SCHEMA_FILL_ALL_ATS;

typedef struct _VDIR_SCHEMA_NAME_SIZE_CNT_TUPLE
{
    int         iAllowedParentSize;
    int         iAllowedParentCnt;
    PSTR*       ppszAllowedParentOCs;

    int         iAllowedChildSize;
    int         iAllowedChildCnt;
    PSTR*       ppszAllowedChildOCs;
} VDIR_SCHEMA_NAME_SIZE_CNT_TUPLE, *PVDIR_SCHEMA_NAME_SIZE_CNT_TUPLE;

typedef enum
{
    VDIR_SCHEMA_NOT_COMPATIBLE = 1,
    VDIR_SCHEMA_COMPATIBLE,         // same definition
    VDIR_SCHEMA_UPDATE_ENTRY,       // entry change (e.g. change description field)
    VDIR_SCHEMA_UPDATE_ENTRY_CACHE  // entry + cache change (e.g. add new attributetypes)
} VDIR_SCHEMA_COMPATIBLE_LEVEL;
