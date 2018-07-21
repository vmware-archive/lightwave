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

import org.apache.commons.lang.Validate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.servlet.ModelAndView;
import org.springframework.web.servlet.view.RedirectView;

import com.vmware.identity.websso.client.MetadataSettings;
import com.vmware.identity.websso.client.SsoRequestSettings;
import com.vmware.identity.websso.client.endpoint.SsoRequestSender;

/**
 * Basic logon controller, simply constructs a REDIRECT to IDP.
 *
 */
@Controller
public class LogonController {
	@Autowired
	private MetadataSettings metadataSettings;

	@Autowired
	private SsoRequestSender ssoRequestSender;

	/**
	 * @return the metadataSettings
	 */
	public MetadataSettings getMetadataSettings() {
		return metadataSettings;
	}

	/**
	 * @param metadataSettings
	 *            the metadataSettings to set
	 */
	public void setMetadataSettings(MetadataSettings metadataSettings) {
		this.metadataSettings = metadataSettings;
	}

	/**
	 * Handle logon
	 */
	@RequestMapping(value = "/logon/{tenant:.*}", method = RequestMethod.GET)
	public ModelAndView logon(@PathVariable(value = "tenant") String tenant) {
		try {
			SsoRequestSettings requestSettings = new SsoRequestSettings(tenant,
					tenant, true, SAMLNames.IDFORMAT_VAL_PERSIST, true, // allowProxy
					false, false, null, // acs index
					null, // acs URL
					null // relayState
			);
			// request renewable and delegable token which can be used for ActAs
			// later
			// turned on when testing against lotus and when requires
			// the token to be delegable and renewable.
			// We should turn off this setting when used with ADFS because is
			// causing problem with sample app as SP for ADFS
			// setup.
			requestSettings.setIsDelegable(true);
			requestSettings.setIsRenewable(true);
			requestSettings.setProxyCount(1);
			String acsURL = metadataSettings.getSPConfiguration(tenant)
					.getAssertionConsumerServices().get(0).getLocation();
			Validate.notEmpty(acsURL,
					"SSOServer-as-SP role ACS URL should be non-empty!");
			requestSettings.setAssertionConsumerServiceUrl(acsURL);
			String redirectUrl = getSsoRequestSender().getRequestUrl(
					requestSettings);
			RedirectView redirect = new RedirectView(redirectUrl);
			return new ModelAndView(redirect);
		} catch (Exception e) {
			return ComponentUtils.getErrorView(e);
		}
	}

	/**
	 * @return the ssoRequestSender
	 */
	public SsoRequestSender getSsoRequestSender() {
		return ssoRequestSender;
	}

	/**
	 * @param ssoRequestSender
	 *            the ssoRequestSender to set
	 */
	public void setSsoRequestSender(SsoRequestSender ssoRequestSender) {
		this.ssoRequestSender = ssoRequestSender;
	}
}
