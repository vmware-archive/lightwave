/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.identity.configure;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.UserPrincipal;
import java.util.ArrayList;
import java.util.Collection;
import java.util.EnumSet;
import java.util.Set;

import com.vmware.identity.installer.ReleaseUtil;
import com.vmware.identity.interop.registry.IRegistryAdapter;
import com.vmware.identity.interop.registry.IRegistryKey;
import com.vmware.identity.interop.registry.RegKeyAccess;
import com.vmware.identity.interop.registry.RegistryAdapterFactory;

public class LinuxInstallerHelper implements InstallerHelper {

    @Override
    public String getConfigFolderPath() {
        return "/etc/vmware-sso";
    }

    @Override
    public String getInstallFolder() {
        return "/usr/lib";
    }

    @Override
    public String getLogPaths() {
        return "/var/log/vmware/sso/";
    }

    @Override
    public String getIDMServiceLogFile() {
        return "/tmp/vmware-sts-idmd.log";
    }

    @Override
    public String getVMIdentityInstallPath() {
        // TODO Auto-generated method stub
        return null;
    }

  /**
  * IDM service (dummy) EXISTS ONLY IN VSPHERE context. This was created only for the purpose of 
  * backward compatibility such that VCHA will address in their code base to remove IDM service. 
  * LIGHTWAVE is totally independent of these thick dependencies, Hence, we avoid this service.
  **/
   @Override
   public String[] getIDMServiceStartCommand() {
        return new String[] { "/etc/init.d/vmware-sts-idmd", "restart" };
    }


    @Override
    public String[] getSTSServiceStartCommand() throws SecureTokenServerInstallerException {
        String[] stsStartCommand = null;
        try {
            if(ReleaseUtil.isLightwave()) {
                 if (Files.exists(Paths.get("/.dockerenv"))) {
                        return new String[] { "/opt/vmware/sbin/vmware-stsd.sh", "start" };
                    } else {
                        return new String[] { "systemctl", "restart", "vmware-stsd" };
            } 
                 }else {
                stsStartCommand = new String[] { "/etc/init.d/vmware-stsd", "restart" };
            }
        } catch (IOException e) {
            System.err.println("Failed to get STS service start command");
            throw new SecureTokenServerInstallerException("Failed to retrieve STS service start command", e);
        }
        return stsStartCommand;
    }

    @Override
    public String getTCBase() throws SecureTokenServerInstallerException {
        String tcRoot = getSSOHomePath();
        return tcRoot + File.separator + "vmware-sts";
    }

    @Override
    public String getReverseProxyPath() {
        return "/etc/vmware-proxy";
    }

    @Override
    public void configRegistry() {
        IRegistryAdapter registryAdapter = null;
        IRegistryKey rootKey = null;
        IRegistryKey configKey = null;
        try{
            registryAdapter = RegistryAdapterFactory.getInstance()
                    .getRegistryAdapter();
            rootKey = registryAdapter
                    .openRootKey((int) RegKeyAccess.KEY_ALL_ACCESS);
            String subkey = "Software\\VMware\\Identity\\Configuration";

            boolean exists = registryAdapter.doesKeyExist(rootKey, subkey);

            if (exists) {
                configKey = registryAdapter.openKey(rootKey, subkey, 0,
                        (int) RegKeyAccess.KEY_ALL_ACCESS);
            } else {
                configKey = registryAdapter.createKey(rootKey, subkey, null,
                        (int) RegKeyAccess.KEY_ALL_ACCESS);
            }

            registryAdapter.setStringValue(configKey, "ConfigStoreType",
                    "vmware_directory");
            registryAdapter.setIntValue(configKey, "Multitenant", 0);
            registryAdapter.setIntValue(configKey, "SystemDomainSearchTimeout", 0);

            Collection<String> domainAttributes = new ArrayList<String>();
            domainAttributes
                    .add("http://schemas.xmlsoap.org/ws/2005/05/identity/claims/givenname:givenName");
            domainAttributes
                    .add("http://schemas.xmlsoap.org/ws/2005/05/identity/claims/surname:sn");
            domainAttributes
                    .add("http://rsa.com/schemas/attr-names/2009/01/GroupIdentity:memberOf");
            domainAttributes
                    .add("http://vmware.com/schemas/attr-names/2011/07/isSolution:subjectType");
            domainAttributes
                    .add("http://schemas.xmlsoap.org/ws/2005/05/identity/claims/emailaddress:mail");
            domainAttributes
                    .add("http://schemas.xmlsoap.org/claims/UPN:userPrincipalName");
            registryAdapter.setMultiStringValue(configKey,
                    "SystemDomainAttributesMap", domainAttributes);

            // TODO: get port as config setting
            registryAdapter.setStringValue(configKey, "StsLocalTcPort", "7444");
            // TODO: use port from reverse proxy
            registryAdapter.setStringValue(configKey, "StsTcPort",
                    Integer.toString(InstallerUtils.REVERSE_PROXY_PORT));
        } finally {
                if(rootKey != null) {
                        rootKey.close();
                }
                if(configKey != null){
                        configKey.close();
                }
        }

    }

    @Override
    public String getReverseProxyServiceLog() {
        return "/tmp/reverse-proxy.log";
    }

    @Override
    public String[] getReverseProxyServiceCommand() {
        return new String[] { "/etc/init.d/vmware-proxyd", "restart" };
    }

    @Override
    public String getConfigureStsPath() {
        return "/tmp/configure-sts.sh";
    }

    @Override
    public String getConfigureStsFileName() {
        return "configure-sts.sh";
    }

    @Override
    public String getSSOCertPath() {
        return getConfigFolderPath() + File.separator + "keys";
    }

    @Override
    public String getCertoolPath() throws SecureTokenServerInstallerException {
        try {
            if(ReleaseUtil.isLightwave()) {
                return "/opt/vmware/bin/certool";
            } else {
                return "/usr/lib/vmware-vmca/bin/certool";
            }
        } catch (IOException e) {
            System.err.println("Failed to get cert tool path");
            throw new SecureTokenServerInstallerException("Failed to get cert tool path", e);
        }
    }

    @Override
    public String getVmcaSvcChkCommand(String hostname) throws SecureTokenServerInstallerException {
        String command = null;
        if (hostname != null && !hostname.isEmpty()) {
            command = getCertoolPath() + " --WaitVMCA --server=" + hostname
                    + " --wait=10";
        } else {
            command = getCertoolPath() + " --WaitVMCA --wait=10";
        }
        return command;
    }

    @Override
    public void setPermissions(Path path) throws IOException {
        UserPrincipal owner;

        owner = path.getFileSystem().getUserPrincipalLookupService()
                .lookupPrincipalByName("root");

        Files.setOwner(path, owner);

        Set<PosixFilePermission> perms = EnumSet.of(
                PosixFilePermission.OWNER_READ,
                PosixFilePermission.OWNER_WRITE,
                PosixFilePermission.OWNER_EXECUTE);

        Files.setPosixFilePermissions(path, perms);
    }

    @Override
    public String getSSOHomePath() throws SecureTokenServerInstallerException {
        try {
            if(ReleaseUtil.isLightwave()) {
                return "/opt/vmware";
            } else {
                return "/usr/lib/vmware-sso";
            }
        } catch (IOException e) {
            System.err.println("Failed to get SSO homepath");
            throw new SecureTokenServerInstallerException("Failed to get SSO homepath", e);
        }
    }

    @Override
    public String getIdmLoginPath(){
        return "/etc/vmware-sso/keys/";
    }

    public String getConfigDirectoryRootKey() {
        return "Services\\vmdir";
    }
}
