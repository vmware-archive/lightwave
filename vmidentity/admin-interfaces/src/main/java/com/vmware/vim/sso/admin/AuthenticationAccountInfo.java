/*
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
 */

package com.vmware.vim.sso.admin;

public class AuthenticationAccountInfo {

    private final String userName;
    private final String spn;
    private final boolean useMachineAccount;

    public AuthenticationAccountInfo(String userName, String spn, Boolean useMachineAccount) {

        this.userName = userName;
        this.spn = spn;
        this.useMachineAccount = useMachineAccount;
    }

    public String getUserName() {
        return this.userName;
    }

    public String getSpn() {
        return this.spn;
    }

    public boolean isUseMachineAccount()
    {
        return this.useMachineAccount;
    }
}
