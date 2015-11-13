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

package com.vmware.identity.interop.accountmanager;

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;

public class LsaUserInfoLevel2Native extends Structure
{
    public LsaUserInfoLevel1Native lsaUserInfo1;

    public int dwDaysToPasswordExpiry;
    public byte bPasswordExpired;
    public byte bPasswordNeverExpires;
    public byte bPromptPasswordChange;
    public byte bUserCanChangePassword;
    public byte bAccountDisabled;
    public byte bAccountExpired;
    public byte bAccountLocked;

    public LsaUserInfoLevel2Native(Pointer p)
    {
        useMemory(p);
        read();
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "lsaUserInfo1",
                "dwDaysToPasswordExpiry", "bPasswordExpired", "bPasswordNeverExpires",
                "bPromptPasswordChange", "bUserCanChangePassword", "bAccountDisabled",
                "bAccountExpired", "bAccountLocked"
        });
    }
}
