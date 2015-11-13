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

import org.apache.commons.lang.Validate;

/**
 * This class encapsulate an {@code LdapOption} and an Object
 * as the value for the option.
 */
public class LdapSetting
{
    private final LdapOption option;
    private final Object     value;

    /**
     *
     * @param anOption  cannot be null
     * @param aValue    can be null, depends on the {@code LdapOption} and check will
     *                  be enforced when the setting is set to the LDAP client.
     */
    public LdapSetting(LdapOption anOption, Object aValue)
    {
        Validate.notNull(anOption);
        option = anOption;
        value = aValue;
    }

    LdapOption getLdapOption()
    {
        return option;
    }

    Object getValue()
    {
        return value;
    }
}
