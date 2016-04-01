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

package com.vmware.identity.interop.idm;

import com.sun.jna.Platform;

public final class IdmClientLibraryFactory
{
    private static IdmClientLibraryFactory _instance = null;

    public static synchronized IdmClientLibraryFactory getInstance()
    {
        if (_instance == null)
        {
            _instance = new IdmClientLibraryFactory();
        }

        return _instance;
    }

    private IdmClientLibraryFactory()
    {
    }

    public IIdmClientLibrary getLibrary()
    {
        if(Platform.isWindows())
        {
            return WindowsIdmNativeAdapter.getInstance();
        }
        else if (Platform.isLinux())
        {
            return LinuxIdmNativeAdapter.getInstance();
        }
        else
        {
            throw new RuntimeException( "Unsupported Os. The only supported ones are: Windows, Linux." );
        }
    }
}