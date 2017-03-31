/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */
package com.vmware.identity.interop.ldap;

public enum LdapSSLProtocols {
    SSLv3(1, "SSLv3"),
    TLSv1_0(2, "TLSv1"),
    TLSv1_1(4, "TLSv1.1"),
    TLSv1_2(8,"TLSv1.2");

    private String name;
    private int code;

    private LdapSSLProtocols(int code, String name) {
        this.code = code;
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public int getCode() {
        return code;
    }

    public boolean isHigher(LdapSSLProtocols protocol) {
        return (this.code < protocol.getCode());
    }

    public static LdapSSLProtocols getDefaultLegacyMinProtocol() {
        return LdapSSLProtocols.SSLv3;
    }

    public static LdapSSLProtocols getDefaultMinProtocol() {
        return LdapSSLProtocols.TLSv1_0;
    }

    public static LdapSSLProtocols getProtocolByCode(int protocolCode) {
        LdapSSLProtocols[] allProtocols = LdapSSLProtocols.values();
        for (LdapSSLProtocols ldapSSLProtocol : allProtocols) {
            if (ldapSSLProtocol.getCode() == protocolCode)
                return ldapSSLProtocol;
        }

        return getDefaultMinProtocol();
    }
}
