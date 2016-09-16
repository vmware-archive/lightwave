/* **********************************************************************
 * Copyright 2014 VMware, Inc.  All rights reserved. VMware Confidential
 * *********************************************************************/

package com.vmware.pscsetup;

import java.net.InetAddress;

import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.vmware.identity.configure.DeployUtilsErrors;
import com.vmware.identity.configure.DomainControllerNativeException;
import com.vmware.identity.configure.IPlatformComponentInstaller;
import com.vmware.identity.configure.IPlatformInstallObserver;
import com.vmware.identity.configure.PlatformInstallComponent;
import com.vmware.identity.interop.Validate;
import com.vmware.pscsetup.interop.DeployUtilsAdapter;

public class PlatformServicesController {

	private IPlatformInstallObserver observer = null;

	public boolean setupInstanceStandalone(
			DomainControllerStandaloneParams standaloneParams)
			throws DomainControllerNativeException {

		Validate.validateNotNull(standaloneParams.getPassword(), "Password");
		Validate.validateNotEmpty(standaloneParams.getDomainName(), "Domain");

		setupInstance(standaloneParams);

		return true;
	}

	public boolean setupInstancePartner(
			DomainControllerPartnerParams partnerParams)
			throws DomainControllerNativeException {

		Validate.validateNotNull(partnerParams.getPassword(), "Password");
		Validate.validateNotEmpty(partnerParams.getServer(), "Server");
		Validate.validateNotEmpty(partnerParams.getDomainName(), "Domain");

		setupInstance(partnerParams);

		return true;
	}

	private void setupInstance(DomainControllerStandaloneParams params)
			throws DomainControllerNativeException {
		if (params.getHostname() == null || params.getHostname().isEmpty())
			try {
				params.setHostname(InetAddress.getLocalHost().getHostName());
			} catch (UnknownHostException e) {
				throw new DomainControllerNativeException(
						DeployUtilsErrors.ERROR_INVALID_NETNAME.getErrorCode(),
						e);
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

	public boolean validatePartnerCredentials(String server, String password,
			String domain) throws DomainControllerNativeException {
		DeployUtilsAdapter.validatePartnerCredentials(server, password, domain);

		return true;
	}

	public String getPartnerDomain(String server)
			throws DomainControllerNativeException {
		return DeployUtilsAdapter.getPartnerDomain(server);
	}

	public String getPartnerSiteName(String server)
			throws DomainControllerNativeException {
		return DeployUtilsAdapter.getPartnerSiteName(server);
	}

	public void setPlatformInstallObserver(IPlatformInstallObserver observer) {
		this.observer = observer;
	}

	private List<IPlatformComponentInstaller> getComponents(
			DomainControllerStandaloneParams standaloneParams) {
		List<IPlatformComponentInstaller> components = new ArrayList<IPlatformComponentInstaller>();
	        components.add(new AuthenticationFrameworkInstaller(standaloneParams));
		return components;
	}

	private void checkPrerequisites(DomainControllerStandaloneParams params) {
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
