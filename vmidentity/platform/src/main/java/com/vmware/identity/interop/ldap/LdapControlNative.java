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

import org.apache.commons.lang.SystemUtils;

import com.sun.jna.Structure;
import com.sun.jna.TypeMapper;
import com.sun.jna.win32.W32APITypeMapper;

public final class LdapControlNative extends Structure
{
    public String          oid;
    public BerValNative    value;
    public char            isCritical;

    /**
     * @deprecated  replaced by {@link #LdapControlNative(String, BerValNative, char, TypeMapper) LdapControlNative}
     * This constructor is used to be backward compatible with other components, like inventory, who are depending on this constructor
     * @param oid
     * @param value
     * @param isCritical
     */
    @Deprecated
    public
    LdapControlNative( String oid, BerValNative value, char isCritical)
    {
        this(oid, value, isCritical, (SystemUtils.IS_OS_WINDOWS)? W32APITypeMapper.UNICODE: null);
    }

    public
    LdapControlNative( String oid, BerValNative value, char isCritical, TypeMapper mapper)
    {
        super(mapper);

        this.oid = oid;
        this.value = value;
        this.isCritical = isCritical;
        write();
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "oid", "value", "isCritical"
        });
    }
}
