/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.identity.configure;

public class STSInstallerException extends Exception {

    private static final long serialVersionUID = 6266133341686243672L;

    public STSInstallerException(String messsage, Exception e) {
        super(messsage, e);
    }
}
