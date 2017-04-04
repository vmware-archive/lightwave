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
    BOOLEAN bPublic;

} VDIR_SYNTAX_DESC;

typedef struct _VDIR_SCHEMA_AT_COLLECION
{
    PLW_HASHMAP byName;
    PLW_HASHMAP byId;

} VDIR_SCHEMA_AT_COLLECTION, *PVDIR_SCHEMA_AT_COLLECTION;

typedef struct _VDIR_SCHEMA_OC_COLLECTION
{
    PLW_HASHMAP byName;

} VDIR_SCHEMA_OC_COLLECTION, *PVDIR_SCHEMA_OC_COLLECTION;

typedef struct _VDIR_SCHEMA_CR_DESC
{
    PVDIR_LDAP_CONTENT_RULE pLdapCr;

    PSTR        pszName;
    PSTR        pszOid;
    PSTR*       ppszAuxOCs;     // ends with NULL PSTR
    PSTR*       ppszMustATs;    // ends with NULL PSTR
    PSTR*       ppszMayATs;     // ends with NULL PSTR
    PSTR*       ppszNotATs;     // ends with NULL PSTR

    BOOLEAN     bObsolete;

} VDIR_SCHEMA_CR_DESC, *PVDIR_SCHEMA_CR_DESC;

typedef struct _VDIR_SCHEMA_CR_COLLECTION
{
    PLW_HASHMAP byName;

} VDIR_SCHEMA_CR_COLLECTION, *PVDIR_SCHEMA_CR_COLLECTION;

typedef struct _VDIR_SCHEMA_SR_DESC
{
    PVDIR_LDAP_STRUCTURE_RULE   pLdapSr;

    PSTR        pszRuleID;
    PSTR        pszName;
    PSTR        pszOid;
    PSTR        pszNameform;    // nameform of this rule.
    USHORT      usNumSupRulesID;
    PSTR*       ppszSupRulesID; // Sup rules in RuleID
    BOOLEAN     bObsolete;

} VDIR_SCHEMA_SR_DESC, *PVDIR_SCHEMA_SR_DESC;

typedef struct _VDIR_SCHEMA_SR_COLLECTION
{
    PLW_HASHMAP byId;

} VDIR_SCHEMA_SR_COLLECTION, *PVDIR_SCHEMA_SR_COLLECTION;

typedef struct _VDIR_SCHEMA_NF_DESC
{
    PVDIR_LDAP_NAME_FORM    pLdapNf;

    PSTR        pszOid;
    PSTR        pszName;
    BOOLEAN     bObsolete;
    PSTR        pszStructOC;        // structural object class name of this rule.
    PSTR*       ppszMustATs;
    PSTR*       ppszMayATs;

} VDIR_SCHEMA_NF_DESC, *PVDIR_SCHEMA_NF_DESC;

typedef struct _VDIR_SCHEMA_NF_COLLECTION
{
    PLW_HASHMAP byName;

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

    // lookup tables
    VDIR_SCHEMA_AT_COLLECTION   attributeTypes;
    VDIR_SCHEMA_OC_COLLECTION   objectClasses;
    VDIR_SCHEMA_CR_COLLECTION   contentRules;
    VDIR_SCHEMA_SR_COLLECTION   structureRules;
    VDIR_SCHEMA_NF_COLLECTION   nameForms;

} VDIR_SCHEMA_INSTANCE, *PVDIR_SCHEMA_INSTANCE;

typedef struct _VDIR_SCHEMA_CTX
{
    PSTR    pszErrorMsg;
    DWORD   dwErrorCode;
    PVDIR_LDAP_SCHEMA       pLdapSchema;
    PVDIR_SCHEMA_INSTANCE   pVdirSchema;

} VDIR_SCHEMA_CTX;

typedef struct _VDIR_SCHEMA_ATTR_ID_MAP
{
    PLW_HASHMAP pStoredIds;
    PLW_HASHMAP pNewIds;
    USHORT      usNextId;

} VDIR_SCHEMA_ATTR_ID_MAP, *PVDIR_SCHEMA_ATTR_ID_MAP;

typedef struct _VDIR_SCHEMA_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...

    // a self reference to active pLdapSchema and pVdirSchema
    PVDIR_SCHEMA_CTX        pCtx;

    // pLdapSchema is schema cache instance for schema management
    PVDIR_LDAP_SCHEMA       pLdapSchema;    // active pLdapSchema

    // pVdirSchema is schema cache instance for server operations
    // pVdirSchema is created from pLdapSchema
    PVDIR_SCHEMA_INSTANCE   pVdirSchema;    // active pVdirSchema

    // pending schema caches will replace live caches after
    // schema update process completes successfully
    PVDIR_LDAP_SCHEMA       pPendingLdapSchema;
    PVDIR_SCHEMA_INSTANCE   pPendingVdirSchema;

    // mutex to synchronize ctx manipulation
    PVMDIR_MUTEX    ctxMutex;

    // mutex to enforce one cache modification at a time
    PVMDIR_MUTEX    cacheModMutex;

    // structure to store and manage attribute IDs
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap;

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
    USHORT                      usEqualityMRSize;
    PVDIR_MATCHING_RULE_DESC    pEqualityMatchingRule;
    USHORT                      usOrderingMRSize;
    PVDIR_MATCHING_RULE_DESC    pOrderingMatchingRule;
    USHORT                      usSubstrMRSize;
    PVDIR_MATCHING_RULE_DESC    pSubstrMatchingRule;

} VDIR_MATCHING_RULE_GLOBALS, *PVDIR_MATCHING_RULE_GLOBALS;

typedef struct _VDIR_SCHEMA_BOOTSTRAP_TABLE
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    USHORT   usAttrID;
    PCSTR    pszDesc;

} VDIR_SCHEMA_BOOTSTRAP_TABLE, *PVDIR_SCHEMA_BOOTSTRAP_TABLE;

typedef struct _VDIR_SCHEMA_REPL_STATUS_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    PVMDIR_RWLOCK       rwlock;
    PVMDIR_MUTEX        mutex;
    PVMDIR_COND         cond;
    PVDIR_THREAD_INFO   pThrInfo;

    PVDIR_LINKED_LIST   pReplStates;
    BOOLEAN             bRefreshInProgress;

} VDIR_SCHEMA_REPL_STATUS_GLOBALS, *PVDIR_SCHEMA_REPL_STATUS_GLOBALS;
