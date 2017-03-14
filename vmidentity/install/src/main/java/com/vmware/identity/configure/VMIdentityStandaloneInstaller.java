/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.identity.configure;

import java.io.Console;


public class VMIdentityStandaloneInstaller {

    public static void main(String[] args) {

        VmIdentityParams params = build(args);

        if (params.isCheckSTSHealth()) {
            try {

                String hostName   = HostnameReader.readHostName();
                String portNumber = HostnameReader.readPortNumber();
                if (hostName == null || portNumber == null){
                    throw new NullPointerException("Hostname or Portnumber are not configured");
                }
                new STSHealthChecker(hostName,portNumber ).checkHealth();
            } catch (Exception e) {
                e.printStackTrace();
                System.exit(1);
            }
        }  else if (params.isHostnameMode() ){
            try {
                new HostinfoWriter(params.getHostname(), params.getHostnameType()).write();
            } catch (Exception ex) {
                ex.printStackTrace();
                System.exit(1);
            }
        } else if (params.isUpgradeMode()) {
            System.out.println("\n\n-----Performing VMIdentity Upgrade-----");
            VMIdentityController idmController = new VMIdentityController();
            idmController
                    .setPlatformInstallObserver(new PlatformInstallObserverDefault());
            try {
                idmController.upgradeStandaloneInstance(params);
            } catch (DomainControllerNativeException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                System.exit(1);
            }
        } else if (params.isMigrationMode()) {
            System.out.println("\n\n-----Performing VMIdentity Migration-----");
            VMIdentityController idmController = new VMIdentityController();
            idmController
                    .setPlatformInstallObserver(new PlatformInstallObserverDefault());
            try {
                idmController.migrateStandaloneInstance(params);
            } catch (DomainControllerNativeException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                System.exit(1);
            }
        } else if (params.getPassword() == null || params.getPassword().isEmpty()) {
            Console cons = System.console();
            char[] passwd;
            if (cons != null
                    && (passwd = cons.readPassword("Password:")) != null) {
                params.setPassword(new String(passwd));
            }
        }
        else
        {
            try {
                VMIdentityController idmController = new VMIdentityController();
                idmController
                        .setPlatformInstallObserver(new PlatformInstallObserverDefault());
                idmController.setupInstanceStandalone(params);
            } catch (DomainControllerNativeException e) {
                System.err.printf("Errorcode: " + e.getErrorCode());
                e.printStackTrace(System.err);
                System.exit(1);
            }
        }
    }

    private static VmIdentityParams build(String[] args) {
        VmIdentityParams params = new VmIdentityParams();
        ParseMode mode = ParseMode.PARSE_MODE_OPEN;

        for (String arg : args) {
            switch (mode) {
            case PARSE_MODE_OPEN:
                if (arg.equals("--hostname")) {
                    mode = ParseMode.PARSE_MODE_HOSTNAME;
                } else if (arg.equals("--hostnametype")) {
                    mode = ParseMode.PARSE_MODE_HOSTNAMETYPE;
                } else if (arg.equals("--password")) {
                    mode = ParseMode.PARSE_MODE_PASSWORD;
                } else if (arg.equals("--domain")) {
                    mode = ParseMode.PARSE_MODE_DOMAIN;
                } else if (arg.equals("--username")) {
                    mode = ParseMode.PARSE_MODE_USERNAME;
                } else if (arg.equals("--upgrade")) {
                    params.SetUpgradeMode();
                } else if (arg.equals("--backup-folder")) {
                    mode = ParseMode.PARSE_MODE_BACKUPDIR;
                } else if (arg.equals("--identity-conf-file-path")) {
                    mode = ParseMode.PARSE_MODE_VMIDENTITY_CONF;
                } else if (arg.equals("--migration")) {
                    params.setMigrationMode(true);
                } else if (arg.equals("--sourceVersion")) {
                    mode = ParseMode.PARSE_MODE_SOURCE_VERSION;
                } else if (arg.equals("--sourcePlatform")) {
                    mode = ParseMode.PARSE_MODE_SOURCE_PLATFORM;
                } else if (arg.equals("--exportFolder")){
                    mode = ParseMode.PARSE_MODE_EXPORT_FOLDER;
                } else if (arg.equals("--set-hostname")) {
                    params.setHostNameMode(true);
                } else if (arg.equals("--sts-health-check")) {
                    params.setCheckSTSHealth(true);
                } else if (arg.equals("--ssl-subject-alt-name")) {
                    mode = ParseMode.PARSE_MODE_SSL_SUBJECT_ALT_NAME;
                 }
                break;
            case PARSE_MODE_HOSTNAME:
                params.setHostname(arg);
                mode = ParseMode.PARSE_MODE_OPEN;
                break;
            case PARSE_MODE_HOSTNAMETYPE:
                params.setHostnameType(arg);
                mode = ParseMode.PARSE_MODE_OPEN;
                break;
            case PARSE_MODE_USERNAME:
                params.setUsername(arg);
                mode = ParseMode.PARSE_MODE_OPEN;
                break;
            case PARSE_MODE_PASSWORD:
                params.setPassword(arg);
                mode = ParseMode.PARSE_MODE_OPEN;
                break;
            case PARSE_MODE_DOMAIN:
                params.setDomainName(arg);
                mode = ParseMode.PARSE_MODE_OPEN;
                break;
            case PARSE_MODE_BACKUPDIR:
                params.setBackupDir(arg);
                mode = ParseMode.PARSE_MODE_OPEN;
                break;
            case PARSE_MODE_VMIDENTITY_CONF:
                params.setVmIdentityConf(arg);
                mode = ParseMode.PARSE_MODE_OPEN;
                break;
            case PARSE_MODE_SOURCE_VERSION:
                params.setSourceVersion(arg);
                mode = ParseMode.PARSE_MODE_OPEN;
                break;
            case PARSE_MODE_SOURCE_PLATFORM:
                params.setSourcePlatform(arg);
                mode = ParseMode.PARSE_MODE_OPEN;
                break;
            case PARSE_MODE_EXPORT_FOLDER:
                params.setVmIdentityConf(arg);
                mode = ParseMode.PARSE_MODE_OPEN;
                break;
            case PARSE_MODE_SSL_SUBJECT_ALT_NAME:
                params.setSubjectAltName(arg);
                mode = ParseMode.PARSE_MODE_OPEN;
                break;
            default:
                break;
            }
        }
        return params;
    }

    enum ParseMode {
        PARSE_MODE_OPEN,
        PARSE_MODE_HOSTNAME,
        PARSE_MODE_HOSTNAMETYPE,
        PARSE_MODE_DOMAIN,
        PARSE_MODE_USERNAME,
        PARSE_MODE_PASSWORD,
        PARSE_MODE_BACKUPDIR,
        PARSE_MODE_UPGRADE,
        PARSE_MODE_VMIDENTITY_CONF,
        PARSE_MODE_SOURCE_VERSION,
        PARSE_MODE_SOURCE_PLATFORM,
        PARSE_MODE_EXPORT_FOLDER,
        PARSE_MODE_SSL_SUBJECT_ALT_NAME
    }
}
