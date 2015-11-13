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

import java.net.URI;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.commons.lang.Validate;

import com.vmware.identity.interop.PlatformUtils;


/**
 * Created by IntelliJ IDEA.
 * User: mpotapova
 * Date: 2/2/12
 * Time: 3:01 PM
 * To change this template use File | Settings | File Templates.
 */
//public class LdapConnectionFactoryEx {
//}
public final class LdapConnectionFactoryEx
{
    private static LdapConnectionFactoryEx _instance = null;

    public static synchronized LdapConnectionFactoryEx getInstance()
    {
        if (_instance == null)
        {
            _instance = new LdapConnectionFactoryEx();
        }

        return _instance;
    }

    /**
     * Create a LDAP connection with the specified host & port parameter
     * @param host  cannot be null
     * @param port  positive number
     * @param connOptions connection options, cannot be null
     * @return the LDAP scheme connection.
     */
    public ILdapConnectionEx getLdapConnection( String host, int port, List<LdapSetting> connOptions)
    {
        Validate.notNull(host);
        Validate.isTrue(port > 0);

        return getLdapConnection(
                PlatformUtils.getConnectionUriDefaultScheme(host, port),
                connOptions);
    }

    /**
     * Create a LDAP connection with the specified host & port parameter
     * @param host  cannot be null
     * @param port  positive number
     * @return the LDAP scheme connection.
     */
    public ILdapConnectionEx getLdapConnection( String host, int port)
    {
        Validate.notNull(host);
        Validate.isTrue(port > 0);
        // keep the implementation unchanged for external client such as lookup service
        // and STS installer by providing empty connection option for external request
        List<LdapSetting> connOptions = Collections.emptyList();

        return getLdapConnection(
                PlatformUtils.getConnectionUriDefaultScheme(host, port),
                connOptions);
    }

    /**
     * Create a LDAP connection with the specified uri & connection options
     * @param uri           cannot be null
     * @param connOptions connection options, cannot be null
     * @return
     */
    public ILdapConnectionEx getLdapConnection(URI uri, List<LdapSetting> connOptions)
    {
        return new LdapConnection(uri, connOptions);
    }

    private LdapConnectionFactoryEx()
    {}
}

