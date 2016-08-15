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
package com.vmware.identity.interop.ossam;

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Memory;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.win32.W32APITypeMapper;
import org.apache.commons.lang.SystemUtils;

public class BaseOsSamNativeStructure extends Structure
{
    protected BaseOsSamNativeStructure()
    {
        super(SystemUtils.IS_OS_WINDOWS ? W32APITypeMapper.UNICODE : LwApiTypeMapper.UNICODE);
    }

    protected BaseOsSamNativeStructure(Pointer p)
    {
        this();
        this.useMemory(p);
        this.read();
    }

    @Override
    protected int getNativeAlignment(Class type, Object value, boolean isFirstElement)
    {
        if ( (type == Memory.class) && (SystemUtils.IS_OS_LINUX) )
        {
            // memory derives from pointer, but somehow the base implementation misses
            // this fact and believes alignment unknown ,,,,
            return super.getNativeAlignment( Pointer.class, null, isFirstElement );
        }
        else
        {
            return super.getNativeAlignment( type, value, isFirstElement );
        }
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
        });
    }
}
