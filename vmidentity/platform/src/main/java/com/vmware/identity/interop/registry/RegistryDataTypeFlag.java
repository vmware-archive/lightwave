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
 * User: snambakam
 * Date: 12/23/11
 * Time: 6:34 PM
 */
public enum RegistryDataTypeFlag
{
    DATA_TYPE_FLAG_NONE(0x00000001),
    DATA_TYPE_FLAG_REG_SZ(0x00000002),        // Restrict type to REG_SZ.
    DATA_TYPE_FLAG_REG_EXPAND_SZ(0x00000004), // Restrict type to REG_EXPAND_SZ.
    DATA_TYPE_FLAG_REG_BINARY(0x00000008),    // Restrict type to REG_BINARY.
    DATA_TYPE_FLAG_REG_DWORD(0x00000010),     // Restrict type to REG_DWORD.
    DATA_TYPE_FLAG_REG_MULTI_SZ(0x00000020),  // Restrict type to REG_MULTI_SZ.
    DATA_TYPE_FLAG_REG_QWORD(0x00000040),     // Restrict type to REG_QWORD.
    DATA_TYPE_FLAG_DWORD(DATA_TYPE_FLAG_REG_BINARY._code |
                         DATA_TYPE_FLAG_REG_DWORD._code),
    DATA_TYPE_FLAG_QWORD(DATA_TYPE_FLAG_REG_BINARY._code |
                         DATA_TYPE_FLAG_REG_QWORD._code),
    DATA_TYPE_FLAG_ANY(0x0000FFFF),           // No type restriction.
    DATA_TYPE_FLAG_NOEXPAND(0x10000000),
    DATA_TYPE_FLAG_ZEROONFAILURE(0x20000000);

    private long _code;

    private RegistryDataTypeFlag(long code)
    {
        _code = code;
    }

    public long getCode()
    {
        return _code;
    }
}
