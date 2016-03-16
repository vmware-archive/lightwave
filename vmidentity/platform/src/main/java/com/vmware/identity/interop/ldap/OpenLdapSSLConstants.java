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

final class OpenLdapSSLConstants {

   static final int SSL_VERIFY_PEER = 0x01;
   // vmdir/thirdparty/openldap/include/ldap.h
   static final int LDAP_OPT_X_TLS_PROTOCOL_SSL3 = (3 << 8);
   static final int LDAP_OPT_X_TLS_PROTOCOL_TLS1_0 = ((3 << 8) + 1);
   static final int LDAP_OPT_X_TLS_PROTOCOL_TLS1_1 = ((3 << 8) + 2);
   static final int LDAP_OPT_X_TLS_PROTOCOL_TLS1_2 = ((3 << 8) + 3);
}
