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

import com.sun.jna.Platform;

/**
 * Created by IntelliJ IDEA.
 * User: mpotapova
 * Date: 2/2/12
 * Time: 1:14 PM
 * To change this template use File | Settings | File Templates.
 */
final class LdapClientLibraryFactory
{
    private static LdapClientLibraryFactory _instance = null;

    public static synchronized LdapClientLibraryFactory getInstance()
    {
        if (_instance == null)
        {
            _instance = new LdapClientLibraryFactory();
        }

        return _instance;
    }

    public ILdapClientLibrary getLdapClientLibrary(boolean useWindowsOpenLdap)
    {
        if(Platform.isWindows())
        {
            if(useWindowsOpenLdap)
                return OpenLdapClientLibrary.getInstance();
            else
                return WinLdapClientLibrary.getInstance();
        }
        else if (Platform.isLinux())
        {
            return OpenLdapClientLibrary.getInstance();
        }
        else
        {
            throw new RuntimeException( "Unsupported Os. The only supported ones are: Windows, Linux." );
        }
    }

    private LdapClientLibraryFactory()
    {}
}
