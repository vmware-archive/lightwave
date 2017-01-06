/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.websso.client.LogonProcessorEx;
import com.vmware.identity.websso.client.LogoutProcessor;
import com.vmware.identity.websso.client.MessageStore;
import com.vmware.identity.websso.client.MetadataSettings;
import com.vmware.identity.websso.client.endpoint.SloListener;
import com.vmware.identity.websso.client.endpoint.SsoResponseListener;

@Controller
public class SPSsoSloController {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(SPSsoSloController.class);

    private final SsoResponseListener ssolibSSOListener;
    private final SloListener ssolibSLOListener;

    @Autowired
    SPSsoSloController(MetadataSettings metadataSettings, MessageStore messageStore, LogonProcessorEx logonProcessor, LogoutProcessor logoutProcessor) {

        ssolibSSOListener = new SsoResponseListener();
        ssolibSLOListener = new SloListener();

        ssolibSSOListener.setLogonProcessor(logonProcessor);
        ssolibSSOListener.setMessageStore(messageStore);
        ssolibSSOListener.setMetadataSettings(metadataSettings);
        ssolibSLOListener.setLogoutProcessor(logoutProcessor);
        ssolibSLOListener.setMessageStore(messageStore);
        ssolibSLOListener.setMetadataSettings(metadataSettings);

        logger.info("SPSsoSloController initialized");
    }

    /**
     * SAML Service Provider ACS endpoint. Process SAML Authentication Response
     * from external IDP.
     * 
     * @Locale locale Client machine Browser locale
     * @param request
     *            http request passed into the call
     * @param ssoResponseMapping
     *            client defined string bean
     * @return void
     * @throws IOException
     */
    @RequestMapping(value = "/websso/SAML2/SP/ACS/{tenant:.*}", method = { RequestMethod.POST })
    public void spSso(Locale locale, @PathVariable(value = "tenant") final String tenant, HttpServletRequest httpRequest,
            HttpServletResponse httpResponse)
            throws IOException {
        logger.info("SSO AuthnResponse is received at ACS endpoint! " + "Client locale: " + locale.toString() + ", Tenant: " + tenant);
        ssolibSSOListener.consumeResponse(locale, tenant, httpRequest, httpResponse);
    }

    /**
     * SAML Service Provider SLO endpoint. It handles SAML LogoutResponse and
     * LogoutRequest from external SAML IDP.
     * 
     * @param tenant
     * 
     * @param request
     *            httprequest captured
     * @param httpResponse
     *            http response obj for sending response.
     * @throws IOException
     * @throws Exception
     */
    @RequestMapping(value = "/websso/SAML2/SP/SLO/{tenant:.*}", method = RequestMethod.GET)
    public void slo(@PathVariable(value = "tenant") final String tenant, final HttpServletRequest httpRequest, final HttpServletResponse httpResponse) {
        logger.info("SLO request or response is received at SP SLO endpoint!" + " Tenant: " + tenant);
        ssolibSLOListener.slo(tenant, httpRequest, httpResponse);
    }
}
