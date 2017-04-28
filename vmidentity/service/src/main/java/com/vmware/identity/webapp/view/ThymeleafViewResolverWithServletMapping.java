/*
 *  Copyright (c) 2017 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.webapp.view;

import java.util.Locale;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.web.servlet.View;
import org.springframework.web.servlet.view.InternalResourceView;
import org.thymeleaf.spring4.view.ThymeleafViewResolver;

public class ThymeleafViewResolverWithServletMapping extends ThymeleafViewResolver {

    private static final Logger vrlogger = LoggerFactory.getLogger(ThymeleafViewResolverWithServletMapping.class);

    @Override
    protected View createView(final String viewName, final Locale locale) throws Exception {
        if (!canHandle(viewName, locale)) {
            vrlogger.trace("[THYMELEAF] View \"{}\" cannot be handled by ThymeleafViewResolverWithServletMapping. Passing on to the next resolver in the chain.", viewName);
            return null;
        }
        if (viewName.startsWith(REDIRECT_URL_PREFIX)) {
            vrlogger.trace("[THYMELEAF] View \"{}\" is a redirect, and will not be handled directly by ThymeleafViewResolverWithServletMapping.", viewName);
            final String redirectUrl = viewName.substring(REDIRECT_URL_PREFIX.length());
            return new RedirectViewWithServletMapping(redirectUrl, isRedirectContextRelative(), isRedirectHttp10Compatible());
        }
        if (viewName.startsWith(FORWARD_URL_PREFIX)) {
            vrlogger.trace("[THYMELEAF] View \"{}\" is a forward, and will not be handled directly by ThymeleafViewResolverWithServletMapping.", viewName);
            final String forwardUrl = viewName.substring(FORWARD_URL_PREFIX.length());
            return new InternalResourceView(forwardUrl);
        }
        vrlogger.trace("[THYMELEAF] View {} will be handled by ThymeleafViewResolver and a " +
                "{} instance will be created for it", viewName, getViewClass().getSimpleName());
        return loadView(viewName, locale);
    }

}
