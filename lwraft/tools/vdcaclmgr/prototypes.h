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

//
// Utility functions.
//
DWORD
_VdcAddCopiesToHashTable(
    PLW_HASHMAP pHashMap,
    PCSTR pszKey,
    PCSTR pszValue
    );

VOID
VdcFreeHashMap(
    PLW_HASHMAP *ppHashMap
    );

DWORD
VdcLdapAddAttributeValues(
    LDAP *pLd,
    PCSTR pszDN,
    PCSTR pszAttribute,
    PCSTR *ppszAttributeValues
    );

DWORD
VdcLdapReplaceAttributeValues(
    LDAP *pLd,
    PCSTR pszDN,
    PCSTR pszAttribute,
    PCSTR *ppszAttributeValues
    );

DWORD
VdcLdapGetObjectList(
    LDAP *pLd,
    PCSTR pBase,
    int ldapScope,
    PCSTR pszFilter,
    PLW_HASHMAP pUserToSidMapping,
    PLW_HASHMAP pSidToUserMapping
    );

DWORD
VdcLdapGetAttributeValue(
    LDAP *pLd,
    PCSTR pBase,
    int ldapScope,
    PCSTR pszFilter,
    PCSTR pszAttribute,
    PSTR *ppszAttributeValue
    );

DWORD
VdcLdapAddContainer(
    LDAP*  pLd,
    PCSTR  pszContainerDN,
    PCSTR  pszContainerName
    );

DWORD
VdcLdapAddGroup(
    LDAP*  pLd,
    PCSTR  pszGroupDN,
    PCSTR  pszGroupName
    );

BOOLEAN
VdcIfDNExist(
    LDAP* pLd,
    PCSTR pszDN);

DWORD
VdcLdapConnect(
    PCSTR pszLdapURI,
    PCSTR pszUserDN,
    PCSTR pszPassword,
    LDAP **ppLd
    );

DWORD
VdcLdapConnectSRP(
    PCSTR pszLdapURI,
    PCSTR pszUserDN,
    PCSTR pszPassword,
    LDAP **ppLd
    );

DWORD
VdcLdapReplaceAttrOnEntries(
    LDAP *pLd,
    PCSTR pBase,
    int ldapScope,
    PCSTR pszFilter,
    PCSTR pAttrName,
    PCSTR pAttrVal
    );

VOID
VdcLdapUnbind(
    LDAP *pLd
    );

//
// StringList routines
//
DWORD
VdcStringListInitialize(
    PSTRING_LIST *ppStringList,
    DWORD dwInitialCount
    );

VOID
VdcStringListFree(
    PSTRING_LIST pStringList
    );

DWORD
VdcStringListAdd(
    PSTRING_LIST pStringList,
    PCSTR pszString
    );

DWORD
VdcStringListRemove(
    PSTRING_LIST pStringList,
    PCSTR pszString
    );

BOOLEAN
VdcStringListContains(
    PSTRING_LIST pStringList,
    PCSTR pszString
    );

//
// ACL routines.
//
DWORD
VdcGrantPermissionToUser(
    LDAP *pLd,
    PLW_HASHMAP pUserToSidMapping,
    PCSTR pszObjectDN,
    PCSTR pszPermissionStatement
    );

DWORD
VdcRemovePermissionFromUser(
    LDAP *pLd,
    PLW_HASHMAP pUserToSidMapping,
    PCSTR pszObjectDN,
    PCSTR pszPermissionStatement
    );

DWORD
VdcLoadUsersAndGroups(
    LDAP *pLd,
    PCSTR pszBaseDN,
    PLW_HASHMAP *ppUserToSidMapping,
    PLW_HASHMAP *ppSidToUserMapping
    );

DWORD
VdcPrintSecurityDescriptorForObject(
    LDAP *pLd,
    PLW_HASHMAP pSidToUserMapping,
    PCSTR pszObjectDN,
    BOOLEAN bVerbose
    );
