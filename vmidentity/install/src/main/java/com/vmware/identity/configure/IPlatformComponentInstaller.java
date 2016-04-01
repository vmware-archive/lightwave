/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved. VMware Confidential
 * *********************************************************************/
package com.vmware.identity.configure;

public interface IPlatformComponentInstaller {

    PlatformInstallComponent getComponentInfo();

    void install() throws Exception;

    void upgrade();

    void uninstall();
}
