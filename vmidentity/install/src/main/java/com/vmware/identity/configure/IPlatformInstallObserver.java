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

    void beginUpgrade(List<PlatformInstallComponent> components);

    void beginComponentUpgrade(String component);

    void endComponentUpgrade(String component, boolean status);

    void endUpgrade(boolean status);

    void beginMigration(List<PlatformInstallComponent> components);

    void beginComponentMigration(String component);

    void endComponentMigration(String component, boolean status);

    void endMigration(boolean status);
}
