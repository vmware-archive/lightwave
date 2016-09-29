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


using System;

namespace RestSsoAdminSnapIn
{
    public  static class VMIdentityConstants
    {
        /*dialog result codes */
        public const int DIALOGOK = 1;
        public const int DIALOGCANCEL = -1;

        /* Certificate Constants */
        public const string CERT_ISSUED_BY = "IssuedBy";
        public const string CERT_ISSUED_DATE = "IssuedDate";
        public const string CERT_EXPIRATION_DATE = "ExpirationDate";
        public const string CERT_INTENDED_PURPOSES = "IntendedPurposes";
        public const string CERT_STATUS = "Status";
        public const string CERT_ISSUED_TO = "IssuedTo";

		/* External Domain constants */
		public const string AD_WIN_AUTH_TITLE = "External Domain (AD using Windows Integrated Auth)";
		public const string AD_AS_LDAP_TITLE = "External Domain (AD as an LDAP server)";
		public const string OPEN_LDAP_TITLE = "External Domain (Open LDAP server)";
		public const string NEW_EXTERNAL_DOMAIN_TITLE = "Add External Domain";
    }
}

