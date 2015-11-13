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

public class LocalGroupInfo1 extends BaseOsSamNativeStructure
{
    static final int Level = 1;
    //typedef struct _LOCALGROUP_INFO_1 {
    //    LPWSTR lgrpi1_name;
    //    LPWSTR lgrpi1_comment;
    //} LOCALGROUP_INFO_1, *PLOCALGROUP_INFO_1, *LPLOCALGROUP_INFO_1;

    public String name;
    public String comment;

    public LocalGroupInfo1()
    {
        super();
    }

    public LocalGroupInfo1(Pointer p)
    {
        super(p);
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "name", "comment"
        });
    }
}
