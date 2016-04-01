/**
 *
 * Copyright 2015 VMware, Inc.  All rights reserved.
 */

package com.vmware.identity.configure;

public enum DirectorySetupMode {
	UNKNOWN(0),
	STANDALONE(1),
	PARTNER(2),
	CLIENT(3);

    int _mode;

    private DirectorySetupMode(int mode)
    {
        this._mode = mode;
    }

    public int getCode()
    {
        return this._mode;
    }

}
