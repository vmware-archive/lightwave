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

import java.nio.ByteBuffer;

import com.vmware.identity.interop.PlatformUtils;
import com.vmware.identity.interop.Validate;

public class LdapValue
{
    private final byte[] bytes;

    public LdapValue(byte[] bytes)
    {
        this.bytes = bytes;
    }

    public LdapValue(String value)
    {
        this((value != null) ? value.getBytes(PlatformUtils.getLdapServerCharSet()) : null);
    }

    public LdapValue(int value)
    {
        this( Integer.toString(value) );
    }

    public LdapValue(long value)
    {
        this( Long.toString(value) );
    }

    public LdapValue(Integer value)
    {
        this((value != null) ? value.toString() : null);
    }

    public boolean isEmpty()
    {
    	return bytes == null || bytes.length == 0;
    }

	public byte[] getValue()
	{
		return bytes;
	}

    public static LdapValue fromString(String value)
    {
        Validate.validateNotEmpty(value, "value");
        return new LdapValue(value);
    }

    @Override
    public String toString()
    {
        return this.getString();
    }

    public String getString()
	{
		String value = null;

		if (bytes != null && bytes.length > 0)
		{
			ByteBuffer bb = ByteBuffer.allocate(bytes.length);

			bb.put(bytes);

			value = new String(bb.array(), PlatformUtils.getLdapServerCharSet());
		}

		return value;
	}

    public Integer getInteger()
    {
        Integer value = null;
        String strValue = this.toString();
        if ((strValue != null) && (strValue.isEmpty() == false) )
        {
            value = new Integer( strValue );
        }

        return value;
    }

    public int getInt()
    {
        Integer value = this.getInteger();
        if(value == null)
        {
            throw new IllegalArgumentException("Integer value is not available.");
        }
        return value.intValue();
    }

    public Long getLong()
    {
        Long value = null;
        String strValue = this.toString();
        if ((strValue != null) && (strValue.isEmpty() == false) )
        {
            value = new Long( strValue );
        }

        return value;
    }

    public long getNativeLong()
    {
        Long value = this.getLong();
        if(value == null)
        {
            throw new IllegalArgumentException("Integer value is not available.");
        }
        return value.longValue();
    }

}
