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



// cert.c

DWORD
DirCliCreateCert(
    PCSTR          pszCertPath,
    PDIR_CLI_CERT* ppCert
    );

VOID
DirCliFreeCert(
    PDIR_CLI_CERT pCert
    );

DWORD
DirCliDERToX509(
    PBYTE pCertBytes,
    DWORD dwLength,
    X509** ppCert
    );

DWORD
DirCliX509ToDER(
    X509* pCert,
    PBYTE* ppCertBytes,
    PDWORD pdwLength
    );

DWORD
DirCliPEMToX509(
    PCSTR  pszCert,
    X509** ppCert
    );

DWORD
DirCliReadCertStringFromFile(
    PCSTR pszFileName,
    PSTR* ppszCertificate
    );

DWORD
DirCliReadCrlStringFromFile(
    PCSTR pszFileName,
    PSTR* ppszCrl
    );

DWORD
DirCliCertToPEM(
    X509* pCertificate,
    PSTR* ppszCertificate
);

DWORD
DirCliGetX509Name(
    X509_NAME *pCertName,
    DWORD dwFlags,
    PSTR* ppszSubjectDN
    );

// cli.c

DWORD
DirCliCreateServiceA(
    PCSTR pszServiceName,
    PCSTR pszCertPath,
    PCSTR pszSsoGroups,
    BOOL bTrustedUserGroup,
    SSO_ADMIN_ROLE ssoAdminRole,
    PCSTR pszLogin,
    PCSTR pszPassword
    );

DWORD
DirCliUpdateServiceA(
    PCSTR pszServiceName,
    PCSTR pszCertPath,
    PCSTR pszLogin,
    PCSTR pszPassword
    );

DWORD
DirCliListServiceA(
    PCSTR pszLogin,
    PCSTR pszPassword
    );

DWORD
DirCliDeleteServiceA(
    PCSTR pszServiceName,
    PCSTR pszLogin,
    PCSTR pszPassword
    );

DWORD
DirCliCreateGroupA(
    PCSTR pszGroupName,
    PCSTR pszDescription,
    PCSTR pszLogin,
    PCSTR pszPassword
    );

DWORD
DirCliAddGroupMemberA(
    PCSTR pszGroupName,
    PCSTR pszAcctName,
    PCSTR pszLogin,
    PCSTR pszPassword
    );

DWORD
DirCliRemoveGroupMemberA(
    PCSTR pszGroupName,
    PCSTR pszAcctName,
    PCSTR pszLogin,
    PCSTR pszPassword
    );

DWORD
DirCliGetUserDN(
    PCSTR pszUser,
    PCSTR pszDomain,
    PSTR* ppszUserDN
    );

DWORD
DirCliGetServiceDN(
    PCSTR pszServiceName,
    PCSTR pszDomain,
    PSTR* ppszServiceDN
    );

DWORD
DirCliGetOrgunitDN(
    PCSTR pszOrgunit,
    PCSTR pszDomain,
    PCSTR pszParentDN,
    PSTR* ppszOrgunitDN
    );

DWORD
DirCliGetDomainDN(
    PCSTR pszDomain,
    PSTR* ppszDomainDN
    );

DWORD
DirCliPublishCertA(
    PCSTR pszCertFile,
    PCSTR pszCrlFile,
    PCSTR pszLogin,
    PCSTR pszPassword,
    BOOL bPublishChain
    );

DWORD
DirCliUnpublishCertA(
    PCSTR pszCertFile,
    PCSTR pszLogin,
    PCSTR pszPassword
    );

DWORD
DirCliPublishCrlA(
    LDAP* pLd,
    PCSTR pszCrlFile,
    PCSTR pszLogin,
    PCSTR pszPassword
    );

DWORD
DirCliGetCertificationAuthoritiesA(
    PCSTR pszCACN,
    PCSTR pszCertFile,
    PCSTR pszCrlFile,
    PCSTR pszLogin,
    PCSTR pszPassword
    );

DWORD
DirCliListCertificationAuthoritiesA(
    PCSTR pszLogin,
    PCSTR pszPassword
    );

DWORD
DirCliGeneratePassword(
    PCSTR pszLogin,
    PCSTR pszPassword,
    PSTR* ppszPassword
    );

DWORD
DirCliChangePassword(
    PCSTR pszAccount,
    PCSTR pszPassword,
    PCSTR pszPasswordNew
    );

DWORD
DirCliResetPassword(
    PCSTR pszAccount,
    PCSTR pszPasswordNew,
    PCSTR pszLogin,
    PCSTR pszPassword
    );

DWORD
DirCliCreateUserA(
    PCSTR pszAcctName,
    PCSTR pszFirstname,
    PCSTR pszLastname,
    PCSTR pszUserPassword,
    PCSTR pszLogin,
    PCSTR pszPassword
    );

DWORD
DirCliDeleteUserA(
    PCSTR pszAccount,
    PCSTR pszLogin,
    PCSTR pszPassword
    );

DWORD
DirCliFindByNameUserA(
    PCSTR pszAccount,
    PCSTR pszLogin,
    PCSTR pszPassword,
    USER_INFO_LEVEL userInfoLevel
    );

DWORD
DirCliModifyAttributeUserA(
    PCSTR pszAccount,
    PCSTR pszLogin,
    PCSTR pszPassword,
    USER_MODIFY_OPT userModifyOpt
    );


DWORD
DirCliMachineAccountReset(
    PCSTR pszServerName, /* OPTIONAL */
    PCSTR pszLogin,
    PCSTR pszPassword
    );

// ldap.c

DWORD
DirCliLdapConnect(
    PCSTR  pszHostName,
    PCSTR  pszUser,
    PCSTR  pszDomain,
    PCSTR  pszPassword,
    LDAP** ppLd
    );

DWORD
DirCliLdapCreateService(
    LDAP*         pLd,
    PCSTR         pszServiceName,
    PCSTR         pszDomain,
    PDIR_CLI_CERT pCert,
    PSTR*         ppszServiceDN
    );

DWORD
DirCliLdapUpdateService(
    LDAP*         pLd,
    PCSTR         pszServiceName,
    PCSTR         pszDomain,
    PDIR_CLI_CERT pCert
    );

DWORD
DirCliLdapFindService(
    LDAP* pLd,
    PCSTR pszServiceName,
    PCSTR pszDomain,
    PSTR* ppszSubjectDN,
    PSTR* ppszDN
    );

DWORD
DirCliLdapBeginEnumServices(
    LDAP*  pLd,
    PCSTR  pszDomain,
    DWORD  dwMaxCount,
    PDIR_CLI_ENUM_SVC_CONTEXT* ppContext
    );

DWORD
DirCliLdapEnumServices(
    PDIR_CLI_ENUM_SVC_CONTEXT pContext,
    PSTR** pppszAccounts,
    PDWORD pdwCount
    );

VOID
DirCliLdapEndEnumServices(
    PDIR_CLI_ENUM_SVC_CONTEXT pContext
    );

DWORD
DirCliLdapBeginEnumMembers(
    LDAP*  pLd,
    PCSTR  pszGroup,
    PCSTR  pszDomain,
    DWORD  dwMaxCount,
    PDIR_CLI_ENUM_GROUP_CONTEXT* ppContext
    );

DWORD
DirCliLdapEnumMembers(
    PDIR_CLI_ENUM_GROUP_CONTEXT pContext,
    PSTR** pppszAccounts,
    PDWORD pdwCount
    );

VOID
DirCliLdapEndEnumMembers(
    PDIR_CLI_ENUM_GROUP_CONTEXT pContext
    );

DWORD
DirCliLdapCreateOrgunit(
    LDAP*         pLd,
    PCSTR         pszOrgunit,
    PCSTR         pszDomain,
    PCSTR         pszParentDN,
    PSTR*         ppszOrgunitDN
    );

DWORD
DirCliLdapBeginEnumOrgunits(
    LDAP*  pLd,
    PCSTR  pszContainerDN,
    PCSTR  pszDomain,
    DWORD  dwMaxCount,
    PDIR_CLI_ENUM_ORGUNIT_CONTEXT* ppContext
    );

DWORD
DirCliLdapEnumOrgunits(
    PDIR_CLI_ENUM_ORGUNIT_CONTEXT pContext,
    PSTR** pppszOrgunits,
    PDWORD pdwCount
    );

VOID
DirCliLdapEndEnumOrgunits(
    PDIR_CLI_ENUM_ORGUNIT_CONTEXT pContext
    );

DWORD
DirCliLdapFindFingerPrintMatch(
    LDAP* pLd,
    PCSTR pszFingerPrint,
    PCSTR pszDomain,
    PSTR* ppszServiceName
    );

DWORD
DirCliLdapDeleteService(
    LDAP* pLd,
    PCSTR pszServiceName,
    PCSTR pszDomain
    );

DWORD
DirCliLdapAddServiceToGroup(
    LDAP* pLd,
    PCSTR pszServiceDN,
    PCSTR pszGroupName,
    PCSTR pszDomain
    );

DWORD
DirCliLdapFindGroup(
    LDAP* pLd,
    PCSTR pszGroup,
    PCSTR pszDomain,
    PSTR* ppszGroupDN
    );

DWORD
DirCliLdapFindUser(
    LDAP* pLd,
    PCSTR pszAccount,
    PCSTR pszDomain,
    PSTR* ppszUserDN
    );

DWORD
DirCliLdapDeleteUser(
    LDAP* pLd,
    PCSTR pszAccount,
    PCSTR pszDomain
    );

DWORD
DirCliLdapGetUserAttr(
    LDAP* pLd,
    PCSTR pszAccount,
    PCSTR pszDomain,
    USER_INFO_LEVEL infoLevel,
    PDIR_CLI_USER_INFO* ppUserInfo
    );

DWORD
DirCliLdapGetUserAttrLevelDefault(
    LDAP* pLd,
    PCSTR pszAccount,
    PCSTR pszDomain,
    PDIR_CLI_USER_INFO* ppUserInfo
    );

DWORD
DirCliLdapGetUserAttrLevelOne(
    LDAP* pLd,
    PCSTR pszAccount,
    PCSTR pszDomain,
    PDIR_CLI_USER_INFO* ppUserInfo
    );

DWORD
DirCliLdapGetUserAttrLevelTwo(
    LDAP* pLd,
    PCSTR pszAccount,
    PCSTR pszDomain,
    PDIR_CLI_USER_INFO* ppUserInfo
    );

DWORD
DirCliUserGetUserPwdExpTimeDays(
    LDAP*  pLd,
    PCSTR  pszDomain,
    PSTR* ppszPwdExpTime
    );

DWORD
DirCliUserGetCurrentPwdExpTime(
    PDIR_CLI_USER_INFO pUserInfo,
    PSTR* ppszParsedPwdExpTime
    );

VOID
DirCliFreeUserInfo(
    PDIR_CLI_USER_INFO pUserInfo
    );

DWORD
DirCliLdapUserModifyAttrPwdNeverExp(
    LDAP* pLd,
    PCSTR pszAccountDN,
    PCSTR pszDomain,
    PCSTR pszAttrValue,
    ATTR_SEARCH_RESULT* pAttrStatus
    );

DWORD
DirCliLdapReplaceUserAttr(
    LDAP* pLd,
    PCSTR pszAccountDN,
    PCSTR pszAttrName,
    PCSTR pszAttrValue
    );

DWORD
DirCliLdapChangePassword(
    LDAP* pLd,
    PCSTR pszUserDN,
    PCSTR pszPasswordCurrent,
    PCSTR pszPasswordNew
    );

DWORD
DirCliLdapResetPassword(
    LDAP* pLd,
    PCSTR pszUserDN,
    PCSTR pszNewPassword
    );

VOID
DirCliLdapClose(
    LDAP* pLd
    );

DWORD
DirCliLdapAddServiceToBuiltinGroup(
    LDAP* pLd,
    PCSTR pszServiceDN,
    PCSTR pszGroupName,
    PCSTR pszDomain
    );

DWORD
DirCliLdapCreateGroup(
    LDAP*         pLd,
    PCSTR         pszGroupName,
    PCSTR         pszDescription,
    PCSTR         pszDomain,
    PSTR*         ppszGroupDN
    );

DWORD
DirCliLdapAddGroupMember(
    LDAP*         pLd,
    PCSTR         pszGroupName,
    PCSTR         pszAcct,
    PCSTR         pszDomain
    );

DWORD
DirCliLdapRemoveGroupMember(
    LDAP*         pLd,
    PCSTR         pszGroupName,
    PCSTR         pszAcct,
    PCSTR         pszDomain
    );

DWORD
DirCliListGroupA(
    PCSTR pszGroup,
    PCSTR pszLogin,
    PCSTR pszPassword
    );

DWORD
DirCliLdapGetDSERootAttribute(
    LDAP* pLotus,
    PSTR  pszAttribute,
    PSTR* ppAttrValue
    );

    DWORD
DirCliLdapCheckCAContainer(
    LDAP*    pLd,
    PCSTR    pszConfigurationDN,
    PBOOLEAN pbExists
    );

DWORD
DirCliLdapCreateCAContainer(
    LDAP* pLd,
    PCSTR pszCAConainter
    );

DWORD
DirCliLdapCheckCAObject(
    LDAP*    pLd,
    PCSTR    pszCAContainerDN,
    PCSTR    pszCADN,
    PCSTR    pszCAIssuerDN,
    PSTR     *ppszObjectDN
    );

DWORD
DirCliLdapCreateCAObject(
    LDAP*    pLd,
    PCSTR    pszCACN,
    PCSTR    pszCADN
    );

DWORD
DirCliLdapDeleteCAObject(
    LDAP*    pLd,
    PCSTR    pszCADN
    );

DWORD
DirCliLdapCheckAttribute(
    LDAP*    pLd,
    PCSTR    pszObjectDN,
    PCSTR    pszAttribute,
    PCSTR    pszValue,
    ATTR_SEARCH_RESULT* pAttrStatus
    );

DWORD
DirCliLdapUpdateAttribute(
    LDAP*   pLd,
    PCSTR   pszObjectDN,
    PSTR    pszAttribute,
    PSTR    pszValue,
    BOOL    bAdd
    );

DWORD
DirCliLdapUpdateCRL(
    LDAP*   pLd,
    PCSTR   pszCADN,
    PSTR    pszCrl,
    BOOL    bAdd
);

DWORD
DirCliGetDSERootAttribute(
    LDAP* pLotus,
    PSTR  pszAttribute,
    PSTR* ppAttrValue
    );

DWORD
DirCliQueryCACerts(
    LDAP* pLotus,
    PCSTR pszCACN,
    BOOL  bDetail,
    PVMAFD_CA_CERT_ARRAY* ppCACertificates
    );

DWORD
DirCliGetFuncLvl(
    PCSTR      pszHostName,
    PCSTR      pszUPN,
    PCSTR      pszPassword,
    PCSTR      pszDomainName,
    PDWORD     pdwFuncLvl
    );

DWORD
DirCliSetFuncLvl(
    PCSTR      pszHostName,
    PCSTR      pszUPN,
    PCSTR      pszPassword,
    PCSTR      pszDomainName,
    PDWORD     pdwFuncLvl
    );

DWORD
DirCliGetState(
    PCSTR      pszHostName,
    PCSTR      pszUPN,
    PCSTR      pszPassword,
    PCSTR      pszDomainName,
    PDWORD     pdwState
    );

DWORD
DirCliSetState(
    PCSTR      pszHostName,
    PCSTR      pszUPN,
    PCSTR      pszPassword,
    PCSTR      pszDomainName,
    DWORD      dwState
    );

DWORD
DirCliGetDCNodesVersion(
    PCSTR      pszHostName,
    PCSTR      pszUserName,
    PCSTR      pszPassword,
    PCSTR      pszDomainName,
    PVMDIR_DC_VERSION_INFO *ppDCVerInfo
    );

DWORD
DirCliListNodesA(
    PCSTR     pszHostName,
    PCSTR     pszLogin,
    PCSTR     pszPassword
    );

DWORD
DirCliCreateTenant(
    PCSTR pszLogin,
    PCSTR pszPassword,
    PCSTR pszDomainName,
    PCSTR pszNewUserName,
    PCSTR pszNewUserPassword
    );

DWORD
DirCliDeleteTenant(
    PCSTR pszLogin,
    PCSTR pszPassword,
    PCSTR pszDomainName
    );

DWORD
DirCliEnumerateTenants(
    PCSTR pszLogin,
    PCSTR pszPassword
    );

DWORD
DirCliCreateOrgunit(
    PCSTR pszLogin,
    PCSTR pszPassword,
    PCSTR pszOrgunit,
    PCSTR pszParentDN
    );

DWORD
DirCliEnumerateOrgunits(
    PCSTR pszLogin,
    PCSTR pszPassword,
    PCSTR pszContainerDN
    );
