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

    private void upgradeInstance (VmIdentityParams params) throws DomainControllerNativeException
    {
        // check services vmafd, vmca and vmdir if they are confgiured.
        try {
            // check authentication services
            // TODO: Need to figure out if we are going
            //  check the status of the VMAFD Service
            //checkVMAFDService();

            // check directory services
            // TODO: Need to figure out if we are going 
            // check the status of the VMDIR service
            checkVMDIRService();

            // check certificate services
            checkVMCAService(params.getHostname());

        } catch (Exception ex) {
            System.err.println("Error: Cannot proceed. Failed to check services.\n");
            ex.printStackTrace(System.err);
            System.exit(1);
        }

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

        // check services vmafd, vmca and vmdir if they are confgiured.
        try {
            // check authentication services
            checkVMAFDService();

            // check directory services
            checkVMDIRService();

            // check certificate services
            checkVMCAService(params.getHostname());

        } catch (Exception ex) {
            System.err.println("Error: Cannot proceed. Failed to check services.\n");
            ex.printStackTrace(System.err);
            System.exit(1);
        }

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
        components.add(new IdentityManagerInstaller(standaloneParams
                .getUsername(), standaloneParams.getDomainName(),
                standaloneParams.getPassword(),false, standaloneParams.isUpgradeMode()));
        components.add(new SecureTokenServerInstaller(standaloneParams));
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

    private void checkVMCAService(String hostname) throws Exception {

        System.out.println("\n-----Checking Certificate service-----");
        int exitCode = -1;
        String command = InstallerUtils.getInstallerHelper()
                .getVmcaSvcChkCommand(hostname);

        try {
            Process p = Runtime.getRuntime().exec(command);
            exitCode = p.waitFor();
        } catch (IOException | InterruptedException e) {
            throw new ServiceCheckException(
                    "Failed to check certificate service. Cannot configure IDM or STS.",
                    e);
        }

        if (exitCode != 0) {
            throw new ServiceCheckException(
                    String.format(
                            "Failed to check certificate service. Cannot configure IDM or STS.",
                            exitCode), null);
        }
        System.out.println("Certificate Service checked successfully.");

    }

    private void checkVMDIRService() throws Exception {/*

        System.out.println("\n-----Checking Directory service-----");
        try{
            STSInstaller installer = new STSInstaller();
            installer.check_dir_svc();
        } catch(Exception ex){
            throw new ServiceCheckException("Failed to check directory service. Cannot configure IDM or STS.", ex);
        }
        System.out.println("Directory Service checked successfully.");
        */
    }

    private void checkVMAFDService() throws Exception { /*

        System.out.println("\n\n-----Checking Authentication service-----");
        try {
            STSInstaller installer = new STSInstaller();
            installer.check_vmafd_svc();
        } catch (Exception ex) {
            throw new ServiceCheckException("Failed to check authentication service. Cannot configure IDM or STS.",
                    ex);
        }
        System.out.println("Authentication Service checked successfully.");
       */
    }
}
