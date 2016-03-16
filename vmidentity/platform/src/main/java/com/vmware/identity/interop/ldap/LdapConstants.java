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

/**
 * Created by IntelliJ IDEA.
 * User: mpotapova
 * Date: 2/3/12
 * Time: 1:11 PM
 * To change this template use File | Settings | File Templates.
 */
public final class LdapConstants {

    // note: ensure all constants are valid for all supported platforms!

    public static final int LDAP_OPT_ON = 0x1;
    public static final int LDAP_OPT_OFF = 0x0;

    public static final int LDAP_PORT = 389;
    // Active Directory global catalog port
    public static final int LDAP_GC_PORT = 3268;
    public static final int LDAP_SSL_PORT = 636;
    public static final int LDAP_SSL_GC_PORT = 3269;
    public static final int LDAP_PORT_LOTUS = 389;

    public static final int LDAP_VERSION1 = 1;
    public static final int LDAP_VERSION2 = 2;
    public static final int LDAP_VERSION3 = 3;
    public static final int LDAP_VERSION = LDAP_VERSION2;

    public static final int LDAP_OPT_X_TLS_NEVER = 0;
    public static final int LDAP_OPT_X_TLS_DEMAND = 2;

    public static final int LDAP_OPT_X_TLS_PROTOCOL_TLSv1 = 1;
}
