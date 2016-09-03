/**
 *
 * Copyright 2014 VMware, Inc.  All rights reserved.
 */

package com.vmware.pscsetup.interop;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.PointerByReference;
import com.vmware.pscsetup.DirectorySetupMode;
import com.vmware.pscsetup.DomainControllerPartnerParams;
import com.vmware.pscsetup.DomainControllerStandaloneParams;
import com.vmware.identity.configure.DomainControllerNativeException;

public class DeployUtilsAdapter extends NativeAdapter {
	public interface DeployUtilsLibrary extends Library {
		DeployUtilsLibrary INSTANCE = (DeployUtilsLibrary) Native.loadLibrary(
				"cfgutils", DeployUtilsLibrary.class);

		int VmwDeploySetupInstance(Pointer pParams);

		int VmwDeployValidatePartnerCredentials(String pszServer,
				String pszPassword, String pszDomain);

		int VmwDeployGetPartnerDomain(String pszServer,
				PointerByReference ppszDomain);

		int VmwDeployGetPartnerSiteName(String pszServer,
				PointerByReference ppszDomain);

		void VmwDeployFreeMemory(Pointer pMemory);

	}

	public static void configureStandalone(
			DomainControllerStandaloneParams params)
			throws DomainControllerNativeException {

		DeployUtilsParamsNative paramsNative = new DeployUtilsParamsNative(
				params.getHostname(), params.getDomainName(),
				params.getPassword(), DirectorySetupMode.STANDALONE.getCode(),
				null, params.getSite(),
                                params.getDNSForwarders(),
                                params.getSubjectAltName());

		setupInstance(paramsNative);
	}

	public static void configurePartner(DomainControllerPartnerParams params)
			throws DomainControllerNativeException {

		DeployUtilsParamsNative paramsNative = new DeployUtilsParamsNative(
				params.getHostname(), params.getDomainName(), params.getPassword(),
				DirectorySetupMode.PARTNER.getCode(), params.getServer(),
				params.getSite(),
                                params.getDNSForwarders(),
                                params.getSubjectAltName());

		setupInstance(paramsNative);

	}

	public static void validatePartnerCredentials(String server,
			String password, String domain)
			throws DomainControllerNativeException {
		int errorCode = DeployUtilsLibrary.INSTANCE
				.VmwDeployValidatePartnerCredentials(server, password, domain);
		DeployUtilsAdapterErrorHandler.handleErrorCode(errorCode);
	}

	public static String getPartnerDomain(String server) throws DomainControllerNativeException {
		PointerByReference ppDomain = new PointerByReference();
		try {
			int errorCode = DeployUtilsLibrary.INSTANCE
					.VmwDeployGetPartnerDomain(server, ppDomain);
			DeployUtilsAdapterErrorHandler.handleErrorCode(errorCode);
			return ppDomain.getValue().getString(0);
		} finally {
			if (ppDomain.getValue() != Pointer.NULL)
			{
				DeployUtilsLibrary.INSTANCE.VmwDeployFreeMemory(ppDomain.getValue());
			}
		}

	}

	public static String getPartnerSiteName(String server) throws DomainControllerNativeException {
		PointerByReference ppSite = new PointerByReference();
		try {
			int errorCode = DeployUtilsLibrary.INSTANCE
					.VmwDeployGetPartnerSiteName(server, ppSite);
			DeployUtilsAdapterErrorHandler.handleErrorCode(errorCode);
			return ppSite.getValue().getString(0);
		} finally {
			if (ppSite.getValue() != Pointer.NULL)
			{
				DeployUtilsLibrary.INSTANCE.VmwDeployFreeMemory(ppSite.getValue());
			}
		}
	}

	private static void setupInstance(DeployUtilsParamsNative paramsNative)
			throws DomainControllerNativeException {
		int errorCode = DeployUtilsLibrary.INSTANCE
				.VmwDeploySetupInstance(paramsNative.getPointer());
		DeployUtilsAdapterErrorHandler.handleErrorCode(errorCode);
	}
}
