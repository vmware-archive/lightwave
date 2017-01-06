/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.identity.configure;

public interface IPlatformComponentInstaller {

    PlatformInstallComponent getComponentInfo();

    void install() throws Exception;

    void upgrade() throws Exception;

    void uninstall();

    void migrate();
}
