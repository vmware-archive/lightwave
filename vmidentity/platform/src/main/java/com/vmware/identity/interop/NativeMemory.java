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
package com.vmware.identity.interop;

import com.sun.jna.Memory;

/**
 * Do not use outside of platforms package.
 *
 * Provides a deterministic memory free on top of com.sun.jna.Memory class.
 * (I.e. IDisposable pattern.)
 * ensure to call .close() after the memory id no longer needed.
 */
public class NativeMemory extends Memory implements AutoCloseable
{
    public NativeMemory(int length)
    {
        super(length);
    }

    @Override
    protected void finalize()
    {
        this.dispose();
    }

    @Override
    protected void dispose()
    {
        if ( super.valid() )
        {
            super.dispose();
        }
    }

    @Override
    public void close()
    {
        this.dispose();
    }
}
