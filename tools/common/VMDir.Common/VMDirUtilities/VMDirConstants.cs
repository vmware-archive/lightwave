/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
        public const string ATTR_DESCRIPTION = "description";
        public const string ATTR_EID = "entryid";
        public const string ATTR_OBJECT_CLASS = "objectClass";
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
        public const string ATTR_VMW_DSEROOT_DN = "cn=DSE Root";

        //Group type int value
        public const int GROUPTYPE_ACCOUNT = 2;
        public const int SERVERTIMEOUT_IN_MILLI = 5000;
        public const int DEFAULT_PAGE_SIZE = 1000;
        public const int SEARCH_TIMEOUT_IN_SEC = 300;
		public static int MAX_SERVER_PERSIST = 9;

        //ERROR Msg
        public const string ERR_FETCH_DATA = "Unable to fetch data for the object specified. Please ensure the user has access";
        public const string ERR_NULL_CONN = "Ldap Connection is null";
        public const string ERR_LOST_CONN = "Failed to do LDAP Search possibly due to lost connection. Close connection and try again";
        public const string ERR_DN_RETRIEVAL = "Failed to retrieve base dn.";

        //Schema Constants for object class
        public const string SUB_CLASS_OF = "subclassof";
        public const string OBJECT_CLASS_CATEGORY = "objectclasscategory";
        public const string GOVERNS_ID = "governsid";
        public const string SCHEMA_ID_GUID = "schemaidguid";
        public const string DESCRIPTION = "description";
        public const string DEFAULT_OBJECT_CATEGORY = "defaultobjectcategory";
        public const string DEFAULT_OBJECT_CLASS_NAME = "classschema";
        public const string DEFAULT_ATTR_SCHEMA_NAME = "attributeschema";
        public const string ATTR_SINGLE_VALUED = "issinglevalued";
        public const string ATTR_SYNTAX = "attributesyntax";
        public const string ATTR_ID = "attributeid";
        public const string SYSTEM_MUST_CONTAIN = "systemmustcontain";
        public const string SYSTEM_MAY_CONTAIN = "systemmaycontain";
        public const string LDAP_DISPLAY_NAME = "ldapdisplayname";
        public const string SYSTEM_AUX_CLASSES = "systemauxiliaryclass";

        //ERROR STRINGS
        public const string ERR_LDAP_CONNECTION_NULL = "Ldap Connection is null";
        public const string ERR_SCHEMA_CONNECTION_NULL = "Schema Connection is null";
        public const string ERR_SERVER_CONNECTION_LOST = "Connection to Server is lost. Please disconnect and connect again.";
		public const string ERR_LOGIN_FAILED = "Please check your server details and credentials.";

        //Warning Msg
        public const string WRN_ATTR = "Please select valid attribute";
        public const string WRN_COND = "Please select valid condition";
        public const string WRN_VAL = "Please enter value";
        public const string WRN_SEARCH_BASE = "Plese enter search base.";
        public const string WRN_OP = "Plese select operator.";
        public const string WRN_SEARCH_SCOPE = "Please select valid search scope";
        public const string WRN_COND_COUNT = "Please enter atleast one condition";
        public const string WRN_TEXT_FILTER = "Please enter text filter";
        public const string WRN_PAGE_SIZE = "Please Enter Page Size";
        public const string WRN_PAGE_SIZE_MINVAL = "Please Enter Page Size greater than 0.";
        public const string WRN_PAGE_SIZE_MAXVAL = "Please Enter Page Size less than 10000";
        public const string WRN_INT_VAL = "Please Enter integer value.";
        public const string WRN_OC_SEL = "Please select one object class from the list";
        public const string WRN_UPN_ENT = "Please enter UPN.";
        public const string WRN_DN_ENT = "Please enter DN.";
        public const string WRN_PWD_ENT = "Please enter Password.";
        public const string WRN_NEW_PWD_ENT = "Please enter new password.";
        public const string WRN_PWD_NO_MATCH = "Passwords do not match";
        public const string WRN_OBJ_NODE_SEL = "Please select a valid object from the tree view";
        public const string WRN_BASE_DN_ENT = "Please enter base dn.";
        public const string WRN_SERVER_ENT = "Please enter server.";
        public const string WRN_CN_ENT = "Please enter cn.";
        public const string WRN_FN_ENT = "Please enter first name.";
        public const string WRN_LN_ENT = "Please enter last name.";
        public const string WRN_SAM_NAME_ENT = "Please enter sAMAccountName.";
        public const string WRN_GRP_NAME_SEL = "Please select a valid group name";
        public const string WRN_GRP_NAME_ENT = "Please enter a valid group name";
        public const string WRN_NO_MORE_PAGES = "All pages has been fetched.";
        public const string WRN_RELOGIN = "Your session has expired! Please relogin.";
        public const string WRN_SER_NODE_SEL = "Please select a valid sever from the tree view";
        public static string WRN_FILE_FORMAT = "Please select format of file to export";
        public static string WRN_SCOPE = "Please select scope of result to export";
        public const string WRN_SEL_ITEM_PRESENT = "Selected item is already present in the current set of attributes";
        public const string WRN_ITEM_ALRDY_SLCTD = "Item is already selected";
        public static string WRN_RDN_ENT = "Please enter RDN.";

        //Status msg
        public const string STAT_SR_NO_MATCH = "Result: No match Found.";
        public const string STAT_SR_MORE_PG = "Result: There are more pages to be fetched.";
        public const string STAT_SR_NO_MORE_PG = "Result: All pages has been fetched.";
        public const string STAT_SR_FETCHING_PG = "Result: Fetching page from server...";
        public const string STAT_SR_FAILED_PG = "Result: Failed to fetch page from server...";
        public const string STAT_QUERY_LOAD_SUCC = "Query Loaded Successfully";
        public const string STAT_QUERY_STORE_SUCC = "Query Stored Successfully";
        public const string STAT_MEMBER_ADD_SUCC = "Successfully Added Member";
        public const string STAT_OBJ_ADD_SUCC = "Successfully Added Object";
        public const string STAT_GRP_ADD_SUCC = "Successfully Added Group";
        public const string STAT_USR_ADD_SUCC = "Successfully Added User";
        public const string STAT_OBJ_DEL_SUCC = "Successfully deleted object";
        public const string STAT_BASE_OBJ_DEL_SUCC = "Deleted base object. Please Refresh the Server";
        public const string STAT_PWD_RESET_SUCC = "Successfully reset password for the user";
        public const string STAT_RES_EXPO_SUCC = "Successfully exported result";
        public const string STAT_SER_REM_SUCC = "Successfully removed server ";
        public const string STAT_PG_SZ_SUCC = "Successfully set page size ";


        //Search Query
        public const string SEARCH_ALL_OC = "(objectClass=*)";
        public const string USER_OC = "user";
        public const string GROUP_OC = "group";
        public static string[] ConditionList = new string[]
        { "Equal To", "Not Equal To", "Beginning With", "Not Beginning With",
            "Ending With", "Not Ending With", "Containing", "Not Containing", "Greater Than Equal To",
            "Not Greater Than Equal To", "Less Than Equal To", "Not Less Than Equal To"
        };
        public static string[] ScopeList = new[] { "Base Object", "Next Level", "Full Subtree" };
        public static string[] OperatorList = new[] { "AND", "OR" };
        public static string[] ResultExportFormatList = new[] { "csv" };
        public static string[] ResultExportScopeList = new[] { "Current Result Page", "All Fetched Result Pages" };
    }
}

