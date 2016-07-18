/*
 *
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 *
 */

package com.vmware.identity.configure;

import java.util.List;

public class PlatformInstallObserverDefault implements IPlatformInstallObserver {

    @Override
    public void beginInstall(List<PlatformInstallComponent> components) {
        System.out.println("\n-----Begin installing components----- ");
        for (PlatformInstallComponent info : components) {
            System.out.println(info.getName());
        }
    }

    @Override
    public void beginComponentInstall(String component) {
        System.out.println("\nBegin installing component: " + component);

    }

    @Override
    public void componentInstallationProgress(String component,
            int percentCompleted) {
        // TODO Auto-generated method stub
    }

    @Override
    public void endComponentInstall(String component, boolean status) {
        if (status) {
            System.out.println("Installed " + component + " succesfully.");
        } else {
            System.out.println(component + " installation failed.");
        }
    }

    @Override
    public void endInstall(boolean status) {
        if (status) {
            System.out.println("Installation completed successfully.");
        } else {
            System.out.println("Installation failed.");
        }
    }

    @Override
    public void beginUpgrade(List<PlatformInstallComponent> components)
    {
        System.out.println("\n-----Begin Upgrade components----- ");
        for (PlatformInstallComponent info : components) {
            System.out.println(info.getName());
        }
    }

    @Override
    public void beginComponentUpgrade(String component)
    {
        System.out.println("\nBegin upgrading component: " + component);
    }

    @Override
    public void endComponentUpgrade(String component, boolean status)
    {
        if (status) {
            System.out.println("Upgrade " + component + " succesfully.");
        } else {
            System.out.println(component + " Upgrade  failed.");
        }
    }
    @Override
    public void endUpgrade(boolean status)
    {
        if (status) {
            System.out.println("Upgrade completed successfully.");
        } else {
            System.out.println("Upgrade failed.");
        }
    }

}
