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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.lang.SystemUtils;

import com.sun.jna.Memory;
import com.sun.jna.Platform;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.win32.W32APITypeMapper;
import com.vmware.identity.interop.ossam.LwApiTypeMapper;

public class UserInfoNative extends Structure {

    public String  pszUsername;
    public int     dwNumGroups;
    public Pointer ppGroupNames;
    public Pointer ppGroupSids;
    public String  pszUserSid;

    protected UserInfoNative()
    {
        super(SystemUtils.IS_OS_WINDOWS ? W32APITypeMapper.UNICODE : LwApiTypeMapper.UNICODE);
    }

    public UserInfoNative(Pointer p)
    {
        this();
        useMemory(p);
        read();
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
    protected List<String> getFieldOrder() {
        return Arrays.asList(
                new String[] { "pszUsername", "dwNumGroups", "ppGroupNames", "ppGroupSids", "pszUserSid" });
    }

    public UserInfo getUserInfo()
    {
        List<String> groups = new ArrayList<String>();
        List<String> groupSids = new ArrayList<String>();

        if (dwNumGroups > 0)
        {
            for (Pointer pValue : ppGroupNames.getPointerArray(0, dwNumGroups))
            {
                if (pValue != Pointer.NULL)
                {
                    String groupName =  Platform.isWindows() ?
                                            pValue.getString(0, true) :
                                            LwApiTypeMapper.getString(pValue);
                    if (groupName != null && !groupName.trim().isEmpty())
                    {
                        groups.add(groupName);
                    }
                }
            }
        }

        if (dwNumGroups > 0)
        {
            for (Pointer pValue : ppGroupSids.getPointerArray(0, dwNumGroups))
            {
                if (pValue != Pointer.NULL)
                {
                    String groupSid =  Platform.isWindows() ?
                                            pValue.getString(0, true) :
                                            LwApiTypeMapper.getString(pValue);
                    if (groupSid != null && !groupSid.trim().isEmpty())
                    {
                        groupSids.add(groupSid);
                    }
                }
            }
        }

        String[] parts = pszUsername.split("@");

        return new UserInfo(parts[0], parts[1], groups, groupSids, pszUserSid);
    }
}
