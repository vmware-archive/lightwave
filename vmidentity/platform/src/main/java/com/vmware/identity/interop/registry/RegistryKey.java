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

import com.sun.jna.Pointer;

/**
 * Created by IntelliJ IDEA.
 * User: snambakam
 * Date: 12/11/11
 * Time: 10:59 PM
 */
public class RegistryKey implements IRegistryKey
{
    private RegistryConnection _connection;
    private Pointer _hKey;
    private RegistryKeyDisposition _disposition;
    
    public 
    RegistryKey(
        RegistryConnection connection,
        Pointer key, RegistryKeyDisposition disposition
        )
    {
        _connection = connection;
        _hKey = key;
        _disposition = disposition;
    }

    public Pointer getKey()
    {
        return _hKey;
    }

    public RegistryKeyDisposition getDisposition()
    {
        return _disposition;
    }

    @Override
    public void close()
    {
        if (_hKey != null)
        {
            assert(_connection != null);

            RegistryAdapter.RegClientLibrary.INSTANCE.LwRegCloseKey(
                                            _connection.getConnection(),
                                            _hKey);
            _hKey = Pointer.NULL;
        }
    }

    protected void finalize() throws Throwable
    {
        try
        {
            close();
        }
        finally
        {
            super.finalize();
        }
    }
}
