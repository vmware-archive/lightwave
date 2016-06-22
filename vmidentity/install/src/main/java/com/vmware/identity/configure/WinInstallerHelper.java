/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved. VMware Confidential
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
import java.util.Collection;
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
        return String.format("%s\\VMware\\CIS",
                System.getenv("ProgramFiles"));
    }

    @Override
    public String[] getIDMServiceStartCommand() {
        return new String[] { "sc", "start", "VMwareIdentityMgmtService" };
    }

    @Override
    public String getLogPaths() {
        return getConfiguredPath(VMIDM_ROOT_KEY, LOGS_PATH);
    }

    @Override
    public String getIDMServiceLogFile() {
        return joinPath(getLogPaths(), "VMwareIdentityMgmtService.log");
    }

    @Override
    public String getVMIdentityInstallPath() {

        return getConfiguredPath(VMIDM_ROOT_KEY, INSTALL_PATH);
    }

    @Override
    public String getTCBase() {
        String tcRoot = joinPath(System.getenv("ProgramData"),
                "VMware\\CIS\\runtime\\");
        return joinPath(tcRoot, "VMwareSTSService");
    }

    @Override
    public String getReverseProxyPath() {
        return joinPath(System.getenv("ProgramData"),
                "VMware\\CIS\\cfg\\vmware-rhttpproxy\\ssl");
    }

    @Override
    public String[] getSTSServiceStartCommand() {
        return new String[] { "sc.exe", "start", "VMwareSTS" };
    }

    @Override
    public void configRegistry() {
        IRegistryAdapter registryAdapter = RegistryAdapterFactory.getInstance()
                .getRegistryAdapter();
        IRegistryKey rootKey = registryAdapter
                .openRootKey((int) RegKeyAccess.KEY_ALL_ACCESS);

        IRegistryKey idmKey;
        String subkey = String
                .format("SOFTWARE\\Wow6432Node\\Apache Software Foundation\\Procrun 2.0\\%s\\Parameters",
                        getIDMServiceName());

        boolean exists = registryAdapter.doesKeyExist(rootKey, subkey);

        if (exists) {
            idmKey = registryAdapter.openKey(rootKey, subkey, 0,
                    (int) RegKeyAccess.KEY_ALL_ACCESS);
        } else {
            idmKey = registryAdapter.createKey(rootKey, subkey, null,
                    (int) RegKeyAccess.KEY_ALL_ACCESS);
        }

        String identityInstallPath = getVMIdentityInstallPath();
        String identityInstallPathCommonLib = InstallerUtils.joinPath(
                getSSOHomePath(), "commonlib");
        // Add Java key
        String javaHomePath = getJavaHomePath();
        IRegistryKey keyJava = registryAdapter.createKey(idmKey, "Java", null,
                (int) RegKeyAccess.KEY_ALL_ACCESS);
        registryAdapter.setStringValue(keyJava, "ClassPath", String
                .format("%s\\lib\\*;%s\\lib\\ext\\*;%s\\*;%s\\*", javaHomePath,
                        javaHomePath, identityInstallPath,
                        identityInstallPathCommonLib));
        registryAdapter.setStringValue(keyJava, "JavaHome",
                String.format("%s\\", javaHomePath));
        registryAdapter.setStringValue(keyJava, "Jvm",
                String.format("%s\\bin\\server\\jvm.dll", javaHomePath));

        // Define options for 'Java' key
        Collection<String> options = new ArrayList<String>();
        options.add(String.format(
                "-Dvmware.log.dir=%s",
                getLogPaths()));
        options.add(String.format(
                "-Djava.security.policy=%s\\server_policy.txt",
                identityInstallPath));
        options.add(String.format(
                "-Dlog4j.configurationFile=file:%s\\log4j2.xml",
                identityInstallPath));
        options.add("-Xmx160m");
        options.add("-XX:MaxPermSize=160m");

        options.add(String.format("-XX:ErrorFile=%s", getLogPaths()));
        registryAdapter.setMultiStringValue(keyJava, "Options", options);

        // Add 'Log' key
        IRegistryKey keyLog = registryAdapter.createKey(idmKey, "Log", null,
                (int) RegKeyAccess.KEY_ALL_ACCESS);
        registryAdapter.setStringValue(keyLog, "Path", getLogPaths());

        // Add 'Start' key
        IRegistryKey keyStart = registryAdapter.createKey(idmKey, "Start",
                null, (int) RegKeyAccess.KEY_ALL_ACCESS);
        registryAdapter.setStringValue(keyStart, "Mode", "jvm");
        registryAdapter.setStringValue(keyStart, "Class",
                "com.vmware.identity.idm.server.IdmServer");
        registryAdapter.setStringValue(keyStart, "Method", "startserver");

        // Add 'Stop' key
        IRegistryKey keyStop = registryAdapter.createKey(idmKey, "Stop", null,
                (int) RegKeyAccess.KEY_ALL_ACCESS);
        registryAdapter.setStringValue(keyStop, "Mode", "jvm");
        registryAdapter.setStringValue(keyStop, "Class",
                "com.vmware.identity.idm.server.IdmServer");
        registryAdapter.setStringValue(keyStop, "Method", "stopserver");
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

    private String getIDMServiceName() {
        return "VMwareIdentityMgmtService";
    }

    @Override
    public String getSSOHomePath() {
        return joinPath(getInstallFolder(), "vmware-sso");
    }

    private String getJavaHomePath() {
        return joinPath(getInstallFolder(), "jre");
    }

    @Override
    public String getIdmLoginPath(){

        return readRegEdit(VMIDM_ROOT_KEY, INSTALL_PATH);
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
