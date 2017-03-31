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
using System.Runtime.InteropServices;
using VMDirInterop.LDAPExceptions;
using VMDirInterop.LDAPConstants;

namespace VMDirInterop.LDAP
{
    public static class ErrorCheckerHelper
    {
        public static void Validate(int error)
        {
            if (error != (int)LdapStatus.LDAP_SUCCESS)
            {
                var errorPointer = LdapClientLibrary.ldap_err2string(error);
                var errorString = Marshal.PtrToStringAnsi(errorPointer);
                var message = string.Format("{0}-{1} (error code = {2})", "Exception thrown from LDAP", errorString, error);
                var exception = new LdapException(message);
                exception.LdapError = (LdapStatus)Enum.Parse(typeof(LdapStatus), error.ToString(), false);
                throw exception;
            }
        }

		public static string ErrorCodeToString(int error)
		{
			if (error != (int)LdapStatus.LDAP_SUCCESS)
			{
				var errorPointer = LdapClientLibrary.ldap_err2string(error);
				var errorString = Marshal.PtrToStringAnsi(errorPointer);
				return errorString;
			}
			return error.ToString ();
		}
    }
}
