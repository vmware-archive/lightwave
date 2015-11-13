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

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;

/**
 * Do not use this class externally.
 * It is intended for internal use by Platform package, 
 * but must stay public for interop ...
 */
public final class BerValNative extends Structure
{
    public int     length;
    public Pointer value;

    public BerValNative()
    {
        length = 0;
        value = null;
        write();
    }

    public BerValNative(Pointer p)
    {
        useMemory(p);
        read();
    }

    public BerValNative(int length, Pointer value)
    {
        this.length = length;
        this.value = value;

        write();
    }

    public static BerValNative[] fromPointerArray(Pointer p, int size)
    {
        BerValNative[] result = new BerValNative[size];

        int i = 0;

        for (Pointer pValue : p.getPointerArray(0, size))
        {
            result[i++] = new BerValNative(pValue);
        }

        return result;
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "length", "value"
        });
    }
}
