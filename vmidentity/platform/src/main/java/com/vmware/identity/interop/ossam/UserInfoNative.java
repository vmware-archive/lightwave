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

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.win32.W32APITypeMapper;

/**
 * Do not use this class externally.
 * It is intended for internal use by Platform package,
 * but must stay public for interop ...
 */
public class UserInfoNative extends BaseOsSamNativeStructure
{
    static final int Level = 20;

    //typedef struct _USER_INFO_20 {
    //    LPWSTR   usri20_name;
    //    LPWSTR   usri20_full_name;
    //    LPWSTR   usri20_comment;
    //    DWORD    usri20_flags;
    //    DWORD    usri20_user_id;
    //}USER_INFO_20, *PUSER_INFO_20, *LPUSER_INFO_20;

    public String name;
    public String fullName;
    public String comment;
    public int flags;
    public int userId;

    public UserInfoNative()
    {
        super();
    }
    public UserInfoNative(Pointer p)
    {
        super(p);
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "name", "fullName", "comment", "flags", "userId"
        });
    }
}
