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

import com.sun.jna.Structure;

public final class TimevalNative extends Structure
{
    public long tv_sec;
    public long tv_usec;

    public
    TimevalNative( long tv_sec, long tv_usec )
    {
        this.tv_sec = tv_sec;
        this.tv_usec = tv_usec;
        write();
    }

    public
    String toString()
    {
        return String.format("Timeval - tv_sec %d, tc_usec %d", tv_sec, tv_usec);
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "tv_sec", "tv_usec"
        });
    }
}
