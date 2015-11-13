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

/**
 * Created by IntelliJ IDEA.
 * User: snambakam
 * Date: 12/12/11
 * Time: 2:26 AM
 * To change this template use File | Settings | File Templates.
 */
public enum RegistryValueType
{
     REG_NONE(0),
     REG_SZ(1),                          // Unicode null terminated string
     REG_EXPAND_SZ(2),                   // hex(2): (Not supported)
     REG_BINARY(3),                      // hex:
     REG_DWORD(4),                       // dword
     REG_DWORD_LITTLE_ENDIAN(4),         // 32-bit number
     REG_DWORD_BIG_ENDIAN(5),            // 32-bit number (Not supported)
     REG_LINK(6),                        // hex(7): (Not supported)
     REG_MULTI_SZ(7),                    // Multiple Unicode strings
     REG_RESOURCE_LIST(8),               // hex(8): (Not supported)
     REG_FULL_RESOURCE_DESCRIPTOR(9),    // hex(9): (Not supported)
     REG_RESOURCE_REQUIREMENTS_LIST(10), // hex(a): (Not supported)
     REG_QWORD(11),                      // hex(b): (Not supported)
     REG_QWORD_LITTLE_ENDIAN(11);        // hex(b):
    
    private int _code;

    private RegistryValueType(int code)
    {
        _code = code;
    }
    
    public int getCode()
    {
        return _code;
    }

    public static
    RegistryValueType
    parse(
        int code
        )
    {
        if (code == REG_NONE._code)
        {
            return REG_NONE;
        }
        else if (code == REG_SZ._code)
        {
            return REG_SZ;
        }
        else if (code == REG_MULTI_SZ._code)
        {
        	return REG_MULTI_SZ;
        }
        else if (code == REG_EXPAND_SZ._code)
        {
            return REG_EXPAND_SZ;
        }
        else if (code == REG_BINARY._code)
        {
            return REG_BINARY;
        }
        else if (code == REG_DWORD._code)
        {
            return REG_DWORD;
        }
        else if (code == REG_DWORD_LITTLE_ENDIAN._code)
        {
            return REG_DWORD_LITTLE_ENDIAN;
        }
        else if (code == REG_DWORD_BIG_ENDIAN._code)
        {
            return REG_DWORD_BIG_ENDIAN;
        }
        else if (code == REG_LINK._code)
        {
            return REG_MULTI_SZ;
        }
        else if (code == REG_RESOURCE_LIST._code)
        {
            return REG_RESOURCE_LIST;
        }
        else if (code == REG_FULL_RESOURCE_DESCRIPTOR._code)
        {
            return REG_FULL_RESOURCE_DESCRIPTOR;
        }
        else if (code == REG_RESOURCE_REQUIREMENTS_LIST._code)
        {
            return REG_RESOURCE_REQUIREMENTS_LIST;
        }
        else if (code == REG_QWORD._code)
        {
            return REG_QWORD;
        }
        else if (code == REG_QWORD_LITTLE_ENDIAN._code)
        {
            return REG_QWORD_LITTLE_ENDIAN;
        }
        else
        {
            throw new IllegalArgumentException(
                            "Unrecognized registry value type");
        }
    }
}
