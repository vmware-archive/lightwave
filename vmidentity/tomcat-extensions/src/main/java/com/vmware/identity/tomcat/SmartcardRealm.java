/*
 *  Copyright (c) 2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.tomcat;

import org.apache.catalina.realm.GenericPrincipal;
import org.apache.catalina.realm.RealmBase;
import org.ietf.jgss.GSSContext;
import java.security.Principal;
import java.util.Arrays;

/**
 * Customized tomcat security realm for smartcard authentication.
 * Allows tomcat to request client cert but skip authenticating the user.
 * Client cert is requested only for a certain resource/url path.
 *
 */
public class SmartcardRealm extends RealmBase
{
    private static final String ROLE_NAME = "X509";

    public SmartcardRealm()
    {
        super();
        this.setValidate(false);
    }

    @Override
    protected String getName()
    {
        return this.getClass().getSimpleName();
    }

    @Override
    protected String getPassword(String username)
    {
        return null;
    }

    @Override
    public Principal authenticate(String username, String credentials)
    {
        return null;
    }

    @Override
    public Principal authenticate(String username, String clientDigest, String nonce,
        String nc, String cnonce, String qop, String realm, String md5a2)
    {
        return null;
    }

    @Override
    public Principal authenticate(GSSContext gssContext, boolean storeCred)
    {
        return null;
    }

    @Override
    protected Principal getPrincipal(String username)
    {
        return new GenericPrincipal(username, null, Arrays.asList(ROLE_NAME));
    }
}