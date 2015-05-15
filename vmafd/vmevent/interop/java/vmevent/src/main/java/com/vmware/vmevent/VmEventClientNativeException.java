/**
 *
 * Copyright 2013 VMware, Inc.  All rights reserved.
 */

package com.vmware.vmevent;

public class VmEventClientNativeException extends RuntimeException
{
    private static final long serialVersionUID = -8446585725701833997L;
    private int _errCode = 0;

    public VmEventClientNativeException(int errCode)
    {
        _errCode = errCode;
    }

    public int getErrorCode()
    {
        return _errCode;
    }
}
