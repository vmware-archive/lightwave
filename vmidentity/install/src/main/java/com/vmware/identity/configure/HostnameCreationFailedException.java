/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved. 
 * *********************************************************************/

package com.vmware.identity.configure;

public class HostnameCreationFailedException extends Exception {
    private static final long serialVersionUID = -4725729245264514063L;

    public HostnameCreationFailedException() {
        super("Failed to create hostname file");
    }

    public HostnameCreationFailedException(Exception nestedException) {
        super("Failed to create hostname file", nestedException);
    }
}
