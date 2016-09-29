/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
 
using System.Collections.Generic;
using VMDirInterop.LDAPConstants;
using System;

namespace VMDir.Common.VMDirUtilities
{
    public static class VMDirError
    {
        public const int UNKNOWN_ERROR = -100;

        public static Dictionary<int, string> ResultCodesTable = new Dictionary<int, string> {
            { (int)LdapStatus.LDAP_SUCCESS       , "The call completed successfully." },
            { -1                           ,  "Request Failed" },

            { (int)LdapStatus.LDAP_OPERATIONS_ERROR       , "Operations error occurred." },
            { (int)LdapStatus.LDAP_PROTOCOL_ERROR        , "Protocol error occurred." }, {
                (int)LdapStatus.LDAP_TIMELIMIT_EXCEEDED      ,
                "Time limit, set by the server side time limit parameter, was exceeded."
            },
            { (int)LdapStatus.LDAP_SIZELIMIT_EXCEEDED        , "Size limit was exceeded." },

            { (int)LdapStatus.LDAP_COMPARE_FALSE       , "The results of a compare operation are false." },
            { (int)LdapStatus.LDAP_COMPARE_TRUE       , "The results of a compare operation are true." }, {
                (int)LdapStatus.LDAP_AUTH_METHOD_NOT_SUPPORTED        ,
                "The client requested an authentication method not supported by the LDAP server."
            }, {
                (int)LdapStatus.LDAP_STRONG_AUTH_REQUIRED      ,
                "The client requested an operation such as delete that requires strong authentication"
            },
            { (int)LdapStatus.LDAP_REFERRAL        , "The server does not hold the target entry of the request" }, {
                (int)LdapStatus.LDAP_ADMIN_LIMIT_EXCEEDED        ,
                "The LDAP server limit set by the administrative authority has been exceeded"
            },
            { (int)LdapStatus.LDAP_UNAVAILABLE_CRITICAL_EXTENSION   , "Unavailable Critical Extension." },
            { (int)LdapStatus.LDAP_CONFIDENTIALITY_REQUIRED        , "Ldap Confidentiality Required" },
            { (int)LdapStatus.LDAP_SASL_BIND_IN_PROGRESS       , "Sasl bind in progress" },

            { (int)LdapStatus.LDAP_NO_SUCH_ATTRIBUTE   , "Requested attribute does not exist." },
            { (int)LdapStatus.LDAP_UNDEFINED_TYPE        , "Type is not defined." },
            { (int)LdapStatus.LDAP_INAPPROPRIATE_MATCHING        , "Type is not defined." },
            { (int)LdapStatus.LDAP_CONSTRAINT_VIOLATION       , "There was a constraint violation." }, { (int)LdapStatus.LDAP_ATTRIBUTE_OR_VALUE_EXISTS           ,
                "The attribute exists or the value has been assigned."
            },

            { (int)LdapStatus.LDAP_INVALID_SYNTAX     , "The syntax is invalid." },
            { (int)LdapStatus.LDAP_NO_SUCH_OBJECT , "The target object cannot be found." },
            { (int)LdapStatus.LDAP_BUSY        , "The server is busy." },
            { (int)LdapStatus.LDAP_OBJECT_CLASS_VIOLATION    , "There was an object class violation." },
            { (int)LdapStatus.LDAP_NAMING_VIOLATION  , "There was a naming violation." }, {
                (int)LdapStatus.LDAP_TIMEOUT      ,
                "The search was aborted due to exceeding the limit of the client side timeout parameter"
            },

            { (int)LdapStatus.LDAP_ALIAS_PROBLEM , "An error occurred when an alias was dereferenced." },
            { (int)LdapStatus.LDAP_INVALID_DN_SYNTAX        , "The syntax of the DN is incorrect. " },
            { (int)LdapStatus.LDAP_IS_LEAF    , "The specified operation cannot be performed on a leaf entry" },
            { (int)LdapStatus.LDAP_ALIAS_DEREF_PROBLEM  , "Ldap Alias Dereferencing Problem" }, {
                (int)LdapStatus.LDAP_INAPPROPRIATE_AUTH      ,
                "Inappropriate authentication"
            },

            { (int)LdapStatus.LDAP_INVALID_CREDENTIALS , "Invalid Credentials" }, {
                (int)LdapStatus.LDAP_INSUFFICIENT_RIGHTS        ,
                "The caller does not have sufficient rights to perform the requested operation."
            },
            { (int)LdapStatus.LDAP_UNAVAILABLE    , "Ldap Server is unavailable" }, {
                (int)LdapStatus.LDAP_UNWILLING_TO_PERFORM  ,
                "The LDAP server cannot process the request because of server-defined restrictions."
            }, {
                (int)LdapStatus.LDAP_LOOP_DETECT      ,
                "The client discovered an alias or referral loop, and is thus unable to complete this request."
            }, {
                (int)LdapStatus.LDAP_NOT_ALLOWED_ON_NONLEAF        ,
                "The requested operation is permitted only on leaf entries."
            }, {
                (int)LdapStatus.LDAP_NOT_ALLOWED_ON_RDN    ,
                "The operation is not allowed on the entry's relative distinguished name."
            }, {
                (int)LdapStatus.LDAP_NO_OBJECT_CLASS_MODS      ,
                "The modify operation attempted to modify the structure rules of an object class."
            },
            { (int)LdapStatus.LDAP_RESULTS_TOO_LARGE , "Requested attribute does not exist." }, {
                (int)LdapStatus.LDAP_AFFECTS_MULTIPLE_DSAS        ,
                "The modify DN operation moves the entry from one LDAP server to another and requires more than one LDAP server."
            },
            { (int)LdapStatus.LDAP_OTHER    , "Unknown error condition." },

            { (int)LdapStatus.LDAP_ALREADY_EXISTS             , "The object already exists." },
            { VMDirError.UNKNOWN_ERROR                   ,"Unknown Error" }

        };

        public static void Check (int resultCode)
        {
            if (resultCode == (int)LdapStatus.LDAP_SUCCESS)
                return;

            string errorString = "";
            if (!ResultCodesTable.TryGetValue ((int)resultCode, out errorString)) {
                System.Diagnostics.Debug.WriteLine ("Unknown Return value " + resultCode);
                return;
            }
            throw new Exception (errorString);
        }
    }
}

