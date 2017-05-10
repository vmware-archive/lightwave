/*
 * Copyright © 2016 VMware, Inc.  All Rights Reserved.
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

#ifndef __VMDIR_COMMON_SCHEMA_H__
#define __VMDIR_COMMON_SCHEMA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ldap_schema.h>

typedef enum
{
    VDIR_LDAP_USER_APPLICATIONS_ATTRIBUTE = 0,
    VDIR_LDAP_DIRECTORY_OPERATION_ATTRIBUTE,
    VDIR_LDAP_DISTRIBUTED_OPERATION_ATTRIBUTE,
    VDIR_LDAP_DSA_OPERATION_ATTRIBUTE

} VDIR_LDAP_ATTRIBUTE_TYPE_USAGE;

typedef enum
{
    VDIR_LDAP_STRUCTURAL_CLASS = 1,
    VDIR_LDAP_ABSTRACT_CLASS,
    VDIR_LDAP_AUXILIARY_CLASS

} VDIR_LDAP_OBJECT_CLASS_TYPE;

typedef struct _VDIR_LDAP_ATTRIBUTE_TYPE
{
    LDAPAttributeType*                  pSource;
    PSTR                                pszName;
    PSTR                                pszOid;
    PSTR                                pszDesc;
    PSTR*                               ppszAliases;
    PSTR                                pszSyntaxOid;
    BOOLEAN                             bSingleValue;
    BOOLEAN                             bCollective;
    BOOLEAN                             bNoUserMod;
    BOOLEAN                             bObsolete;
    VDIR_LDAP_ATTRIBUTE_TYPE_USAGE      usage;

    // index configuration attributes
    DWORD                               dwSearchFlags;
    PSTR*                               ppszUniqueScopes;

} VDIR_LDAP_ATTRIBUTE_TYPE, *PVDIR_LDAP_ATTRIBUTE_TYPE;

typedef struct _VDIR_LDAP_OBJECT_CLASS
{
    LDAPObjectClass*                    pSource;
    PSTR                                pszName;
    PSTR                                pszOid;
    PSTR                                pszDesc;
    PSTR                                pszSup;
    PSTR*                               ppszMust;
    PSTR*                               ppszMay;
    BOOLEAN                             bObsolete;
    VDIR_LDAP_OBJECT_CLASS_TYPE         type;

} VDIR_LDAP_OBJECT_CLASS, *PVDIR_LDAP_OBJECT_CLASS;

typedef struct _VDIR_LDAP_CONTENT_RULE
{
    LDAPContentRule*                    pSource;
    PSTR                                pszName;
    PSTR                                pszOid;
    PSTR*                               ppszMust;
    PSTR*                               ppszMay;
    PSTR*                               ppszNot;
    PSTR*                               ppszAux;
    BOOLEAN                             bObsolete;

} VDIR_LDAP_CONTENT_RULE, *PVDIR_LDAP_CONTENT_RULE;

typedef struct _VDIR_LDAP_STRUCTURE_RULE
{
    LDAPStructureRule*                  pSource;
    PSTR                                pszName;
    // TODO

} VDIR_LDAP_STRUCTURE_RULE, *PVDIR_LDAP_STRUCTURE_RULE;

typedef struct _VDIR_LDAP_NAME_FORM
{
    LDAPNameForm*                       pSource;
    PSTR                                pszName;
    // TODO

} VDIR_LDAP_NAME_FORM, *PVDIR_LDAP_NAME_FORM;

typedef PLW_HASHMAP PVDIR_LDAP_ATTRIBUTE_TYPE_MAP;
typedef PLW_HASHMAP PVDIR_LDAP_OBJECT_CLASS_MAP;
typedef PLW_HASHMAP PVDIR_LDAP_CONTENT_RULE_MAP;
typedef PLW_HASHMAP PVDIR_LDAP_STRUCTURE_RULE_MAP;
typedef PLW_HASHMAP PVDIR_LDAP_NAME_FORM_MAP;

typedef struct _VDIR_LDAP_SCHEMA
{
    PVDIR_LDAP_ATTRIBUTE_TYPE_MAP   attributeTypes;
    PVDIR_LDAP_OBJECT_CLASS_MAP     objectClasses;
    PVDIR_LDAP_CONTENT_RULE_MAP     contentRules;
    PVDIR_LDAP_STRUCTURE_RULE_MAP   structureRules;
    PVDIR_LDAP_NAME_FORM_MAP        nameForms;

} VDIR_LDAP_SCHEMA, *PVDIR_LDAP_SCHEMA;

// make sure enum stat with 0 and w/o gap.
typedef enum
{
    MOD_OP_ADD = 0,
    MOD_OP_DELETE,
    MOD_OP_REPLACE

} VDIR_LDAP_MOD_OP;

typedef struct _VDIR_LDAP_MOD
{
    VDIR_LDAP_MOD_OP    op;
    PSTR                pszType;
    PVMDIR_STRING_LIST  pVals;

} VDIR_LDAP_MOD, *PVDIR_LDAP_MOD;

typedef PLW_HASHMAP PVDIR_LDAP_MOD_MAP;

typedef struct _VDIR_LDAP_SCHEMA_OBJECT_DIFF
{
    PSTR                pszCN;
    PSTR                pszDN;
    PVDIR_LDAP_MOD_MAP  mods;

} VDIR_LDAP_SCHEMA_OBJECT_DIFF, *PVDIR_LDAP_SCHEMA_OBJECT_DIFF;

typedef struct _VDIR_LDAP_SCHEMA_DIFF
{
    PVDIR_LINKED_LIST   attrToAdd;
    PVDIR_LINKED_LIST   attrToModify;
    PVDIR_LINKED_LIST   classToAdd;
    PVDIR_LINKED_LIST   classToModify;

} VDIR_LDAP_SCHEMA_DIFF, *PVDIR_LDAP_SCHEMA_DIFF;


// copy.c
DWORD
VmDirLdapSchemaCopy(
    PVDIR_LDAP_SCHEMA   pOrgSchema,
    PVDIR_LDAP_SCHEMA*  ppCopySchema
    );

DWORD
VmDirLdapSchemaDeepCopy(
    PVDIR_LDAP_SCHEMA   pOrgSchema,
    PVDIR_LDAP_SCHEMA*  ppCopySchema
    );

DWORD
VmDirLdapAtDeepCopy(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pOrgAt,
    PVDIR_LDAP_ATTRIBUTE_TYPE*  ppCopyAt
    );

DWORD
VmDirLdapOcDeepCopy(
    PVDIR_LDAP_OBJECT_CLASS     pOrgOc,
    PVDIR_LDAP_OBJECT_CLASS*    ppCopyOc
    );

DWORD
VmDirLdapCrDeepCopy(
    PVDIR_LDAP_CONTENT_RULE     pOrgCr,
    PVDIR_LDAP_CONTENT_RULE*    ppCopyCr
    );

DWORD
VmDirLdapSrDeepCopy(
    PVDIR_LDAP_STRUCTURE_RULE   pOrgSr,
    PVDIR_LDAP_STRUCTURE_RULE*  ppCopySr
    );

DWORD
VmDirLdapNfDeepCopy(
    PVDIR_LDAP_NAME_FORM    pOrgNf,
    PVDIR_LDAP_NAME_FORM*   ppCopyNf
    );

// def.c
DWORD
VmDirLdapAtCreate(
    LDAPAttributeType*          pSource,
    PVDIR_LDAP_ATTRIBUTE_TYPE*  ppAt
    );

DWORD
VmDirLdapOcCreate(
    LDAPObjectClass*            pSource,
    PVDIR_LDAP_OBJECT_CLASS*    ppOc
    );

DWORD
VmDirLdapCrCreate(
    LDAPContentRule*            pSource,
    PVDIR_LDAP_CONTENT_RULE*    ppCr
    );

DWORD
VmDirLdapSrCreate(
    LDAPStructureRule*          pSource,
    PVDIR_LDAP_STRUCTURE_RULE*  ppSr
    );

DWORD
VmDirLdapNfCreate(
    LDAPNameForm*           pSource,
    PVDIR_LDAP_NAME_FORM*   ppNf
    );

VOID
VmDirFreeLdapAt(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt
    );

VOID
VmDirFreeLdapOc(
    PVDIR_LDAP_OBJECT_CLASS pOc
    );

VOID
VmDirFreeLdapCr(
    PVDIR_LDAP_CONTENT_RULE pCr
    );

VOID
VmDirFreeLdapSr(
    PVDIR_LDAP_STRUCTURE_RULE   pSr
    );

VOID
VmDirFreeLdapNf(
    PVDIR_LDAP_NAME_FORM    pNf
    );

// diff.c
DWORD
VmDirLdapSchemaGetDiff(
    PVDIR_LDAP_SCHEMA       pOldSchema,
    PVDIR_LDAP_SCHEMA       pNewSchema,
    PVDIR_LDAP_SCHEMA_DIFF* ppSchemaDiff
    );

VOID
VmDirFreeLdapSchemaDiff(
    PVDIR_LDAP_SCHEMA_DIFF  pSchemaDiff
    );

// file.c
DWORD
VmDirGetDefaultSchemaFile(
    PSTR*   ppszSchemaFile
    );

DWORD
VmDirReadSchemaFile(
    PCSTR               pszSchemaFilePath,
    PVMDIR_STRING_LIST* ppAtStrList,
    PVMDIR_STRING_LIST* ppOcStrList,
    PVMDIR_STRING_LIST* ppCrStrList
    );

// load.c
DWORD
VmDirLdapSchemaLoadStrLists(
    PVDIR_LDAP_SCHEMA   pSchema,
    PVMDIR_STRING_LIST  pAtStrList,
    PVMDIR_STRING_LIST  pOcStrList,
    PVMDIR_STRING_LIST  pCrStrList
    );

DWORD
VmDirLdapSchemaLoadFile(
    PVDIR_LDAP_SCHEMA   pSchema,
    PCSTR               pszSchemaFilePath
    );

DWORD
VmDirLdapSchemaLoadRemoteSchema(
    PVDIR_LDAP_SCHEMA   pSchema,
    LDAP*               pLd
    );

// merge.c
DWORD
VmDirLdapSchemaMerge(
    PVDIR_LDAP_SCHEMA   pOldSchema,
    PVDIR_LDAP_SCHEMA   pNewSchema,
    PVDIR_LDAP_SCHEMA*  ppMergedSchema
    );

// parse.c
DWORD
VmDirLdapAtParseStr(
    PCSTR                       pcszStr,
    PVDIR_LDAP_ATTRIBUTE_TYPE*  ppAt
    );

DWORD
VmDirLdapOcParseStr(
    PCSTR                       pcszStr,
    PVDIR_LDAP_OBJECT_CLASS*    ppOc
    );

DWORD
VmDirLdapCrParseStr(
    PCSTR                       pcszStr,
    PVDIR_LDAP_CONTENT_RULE*    ppCr
    );

DWORD
VmDirLdapSrParseStr(
    PCSTR                       pcszStr,
    PVDIR_LDAP_STRUCTURE_RULE*  ppSr
    );

DWORD
VmDirLdapNfParseStr(
    PCSTR                   pcszStr,
    PVDIR_LDAP_NAME_FORM*   ppNf
    );

DWORD
VmDirLdapAtParseLDAPEntry(
    LDAP*                       pLd,
    LDAPMessage*                pEntry,
    PVDIR_LDAP_ATTRIBUTE_TYPE*  ppAt
    );

DWORD
VmDirLdapOcParseLDAPEntry(
    LDAP*                       pLd,
    LDAPMessage*                pEntry,
    PVDIR_LDAP_OBJECT_CLASS*    ppOc
    );

DWORD
VmDirLdapCrParseLDAPEntry(
    LDAP*                       pLd,
    LDAPMessage*                pEntry,
    PVDIR_LDAP_CONTENT_RULE*    ppCr
    );

DWORD
VmDirLdapAtToStr(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt,
    PSTR*                       ppszStr
    );

DWORD
VmDirLdapOcToStr(
    PVDIR_LDAP_OBJECT_CLASS pOc,
    PSTR*                   ppszStr
    );

DWORD
VmDirLdapCrToStr(
    PVDIR_LDAP_CONTENT_RULE pCr,
    PSTR*                   ppszStr
    );

DWORD
VmDirLdapSrToStr(
    PVDIR_LDAP_STRUCTURE_RULE   pSr,
    PSTR*                       ppszStr
    );

DWORD
VmDirLdapNfToStr(
    PVDIR_LDAP_NAME_FORM    pNf,
    PSTR*                   ppszStr
    );

// patch.c
DWORD
VmDirPatchRemoteSchemaObjects(
    LDAP*               pLd,
    PVDIR_LDAP_SCHEMA   pNewSchema
    );

// resolve.c
DWORD
VmDirLdapOcResolveSup(
    PVDIR_LDAP_SCHEMA           pSchema,
    PVDIR_LDAP_OBJECT_CLASS     pOc
    );

// schema.c
DWORD
VmDirLdapSchemaInit(
    PVDIR_LDAP_SCHEMA*  ppSchema
    );

DWORD
VmDirLdapSchemaAddAt(
    PVDIR_LDAP_SCHEMA           pSchema,
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt
    );

DWORD
VmDirLdapSchemaAddOc(
    PVDIR_LDAP_SCHEMA       pSchema,
    PVDIR_LDAP_OBJECT_CLASS pOc
    );

DWORD
VmDirLdapSchemaAddCr(
    PVDIR_LDAP_SCHEMA       pSchema,
    PVDIR_LDAP_CONTENT_RULE pCr
    );

DWORD
VmDirLdapSchemaAddSr(
    PVDIR_LDAP_SCHEMA           pSchema,
    PVDIR_LDAP_STRUCTURE_RULE   pSr
    );

DWORD
VmDirLdapSchemaAddNf(
    PVDIR_LDAP_SCHEMA       pSchema,
    PVDIR_LDAP_NAME_FORM    pNf
    );

DWORD
VmDirLdapSchemaResolveAndVerifyAll(
    PVDIR_LDAP_SCHEMA   pSchema
    );

DWORD
VmDirLdapSchemaRemoveNoopData(
    PVDIR_LDAP_SCHEMA   pSchema
    );

BOOLEAN
VmDirLdapSchemaIsEmpty(
    PVDIR_LDAP_SCHEMA   pSchema
    );

VOID
VmDirFreeLdapSchema(
    PVDIR_LDAP_SCHEMA   pSchema
    );

// verify.c
DWORD
VmDirLdapAtVerify(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt
    );

DWORD
VmDirLdapOcVerify(
    PVDIR_LDAP_SCHEMA       pSchema,
    PVDIR_LDAP_OBJECT_CLASS pOc
    );

DWORD
VmDirLdapCrVerify(
    PVDIR_LDAP_SCHEMA       pSchema,
    PVDIR_LDAP_CONTENT_RULE pCRDesc
    );

//////////////////////////////////////
// Legacy support structs/functions //
//////////////////////////////////////

typedef struct _VDIR_LEGACY_SCHEMA
{
    PLW_HASHMAP pAtDefStrMap;
    PLW_HASHMAP pOcDefStrMap;
    PLW_HASHMAP pCrDefStrMap;
    PVDIR_LDAP_SCHEMA   pSchema;

} VDIR_LEGACY_SCHEMA, *PVDIR_LEGACY_SCHEMA;

typedef struct _VDIR_LEGACY_SCHEMA_MOD
{
    PVMDIR_STRING_LIST  pDelAt;
    PVMDIR_STRING_LIST  pAddAt;
    PVMDIR_STRING_LIST  pDelOc;
    PVMDIR_STRING_LIST  pAddOc;
    PVMDIR_STRING_LIST  pDelCr;
    PVMDIR_STRING_LIST  pAddCr;

} VDIR_LEGACY_SCHEMA_MOD, *PVDIR_LEGACY_SCHEMA_MOD;

// legacy/legacyload.c
DWORD
VmDirLegacySchemaLoadRemoteSchema(
    PVDIR_LEGACY_SCHEMA pLegacySchema,
    LDAP*               pLd
    );

// legacy/legacypatch.c
DWORD
VmDirPatchRemoteSubSchemaSubEntry(
    LDAP*               pLd,
    PVDIR_LDAP_SCHEMA   pNewSchema
    );

// legacy/legacyschema.c
DWORD
VmDirLegacySchemaInit(
    PVDIR_LEGACY_SCHEMA*    ppLegacySchema
    );

VOID
VmDirFreeLegacySchema(
    PVDIR_LEGACY_SCHEMA pLegacySchema
    );

// legacy/legacyschemamod.c
DWORD
VmDirLegacySchemaModInit(
    PVDIR_LEGACY_SCHEMA_MOD*    ppLegacySchemaMod
    );

DWORD
VmDirLegacySchemaModPopulate(
    PVDIR_LEGACY_SCHEMA_MOD pLegacySchemaMod,
    PVDIR_LEGACY_SCHEMA     pLegacySchema,
    PVDIR_LDAP_SCHEMA       pNewSchema
    );

VOID
VmDirFreeLegacySchemaMod(
    PVDIR_LEGACY_SCHEMA_MOD pLegacySchemaMod
    );

// legacy/legacyutil.c
DWORD
VmDirLdapSearchSubSchemaSubEntry(
    LDAP*           pLd,
    LDAPMessage**   ppResult,
    LDAPMessage**   ppEntry
    );

DWORD
VmDirFixLegacySchemaDefSyntaxErr(
    PSTR    pszDef,
    PSTR*   ppszFixedDef
    );

#ifdef __cplusplus
}
#endif

#endif /* __VMDIR_COMMON_SCHEMA_H__ */
