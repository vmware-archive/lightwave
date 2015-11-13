/*
 *
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 *
 */

package com.vmware.identity.idm.server;

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Pointer;
import com.vmware.identity.interop.ossam.BaseOsSamNativeStructure;

public class UserInfo1 extends BaseOsSamNativeStructure
{
    static final int Level = 1;
    //typedef struct _USER_INFO_1 {
    //    LPWSTR usri1_name;
    //    LPWSTR usri1_password;
    //    DWORD  usri1_password_age;
    //    DWORD  usri1_priv;
    //    LPWSTR usri1_home_dir;
    //    LPWSTR usri1_comment;
    //    DWORD  usri1_flags;
    //    LPWSTR usri1_script_path;
    //} USER_INFO_1, *PUSER_INFO_1, *LPUSER_INFO_1;

    public String name;
    public String password;
    public int passwordAge;
    public int privs;
    public String homeDir;
    public String comment;
    public int flags;
    public String scriptPath;

    public UserInfo1()
    {
        super();
    }

    public UserInfo1(Pointer p)
    {
        super(p);
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "name", "password", "passwordAge", "privs",
                "homeDir", "comment", "flags", "scriptPath"
        });
    }
}
