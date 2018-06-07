/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.idm.server.test.integration.util;

import java.net.URI;
import java.util.Collection;

import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.server.LdapCertificateValidationSettings;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.provider.ILdapConnectionProvider;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;

public class LdapConnectionProvider implements ILdapConnectionProvider
{
    @Override
    public ILdapConnectionEx getConnection(Collection<URI> uris, String userName, String password,
            AuthenticationType authType, boolean useGcPort,
            LdapCertificateValidationSettings certValidationsettings) throws Exception {
        return ServerUtils.getLdapConnectionByURIs(
            uris,
            userName,
            password,
            authType,
            useGcPort,
            null);
    }
}