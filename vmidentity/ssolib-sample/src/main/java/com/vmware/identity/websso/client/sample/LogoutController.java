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

import javax.servlet.http.HttpServletRequest;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.servlet.ModelAndView;
import org.springframework.web.servlet.view.RedirectView;

import com.vmware.identity.websso.client.MetadataSettings;
import com.vmware.identity.websso.client.SloRequestSettings;
import com.vmware.identity.websso.client.endpoint.SloRequestSender;

/**
 * Basic logout controller, simply constructs a REDIRECT to IDP.
 *
 */
@Controller
public class LogoutController {
    @Autowired
    private MetadataSettings metadataSettings;

    @Autowired
    private SloRequestSender sloRequestSender;

    /**
     * @return the metadataSettings
     */
    public MetadataSettings getMetadataSettings() {
        return metadataSettings;
    }

    /**
     * @param metadataSettings the metadataSettings to set
     */
    public void setMetadataSettings(MetadataSettings metadataSettings) {
        this.metadataSettings = metadataSettings;
    }

    /**
     * Handle logout
     */
    @RequestMapping(value = "/logout/{tenant:.*}", method = RequestMethod.GET)
    public ModelAndView logout (HttpServletRequest request, @PathVariable(value = "tenant") String tenant) {
        try {
            String redirectUrl = getSloRequestSender().getRequestUrl(
                    new SloRequestSettings(
                            tenant,
                            tenant,
                            true,
                            ComponentUtils.getSessionCookieValue(request),
                            SAMLNames.IDFORMAT_VAL_PERSIST,
                            ComponentUtils.getSessionIndexCookieValue(request),
                            null));
            RedirectView redirect = new RedirectView(redirectUrl);
            return new ModelAndView(redirect);
        } catch (Exception e) {
            return ComponentUtils.getErrorView(e);
        }
    }

    /**
     * @return the sloRequestSender
     */
    public SloRequestSender getSloRequestSender() {
        return sloRequestSender;
    }

    /**
     * @param sloRequestSender the sloRequestSender to set
     */
    public void setSloRequestSender(SloRequestSender sloRequestSender) {
        this.sloRequestSender = sloRequestSender;
    }
}
