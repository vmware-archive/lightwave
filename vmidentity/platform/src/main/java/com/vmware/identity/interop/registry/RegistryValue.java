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

package com.vmware.identity.interop.registry;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collection;

/**
 * Created by IntelliJ IDEA.
 * User: snambakam
 * Date: 12/12/11
 * Time: 2:49 AM
 * To change this template use File | Settings | File Templates.
 */
public class RegistryValue
{
    private RegistryValueType _type;
    private byte[]            _value;
    private int               _length;
    
    public RegistryValue(RegistryValueType type)
    {
        _type = type;
    }
    
    public RegistryValue(RegistryValueType type, byte[] value, int length)
    {
        _type   = type;
        _value  = value;
        _length = length;
    }
    
    public RegistryValueType getType()
    {
        return _type;
    }

    public byte[] getValue()
    {
        return _value;
    }
    
    public int getLength()
    {
        return _length;
    }
    
    public static
    RegistryValue
    build(String rawValue)
    {
    	byte[] bytes = null;
    	
    	if (rawValue != null)
    	{
    		ByteBuffer byteBuffer = ByteBuffer.allocate(rawValue.length() + 1);
    		
    		byteBuffer.put(rawValue.getBytes(), 0, rawValue.length());
    		
    		byteBuffer.position(rawValue.length());
    		
    		byteBuffer.put((byte)0x0);
    		
    		bytes = byteBuffer.array();
    	}

        return new RegistryValue(
                        RegistryValueType.REG_SZ,
                        bytes,
                        bytes != null ? bytes.length : 0);
    }

    public static
    RegistryValue
    build(int rawValue)
    {
        ByteBuffer buf = ByteBuffer.allocate(4);

        buf.putInt(rawValue);
        
        byte[] bytes = buf.array();

        return new RegistryValue(
                        RegistryValueType.REG_DWORD,
                        bytes,
                        bytes != null ? bytes.length : 0);
    }

    public static
    RegistryValue
    build(byte[] bytes)
    {
        return new RegistryValue(
                        RegistryValueType.REG_BINARY,
                        bytes,
                        bytes != null ? bytes.length : 0);
    }
    
    public static
    RegistryValue
    build(Collection<String> values)
    {
        byte[] bytes = RegistryValue.getBytes(values);

        return new RegistryValue(
                        RegistryValueType.REG_MULTI_SZ,
                        bytes,
                        bytes != null ? bytes.length : 0);
    }

    public static
    byte[]
    getBytes(Collection<String> values)
    {
        byte[] bytes = null;

        if (values != null)
        {
            int totalLength = 0;

            for (String value : values)
            {
                totalLength += value.length() + 1;
            }

            totalLength++;

            bytes = new byte[totalLength];

            int idx = 0;

            for (String value : values)
            {
                for (byte b : value.getBytes())
                {
                    bytes[idx++] = b;
                }

                bytes[idx++] = 0;
            }

            bytes[idx++] = 0;
        }

        return bytes;
    }

    public static Collection<String>
    getStrings(byte[] value)
    {
        ArrayList<String> valueList = null;

        if ( ( value != null ) && (value.length > 0) )
        {
            valueList = new ArrayList<String>();

            StringBuilder sb = new StringBuilder();
            boolean previousCharZero = false;
    
            for (int idx = 0; idx < value.length; idx++)
            {
                if (value[idx] == 0)
                {
                    if(previousCharZero)
                    {
                        // \0\0 terminates the list....
                        break;
                    }

                    if (sb.length() > 0)
                    {
                        valueList.add(sb.toString());
    
                        sb = new StringBuilder();
                    }

                    previousCharZero = true;
                }
                else
                {
                    previousCharZero = false;
                    sb.append((char)value[idx]);
                }
            }
        }

        return valueList;
    }
}
