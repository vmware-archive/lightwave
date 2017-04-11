/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity;

import java.io.IOException;
import java.util.Locale;
import java.util.UUID;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.opensaml.saml2.core.AuthnRequest;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.MessageSource;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.servlet.ModelAndView;
import org.springframework.web.servlet.view.RedirectView;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.samlservice.DefaultIdmAccessorFactory;
import com.vmware.identity.samlservice.DefaultSamlServiceFactory;
import com.vmware.identity.samlservice.IdmAccessor;
import com.vmware.identity.samlservice.SamlService;
import com.vmware.identity.samlservice.SamlServiceFactory;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.Shared;

/**
 * Controller for IdP initiated SSO requests
 *
 */
@Controller
public class IdpSsoController {
	// query string parameters
	public static final String PROVIDER_ENTITY_ID = "EntityID";
	public static final String NAME_ID_FORMAT = "NameIDFormat";
	public static final String ASSERTION_CONSUMER_SERVICE_INDEX = "AssertionConsumerServiceIndex";
	public static final String ATTRIBUTE_CONSUMER_SERVICE_INDEX = "AttributeConsumerServiceIndex";
	public static final String FORCE_AUTHN = "ForceAuthn";
	public static final String IS_PASSIVE = "IsPassive";

	@Autowired
	private MessageSource messageSource;

	private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(IdpSsoController.class);

	public IdpSsoController() {}

	/**
	 * Handle authentication request
	 */
	@RequestMapping(value = "/websso/SAML2/IDPSSO/{tenant:.*}", method = RequestMethod.GET)
	public ModelAndView sso(Locale locale, @PathVariable(value = "tenant") String tenant, HttpServletRequest request, HttpServletResponse response) throws IOException {
		logger.info("Welcome to IDP-initiated AuthnRequest handler! " +
				"The client locale is "+ locale.toString() + ", tenant is " + tenant);

		ModelAndView retval = null;

		//TODO - check for correlation id in the headers PR1561606
        String correlationId = UUID.randomUUID().toString();
		// load up config
		DefaultIdmAccessorFactory factory = new DefaultIdmAccessorFactory(correlationId);
		try {
			SamlServiceFactory samlFactory = new DefaultSamlServiceFactory();
			SamlService service = samlFactory.createSamlService(null, null, null, null, null);
			IdmAccessor accessor = factory.getIdmAccessor();
			accessor.setTenant(tenant);
			String destination = accessor.getIdpSsoEndpoint();

			// get parameters from the request
			String providerEntityID = request.getParameter(PROVIDER_ENTITY_ID);
			if (providerEntityID == null) {
				// this MUST be present
				logger.debug("MISSING entity ID in IDPSSO request");
				throw new Exception("Entity ID required");
			}
			String nameIDFormat = request.getParameter(NAME_ID_FORMAT);
			String assertionIndex = request.getParameter(ASSERTION_CONSUMER_SERVICE_INDEX);
			Integer index1 = null;
			if (assertionIndex != null) {
				index1 = Integer.parseInt(assertionIndex);
			}
			String attributeIndex = request.getParameter(ATTRIBUTE_CONSUMER_SERVICE_INDEX);
			Integer index2 = null;
			if (attributeIndex != null) {
				index2 = Integer.parseInt(attributeIndex);
			}
			String forceAuthn = request.getParameter(FORCE_AUTHN);
			Boolean b1 = null;
			if (forceAuthn != null) {
				b1 = Boolean.parseBoolean(forceAuthn);
			}
			String isPassive = request.getParameter(IS_PASSIVE);
			Boolean b2 = null;
			if (isPassive != null) {
				b2 = Boolean.parseBoolean(isPassive);
			}

			// create SAML request
			AuthnRequest authnRequest = service.createSamlAuthnRequest(
					null, // generate new id
					destination,
					providerEntityID,
					nameIDFormat,
					index1,
					index2,
					b1,
					b2);

			// send redirect
			logger.info("AuthnRequest formed {}", authnRequest);
			String samlRequestParameter = service.encodeSAMLObject(authnRequest);

			StringBuilder stringBuilder = new StringBuilder();
			stringBuilder.append(authnRequest.getDestination());
			stringBuilder.append("?");
			stringBuilder.append(
			        service.generateRedirectUrlQueryStringParameters(
			                samlRequestParameter, null,
			                request.getParameter(Shared.RELAY_STATE_PARAMETER),
	                        null, null));
			String redirectUrl = stringBuilder.toString();

			logger.info("Redirecting you to: {}" , redirectUrl);
			RedirectView redirect = new RedirectView(redirectUrl);
			retval = new ModelAndView(redirect);

		} catch (Exception e) {
			logger.error("Caught exception", e);
			ValidationResult vr = new ValidationResult(
					HttpServletResponse.SC_BAD_REQUEST, "BadRequest", null);
			String message = vr.getMessage(messageSource, locale);
			response.sendError(
					vr.getResponseCode(),
					message);
			logger.info("Responded with ERROR " + vr.getResponseCode() + ", message " + message);
			retval = null;
		}

		return retval;
	}

	/**
	 * Default tenant is not enabled for IDP-initiated requests
	 * User MUST specify relying party URL as part of request,
	 *  then that means that user MUST know tenant as well
	 */
	@RequestMapping(value = "/websso/SAML2/IDPSSO")
	public void ssoDefaultTenantError(Locale locale, HttpServletResponse response) throws IOException {
		logger.info("IDP SSO error! The client locale is "+ locale.toString() + ", DEFAULT tenant");

		// use validation result code to return error to client
		ValidationResult vr = new ValidationResult(
				HttpServletResponse.SC_BAD_REQUEST, "BadRequest", "Binding");
		String message = vr.getMessage(messageSource, locale);
		response.sendError(
				vr.getResponseCode(),
				message);
		logger.info("Responded with ERROR " + vr.getResponseCode() + ", message " + message);
	}

	/**
	 * Handle request sent with a wrong binding
	 */
	@RequestMapping(value = "/websso/SAML2/IDPSSO/{tenant:.*}")
	public void ssoBindingError(Locale locale, @PathVariable(value = "tenant") String tenant, HttpServletResponse response) throws IOException {
		logger.info("IDP SSO binding error! The client locale is "+ locale.toString() + ", tenant is " + tenant);

		ssoDefaultTenantError(locale, response);
	}

	public MessageSource getMessageSource() {
		return messageSource;
	}

	public void setMessageSource(MessageSource ms) {
		messageSource = ms;
	}
}
