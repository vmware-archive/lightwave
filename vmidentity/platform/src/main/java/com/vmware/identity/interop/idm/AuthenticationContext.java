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

import java.io.Closeable;

import com.sun.jna.Pointer;
import com.vmware.identity.interop.Validate;

public class AuthenticationContext implements Closeable {

    private Pointer _pAuthContext;
    private IIdmClientLibrary.SSPI_AUTH_STATUS _authStatus;

    public AuthenticationContext(Pointer pAuthContext)
    {
        Validate.validateNotNull(pAuthContext, "Authentication Context");

        _pAuthContext = pAuthContext;
        _authStatus = IIdmClientLibrary.SSPI_AUTH_STATUS.INITIAL;
    }

    public Pointer getNativeContext()
    {
        return _pAuthContext;
    }

    public void setAuthStatus(IIdmClientLibrary.SSPI_AUTH_STATUS status)
    {
        _authStatus = status;
    }

    public IIdmClientLibrary.SSPI_AUTH_STATUS getAuthStatus()
    {
        return _authStatus;
    }

    @Override
    public void close() {

        if (_pAuthContext != Pointer.NULL)
        {
            IdmClientLibraryFactory.getInstance().getLibrary().FreeAuthenticationContext(_pAuthContext);
            _pAuthContext = Pointer.NULL;
        }
    }
}
