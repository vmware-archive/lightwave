/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved. VMware Confidential
 * *********************************************************************/

package com.vmware.identity.configure;

import java.io.IOException;
import java.nio.file.Path;

public interface InstallerHelper {
    public String getSSOCertPath();

    public String getCertoolPath();

    public String getVmcaSvcChkCommand(String hostname);

    public String getConfigFolderPath();

    public String getInitTcInstancePath();

    public String getReverseProxyServiceLog();

    public String[] getReverseProxyServiceCommand();

    public String getReverseProxyPath();

    public String[] getSTSServiceStartCommand();

    public void configRegistry();

    public String getVMIdentityInstallPath();

    public String getIDMServiceLogFile();

    public String getInstallFolder();

    public String[] getIDMServiceStartCommand();

    public String getLogPaths();

    public String getTCSetenvName();

    public String getTCBase();

    public String getConfigureStsPath();

    public String getConfigureStsFileName();

    public void setPermissions(Path path) throws IOException;

    public String getGcLogFile();

    public String getSetEnvReplacement();

    public String getSSOHomePath();

    public String getIdmLoginPath();
}