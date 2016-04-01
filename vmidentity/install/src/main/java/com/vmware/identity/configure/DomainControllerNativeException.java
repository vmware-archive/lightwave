/**
 *
 * Copyright 2015 VMware, Inc.  All rights reserved.
 */

package com.vmware.identity.configure;

public class DomainControllerNativeException extends Exception {

    private static final long serialVersionUID = 6831117971351358554L;

    private int errCode = 0;

    public DomainControllerNativeException(int errCode) {
        this.errCode = errCode;
    }

    public DomainControllerNativeException(int errCode, Throwable e) {
        super(e);
        this.errCode = errCode;
    }

    public int getErrorCode() {
        return errCode;
    }

}
