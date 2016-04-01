/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved. 
 * *********************************************************************/

package com.vmware.identity.configure;

import java.util.List;

public interface IPlatformInstallObserver {
    void beginInstall(List<PlatformInstallComponent> components);

    void beginComponentInstall(String component);

    void componentInstallationProgress(String component, int percentCompleted);

    void endComponentInstall(String component, boolean status);

    void endInstall(boolean status);
}
