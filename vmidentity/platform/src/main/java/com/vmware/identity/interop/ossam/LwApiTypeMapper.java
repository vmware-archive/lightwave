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

package com.vmware.identity.interop.ossam;

import com.sun.jna.*;
import com.vmware.identity.interop.NativeMemory;

import java.io.UnsupportedEncodingException;

public class LwApiTypeMapper extends DefaultTypeMapper
{
    public static final TypeMapper UNICODE = new LwApiTypeMapper();

    private static final String ENCODING = "UTF-16LE";

    private LwApiTypeMapper()
    {
        TypeConverter stringConverter = new TypeConverter() {
            @Override
            public Object fromNative(Object o, FromNativeContext fromNativeContext)
            {
                String str = null;
                if ( o instanceof Pointer)
                {
                    str = LwApiTypeMapper.getString( (Pointer)o );
                }

                return str;
            }

            @Override
            public Class nativeType() {
                return Pointer.class;
            }

            @Override
            public Object toNative(Object o, ToNativeContext toNativeContext)
            {
                Pointer res = null;
                if( o instanceof String )
                {
                    res = LwApiTypeMapper.getPointer( o.toString() );
                }
                return res;
            }
        };

        this.addTypeConverter( String.class, stringConverter );

    }

    private static Pointer getPointer( String string )
    {
        //  Right now this code relies on java's GC
        // ideally we should be able to free this explicitly once the pointer is no longer needed
        // however with the current type mapper code it is unclear when can we call an explicit dispose ...
        // This code is to used at the moment though
        NativeMemory res = null;
        if( string != null )
        {
            byte[] array = null;
            try
            {
                array = string.toString().getBytes( ENCODING );
            }
            catch(UnsupportedEncodingException ex)
            {
                throw new RuntimeException(ex.getMessage(), ex);
            }
            res = new NativeMemory(array.length + 2); // + null terminator
            res.write(0, array, 0, array.length);
            res.setByte( array.length, (byte)0x0 );
            res.setByte( array.length+1, (byte)0x0 );
        }

        return res;
    }

    public static String getString(Pointer p)
    {
        String str = null;
        if( ( p != null ) &&( p != Pointer.NULL ) )
        {
            // find the \0\0
            int bufLen = -1;
            for (int i = 0; i < Integer.MAX_VALUE/2; i++)
            {
                byte[] next2Bytes = p.getByteArray(i*2, 2);
                if (next2Bytes[0] == 0 && next2Bytes[1] == 0)
                {
                    // Reached end of string
                    bufLen = i*2;
                    break;
                }
            }

            if ( ( bufLen == -1 ) || (bufLen > Integer.MAX_VALUE) )
            {
                throw new RuntimeException("Invalid native string.");
            }
            else
            {
                if(bufLen == 0)
                {
                    str = "";
                }
                else
                {
                    byte[] chars = p.getByteArray( 0 , bufLen );
                    try
                    {
                        str = new String(chars, ENCODING);
                    }
                    catch(UnsupportedEncodingException ex)
                    {
                        throw new RuntimeException( ex.getMessage(), ex);
                    }
                }
            }
        }

        return str;
    }
}
