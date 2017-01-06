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

import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.MessageSource;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.samlservice.DefaultIdmAccessorFactory;
import com.vmware.identity.samlservice.IdmAccessor;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.Shared;

/**
 * Metadata export controller
 * This responds with SAML xml metadata for default or named tenant
 *
 */
@Controller
public class WebssoMetadataController {

    @Autowired
    private MessageSource messageSource;

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(WebssoMetadataController.class);
    private static final String SAML_METADATA_FILENAME = "vsphere.local.xml";

    /**
     * Handle GET request for the metadata
     */
    @RequestMapping(value = "/websso/SAML2/Metadata/{tenant:.*}", method = RequestMethod.GET)
    public void  metadata(Locale locale, @PathVariable(value = "tenant") String tenant, Model model, HttpServletResponse response) throws IOException {
        logger.info("Welcome to Metadata handler! " +
                "The client locale is "+ locale.toString() + ", tenant is " + tenant);

        //TODO - check for correlation id in the headers PR1561606
        String correlationId = UUID.randomUUID().toString();
        DefaultIdmAccessorFactory factory = new DefaultIdmAccessorFactory(correlationId);
        try {
            IdmAccessor accessor = factory.getIdmAccessor();
            accessor.setTenant(tenant);
            String metadata = accessor.exportConfigurationAsString();
            Validate.notNull(metadata);
            response.setHeader("Content-Disposition", "attachment; filename=" + SAML_METADATA_FILENAME);
            Shared.sendResponse(response, Shared.METADATA_CONTENT_TYPE, metadata);
        } catch (Exception e) {
            logger.error("Caught exception", e);
            ValidationResult vr = new ValidationResult(
                    HttpServletResponse.SC_BAD_REQUEST, "BadRequest", null);
            String message = vr.getMessage(messageSource, locale);
            response.sendError(
                    vr.getResponseCode(),
                    message);
            logger.info("Responded with ERROR " + vr.getResponseCode() + ", message " + message);
        }

        model.addAttribute("tenant", tenant);
    }

    /**
     * Handle GET request for the metadata for default tenant
     */
    @RequestMapping(value = "/websso/SAML2/Metadata", method = RequestMethod.GET)
    public void metadataDefaultTenant(Locale locale, Model model, HttpServletResponse response) throws IOException {
        logger.info("Welcome to Metadata handler! The client locale is {}, DEFAULT tenant",locale.toString());

        metadata(locale, Shared.getDefaultTenant(), model, response);
    }

    /**
     * Handle request sent with a wrong binding
     */
    @RequestMapping(value = "/websso/SAML2/Metadata/{tenant:.*}")
    public void metadataError(Locale locale, @PathVariable(value = "tenant") String tenant, HttpServletResponse response) throws IOException {
        logger.info("Metadata binding error! The client locale is {}, tenant is {}",locale.toString() , tenant);

        metadataDefaultTenantBindingError(locale, response);
    }

    /**
     * Handle default tenant request sent with a wrong binding
     */
    @RequestMapping(value = "/websso/SAML2/Metadata")
    public void metadataDefaultTenantBindingError(Locale locale, HttpServletResponse response) throws IOException {
        logger.info("Metadata binding error! The client locale is {}, DEFAULT tenant",locale.toString());

        // use validation result code to return error to client
        ValidationResult vr = new ValidationResult(
                HttpServletResponse.SC_BAD_REQUEST, "BadRequest", "Binding");
        String message = vr.getMessage(messageSource, locale);
        response.sendError(vr.getResponseCode(), message);
        logger.info("Responded with ERROR " + vr.getResponseCode()
                + ", message " + message);
    }


    public MessageSource getMessageSource() {
        return messageSource;
    }

    public void setMessageSource(MessageSource ms) {
        messageSource = ms;
    }
}
