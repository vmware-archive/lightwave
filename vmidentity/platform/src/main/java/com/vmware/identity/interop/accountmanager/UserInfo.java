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

/**
 * Created by IntelliJ IDEA.
 * User: wfu
 * Date: 03/15/13
 * Time: 10:59 PM
 */
public class UserInfo
{
    public String accountName;
    public String accountSid;
    public boolean bIsLocked;
    public boolean bIsDisabled;

    public UserInfo(String accountName, String accountSid, boolean bIsLocked, boolean bIsDisabled)
    {
        this.accountName = accountName;
        this.accountSid = accountSid;
        this.bIsLocked = bIsLocked;
        this.bIsDisabled = bIsDisabled;
    }
}

