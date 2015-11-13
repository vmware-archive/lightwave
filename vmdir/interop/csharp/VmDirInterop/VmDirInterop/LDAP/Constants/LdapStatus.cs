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
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace VMDirInterop.LDAPConstants
{
    public enum LdapStatus
    {
        LDAP_SUCCESS = 0,
        //LDAP_OPERATIONS_ERROR = 1,
        //LDAP_PROTOCOL_ERROR = 2,
        LDAP_TIMELIMIT_EXCEEDED = 3,
        LDAP_SIZELIMIT_EXCEEDED = 4,
        //LDAP_COMPARE_FALSE = 5,
        //LDAP_COMPARE_TRUE = 6,
        LDAP_AUTH_METHOD_NOT_SUPPORTED = 7,
        //LDAP_STRONG_AUTH_REQUIRED = 8,
        //LDAP_REFERRAL = 9,
        //LDAP_ADMIN_LIMIT_EXCEEDED = 11,
        //LDAP_UNAVAILABLE_CRITICAL_EXTENSION = 12,
        //LDAP_CONFIDENTIALITY_REQUIRED = 13,
        LDAP_SASL_BIND_IN_PROGRESS = 14,

        LDAP_NO_SUCH_ATTRIBUTE = 16,
        LDAP_UNDEFINED_TYPE = 17,
        //LDAP_INAPPROPRIATE_MATCHING = 18,
        LDAP_CONSTRAINT_VIOLATION = 19,
        LDAP_TYPE_OR_VALUE_EXISTS = 20,
        LDAP_INVALID_SYNTAX = 21,

        LDAP_NO_SUCH_OBJECT = 32,
        //LDAP_ALIAS_PROBLEM = 33,
        LDAP_INVALID_DN_SYNTAX = 34,
        //LDAP_IS_LEAF = 35,
        //LDAP_ALIAS_DEREF_PROBLEM = 36,

        //LDAP_INAPPROPRIATE_AUTH = 48,
        LDAP_INVALID_CREDENTIALS = 49,
        LDAP_INSUFFICIENT_ACCESS = 50,
        LDAP_BUSY = 51,
        LDAP_UNAVAILABLE = 52,
        LDAP_UNWILLING_TO_PERFORM = 53,
        //LDAP_LOOP_DETECT = 54,

        //LDAP_NAMING_VIOLATION = 64,
        LDAP_OBJECT_CLASS_VIOLATION = 65,
        LDAP_NOT_ALLOWED_ON_NONLEAF = 66,
        //LDAP_NOT_ALLOWED_ON_RDN = 67,
        LDAP_ALREADY_EXISTS = 68,
        //LDAP_NO_OBJECT_CLASS_MODS = 69,
        //LDAP_RESULTS_TOO_LARGE = 70,
        //LDAP_AFFECTS_MULTIPLE_DSAS = 71,
        //LDAP_OTHER = 80,


        LDAP_SERVER_DOWN = -1,
        //LDAP_LOCAL_ERROR = -2,
        //LDAP_ENCODING_ERROR = -3,
        //LDAP_DECODING_ERROR = -4,
        //LDAP_TIMEOUT = -5,
        //LDAP_AUTH_UNKNOWN = -6,
        //LDAP_FILTER_ERROR = -7,
        //LDAP_USER_CANCELLED = -8,
        //LDAP_PARAM_ERROR = -9,
        //LDAP_NO_MEMORY = -10,
        //LDAP_CONNECT_ERROR = -11,
        //LDAP_NOT_SUPPORTED = -12,
        //LDAP_CONTROL_NOT_FOUND = -13,
        //LDAP_NO_RESULTS_RETURNED = -14,
        //LDAP_MORE_RESULTS_TO_RETURN = -15,

        //LDAP_CLIENT_LOOP = -16,
        //LDAP_REFERRAL_LIMIT_EXCEEDED = -17,

        VMDIR_SUCCESS = 0,
        //VMDIR_ERROR_OPERATION_NOT_PERMITTED = 9001,
        //VMDIR_ERROR_NO_SUCH_FILE_OR_DIRECTORY = 9002,
        //VMDIR_ERROR_IO = 9003,
        //VMDIR_ERROR_NO_MEMORY = 9004,
        //VMDIR_ERROR_INVALID_PARAMETER = 9005,
        //VMDIR_ERROR_NOT_FOUND = 9006,
        //VMDIR_ERROR_CANNOT_LOAD_LIBRARY = 9007,

        //VMDIR_ERROR_GENERIC=9100,
        //VMDIR_ERROR_INVALID_CONFIGURATION = 9101,
        VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION = 9102,
        //VMDIR_ERROR_BAD_ATTRIBUTE_DATA = 9103,
        //VMDIR_ERROR_INVALID_REALM = 9104,
        //VMDIR_ERROR_PARENT_NOT_FOUND = 9105,
        VMDIR_ERROR_ENTRY_NOT_FOUND = 9106,
        VMDIR_ERROR_ENTRY_ALREADY_EXIST = 9107,
        VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED = 9108,
        VMDIR_ERROR_TIMELIMIT_EXCEEDED = 9109,
        VMDIR_ERROR_SIZELIMIT_EXCEEDED = 9110,
        VMDIR_ERROR_SASL_BIND_IN_PROGRESS = 9111,
        VMDIR_ERROR_BUSY = 9112,
        VMDIR_ERROR_UNAVAILABLE = 9113,
        VMDIR_ERROR_UNWILLING_TO_PERFORM = 9114,
        //VMDIR_ERROR_LOCK_DEADLOCK = 9115,
        //VMDIR_ERROR_NO_USN = 9116,
        VMDIR_ERROR_NOT_ALLOWED_ON_NONLEAF = 9117,
        //VMDIR_ERROR_DOMAIN_NOT_FOUND = 9118,
        //VMDIR_ERROR_NO_SSL_CTX = 9119,
        //VMDIR_ERROR_SSL_CERT_FILE_NOT_FOUND = 9120,
        //VMDIR_ERROR_SSL = 9121,
        //VMDIR_ERROR_SRP = 9122,
        //VMDIR_ERROR_INVALID_REQUEST = 9123,
        //VMDIR_ERROR_INVALID_RESULT = 9124,
        //VMDIR_ERROR_ACCESS_DENIED = 9125,
        //VMDIR_ERROR_CANNOT_CONNECT_VMDIR = 9126,
        VMDIR_ERROR_SERVER_DOWN = 9127,

        //VMDIR_ERROR_RID_LIMIT_EXCEEDED = 9200,
        //VMDIR_ERROR_ORG_ID_GEN_FAILED = 9201,
        //VMDIR_ERROR_NO_OBJECT_SID_GEN = 9202,
        //VMDIR_ERROR_NO_SECURITY_DESCRIPTOR = 9203,
        //VMDIR_ERROR_NO_OBJECTSID_ATTR = 9204,
        //VMDIR_ERROR_TOKEN_IN_USE = 9205,
        //VMDIR_ERROR_NO_MYSELF = 9206,
        VMDIR_ERROR_INSUFFICIENT_ACCESS = 9207,

        //VMDIR_ERROR_PASSWORD_TOO_LONG = 9230,
        //VMDIR_ERROR_PASSWORD_HASH = 9231,
        //VMDIR_ERROR_PASSWORD_POLICY_VIOLATION = 9232,
        //VMDIR_ERROR_USER_LOCKOUT = 9233,
        VMDIR_ERROR_USER_INVALID_CREDENTIAL = 9234,
        //VMDIR_ERROR_KERBEROS_ERROR = 9235,
        //VMDIR_ERROR_KERBEROS_REALM_OFFLINE = 9236,
        //VMDIR_ERROR_NO_CRED_CACHE_NAME = 9237,
        //VMDIR_ERROR_NO_CRED_CACHE_FOUND = 9238,
        //VMDIR_ERROR_PASSWORD_EXPIRED = 9239,
        //VMDIR_ERROR_ACCOUNT_DISABLED = 9240,
        //VMDIR_ERROR_ACCOUNT_LOCKED = 9241,
        //VMDIR_ERROR_INVALID_POLICY_DEFINITION = 9242,
        //VMDIR_ERROR_USER_NO_CREDENTIAL = 9243,

        //VMDIR_ERROR_VDCPROMO = 9270,
        //VMDIR_ERROR_VDCMERGE = 9271,
        //VMDIR_ERROR_VDCSPLIT = 9272,
        //VMDIR_ERROR_VDCREPADMIN_GENERAL = 9273,
        //VMDIR_ERROR_VDCREPADMIN_TOO_FEW_REPLICATION_PARTNERS = 9274,

        //VMDIR_ERROR_NO_SCHEMA = 9600,
        //VMDIR_ERROR_INVALID_SCHEMA = 9601,
        VMDIR_ERROR_INVALID_DN = 9602,
        VMDIR_ERROR_INVALID_SYNTAX = 9603,
        //VMDIR_ERROR_INVALID_ENTRY = 9604,
        //VMDIR_ERROR_INVALID_ATTRIBUTETYPES = 9605,
        //VMDIR_ERROR_INVALID_OBJECTCLASSES = 9606,
        //VMDIR_ERROR_INVALID_DITCONTENTRULES = 9607,
        //VMDIR_ERROR_INVALID_NAMEFORMS = 9608,
        //VMDIR_ERROR_INVALID_DITSTRUCTURERULES = 9609,
        //VMDIR_ERROR_NO_SUCH_SYNTAX = 9610,
        VMDIR_ERROR_NO_SUCH_ATTRIBUTE = 9611,
        //VMDIR_ERROR_NO_SUCH_OBJECTCLASS = 9612,
        //VMDIR_ERROR_NO_SUCH_DITCONTENTRULES = 9613,
        //VMDIR_ERROR_NO_SUCH_NAMEFORMS = 9614,
        //VMDIR_ERROR_NO_SUCH_DITSTRUCTURERULES = 9615,
        VMDIR_ERROR_OBJECTCLASS_VIOLATION = 9616,
        //VMDIR_ERROR_STRUCTURE_VIOLATION = 9617,
        //VMDIR_ERROR_NAMING_VIOLATION = 9618,
        VMDIR_ERROR_TYPE_OR_VALUE_EXISTS = 9619,
        VMDIR_ERROR_UNDEFINED_TYPE = 9620,
        //VMDIR_ERROR_SCHEMA_MISMATCH = 9621,

        //VMDIR_ERROR_BACKEND_ERROR = 9700,
        //VMDIR_ERROR_BACKEND_MAX_RETRY = 9701,
        //VMDIR_ERROR_BACKEND_DEADLOCK = 9702,
        //VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND = 9703,
        //VMDIR_ERROR_BACKEND_PARENT_NOTFOUND = 9704,
        //VMDIR_ERROR_BACKEND_CONSTRAINT = 9705,
        //VMDIR_ERROR_BACKEND_ENTRY_EXISTS = 9706,
        //VMDIR_ERROR_BACKEND_OPERATIONS = 9707,
        //VMDIR_ERROR_BACKEND_ATTR_META_DATA_NOTFOUND = 9708,
    }

    public class LdapError
    {
        public static int VmDirMapLdapError(int retVal)
        {
            switch (retVal)
            {
                case (int)LdapStatus.VMDIR_SUCCESS:
                    return (int)LdapStatus.LDAP_SUCCESS;
                case (int)LdapStatus.VMDIR_ERROR_SERVER_DOWN:
                    return (int)LdapStatus.LDAP_SERVER_DOWN;
                case (int)LdapStatus.VMDIR_ERROR_UNWILLING_TO_PERFORM:
                    return (int)LdapStatus.LDAP_UNWILLING_TO_PERFORM;
                case (int)LdapStatus.VMDIR_ERROR_SASL_BIND_IN_PROGRESS:
                    return (int)LdapStatus.LDAP_SASL_BIND_IN_PROGRESS;
                case (int)LdapStatus.VMDIR_ERROR_ENTRY_ALREADY_EXIST:
                    return (int)LdapStatus.LDAP_ALREADY_EXISTS;
                case (int)LdapStatus.VMDIR_ERROR_INSUFFICIENT_ACCESS:
                    return (int)LdapStatus.LDAP_INSUFFICIENT_ACCESS;
                case (int)LdapStatus.VMDIR_ERROR_NO_SUCH_ATTRIBUTE:
                    return (int)LdapStatus.LDAP_NO_SUCH_ATTRIBUTE;
                case (int)LdapStatus.VMDIR_ERROR_INVALID_SYNTAX:
                    return (int)LdapStatus.LDAP_INVALID_SYNTAX;
                case (int)LdapStatus.VMDIR_ERROR_SIZELIMIT_EXCEEDED:
                    return (int)LdapStatus.LDAP_SIZELIMIT_EXCEEDED;
                case (int)LdapStatus.VMDIR_ERROR_TYPE_OR_VALUE_EXISTS:
                    return (int)LdapStatus.LDAP_TYPE_OR_VALUE_EXISTS;
                case (int)LdapStatus.VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED:
                    return (int)LdapStatus.LDAP_AUTH_METHOD_NOT_SUPPORTED;
                case (int)LdapStatus.VMDIR_ERROR_ENTRY_NOT_FOUND:
                    return (int)LdapStatus.LDAP_NO_SUCH_OBJECT;
                case (int)LdapStatus.VMDIR_ERROR_UNDEFINED_TYPE:
                    return (int)LdapStatus.LDAP_UNDEFINED_TYPE;
                case (int)LdapStatus.VMDIR_ERROR_TIMELIMIT_EXCEEDED:
                    return (int)LdapStatus.LDAP_TIMELIMIT_EXCEEDED;
                case (int)LdapStatus.VMDIR_ERROR_INVALID_DN:
                    return (int)LdapStatus.LDAP_INVALID_DN_SYNTAX;
                case (int)LdapStatus.VMDIR_ERROR_NOT_ALLOWED_ON_NONLEAF:
                    return (int)LdapStatus.LDAP_NOT_ALLOWED_ON_NONLEAF;
                case (int)LdapStatus.VMDIR_ERROR_OBJECTCLASS_VIOLATION:
                    return (int)LdapStatus.LDAP_OBJECT_CLASS_VIOLATION;
                case (int)LdapStatus.VMDIR_ERROR_USER_INVALID_CREDENTIAL:
                    return (int)LdapStatus.LDAP_INVALID_CREDENTIALS;
                case (int)LdapStatus.VMDIR_ERROR_UNAVAILABLE:
                    return (int)LdapStatus.LDAP_UNAVAILABLE;
                case (int)LdapStatus.VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION:
                    return (int)LdapStatus.LDAP_CONSTRAINT_VIOLATION;
                case (int)LdapStatus.VMDIR_ERROR_BUSY:
                    return (int)LdapStatus.LDAP_BUSY;
                default:
                    return retVal;
            }
        }
    }
}
