/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.identity.configure;

import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.vmware.identity.installer.ReleaseUtil;

public class VMIdentityController {

    private IPlatformInstallObserver observer = null;

    public boolean setupInstanceStandalone(
            VmIdentityParams standaloneParams)
            throws DomainControllerNativeException {

        Validate.validateNotNull(standaloneParams.getUsername(), "Username");
        Validate.validateNotNull(standaloneParams.getPassword(), "Password");
        Validate.validateNotEmpty(standaloneParams.getDomainName(), "Domain");

        setupInstance(standaloneParams);

        return true;
    }

    public boolean upgradeStandaloneInstance(
            VmIdentityParams standaloneParams) throws DomainControllerNativeException {

        upgradeInstance(standaloneParams);

        return true;
    }

    public boolean migrateStandaloneInstance(
            VmIdentityParams standaloneParams) throws DomainControllerNativeException {

        upgradeInstance(standaloneParams);
        return true;
    }

    private void upgradeInstance (VmIdentityParams params) throws DomainControllerNativeException
    {
        List<IPlatformComponentInstaller> components = getComponents(params);
        List<PlatformInstallComponent> componentsInfo = new ArrayList<>();

        if (observer != null) {
            for (IPlatformComponentInstaller comp : components) {
                componentsInfo.add(comp.getComponentInfo());
            }
            observer.beginUpgrade(componentsInfo);
        }

        boolean status = true;
        try {
            for (IPlatformComponentInstaller comp : components) {
                try {
                    if (observer != null)
                        observer.beginComponentUpgrade(comp.getComponentInfo()
                                .getId());

                    comp.upgrade();

                } catch (Exception e) {
                    status = false;
                    throw new DomainControllerNativeException(-1, e);
                } finally {
                    if (observer != null)
                        observer.endComponentUpgrade(comp.getComponentInfo()
                                .getId(), status);
                }
            }
        } finally {
            if (observer != null)
                observer.endUpgrade(status);
        }
    }
    private void setupInstance(VmIdentityParams params)
            throws DomainControllerNativeException {
        if (params.getHostname() == null || params.getHostname().isEmpty()) {
            try {
                params.setHostname(InetAddress.getLocalHost().getHostAddress());
            } catch (UnknownHostException e) {
                throw new DomainControllerNativeException(
                        DeployUtilsErrors.ERROR_INVALID_NETNAME.getErrorCode(),
                        e);
            }
        }

        checkPrerequisites(params);

        List<IPlatformComponentInstaller> components = getComponents(params);
        List<PlatformInstallComponent> componentsInfo = new ArrayList<>();

        if (observer != null) {
            for (IPlatformComponentInstaller comp : components) {
                componentsInfo.add(comp.getComponentInfo());
            }
            observer.beginInstall(componentsInfo);
        }

        boolean status = true;
        try {
            for (IPlatformComponentInstaller comp : components) {
                try {
                    if (observer != null)
                        observer.beginComponentInstall(comp.getComponentInfo()
                                .getId());

                    comp.install();

                } catch (DomainControllerNativeException e) {
                    status = false;
                    throw e;
                } catch (Exception e) {
                    status = false;
                    e.printStackTrace();
                    System.out.println(e.getMessage());

                    throw new DomainControllerNativeException(-1, e);
                } finally {
                    if (observer != null)
                        observer.endComponentInstall(comp.getComponentInfo()
                                .getId(), status);
                }
            }
        } finally {
            if (observer != null)
                observer.endInstall(status);
        }
    }

    public void setPlatformInstallObserver(IPlatformInstallObserver observer) {
        this.observer = observer;
    }

    private List<IPlatformComponentInstaller> getComponents(
            VmIdentityParams standaloneParams) {
        List<IPlatformComponentInstaller> components = new ArrayList<IPlatformComponentInstaller>();
        try {
            if(ReleaseUtil.isLightwave()) {
                components.add(new IdentityManagerInstaller(false, standaloneParams));
                components.add(new SecureTokenServerInstaller(standaloneParams));
            } else {
                components.add(new IdentityManagerInstaller(false, standaloneParams));
                // components.add(new LookupServiceInstaller(false));
                components.add(new SecureTokenServerInstaller(standaloneParams));
                }
        } catch (IOException e) {
            System.err.println("Failed to fetch components for SecureTokenservice");
            System.exit(1);
        }
        return components;
    }

    private void checkPrerequisites(VmIdentityParams params) {
        Validate.validateNotEmpty(params.getHostname(), "Hostname");

        Set<String> illegalHostanames = new HashSet<String>();
        illegalHostanames.add("localhost.localdomain");
        illegalHostanames.add("localhost");
        illegalHostanames.add("localhost.localdom");
        if (illegalHostanames.contains(params.getHostname().toLowerCase())) {
            throw new IllegalArgumentException(String.format(
                    "Invalid host name - %s", params.getHostname()));
        }
    }
}
