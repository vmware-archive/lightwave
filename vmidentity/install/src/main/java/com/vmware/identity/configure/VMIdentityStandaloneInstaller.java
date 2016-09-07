/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.identity.configure;

import java.io.Console;


public class VMIdentityStandaloneInstaller {

    public static void main(String[] args) {

        VmIdentityParams params = build(args);
        if (params.isUpgradeMode()) {
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
        } else if (params.getPassword() == null || params.getPassword().isEmpty()) {
            Console cons = System.console();
            char[] passwd;
            if (cons != null
                    && (passwd = cons.readPassword("Password:")) != null) {
                params.setPassword(new String(passwd));
            }
        } 
        if (!params.isUpgradeMode())
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
                } else if (arg.equals("--start-service")) {
                    params.setServiceStart();
		}
                break;
            case PARSE_MODE_HOSTNAME:
                params.setHostname(arg);
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
            default:
                break;
            }
        }
        return params;
    }

    enum ParseMode {
        PARSE_MODE_OPEN,
        PARSE_MODE_HOSTNAME,
        PARSE_MODE_DOMAIN,
        PARSE_MODE_USERNAME,
        PARSE_MODE_PASSWORD,
        PARSE_MODE_BACKUPDIR,
        PARSE_MODE_UPGRADE,
        PARSE_MODE_VMIDENTITY_CONF
    }
}
