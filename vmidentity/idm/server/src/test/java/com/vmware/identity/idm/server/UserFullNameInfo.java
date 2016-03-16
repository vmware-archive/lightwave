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

public class UserFullNameInfo extends BaseOsSamNativeStructure
{
    static final int Level = 1011;
    //typedef struct _USER_INFO_1011 {
    //    LPWSTR usri1011_full_name;
    //} USER_INFO_1011, *PUSER_INFO_1011, *LPUSER_INFO_1011;

    public String fullName;

    public UserFullNameInfo()
    {
        super();
    }

    public UserFullNameInfo(Pointer p)
    {
        super(p);
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "fullName"
        });
    }
}