/*
 *  Copyright (c) 2012-2018 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */
package com.vmware.identity.websso.client.sample;

import javax.annotation.PostConstruct;
import javax.servlet.http.HttpServletResponse;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

import com.vmware.identity.websso.client.IDPConfiguration;
import com.vmware.identity.websso.client.MetadataSettings;
import com.vmware.identity.websso.client.SPConfiguration;

/**
 * Export/import metadata pages are controlled from here
 *
 */
@Controller
public class MetadataController {
	public static final String METADATA_CONTENT_TYPE = "application/samlmetadata+xml";

	@Autowired
	private MetadataSettings metadataSettings;

	@Autowired
	private String identityProviderFQDN;

	@Autowired
	private String tenantAdminUsername;

	@Autowired
	private String tenantAdminPassword;

	@Autowired
	private String spPort;

	@Autowired
	private String tenantName;

	/**
	 * @return the metadataSettings
	 */
	public MetadataSettings getMetadataSettings() {
		return metadataSettings;
	}

	/**
	 * @param metadataSettings
	 *            the metadataSettings to set
	 * @throws Exception
	 */
	public void setMetadataSettings(MetadataSettings metadataSettings) {
		this.metadataSettings = metadataSettings;
	}

	/**
	 * Export metadata
	 *
	 * @throws Exception
	 */
	@RequestMapping(value = "/SsoClient/Metadata/{tenant:.*}", method = RequestMethod.GET)
	public void export(@PathVariable(value = "tenant") String tenant,
			HttpServletResponse response) throws Exception {
		SPConfiguration spConfiguration = this.metadataSettings
				.getSPConfiguration(tenant);
		String metadata = ComponentUtils
				.getMetadataFromSPConfiguration(spConfiguration);

		ComponentUtils.sendResponse(response, METADATA_CONTENT_TYPE, metadata);
	}

	/**
	 * Export metadata to IDP
	 *
	 * @throws Exception
	 */
	@RequestMapping(value = "/SsoClient/Metadata/export/{tenant:.*}", method = RequestMethod.GET)
	public String exportToIDPGet(@PathVariable(value = "tenant") String tenant,
			Model model) throws Exception {
		SPConfiguration spConfiguration = this.metadataSettings
				.getSPConfiguration(tenant);
		String metadata = ComponentUtils
				.getMetadataFromSPConfiguration(spConfiguration);

		ExportForm exportForm = new ExportForm();
		exportForm.setMetadata(metadata);
		exportForm.setIdentityProviderFQDN(getIdentityProviderFQDN());
		model.addAttribute("tenant", tenant);
		model.addAttribute("exportForm", exportForm);
		return "export";
	}

	/**
	 * Handles POST of metadata export to IDP
	 *
	 * @throws Exception
	 */
	@RequestMapping(value = "/SsoClient/Metadata/export/{tenant:.*}", method = RequestMethod.POST)
	public String exportToIDPPost(
			@PathVariable(value = "tenant") String tenant, Model model,
			ExportForm exportForm) throws Exception {
		// take metadata and import it to IDP

		model.addAttribute("tenant", tenant);
		model.addAttribute("exportForm", exportForm);

		try {
			ServiceLocator serviceLocator = new ServiceLocator(
					exportForm.getIdentityProviderFQDN(), getTenantNameInIDP());
			ComponentUtils.exportSPConfigToIDP(serviceLocator,
					getTenantAdminUsername(), getTenantAdminPassword(),
					tenant, exportForm.getMetadata());
		} catch (Exception e) {
			model.addAttribute("Data",
					e.getMessage() + "\n\n" + ComponentUtils.getStackTrace(e));
			return "error";
		}

		exportForm.setMessage("Export successful to "
				+ exportForm.getIdentityProviderFQDN());

		return "export";
	}

	/**
	 * Shows form for metadata import
	 */
	@RequestMapping(value = "/SsoClient/Metadata/import/{tenant:.*}", method = RequestMethod.GET)
	public String importGet(@PathVariable(value = "tenant") String tenant,
			Model model) {
		model.addAttribute("tenant", tenant);
		model.addAttribute("importForm", new ImportForm());
		return "import";
	}

	/**
	 * Handles POST of metadata import
	 *
	 * @throws Exception
	 */
	@RequestMapping(value = "/SsoClient/Metadata/import/{tenant:.*}", method = RequestMethod.POST)
	public String importPost(@PathVariable(value = "tenant") String tenant,
			Model model, ImportForm importForm) throws Exception {
		// take metadata and import it
		IDPConfiguration idpConfiguration = ComponentUtils
				.getIDPConfigurationFromMetadata(tenant,
						importForm.getMetadata());
		this.getMetadataSettings().addIDPConfiguration(idpConfiguration);
		importForm.setMessage("Import successful");
		model.addAttribute("tenant", tenant);
		model.addAttribute("importForm", importForm);
		return "import";
	}

	/**
	 * @return the identityProviderFQDN
	 */
	public String getIdentityProviderFQDN() {
		return identityProviderFQDN;
	}

	/**
	 * @param identityProviderFQDN
	 *            the identity provider FQDN to set
	 * @throws Exception
	 */
	public void setIdentityProviderFQDN(String identityProviderFQDN) {
		this.identityProviderFQDN = identityProviderFQDN;
	}

	/**
	 * @param TenantAdminUsername
	 *            the tenant username used by websso and sts
	 * @throws Exception
	 */
	public void setTenantAdminUsername(String tenantAdminUsername) {
		this.tenantAdminUsername = tenantAdminUsername;
	}

	/**
	 * @return the tenantAdminUsername
	 */
	public String getTenantAdminUsername() {
		return tenantAdminUsername;
	}

	/**
	 * @param TenantAdminPassword
	 *            the tenant password used by websso and sts
	 * @throws Exception
	 */
	public void setTenantAdminPassword(String tenantAdminPassword) {
		this.tenantAdminPassword = tenantAdminPassword;
	}

	/**
	 * @return the tenantAdminPassword
	 */
	public String getTenantAdminPassword() {
		return tenantAdminPassword;
	}

	/**
	 * @param spPort
	 *            the port used by the SP
	 */
	public void setSPPortPassword(String spPort) {
		this.spPort = spPort;
	}

	/**
	 * @return the SP port
	 */
	public String getSPPort() {
		return spPort;
	}

	/**
	 * @return the tenant name
	 */
	public String getTenantNameInIDP() {
		return tenantName;
	}

	/**
	 * Try and populate Org settings if all parameters are auto-wired
	 *
	 * @throws Exception
	 */
	@PostConstruct
	public void tryPopulateSettings() throws Exception {
		if (getMetadataSettings() != null && getIdentityProviderFQDN() != null) {
			ComponentUtils.populateSettings(getMetadataSettings(),
					getIdentityProviderFQDN(), getTenantAdminUsername(),
					getTenantAdminPassword(), getTenantNameInIDP(), getSPPort());
		}
	}
}
