/* **********************************************************************
 * Copyright 2014 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.pscsetup;

public interface IPlatformComponentInstaller {

    PlatformInstallComponent getComponentInfo();
    void install() throws Exception;
    void upgrade();
    void uninstall();
}
