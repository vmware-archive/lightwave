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
 * User: mpotapova
 * Date: 1/5/12
 * Time: 11:53 AM
 * To change this template use File | Settings | File Templates.
 */
public interface IAccountAdapter {
    public enum ACCOUNT_TYPE
    {
        USER(0),
        GROUP(1),
        OTHER(2);

        private final int _code;

        private ACCOUNT_TYPE(int code)
        {
           _code = code;
        }

        public int getCode()
        {
           return _code;
        }
    }

    /* Authenticate a user @ domainName with password */
    public boolean authenticate(String userPrincipalName, String password);

    /* Look up an account by its objectSid, the netbios name of the account is returned*/
    public String lookupByObjectSid(String objectSid);

    public AccountInfo lookupByName(String name);

    public UserInfo findUserByName(String samAccountName, String domainName);

    public MachineAccountInfo getMachineAccountInfo();
}
