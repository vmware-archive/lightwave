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



/*prototypes.h*/

DWORD
VmDirParseArgs(
    int     argc,
    char*   argv[],
    PSTR*   ppszServerName,
    PSTR*   ppszAdminUPN,
    PSTR*   ppszPassword,
    PSTR*   ppszPasswordFile,
    PBOOLEAN pbAclOnly,
    PSTR*   ppszPnidFixDcAccountName,
    PSTR*   ppszPnidFixNewSamAccount
    );

VOID
ShowUsage(
    VOID
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
    PCSTR pAttrVal,
    int *pCnt,
    int *pFailedCnt
    );

VOID
VdcLdapUnbind(
    LDAP *pLd
    );
