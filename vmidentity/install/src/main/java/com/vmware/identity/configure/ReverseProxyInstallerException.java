/* **********************************************************************
 * Copyright 2014 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.identity.configure;

public class ReverseProxyInstallerException extends Exception {
    private static final long serialVersionUID = 6246033343676283672L;

    public ReverseProxyInstallerException(String message, Exception e) {
	super(message, e);
    }
}
