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

// compat.c
DWORD
VmDirLdapAtAreCompat(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pPrevAt,
    PVDIR_LDAP_ATTRIBUTE_TYPE   pNewAt
    );

DWORD
VmDirLdapOcAreCompat(
    PVDIR_LDAP_OBJECT_CLASS pPrevOc,
    PVDIR_LDAP_OBJECT_CLASS pNewOc
    );

DWORD
VmDirLdapCrAreCompat(
    PVDIR_LDAP_CONTENT_RULE pPrevCr,
    PVDIR_LDAP_CONTENT_RULE pNewCr
    );

DWORD
VmDirLdapSrAreCompat(
    PVDIR_LDAP_STRUCTURE_RULE   pPrevSr,
    PVDIR_LDAP_STRUCTURE_RULE   pNewSr
    );

DWORD
VmDirLdapNfAreCompat(
    PVDIR_LDAP_NAME_FORM    pPrevNf,
    PVDIR_LDAP_NAME_FORM    pNewNf
    );

DWORD
VmDirLdapDefAreCompat(
    PVDIR_LDAP_DEFINITION   pPrevDef,
    PVDIR_LDAP_DEFINITION   pNewDef
    );

// deflist.c
DWORD
VmDirLdapDefListCreate(
    PVDIR_LDAP_DEFINITION_LIST* ppDefList
    );

DWORD
VmDirLdapDefListUpdateHead(
    PVDIR_LDAP_DEFINITION_LIST  pDefList,
    PVDIR_LDAP_DEFINITION       pDef
    );

VOID
VmDirLdapDefListRelease(
    PVDIR_LDAP_DEFINITION_LIST  pDefList,
    PVDIR_LDAP_DEFINITION       pDef
    );

VOID
VmDirFreeLdapDefList(
    PVDIR_LDAP_DEFINITION_LIST  pDefList
    );

// diff.c
DWORD
VmDirLdapAtGetDiff(
    PVDIR_LDAP_ATTRIBUTE_TYPE       pOldAt,
    PVDIR_LDAP_ATTRIBUTE_TYPE       pNewAt,
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF*  ppAtDiff
    );

DWORD
VmDirLdapOcGetDiff(
    PVDIR_LDAP_OBJECT_CLASS         pOldOc,
    PVDIR_LDAP_OBJECT_CLASS         pNewOc,
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF*  ppOcDiff
    );

DWORD
VmDirLdapCrGetDiff(
    PVDIR_LDAP_CONTENT_RULE         pOldCr,
    PVDIR_LDAP_CONTENT_RULE         pNewCr,
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pOcDiff,
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF*  ppCrDiff
    );

VOID
VmDirFreeLdapMod(
    PVDIR_LDAP_MOD  pMod
    );

VOID
VmDirFreeLdapSchemaObjectDiff(
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pObjectDiff
    );

// merge.c
DWORD
VmDirLdapAtMerge(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pOldAt,
    PVDIR_LDAP_ATTRIBUTE_TYPE   pNewAt,
    PVDIR_LDAP_ATTRIBUTE_TYPE*  ppMergedAt
    );

DWORD
VmDirLdapOcMerge(
    PVDIR_LDAP_OBJECT_CLASS     pOldOc,
    PVDIR_LDAP_OBJECT_CLASS     pNewOc,
    PVDIR_LDAP_OBJECT_CLASS*    ppMergedOc
    );

DWORD
VmDirLdapCrMerge(
    PVDIR_LDAP_CONTENT_RULE     pOldCr,
    PVDIR_LDAP_CONTENT_RULE     pNewCr,
    PVDIR_LDAP_CONTENT_RULE*    ppMergedCr
    );

// parse.c
VOID
VmDirFreeLdapDef(
    PVDIR_LDAP_DEFINITION  pDef
    );

// resolve.c
DWORD
VmDirLdapAtResolveAliases(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt,
    PVDIR_LDAP_ATTRIBUTE_TYPE** ppAtList
    );

DWORD
VmDirLdapAtResolveSup(
    PVDIR_LDAP_SCHEMA           pSchema,
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt
    );

DWORD
VmDirLdapCrResolveOid(
    PVDIR_LDAP_SCHEMA       pSchema,
    PVDIR_LDAP_CONTENT_RULE pCr
    );
