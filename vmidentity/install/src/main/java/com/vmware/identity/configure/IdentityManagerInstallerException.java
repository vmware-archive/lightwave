/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.identity.configure;

public class IdentityManagerInstallerException extends Exception {
    private static final long serialVersionUID = 6266033341676243672L;

    public IdentityManagerInstallerException(String message, Exception e) {
        super(message, e);
    }
}
