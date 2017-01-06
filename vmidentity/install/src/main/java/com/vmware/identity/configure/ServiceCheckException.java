/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.identity.configure;

public class ServiceCheckException extends Exception {

    private static final long serialVersionUID = 6266033342676143772L;

    public ServiceCheckException(String message) {
        super(message);
    }

    public ServiceCheckException(String message, Exception e) {
        super(message, e);
    }
}
