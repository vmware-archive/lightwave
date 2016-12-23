/* **********************************************************************
 * Copyright 2014 VMware, Inc.  All rights reserved. VMware Confidential
 * *********************************************************************/

package com.vmware.pscsetup;

import java.io.Console;

import com.vmware.identity.configure.DomainControllerNativeException;
import com.vmware.identity.configure.PlatformInstallObserverDefault;

public class PlatformServicesControllerDeploy {

    private static PlatformServicesController psc = new PlatformServicesController();

	public static void main(String[] args) {

		DomainControllerStandaloneParams params = build(args);

		if (params.getPassword() == null || params.getPassword().isEmpty()) {
			Console cons = System.console();
			char[] passwd;
			if (cons != null
					&& (passwd = cons.readPassword("Password:")) != null) {
				params.setPassword(new String(passwd));
			}
		}

		if (params instanceof DomainControllerPartnerParams) {
			try {
				psc.setPlatformInstallObserver(new PlatformInstallObserverDefault());
				psc.setupInstancePartner((DomainControllerPartnerParams) params);
			} catch (DomainControllerNativeException e) {
				System.err.println("Errorcode: " + e.getErrorCode());
				e.printStackTrace(System.err);
				System.exit(e.getErrorCode());
			}
		} else {
			try {
				psc.setPlatformInstallObserver(new PlatformInstallObserverDefault());
				psc.setupInstanceStandalone(params);
			} catch (DomainControllerNativeException e) {
				System.err.printf("Errorcode: " + e.getErrorCode());
				e.printStackTrace(System.err);
				System.exit(e.getErrorCode());
			}
		}
    }

    private static DomainControllerStandaloneParams build(String[] args) {
	DomainControllerStandaloneParams params = new DomainControllerStandaloneParams();
	ParseMode mode = ParseMode.PARSE_MODE_OPEN;

	if (args.length < 2 || !args[0].equals("--mode"))
	    throw new IllegalArgumentException(
		    "Parameter --mode is required first (--mode standalone or --mode partner)");

	DirectorySetupMode setupMode = Enum.valueOf(DirectorySetupMode.class,
		args[1].toUpperCase());
	if (setupMode == DirectorySetupMode.STANDALONE)
	    params = new DomainControllerStandaloneParams();
	else if (setupMode == DirectorySetupMode.PARTNER)
	    params = new DomainControllerPartnerParams();

	for (String arg : args) {
            switch (mode) {
            case PARSE_MODE_OPEN:
                if (arg.equals("--hostname")) {
                    mode = ParseMode.PARSE_MODE_HOSTNAME;
                } else if (arg.equals("--password")) {
                    mode = ParseMode.PARSE_MODE_PASSWORD;
                } else if (arg.equals("--domain")) {
                    mode = ParseMode.PARSE_MODE_DOMAIN;
                } else if (arg.equals("--server")) {
                    mode = ParseMode.PARSE_MODE_SERVER;
                } else if (arg.equals("--site")) {
                    mode = ParseMode.PARSE_MODE_SITE;
                } else if (arg.equals("--dns-forwarders")) {
                    mode = ParseMode.PARSE_MODE_DNS_FORWARDERS;
                } else if (arg.equals("--ssl-subject-alt-name")) {
                    mode = ParseMode.PARSE_MODE_SSL_SUBJECT_ALT_NAME;
                }
                break;
            case PARSE_MODE_HOSTNAME:
                params.setHostname(arg);
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
            case PARSE_MODE_SITE:
                params.setSite(arg);
                mode = ParseMode.PARSE_MODE_OPEN;
                break;
            case PARSE_MODE_SERVER:
                if (setupMode == DirectorySetupMode.PARTNER)
                    ((DomainControllerPartnerParams) params).setServer(arg);
                mode = ParseMode.PARSE_MODE_OPEN;
                break;
            case PARSE_MODE_DNS_FORWARDERS:
                params.setDNSForwarders(arg);
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
	    PARSE_MODE_OPEN, PARSE_MODE_MODE, PARSE_MODE_HOSTNAME, PARSE_MODE_DOMAIN, PARSE_MODE_PASSWORD, PARSE_MODE_SITE, PARSE_MODE_SERVER, PARSE_MODE_DNS_FORWARDERS, PARSE_MODE_SSL_SUBJECT_ALT_NAME
    }
}
