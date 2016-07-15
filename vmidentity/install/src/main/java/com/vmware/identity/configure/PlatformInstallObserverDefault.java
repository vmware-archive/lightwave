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
