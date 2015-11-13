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

import com.vmware.identity.interop.accountmanager.IAccountAdapter.ACCOUNT_TYPE;

/**
 * Created by IntelliJ IDEA.
 * User: wfu
 * Date: 03/15/13
 * Time: 10:59 PM
 */
public class AccountInfo
{
    public ACCOUNT_TYPE acctType;
    public String accountName;
    public String accountSid;

    public AccountInfo(ACCOUNT_TYPE acctType, String accountName, String accountSid)
    {
        this.acctType = acctType;
        this.accountName = accountName;
        this.accountSid = accountSid;
    }
}

