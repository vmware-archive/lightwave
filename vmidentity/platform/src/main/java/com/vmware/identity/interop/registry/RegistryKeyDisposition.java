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
 * Time: 5:03 PM
 */
public enum RegistryKeyDisposition
{
    REGISTRY_KEY_DISPOSITION_NEW(1),
    REGISTRY_KEY_DISPOSITION_OPENED(2);
    
    private int _code;
    
    private RegistryKeyDisposition(int code)
    {
        _code = code;
    }
    
    public int getCode()
    {
        return _code;
    }
    
    public static RegistryKeyDisposition parse(int code)
    {
        if (code == REGISTRY_KEY_DISPOSITION_NEW._code)
        {
            return REGISTRY_KEY_DISPOSITION_NEW;
        }
        else if (code == REGISTRY_KEY_DISPOSITION_OPENED._code)
        {
            return REGISTRY_KEY_DISPOSITION_OPENED;
        }
        else
        {
            throw new IllegalArgumentException(
                        "Unrecognized value for Registry Key Disposition");
        }
    }
}
