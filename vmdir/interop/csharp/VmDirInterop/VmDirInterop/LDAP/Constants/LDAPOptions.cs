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

namespace VMDirInterop
{
    public class LDAPOption
    {
        public const int LDAP_OPT_API_INFO = 0x00;
        public const int LDAP_OPT_DESC = 0x01;
        public const int LDAP_OPT_DEREF = 0x02;
        public const int LDAP_OPT_SIZELIMIT = 0x03;
        public const int LDAP_OPT_TIMELIMIT = 0x04;

        public const int LDAP_OPT_REFERRALS = 0x0008;
        public const int LDAP_OPT_RESTART = 0x09;

        public const int LDAP_OPT_PROTOCOL_VERSION = 0x0011;
        public const int LDAP_OPT_SERVER_CONTROLS = 0x0012;
        public const int LDAP_OPT_CLIENT_CONTROLS = 0x0013;
        public const int LDAP_OPT_API_FEATURE_INFO = 0x15;

        public const int LDAP_OPT_HOST_NAME = 0x30;
        public const int LDAP_OPT_RESULT_CODE = 0x31;
        public const int LDAP_OPT_DIAGNOSTIC_MESSAGE = 0x32;

        public const int LDAP_OPT_SIGN = 0x0095;
        public const int LDAP_OPT_ENCRYPT = 0x0096;
        public const int LDAP_OPT_NETWORK_TIMEOUT = 0x5005;

        public const int LDAP_OPT_X_TLS_REQUIRE_CERT = 0x6006;

        public const int LDAP_VERSION = 0x03;
        public const int LDAP_PORT = 389;

    }
}
