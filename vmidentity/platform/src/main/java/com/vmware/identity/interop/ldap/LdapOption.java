/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.interop.ldap;

public enum LdapOption
{
    // note: ensure all options are valid for all supported platforms!

    LDAP_OPT_API_INFO(0x00),
    LDAP_OPT_DESC(0x01),
    LDAP_OPT_DEREF(0x02),
    LDAP_OPT_SIZELIMIT(0x03),
    LDAP_OPT_TIMELIMIT(0x04),

    LDAP_OPT_REFERRALS(0x0008),
    LDAP_OPT_RESTART(0x09),

    LDAP_OPT_PROTOCOL_VERSION(0x0011),
    LDAP_OPT_SERVER_CONTROLS(0x0012),
    LDAP_OPT_CLIENT_CONTROLS(0x0013),
    LDAP_OPT_API_FEATURE_INFO(0x15),

    LDAP_OPT_HOST_NAME(0x30),
    LDAP_OPT_RESULT_CODE(0x31),
    LDAP_OPT_DIAGNOSTIC_MESSAGE(0x32),

    LDAP_OPT_SIGN(0x0095),
    LDAP_OPT_ENCRYPT(0x0096),
    LDAP_OPT_NETWORK_TIMEOUT(0x5005),

    LDAP_OPT_X_TLS_REQUIRE_CERT(0x6006),

    // Open ldap option to DISABLE the reverse DNS lookup for SASL binding.
    LDAP_OPT_X_SASL_NOCANON(0x610b),

    //option code 0x7000 ~ 0x7fff is the OpenLdap option code range reserved for application use
    //and it is not reserved value on winldap.h, as can be found here:
    //http://sourceforge.net/p/mingw/mingw-org-wsl/ci/master/tree/include/winldap.h#l360
    LDAP_OPT_X_CLIENT_TRUSTED_FP_CALLBACK(0x7100),       //input
    LDAP_OPT_X_TLS_PROTOCOL(0x7101);


    private int _code;

    private LdapOption(int code)
    {
       _code = code;
    }

    public int getCode()
    {
       return _code;
    }
}
