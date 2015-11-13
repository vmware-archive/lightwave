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

/**
 * Created by IntelliJ IDEA.
 * User: krishnag
 * Date: 12/19/11
 * Time: 2:13 PM
 * To change this template use File | Settings | File Templates.
 */

public enum LdapBindMethod
{
    LDAP_BIND_SIMPLE(0x80),
    LDAP_AUTH_NEGOTIATE(0x86 | 0x400);

    private int _code;

    private LdapBindMethod(int code)
    {
        _code = code;
    }

    public int getCode()
    {
        return _code;
    }
}
