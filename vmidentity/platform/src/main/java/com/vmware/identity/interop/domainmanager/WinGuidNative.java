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

package com.vmware.identity.interop.domainmanager;

import java.util.Arrays;
import java.util.List;

import org.apache.commons.lang.SystemUtils;

import com.sun.jna.Structure;
import com.sun.jna.win32.W32APITypeMapper;


public class WinGuidNative extends Structure
{
    public int data1;
    public short data2;
    public short data3;
    public byte[] data4;

    public WinGuidNative()
    {
        super(SystemUtils.IS_OS_WINDOWS ? W32APITypeMapper.UNICODE : null);
        data4 = new byte[WinDomainAdapter.WIN_GUID_DATA4_SIZE];
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "data1", "data2", "data3", "data4"
        });
    }
}



