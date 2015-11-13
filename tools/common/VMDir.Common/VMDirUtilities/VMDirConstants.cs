/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
using System;

namespace VMDir.Common
{
    public static class VMDirConstants
    {
        // Attributes
        public const string ATTR_CN = "cn";
        public const string ATTR_SN = "sn";
        public const string ATTR_GIVEN_NAME = "givenName";

        public const string ATTR_DN = "entryDN";
        public const string ATTR_EID = "entryid";
        public const string ATTR_OBJECT_CLASS = "objectclass";
        public const string ATTR_SUB_SCHEMA_SUB_ENTRY = "subschemaSubentry";
        public const string ATTR_SUPPORTED_LDAP_VERSION = "supportedLDAPVersion";
        public const string ATTR_USER_PASSWORD = "userPassword";
        public const string ATTR_PASSWORD_SCHEME = "passwordHashScheme";
        public const string ATTR_OLD_USER_PASSWORD = "oldUserPassword";
        public const string ATTR_PWD_LAST_SET = "pwdLastSet";
        public const string ATTR_MEMBER = "member";
        public const string ATTR_MEMBEROF = "memberOf";

        public const string ATTR_USN_CREATED = "uSNCreated";

        public const string ATTR_USN_CHANGED = "uSNChanged";

        public const string ATTR_ATTR_META_DATA = "attributeMetaData";

        public const string ATTR_IS_DELETED = "isDeleted";

        public const string ATTR_OBJECT_GUID = "objectGUID";

        public const string ATTR_LAST_KNOWN_DN = "lastKnownDn";
        public const string ATTR_NAMING_CONTEXTS = "namingContexts";
        public const string ATTR_ROOT_DOMAIN_NAMING_CONTEXT = "rootDomainNamingContext";
        public const string ATTR_DEFAULT_NAMING_CONTEXT = "defaultNamingContext";
        public const string ATTR_CONFIG_NAMING_CONTEXT = "configurationNamingContext";
        public const string ATTR_SCHEMA_NAMING_CONTEXT = "schemaNamingContext";
        public const string ATTR_SERVER_ID = "serverId";
        public const string ATTR_SERVER_GUID = "vmwServerGUID";
        public const string ATTR_LDU_GUID = "vmwLDUGuid";
        public const string ATTR_MACHINE_GUID = "vmwMachineGuid";
        public const string ATTR_DEFAULT_ADMIN_DN = "vmwAdministratorDN";
        public const string ATTR_SERVER_NAME = "serverName";
        public const string ATTR_DEL_OBJS_CONTAINER = "deletedObjectsContainer";
        public const string ATTR_SUPPORTED_CONTROL = "supportedControl";
        public const string ATTR_SUPPORTED_SASL_MECHANISMS = "supportedSASLMechanisms";
        public const string ATTR_LABELED_URI = "labeledURI";
        public const string ATTR_USER_CERTIFICATE = "userCertificate";
        public const string ATTR_LAST_LOCAL_USN_PROCESSED = "lastLocalUsnProcessed";
        public const string ATTR_REPL_INTERVAL = "replInterval";
        public const string ATTR_UP_TO_DATE_VECTOR = "upToDateVector";
        public const string ATTR_REPL_PAGE_SIZE = "replPageSize";
        public const string ATTR_MODIFYTIMESTAMP = "modifyTimeStamp";
        public const string ATTR_CREATETIMESTAMP = "createTimeStamp";
        public const string ATTR_ENABLED = "enabled";
        public const string ATTR_USER_ACCOUNT_CONTROL = "userAccountControl";
        public const string ATTR_OBJECT_SID = "objectSid";
        public const string ATTR_FSP_OBJECTID = "externalObjectId";
        public const string ATTR_EID_SEQUENCE_NUMBER = "vmwEntryIdSequenceNumber";
        public const string ATTR_USN_SEQUENCE_NUMBER = "vmwUSNSequenceNumber";
        public const string ATTR_DOMAIN_COMPONENT = "dc";

        public const string ATTR_ORGANIZATION = "o";
        public const string ATTR_SERVER_RUNTIME_STATUS = "vmwServerRunTimeStatus";
        public const string ATTR_SITE_ID = "vmwCisSiteId";
        public const string ATTR_DISPLAY_NAME = "displayName";
        public const string ATTR_KRB_PRINCIPAL_NAME = "krbPrincipalName";

        public const string ATTR_KRB_PRINCIPAL_KEY = "krbPrincipalKey";

        public const string ATTR_KRB_UPN = "userPrincipalName";
        public const string ATTR_KRB_SPN = "servicePrincipalName";
        public const string ATTR_KRB_MASTER_KEY = "krbMKey";
        public const string ATTR_GROUPTYPE = "groupType";
        public const string ATTR_NAME = "name";
        public const string ATTR_SAM_ACCOUNT_NAME = "sAMAccountName";
        public const string ATTR_CREATORS_NAME = "creatorsName";
        public const string ATTR_MODIFIERS_NAME = "modifiersName";
        public const string ATTR_SITE_NAME = "msDS-SiteName";
        public const string ATTR_VMW_STS_PASSWORD = "vmwSTSPassword";
        public const string ATTR_VMW_STS_TENANT_KEY = "vmwSTSTenantKey";

        public const string ATTR_SRP_SECRET = "vmwSRPSecret";

        public const string ATTR_SITE_GUID = "siteGUID";


        public const string ATTR_VMW_OBJECT_SECURITY_DESCRIPTOR = "vmwSecurityDescriptor";
        public const string ATTR_VMW_ORGANIZATION_GUID = "vmwOrganizationGuid";

        public const string ATTR_OBJECT_SECURITY_DESCRIPTOR = "nTSecurityDescriptor";
        public const string ATTR_ORG_LIST_DESC = "vmwAttrOrganizationList";
        public const string VDIR_ATTRIBUTE_SEQUENCE_RID = "vmwRidSequenceNumber";

        //Group type int value
        public const int GROUPTYPE_ACCOUNT = 2;

        public const int SERVERTIMEOUT_IN_MILLI = 5000;
    }
}

