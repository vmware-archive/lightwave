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

package com.vmware.identity.interop.registry;

import com.sun.jna.Platform;

/**
 * Created by IntelliJ IDEA.
 * User: mpotapova
 * Date: 1/5/12
 * Time: 2:13 PM
 * To change this template use File | Settings | File Templates.
 */
public final class RegistryAdapterFactory {

    private static RegistryAdapterFactory _instance = null;

    public static synchronized RegistryAdapterFactory getInstance()
    {
        if (_instance == null)
        {
            _instance = new RegistryAdapterFactory();
        }

        return _instance;
    }

    public IRegistryAdapter getRegistryAdapter()
    {
        if(Platform.isWindows())
        {
            return WinRegistryAdapter.getInstance();
        }
        else if (Platform.isLinux())
        {
            return new LinuxRegistryAdapter();
        }
        else
        {
            throw new RuntimeException( "Unsupported Os. The only supported ones are: Windows, Linux." );
        }
    }

    private RegistryAdapterFactory()
    {}
}
