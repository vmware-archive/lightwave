/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.identity.configure;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.attribute.AclEntry;
import java.nio.file.attribute.AclEntryFlag;
import java.nio.file.attribute.AclEntryPermission;
import java.nio.file.attribute.AclEntryType;
import java.nio.file.attribute.AclFileAttributeView;
import java.nio.file.attribute.UserPrincipal;
import java.util.ArrayList;
import java.util.EnumSet;
import java.util.List;
import java.util.Set;

import com.vmware.identity.interop.registry.IRegistryAdapter;
import com.vmware.identity.interop.registry.IRegistryKey;
import com.vmware.identity.interop.registry.RegKeyAccess;
import com.vmware.identity.interop.registry.RegistryAdapterFactory;

public class WinInstallerHelper implements InstallerHelper {

    private static final String VMCA_ROOT_KEY = "SOFTWARE\\VMware, Inc.\\VMware Certificate Services";
    private static final String VMIDM_ROOT_KEY = "SOFTWARE\\VMware, Inc.\\VMware Identity Services";
    private static final String VM_TOMCAT_ROOT_KEY = "SOFTWARE\\VMware, Inc.\\vtcServer";
    private static final String INSTALL_PATH = "InstallPath";
    private static final String CONFIG_PATH = "ConfigPath";
    private static final String LOGS_PATH = "LogsPath";
    private static final String TCSERVER = "tcServer";

    @Override
    public String getConfigFolderPath() {
        return getConfiguredPath(VMIDM_ROOT_KEY, CONFIG_PATH);
    }

    @Override
    public String getInstallFolder() {
        String installFolder  = System.getenv("VMWARE_CIS_HOME");
        if (installFolder == null) {
            installFolder = String.format("%s\\VMware\\CIS",
                    System.getenv("ProgramFiles"));
        }
        return installFolder;
    }

    @Override
    public String getLogPaths() {
        return getConfiguredPath(VMIDM_ROOT_KEY, LOGS_PATH);
    }

    @Override
    public String getSecureTokenServiceLogFile() {
        return joinPath(getLogPaths(), "VMwareSecureTokenService.log");
    }

    @Override
    public String getVMIdentityInstallPath() {

        return getConfiguredPath(VMIDM_ROOT_KEY, INSTALL_PATH);
    }

    @Override
    public String getTCBase() {
        String runTimeFolder = System.getenv("VMWARE_RUNTIME_DATA_DIR");
        if (runTimeFolder == null) {
            String tcRoot = joinPath(System.getenv("ProgramData"),
                    "VMware\\CIS\\runtime\\");
            return joinPath(tcRoot, "VMwareSTSService");
        }else {
            return joinPath(runTimeFolder,"VMwareSTSService");
        }
    }

    @Override
    public String getReverseProxyPath() {
        String reverseProxyPath;
        String cfgFolderPath= System.getenv("VMWARE_CFG_DIR");
        if(cfgFolderPath == null) {
            reverseProxyPath = joinPath(System.getenv("ProgramData"),
                    "VMware\\CIS\\cfg\\vmware-rhttpproxy\\ssl");
        } else {
            reverseProxyPath = joinPath(cfgFolderPath,
                    "vmware-rhttpproxy\\ssl");
        }
        return reverseProxyPath;
    }

    @Override
    public String[] getSTSServiceStartCommand() {
        return new String[] { "sc.exe", "start", "VMwareSTS" };
    }

    @Override
    public void configRegistry() {
        // no OP on the Windows side.
    }

    @Override
    public String getReverseProxyServiceLog() {
        return getLogPaths() + File.separator + "rhttpproxy.log";
    }

    @Override
    public String[] getReverseProxyServiceCommand() {
        return new String[] { "sc", "restart", "proxyd" };
    }

    @Override
    public String getConfigureStsPath() {
        return"";
    }

    @Override
    public String getConfigureStsFileName() {
        return "";
    }

    @Override
    public String getSSOCertPath() {
        return joinPath(getConfigFolderPath(), "keys");
    }

    @Override
    public String getCertoolPath() {
        return "\""
                + joinPath(readRegEdit(VMCA_ROOT_KEY, INSTALL_PATH), "certool")
                + "\"";
    }

    @Override
    public String getVmcaSvcChkCommand(String hostname) {
        if (hostname != null && !hostname.isEmpty()) {
            return getCertoolPath() + " --WaitVMCA --server=" + hostname
                    + " --wait=10";
        } else {
            return getCertoolPath() + " --WaitVMCA --wait=10";
        }
    }

    @Override
    public void setPermissions(Path path) throws IOException {
        AclFileAttributeView aclView = Files.getFileAttributeView(path,
                AclFileAttributeView.class);
        List<AclEntry> securityDesc = new ArrayList<AclEntry>();

        UserPrincipal systemUser = path.getFileSystem()
                .getUserPrincipalLookupService()
                .lookupPrincipalByName("NT AUTHORITY\\SYSTEM");

        UserPrincipal admin = path.getFileSystem()
                .getUserPrincipalLookupService()
                .lookupPrincipalByName("BUILTIN\\Administrators");

        Set<AclEntryPermission> permissions = EnumSet.of(
                AclEntryPermission.READ_DATA, AclEntryPermission.WRITE_DATA,
                AclEntryPermission.APPEND_DATA,
                AclEntryPermission.READ_NAMED_ATTRS,
                AclEntryPermission.WRITE_NAMED_ATTRS,
                AclEntryPermission.EXECUTE, AclEntryPermission.DELETE_CHILD,
                AclEntryPermission.READ_ATTRIBUTES,
                AclEntryPermission.WRITE_ATTRIBUTES, AclEntryPermission.DELETE,
                AclEntryPermission.READ_ACL, AclEntryPermission.WRITE_ACL,
                AclEntryPermission.WRITE_OWNER, AclEntryPermission.SYNCHRONIZE);

        AclEntry.Builder aclEntryBuilder = AclEntry.newBuilder();
        aclEntryBuilder.setType(AclEntryType.ALLOW);
        aclEntryBuilder.setPrincipal(systemUser);
        aclEntryBuilder.setPermissions(permissions);

        aclEntryBuilder.setFlags(AclEntryFlag.FILE_INHERIT,
                AclEntryFlag.DIRECTORY_INHERIT);

        securityDesc.add(aclEntryBuilder.build());

        aclEntryBuilder = AclEntry.newBuilder();
        aclEntryBuilder.setType(AclEntryType.ALLOW);
        aclEntryBuilder.setPrincipal(admin);
        aclEntryBuilder.setPermissions(permissions);
        aclEntryBuilder.setFlags(AclEntryFlag.FILE_INHERIT,
                AclEntryFlag.DIRECTORY_INHERIT);

        securityDesc.add(aclEntryBuilder.build());

        aclView.setAcl(securityDesc);
    }

    @Override
    public String getSSOHomePath() {
        return joinPath(getInstallFolder(), "vmware-sso");
    }

    private String getJavaHomePath() {
        String java_home = System.getenv("VMWARE_JAVE)HOME");
        if(java_home == null) {
            java_home = joinPath(getInstallFolder(), "jre");
        }
        return java_home;
    }

    @Override
    public String getIdmLoginPath(){

        return joinPath(readRegEdit(VMIDM_ROOT_KEY, CONFIG_PATH),"keys");
    }

    @Override
    public String getConfigDirectoryRootKey() {
        return "System\\CurrentControlset\\Services\\VMwareDirectoryService";
    }

    public String joinPath(String path1, String path2) {
        return String.format("%s%s%s", path1, File.separator, path2);
    }

    public String joinPath(String path1, String path2, String path3) {
        return String.format("%s%s%s%s%s", path1, File.separator, path2,
                File.separator, path3);
    }

    public String getTcRootPath() {

        return joinPath(getConfiguredPath(VM_TOMCAT_ROOT_KEY, INSTALL_PATH), TCSERVER);
    }

    public String getWrapperBinPath() {

        return joinPath(joinPath(getTcRootPath(), "templates","base"), "bin", "winx86_64");
    }

    public static String readRegEdit(String rootPath, String param) {

        IRegistryAdapter regAdapter = RegistryAdapterFactory.getInstance()
                .getRegistryAdapter();

        IRegistryKey rootKey = regAdapter
                .openRootKey((int) RegKeyAccess.KEY_READ);

        String regValue = null;
        try {
            regValue = regAdapter.getStringValue(rootKey, rootPath, param,
                    false);
        } finally {
            rootKey.close();
        }
        return regValue;
    }

    private static String getConfiguredPath(String subkey, String configName) {
        IRegistryAdapter registryAdapter = RegistryAdapterFactory.getInstance()
                .getRegistryAdapter();
        IRegistryKey rootKey = registryAdapter
                .openRootKey((int) RegKeyAccess.KEY_ALL_ACCESS);

        String path = registryAdapter.getStringValue(rootKey, subkey,
                configName, false);

        return path;
    }

}
