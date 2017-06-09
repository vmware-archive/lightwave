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




#ifndef VMDIR_H_
#define VMDIR_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <ldap.h>

#include "vmdirtypes.h"

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// Internal error code indicating that a Notice of Disconnection SHOULD be sent, and connection MUST be terminated.
#define LDAP_NOTICE_OF_DISCONNECT    (-1)

#define SUPPORTED_LDAP_VERSION          "3"

#define DEFAULT_LDAP_PORT_NUM           389
#define DEFAULT_LDAP_PORT_STR           "389"
#define DEFAULT_LDAPS_PORT_NUM          636
#define DEFAULT_LDAPS_PORT_STR          "636"

#define DEFAULT_REST_PORT_NUM           7577
#define DEFAULT_REST_PORT_STR           "7577"

#define LEGACY_DEFAULT_LDAP_PORT_NUM       11711
#define LEGACY_DEFAULT_LDAP_PORT_STR       "11711"
#define LEGACY_DEFAULT_LDAPS_PORT_NUM      11712
#define LEGACY_DEFAULT_LDAPS_PORT_STR      "11712"

// Fixed DNs
#define DSE_ROOT_DN                             ""
#define PERSISTED_DSE_ROOT_DN                   "cn=DSE Root"
#define PERSISTED_DSE_ROOT_NAMING_ATTR_VALUE    "DSE Root"
#define SCHEMA_NAMING_CONTEXT_DN                "cn=schemacontext"
#define SCHEMA_NAMING_CONTEXT_DN_LEN            sizeof(SCHEMA_NAMING_CONTEXT_DN)-1
#define SUB_SCHEMA_SUB_ENTRY_DN                 "cn=aggregate,cn=schemacontext"
#define CFG_ROOT_DN                             "cn=config"
#define CFG_ROOT_DN_LEN                         sizeof(CFG_ROOT_DN)-1
#define CFG_INDEX_ENTRY_DN                      "cn=indices,cn=config"
#define CFG_INDEX_ORGANIZATION_DN               "cn=organization,cn=config"
#define SERVER_STATUS_DN                        "cn=serverstatus"
#define REPLICATION_STATUS_DN                   "cn=replicationstatus"
#define RAFT_CONTEXT_DN                         "cn=raftcontext"
#define RAFT_LOGS_CONTAINER_DN                  "cn=logs,cn=raftcontext"
#define RAFT_PERSIST_STATE_DN                   "cn=persiststate,cn=raftcontext"
#define RAFT_STATE_DN                           "cn=raftstate"

#define VMDIR_DOMAIN_CONTROLLERS_RDN_VAL        "Raft Clusters"
#define VMDIR_COMPUTERS_RDN_VAL                 "Computers"
#define VMDIR_MSAS_RDN_VAL                      "Managed Service Accounts"
#define VMDIR_CONFIGURATION_CONTAINER_NAME      "Configuration"
#define VMDIR_CA_CONTAINER_NAME                 "Certificate-Authorities"
#define RAFT_CONTEXT_CONTAINER_NAME             "raftcontext"
#define RAFT_PERSIST_STATE_NAME                 "persiststate"
#define RAFT_LOGS_CONTAINER_NAME                "logs"

#define FSP_CONTAINER_RDN_ATTR                  "cn"
#define FSP_CONTAINER_RDN_ATTR_VALUE            "ForeignSecurityPrincipals"

#define RDN_VALUE_ESC_CHAR             '\\'
#define RDN_SEPARATOR_CHAR             ','
#define SID_SEPARATOR_CHAR             '-'
#define RDN_TYPE_VAL_SEPARATOR         '='

#define PASSWD_SCHEME_VMDIRD        "vmdird-digest"
#define PASSWD_SCHEME_DEFAULT       "DEFAULT-vmdird-v1"
#define PASSWD_SCHEME_SSO_V1_1      "SSO-V1-1"
#define PASSWD_SCHEME_SSO_V1_2      "SSO-V1-2"
#define PASSWD_SCHEME_SHA1          "SHA1"

#define DSE_ROOT_ENTRY_ID              1
#define SCHEMA_NAMING_CONTEXT_ID       2
#define SUB_SCEHMA_SUB_ENTRY_ID        3
#define CFG_ROOT_ENTRY_ID              4
#define CFG_INDEX_ENTRY_ID             5
#define CFG_ORGANIZATION_ENTRY_ID      6
#define DEL_ENTRY_CONTAINER_ENTRY_ID   7
#define DEFAULT_ADMINISTRATOR_ENTRY_ID 8
#define RAFT_CONTEXT_ENTRY_ID          9
#define RAFT_CONTEXT_PS_ENTRY_ID       10
#define RAFT_LOGS_CONTAINER_ENTRY_ID   11
#define RAFT_PERSIST_STATE_ENTRY_ID    12

// Schema related constants.

// Attributes
#define ATTR_CN                             "cn"
#define ATTR_CN_LEN                         sizeof(ATTR_CN)-1
#define ATTR_SN                             "sn"
#define ATTR_SN_LEN                         sizeof(ATTR_SN)-1
#define ATTR_GIVEN_NAME                     "givenName"
#define ATTR_GIVEN_NAME_LEN                 sizeof(ATTR_GIVEN_NAME)-1
#define ATTR_DESCRIPTION                    "description"
#define ATTR_DESCRIPTION_LEN                sizeof(ATTR_DESCRIPTION)-1
#define ATTR_DN                             "entryDN"
#define ATTR_DN_LEN                         sizeof(ATTR_DN)-1
#define ATTR_EID                            "entryid"
#define ATTR_EID_LEN                        sizeof(ATTR_EID)-1
#define ATTR_OBJECT_CLASS                   "objectclass"
#define ATTR_OBJECT_CLASS_LEN               sizeof(ATTR_OBJECT_CLASS)-1
#define ATTR_PARENT_ID                      "parentid" /* Not really an attribute, just identifies the index */
#define ATTR_PARENT_ID_LEN                  sizeof(ATTR_PARENT_ID)-1
#define ATTR_SUB_SCHEMA_SUB_ENTRY           "subschemaSubentry"
#define ATTR_SUPPORTED_LDAP_VERSION         "supportedLDAPVersion"
#define ATTR_USER_PASSWORD                  "userPassword"
#define ATTR_PASSWORD_SCHEME                "passwordHashScheme"
#define ATTR_OLD_USER_PASSWORD              "oldUserPassword"
#define ATTR_OLD_USER_PASSWORD_LEN          sizeof(ATTR_OLD_USER_PASSWORD)-1
#define ATTR_PWD_LAST_SET                   "pwdLastSet"
#define ATTR_PWD_LAST_SET_LEN               sizeof(ATTR_PWD_LAST_SET)-1
#define ATTR_MEMBER                         "member"
#define ATTR_MEMBER_LEN                     sizeof(ATTR_MEMBER)-1
#define ATTR_MEMBEROF                       "memberOf"
#define ATTR_MEMBEROF_LEN                   sizeof(ATTR_MEMBEROF)-1
#define ATTR_USN_CREATED                    "uSNCreated"
#define ATTR_USN_CREATED_LEN                sizeof(ATTR_USN_CREATED)-1
#define ATTR_USN_CHANGED                    "uSNChanged"
#define ATTR_USN_CHANGED_LEN                sizeof(ATTR_USN_CHANGED)-1
#define ATTR_ATTR_META_DATA                 "attributeMetaData"
#define ATTR_ATTR_META_DATA_LEN             sizeof(ATTR_ATTR_META_DATA)-1
#define ATTR_ATTR_VALUE_META_DATA           "attributeValueMetaData"
#define ATTR_ATTR_VALUE_META_DATA_LEN       sizeof(ATTR_ATTR_VALUE_META_DATA)-1
#define ATTR_IS_DELETED                     "isDeleted"
#define ATTR_IS_DELETED_LEN                 sizeof(ATTR_IS_DELETED)-1
#define ATTR_OBJECT_GUID                    "objectGUID"
#define ATTR_OBJECT_GUID_LEN                sizeof(ATTR_OBJECT_GUID)-1
#define ATTR_LAST_KNOWN_DN                  "lastKnownDn"
#define ATTR_LAST_KNOWN_DN_LEN              sizeof(ATTR_LAST_KNOWN_DN)-1
#define ATTR_NAMING_CONTEXTS                "namingContexts"
#define ATTR_ROOT_DOMAIN_NAMING_CONTEXT     "rootDomainNamingContext"
#define ATTR_DEFAULT_NAMING_CONTEXT         "defaultNamingContext"
#define ATTR_CONFIG_NAMING_CONTEXT          "configurationNamingContext"
#define ATTR_SCHEMA_NAMING_CONTEXT          "schemaNamingContext"
#define ATTR_SERVER_ID                      "serverId"
#define ATTR_SERVER_GUID                    "vmwServerGUID"
#define ATTR_LDU_GUID                       "vmwLDUGuid"
#define ATTR_MACHINE_GUID                   "vmwMachineGuid"
#define ATTR_DEFAULT_ADMIN_DN               "vmwAdministratorDN"
#define ATTR_SERVER_NAME                    "serverName"
#define ATTR_SERVER_NAME_LEN                sizeof(ATTR_SERVER_NAME)-1
#define ATTR_DEL_OBJS_CONTAINER             "deletedObjectsContainer"
#define ATTR_SUPPORTED_CONTROL              "supportedControl"
#define ATTR_SUPPORTED_SASL_MECHANISMS      "supportedSASLMechanisms"
#define ATTR_LABELED_URI                    "labeledURI"
#define ATTR_USER_CERTIFICATE               "userCertificate"
#define ATTR_USER_CERTIFICATE_LEN           sizeof(ATTR_USER_CERTIFICATE)-1
#define ATTR_LAST_LOCAL_USN_PROCESSED       "lastLocalUsnProcessed"
#define ATTR_LAST_LOCAL_USN_PROCESSED_LEN    sizeof(ATTR_LAST_LOCAL_USN_PROCESSED)-1
#define ATTR_REPL_INTERVAL                  "replInterval"
#define ATTR_UP_TO_DATE_VECTOR              "upToDateVector"
#define ATTR_UP_TO_DATE_VECTOR_LEN          sizeof(ATTR_UP_TO_DATE_VECTOR)-1
#define ATTR_REPL_PAGE_SIZE                 "replPageSize"
#define ATTR_MODIFYTIMESTAMP                "modifyTimeStamp"
#define ATTR_MODIFYTIMESTAMP_LEN            sizeof(ATTR_MODIFYTIMESTAMP)-1
#define ATTR_CREATETIMESTAMP                "createTimeStamp"
#define ATTR_CREATETIMESTAMP_LEN            sizeof(ATTR_CREATETIMESTAMP)-1
#define ATTR_ENABLED                        "enabled"
#define ATTR_ENABLED_LEN                    sizeof(ATTR_ENABLED)-1
#define ATTR_USER_ACCOUNT_CONTROL           "userAccountControl"
#define ATTR_USER_ACCOUNT_CONTROL_LEN       sizeof(ATTR_USER_ACCOUNT_CONTROL)-1
#define ATTR_OBJECT_SID                     "objectSid"
#define ATTR_OBJECT_SID_LEN                 sizeof(ATTR_OBJECT_SID)-1
#define ATTR_FSP_OBJECTID                   "externalObjectId"
#define ATTR_FSP_OBJECTID_LEN               sizeof(ATTR_FSP_OBJECTID)-1
#define ATTR_EID_SEQUENCE_NUMBER            "vmwEntryIdSequenceNumber"
#define ATTR_USN_SEQUENCE_NUMBER            "vmwUSNSequenceNumber"
#define ATTR_DOMAIN_COMPONENT               "dc"
#define ATTR_DOMAIN_COMPONENT_LEN           sizeof(ATTR_DOMAIN_COMPONENT)-1
#define ATTR_ORGANIZATION                   "o"
#define ATTR_SERVER_RUNTIME_STATUS          "vmwServerRunTimeStatus"
#define ATTR_SITE_ID                        "vmwCisSiteId"
#define ATTR_DISPLAY_NAME                   "displayName"
#define ATTR_KRB_PRINCIPAL_NAME             "krbPrincipalName"
#define ATTR_KRB_PRINCIPAL_NAME_LEN         sizeof(ATTR_KRB_PRINCIPAL_NAME)-1
#define ATTR_KRB_PRINCIPAL_KEY              "krbPrincipalKey"
#define ATTR_KRB_PRINCIPAL_KEY_LEN          sizeof(ATTR_KRB_PRINCIPAL_KEY)-1
#define ATTR_KRB_UPN                        "userPrincipalName"
#define ATTR_KRB_UPN_LEN                    sizeof(ATTR_KRB_UPN)-1
#define ATTR_KRB_SPN                        "servicePrincipalName"
#define ATTR_KRB_SPN_LEN                    sizeof(ATTR_KRB_SPN)-1
#define ATTR_KRB_MASTER_KEY                 "krbMKey"
#define ATTR_KRB_MASTER_KEY_LEN             sizeof(ATTR_KRB_MASTER_KEY)-1
#define ATTR_GROUPTYPE                      "groupType"
#define ATTR_GROUPTYPE_LEN                  sizeof(ATTR_GROUPTYPE)-1
#define ATTR_NAME                           "name"
#define ATTR_NAME_LEN                       sizeof(ATTR_NAME)-1
#define ATTR_SAM_ACCOUNT_NAME               "sAMAccountName"
#define ATTR_SAM_ACCOUNT_LEN                 sizeof(ATTR_SAM_ACCOUNT_NAME)-1
#define ATTR_ENABLED                        "enabled"
#define ATTR_ENABLED_LEN                    sizeof(ATTR_ENABLED)-1
#define ATTR_CREATORS_NAME                  "creatorsName"
#define ATTR_CREATORS_NAME_LEN              sizeof(ATTR_CREATORS_NAME)-1
#define ATTR_MODIFIERS_NAME                 "modifiersName"
#define ATTR_MODIFIERS_NAME_LEN             sizeof(ATTR_MODIFIERS_NAME)-1
#define ATTR_SITE_NAME                      "msDS-SiteName"
#define ATTR_SITE_NAME_LEN                  sizeof(ATTR_SITE_NAME)-1
#define ATTR_VMW_STS_PASSWORD               "vmwSTSPassword"
#define ATTR_VMW_STS_PASSWORD_LEN           sizeof(ATTR_VMW_STS_PASSWORD)-1
#define ATTR_VMW_STS_TENANT_KEY             "vmwSTSTenantKey"
#define ATTR_VMW_STS_TENANT_KEY_LEN         sizeof(ATTR_VMW_STS_TENANT_KEY)-1
#define ATTR_SRP_SECRET                     "vmwSRPSecret"
#define ATTR_SRP_SECRET_LEN                 sizeof(ATTR_SRP_SECRET)-1
#define ATTR_SITE_GUID                      "siteGUID"
#define ATTR_SITE_GUID_LEN                  sizeof(ATTR_SITE_GUID)-1
#define ATTR_VMWITCUSTOMERNUMBER            "vmwitcustomernumber"
#define ATTR_VMWITCUSTOMERNUMBER_LEN        sizeof(ATTR_VMWITCUSTOMERNUMBER)-1
#define ATTR_UID                            "uid"
#define ATTR_UID_LEN                        sizeof(ATTR_UID)-1
#define ATTR_VMWITUSERGUID                  "vmwituserguid"
#define ATTR_VMWITUSERGUID_LEN              sizeof(ATTR_VMWITUSERGUID)-1
#define ATTR_LASTLOGONTIMESTAMP             "lastLogonTimestamp"
#define ATTR_LASTLOGONTIMESTAMP_LEN         sizeof(ATTR_LASTLOGONTIMESTAMP)-1

#define ATTR_VMW_OBJECT_SECURITY_DESCRIPTOR   "vmwSecurityDescriptor"
#define ATTR_VMW_ORGANIZATION_GUID            "vmwOrganizationGuid"

#define ATTR_OBJECT_SECURITY_DESCRIPTOR       "nTSecurityDescriptor"
#define ATTR_OBJECT_SECURITY_DESCRIPTOR_LEN   sizeof(ATTR_OBJECT_SECURITY_DESCRIPTOR)-1
#define ATTR_ORG_LIST_DESC                    "vmwAttrOrganizationList"
#define VDIR_ATTRIBUTE_SEQUENCE_RID           "vmwRidSequenceNumber"

// password and lockout policy attribute
#define PASSWD_LOCKOUT_POLICY_DEFAULT_CN    "password and lockout policy"
#define ATTR_PASS_AUTO_UNLOCK_SEC           "vmwPasswordChangeAutoUnlockIntervalSec"
#define ATTR_PASS_FAIL_ATTEMPT_SEC          "vmwPasswordChangeFailedAttemptIntervalSec"
#define ATTR_PASS_MAX_FAIL_ATTEMPT          "vmwPasswordChangeMaxFailedAttempts"
#define ATTR_PASS_MAX_SAME_ADJ_CHAR         "vmwPasswordMaxIdenticalAdjacentChars"
#define ATTR_PASS_MIN_SP_CHAR               "vmwPasswordMinSpecialCharCount"
#define ATTR_PASS_MIN_MUN_CHAR              "vmwPasswordMinNumericCount"
#define ATTR_PASS_MIN_UPPER_CHAR            "vmwPasswordMinUpperCaseCount"
#define ATTR_PASS_MIN_LOWER_CHAR            "vmwPasswordMinLowerCaseCount"
#define ATTR_PASS_MIN_ALPHA_CHAR            "vmwPasswordMinAlphabeticCount"
#define ATTR_PASS_MIN_SIZE                  "vmwPasswordMinLength"
#define ATTR_PASS_MAX_SIZE                  "vmwPasswordMaxLength"
#define ATTR_PASS_EXP_IN_DAY                "vmwPasswordLifetimeDays"
#define ATTR_PASS_RECYCLE_CNT               "vmwPasswordProhibitedPreviousCount"
#define ATTR_PASS_SPECIAL_CHARS             "vmwPasswordSpecialChars"
// Attributes related to support for "functional levels" in vmdir
#define ATTR_DOMAIN_FUNCTIONAL_LEVEL        "vmwDomainFunctionalLevel"
#define ATTR_FOREST_FUNCTIONAL_LEVEL        "vmwForestFunctionalLevel"
#define ATTR_SERVER_VERSION                 "vmwServerVersion"
#define ATTR_PSC_VERSION                    "vmwPlatformServicesControllerVersion"
#define ATTR_PSC_VERSION_LEN                sizeof(ATTR_PSC_VERSION)-1

#define ATTR_OU                             "ou"
#define ATTR_DC_ACCOUNT_DN                  "vmwDCAccountDN"
#define ATTR_DC_ACCOUNT_DN_LEN              sizeof(ATTR_DC_ACCOUNT_DN)-1
#define ATTR_DC_ACCOUNT_UPN                 "vmwDCAccountUPN"
#define ATTR_DC_ACCOUNT_UPN_LEN             sizeof(ATTR_DC_ACCOUNT_UPN)-1
#define DEFAULT_USER_CONTAINER_RDN          "cn=users"

#define ATTR_MAX_SERVER_ID                  "vmwMaxServerId"
#define ATTR_MAX_SERVER_ID_LEN              sizeof(ATTR_MAX_SERVER_ID)-1

#define ATTR_ATTRIBUTETYPES                 "attributetypes"
#define ATTR_ATTRIBUTETYPES_LEN             sizeof(ATTR_ATTRIBUTETYPES)-1
#define ATTR_OBJECTCLASSES                  "objectclasses"
#define ATTR_OBJECTCLASSES_LEN              sizeof(ATTR_OBJECTCLASSES)-1
#define ATTR_DITCONTENTRULES                "ditcontentrules"
#define ATTR_DITCONTENTRULES_LEN            sizeof(ATTR_DITCONTENTRULES)-1
#define ATTR_LDAPSYNTAXES                   "ldapsyntaxes"
#define ATTR_LDAPSYNTAXES_LEN               sizeof(ATTR_LDAPSYNTAXES)-1

// ADSI support related attribute
#define ATTR_ALLOWD_CHILD_CLASSES_EFFECTIVE     "allowedChildClassesEffective"
#define ATTR_ALLOWD_CHILD_CLASSES_EFFECTIVE_LEN sizeof(ATTR_ALLOWD_CHILD_CLASSES_EFFECTIVE)-1

#define ATTR_PASSWORD_NEVER_EXPIRES         "vmwPasswordNeverExpires"
#define ATTR_PASSWORD_NEVER_EXPIRES_LEN     sizeof(ATTR_PASSWORD_NEVER_EXPIRES)-1

#define ATTR_HIGHEST_COMMITTED_USN          "highestCommittedUSN"
#define ATTR_HIGHEST_COMMITTED_USN_LEN      sizeof(ATTR_HIGHEST_COMMITTED_USN)-1

#define ATTR_INVOCATION_ID                  "invocationId"
#define ATTR_INVOCATION_ID_LEN              sizeof(ATTR_INVOCATION_ID)-1

#define ATTR_ACL_STRING                     "vmwAclString"
#define ATTR_ACL_STRING_LEN                 sizeof(ATTR_ACL_STRING)-1

#define ATTR_COMMENT                        "comment"
#define ATTR_COMMENT_LEN                    sizeof(ATTR_COMMENT)-1

// Attribute schema objects
#define ATTR_IS_SINGLE_VALUED               "issinglevalued"
#define ATTR_IS_SINGLE_VALUED_LEN           sizeof(ATTR_IS_SINGLE_VALUED)-1
#define ATTR_ATTRIBUTE_SYNTAX               "attributesyntax"
#define ATTR_ATTRIBUTE_SYNTAX_LEN           sizeof(ATTR_ATTRIBUTE_SYNTAX)-1
#define ATTR_LDAP_DISPLAYNAME               "ldapdisplayname"
#define ATTR_LDAP_DISPLAYNAME_LEN           sizeof(ATTR_LDAP_DISPLAYNAME)-1
#define ATTR_ATTRIBUTE_ID                   "attributeid"
#define ATTR_ATTRIBUTE_ID_LEN               sizeof(ATTR_ATTRIBUTE_ID)-1
#define ATTR_OMSYNTAX                       "omsyntax"
#define ATTR_OMSYNTAX_LEN                   sizeof(ATTR_OMSYNTAX)-1
#define ATTR_SCHEMAID_GUID                  "schemaidguid"
#define ATTR_SCHEMAID_GUID_LEN              sizeof(ATTR_SCHEMAID_GUID)-1
#define ATTR_VMW_ATTRIBUTE_USAGE            "vmwattributeusage"
#define ATTR_VMW_ATTRIBUTE_USAGE_LEN        sizeof(ATTR_VMW_ATTRIBUTE_USAGE)-1
#define ATTR_SEARCH_FLAGS                   "searchflags"
#define ATTR_SEARCH_FLAGS_LEN               sizeof(ATTR_SEARCH_FLAGS)-1
#define ATTR_UNIQUENESS_SCOPE               "vmwattruniquenessscope"
#define ATTR_UNIQUENESS_SCOPE_LEN           sizeof(ATTR_UNIQUENESS_SCOPE)-1

#define ATTR_SUBCLASSOF                     "subclassof"
#define ATTR_SUBCLASSOF_LEN                 sizeof(ATTR_SUBCLASSOF)-1
#define ATTR_GOVERNSID                      "governsid"
#define ATTR_GOVERNSID_LEN                  sizeof(ATTR_GOVERNSID)-1
#define ATTR_OBJECTCLASS_CATEGORY           "objectclasscategory"
#define ATTR_OBJECTCLASS_CATEGORY_LEN       sizeof(ATTR_OBJECTCLASS_CATEGORY)-1
#define ATTR_DEFAULT_OBJECT_CATEGORY        "defaultobjectcategory"
#define ATTR_DEFAULT_OBJECT_CATEGORY_LEN    sizeof(ATTR_DEFAULT_OBJECT_CATEGORY)-1
#define ATTR_SYSTEMMUSTCONTAIN              "systemmustcontain"
#define ATTR_SYSTEMMUSTCONTAIN_LEN           sizeof(ATTR_SYSTEMMUSTCONTAIN)-1
#define ATTR_SYSTEMMAYCONTAIN               "systemmaycontain"
#define ATTR_SYSTEMMAYCONTAIN_LEN            sizeof(ATTR_SYSTEMMAYCONTAIN)-1
#define ATTR_SYSTEMAUXILIARY_CLASS          "systemauxiliaryclass"
#define ATTR_SYSTEMAUXILIARY_CLASS_LEN      sizeof(ATTR_SYSTEMAUXILIARY_CLASS)-1
#define ATTR_MUSTCONTAIN                    "mustcontain"
#define ATTR_MUSTCONTAIN_LEN                sizeof(ATTR_MUSTCONTAIN)-1
#define ATTR_MAYCONTAIN                     "maycontain"
#define ATTR_MAYCONTAIN_LEN                 sizeof(ATTR_MAYCONTAIN)-1
#define ATTR_AUXILIARY_CLASS                "auxiliaryclass"
#define ATTR_AUXILIARY_CLASS_LEN            sizeof(ATTR_AUXILIARY_CLASS)-1
//Raft attributes
#define ATTR_RAFT_LOGINDEX                  "vmwRaftLogindex"
#define ATTR_RAFT_TERM                      "vmwRaftTerm"
#define ATTR_RAFT_LAST_APPLIED              "vmwRaftLastApplied"
#define ATTR_RAFT_FIRST_LOGINDEX            "vmwRaftFirstLogindex"
#define ATTR_RAFT_VOTEDFOR_TERM             "vmwRaftVotedForTerm"
#define ATTR_RAFT_VOTEDFOR                  "vmwRaftVotedFor"
#define ATTR_RAFT_LOG_ENTRIES               "vmwRaftLogEntries"
#define ATTR_REF                            "ref"
#define ATTR_RAFT_LEADER                    "vmwRaftLeader"
#define ATTR_RAFT_FOLLOWERS                 "vmwRaftActiveFollower"
#define ATTR_RAFT_MEMBERS                   "vmwRaftMember"
#define ATTR_RAFT_STATE                     "vmwRaftState"

// Object classes
#define OC_TOP                              "top"
#define OC_TOP_LEN                          sizeof(OC_TOP)-1
#define OC_DSE_ROOT                         "vmwDseRoot"
#define OC_SUB_SCHEMA                       "subschema"
#define OC_SUB_SCHEMA_LEN                   sizeof(OC_SUB_SCHEMA)-1
#define OC_DIR_SERVER                       "vmwDirServer"
#define OC_PKI_CA                           "pkiCA"
#define OC_REPLICATION_AGREEMENT            "vmwReplicationAgreement"
#define OC_REPLICATION_AGREEMENT_LEN        sizeof(OC_REPLICATION_AGREEMENT)-1

#define OC_DC_OBJECT                     "dcObject"
#define OC_DC_OBJECT_LEN                 sizeof(OC_DC_OBJECT)-1
#define OC_DOMAIN                        "domain"
#define OC_DOMAIN_LEN                    sizeof(OC_DOMAIN)-1
#define OC_DOMAIN_DNS                    "domainDNS"
#define OC_DOMAIN_DNS_LEN                sizeof(OC_DOMAIN_DNS)-1
#define OC_BUILTIN_DOMAIN                "builtinDomain"
#define OC_BUILTIN_DOMAIN_LEN            sizeof(OC_BUILTIN_DOMAIN)-1

#define OC_ORGANIZATION_OBJECT           "organization"
#define OC_FOREIGN_SECURITY_PRINCIPAL    "foreignSecurityPrincipal"

#define OC_VMW_POLICY                    "vmwPolicy"
#define OC_VMW_PASSWORD_POLICY           "vmwPasswordPolicy"
#define OC_VMW_LOCKOUT_POLICY            "vmwLockoutPolicy"
#define OC_VMW_SERVICEPRINCIPAL          "vmwServicePrincipal"

#define OC_VMW_CONTAINER                 "vmwContainer"

#define OC_CONTAINER                     "container"
#define OC_SERVER_STATUS                 "vmwDirServerStatus"

#define OC_KRB_PRINCIPAL_AUX            "krbPrincipalAux"
#define OC_KRB_PRINCIPAL_AUX_LEN        sizeof(OC_KRB_PRINCIPAL_AUX)-1

#define OC_ORGANIZATIONAL_UNIT           "organizationalUnit"
#define OC_PERSON                        "person"
#define OC_ORGANIZATIONAL_PERSON         "organizationalPerson"
#define OC_USER                          "user"
#define OC_USER_LEN                      sizeof(OC_USER)-1
#define OC_COMPUTER                      "computer"
#define OC_MANAGED_SERVICE_ACCOUNT       "msDS-ManagedServiceAccount"
#define OC_GROUP                         "group"
#define OC_GROUP_LEN                     sizeo(OC_GROUP)-1
#define OC_ATTRIBUTE_SCHEMA             "attributeschema"
#define OC_ATTRIBUTE_SCHEMA_LEN         sizeof(OC_ATTRIBUTE_SCHEMA)-1
#define OC_CLASS_SCHEMA                 "classschema"
#define OC_CLASS_SCHEMA_LEN             sizeof(OC_CLASS_SCHEMA)-1
#define OC_CLASS_RAFT_PERSIST_STATE     "vmwraftpersiststate"
#define OC_CLASS_RAFT_LOG_ENTRY         "vmwraftlogentry"
#define OC_CLASS_RAFT_STATE             "vmwRaftClusterState"
#define RAFT_CONTEXT_DN_MAX_LEN         64

#define CM_COMPONENTMANAGER             "ComponentManager"
#define CM_SITE                         "CMSites"
#define CM_LDUS                         "Ldus"
#define SSO_SERVICES                    "Services"

#define VMDIR_SITES_RDN_VAL             "Sites"

#define CM_DISPLAYNAME_SITE             "<Default Site>"
#define CM_DISPLAYNAME_LDU              "<Default Group>"
#define CM_OBJECTCLASS_SITE             "vmwCisSite"
#define CM_OBJECTCLASS_LDU              "vmwCisLdu"

// cn=replicationstatus sudo entry
#define REPLICATION_STATUS_CN           "ReplicationStatus"
#define REPL_STATUS_SERVER_NAME         "Server Name: "
#define REPL_STATUS_SERVER_NAME_LEN     sizeof(REPL_STATUS_SERVER_NAME)-1
#define REPL_STATUS_VISIBLE_USN         "USN: "
#define REPL_STATUS_VISIBLE_USN_LEN     sizeof(REPL_STATUS_VISIBLE_USN)-1
#define REPL_STATUS_CYCLE_COUNT         "Replication Cycle Count: "
#define REPL_STATUS_CYCLE_COUNT_LEN     sizeof(REPL_STATUS_CYCLE_COUNT)-1
#define REPL_STATUS_INVOCATION_ID       "InvocationID: "
#define REPL_STATUS_INVOCATION_ID_LEN   sizeof(REPL_STATUS_INVOCATION_ID)-1
#define REPL_STATUS_UTDVECTOR           "UtdVector: "
#define REPL_STATUS_UTDVECTOR_LEN       sizeof(REPL_STATUS_UTDVECTOR)-1
#define REPL_STATUS_PROCESSED_USN_VECTOR        "RAProcessedUSN Vector: "
#define REPL_STATUS_PROCESSED_USN_VECTOR_LEN    sizeof(REPL_STATUS_PROCESSED_USN_VECTOR)-1
#define REPL_STATUS_ORIGINATING_USN     "MaxOriginatingUSN: "
#define REPL_STATUS_ORIGINATING_USN_LEN  sizeof(REPL_STATUS_ORIGINATING_USN)-1

#define VMDIR_REPL_AGRS_CONTAINER_NAME  "Replication Agreements"
#define VMDIR_SERVERS_CONTAINER_NAME    "Servers"
#define VMDIR_SERVICES_CONTAINER_NAME   "Services"
#define VMDIR_DC_GROUP_NAME             "DCAdmins"
#define VMDIR_DCCLIENT_GROUP_NAME       "DCClients"
#define VMDIR_CERT_GROUP_NAME           "CAAdmins"
#define VMDIR_BUILTIN_CONTAINER_NAME    "Builtin"

#define VDIR_SERVER_VERSION             "1.0"

#define SASL_MECH                       "GSSAPI SRP"

#define VDIR_LDAP_BOOLEN_SYNTAX_TRUE_STR    "TRUE"

// User Account Control Flags
#define USER_ACCOUNT_CONTROL_DISABLE_FLAG              0x00000002
#define USER_ACCOUNT_CONTROL_LOCKOUT_FLAG              0x00000010
#define USER_ACCOUNT_CONTROL_PASSWORD_EXPIRE_FLAG      0x00800000

// Group type  (only support global scope now)
#define GROUPTYPE_GLOBAL_SCOPE          "2"

// Supported LDAP Request controls
#define VDIR_LDAP_CONTROL_SHOW_DELETED_OBJECTS    "1.2.840.113556.1.4.417" // value same as defined by AD

#define VDIR_LDAP_CONTROL_MANAGEDDSAIT            "2.16.840.1.113730.3.4.2" //RFC 3269 - ManageDsaIT control

#define VDIR_LDAP_CONTROL_SHOW_MASTER_KEY         "9999.9990.9900.9000.1" //shouldn't be published
// #define LDAP_CONTROL_SYNC       LDAP_SYNC_OID ".1" defined in ldap.h

// Logging stuff
#define MAX_LOG_MESSAGE_LEN    4096

// vmw OID for Strong Consistency Write Control
#define LDAP_CONTROL_CONSISTENT_WRITE                  "1.3.6.1.4.1.6876.40.10.1"

#ifndef _WIN32
#define LWRAFT_NCALRPC_END_POINT "lwraftsvc"
#else
// note: keep in sync with /vmdir/main/idl/vmdir.idl
#define LWRAFT_NCALRPC_END_POINT "LightwaveRaftService"
#endif

#define LWRAFT_RPC_TCP_END_POINT "2011"
#define VMDIR_MAX_SERVER_ID     255

#define NSECS_PER_SEC       1000000000
#define NSECS_PER_MSEC      1000000
#define MSECS_PER_SEC       1000

#ifdef __cplusplus
}
#endif

#endif /* VMDIR_H_ */
