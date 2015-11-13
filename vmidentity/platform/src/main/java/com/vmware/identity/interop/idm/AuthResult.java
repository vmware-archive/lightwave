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

package com.vmware.identity.interop.idm;

import com.vmware.identity.interop.idm.IIdmClientLibrary.SSPI_AUTH_STATUS;

public class AuthResult {

    private final SSPI_AUTH_STATUS _status;
    private final byte[] _gssBLOB;

    public AuthResult(SSPI_AUTH_STATUS status, byte[] gssBLOB)
    {
        _status = status;
        _gssBLOB = gssBLOB;
    }

    public SSPI_AUTH_STATUS getStatus()
    {
        return _status;
    }

    public byte[] getGssBLOB()
    {
        return _gssBLOB;
    }
}
