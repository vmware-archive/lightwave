/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.certificate;

import java.io.IOException;

import com.sun.jna.Pointer;

public class VMCAServerContext implements AutoCloseable {

    private String  _networkAddress;
    private Pointer _pContext = Pointer.NULL;

    public VMCAServerContext(String networkAddress, Pointer pContext)
    {
        _networkAddress = networkAddress;
        _pContext = pContext;
    }

    public String getNetworkAddress()
    {
        return _networkAddress;
    }

    public Pointer getContext()
    {
        return _pContext;
    }

    @Override
    public void close() throws IOException {
        dispose();
    }

    @Override
    protected void finalize() throws Throwable
    {
        try
        {
            dispose();
        }
        finally
        {
            super.finalize();
        }
    }

    protected void dispose()
    {
        if (_pContext != Pointer.NULL)
        {
            VMCAAdapter2.VMCA.INSTANCE.VMCACloseServer(_pContext);
            _pContext = Pointer.NULL;
        }
    }
}
