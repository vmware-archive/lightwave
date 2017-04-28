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

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.Collections;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.springframework.util.CollectionUtils;
import org.springframework.util.StringUtils;
import org.springframework.web.servlet.FlashMap;
import org.springframework.web.servlet.FlashMapManager;
import org.springframework.web.servlet.HandlerMapping;
import org.springframework.web.servlet.support.RequestContextUtils;
import org.springframework.web.servlet.view.RedirectView;
import org.springframework.web.util.UriComponents;
import org.springframework.web.util.UriComponentsBuilder;
import org.springframework.web.util.WebUtils;

public class RedirectViewWithServletMapping extends RedirectView {

    private boolean contextRelative = false;

    private boolean http10Compatible = true;

    private boolean exposeModelAttributes = true;

    private String encodingScheme;

    private boolean expandUriTemplateVariables = true;

    /**
     * Constructor for use as a bean.
     */
    public RedirectViewWithServletMapping() {
        setExposePathVariables(false);
    }

    /**
     * Create a new RedirectViewWithServletMapping with the given URL.
     * <p>The given URL will be considered as relative to the web server,
     * not as relative to the current ServletContext.
     * @param url the URL to redirect to
     * @see #RedirectViewWithServletMapping(String, boolean)
     */
    public RedirectViewWithServletMapping(String url) {
        super(url);
        setExposePathVariables(false);
    }

    /**
     * Create a new RedirectViewWithServletMapping with the given URL.
     * @param url the URL to redirect to
     * @param contextRelative whether to interpret the given URL as
     * relative to the current ServletContext
     */
    public RedirectViewWithServletMapping(String url, boolean contextRelative) {
        super(url, contextRelative);
        this.contextRelative = contextRelative;
        setExposePathVariables(false);
    }

    /**
     * Create a new RedirectViewWithServletMapping with the given URL.
     * @param url the URL to redirect to
     * @param contextRelative whether to interpret the given URL as
     * relative to the current ServletContext
     * @param http10Compatible whether to stay compatible with HTTP 1.0 clients
     */
    public RedirectViewWithServletMapping(String url, boolean contextRelative, boolean http10Compatible) {
        super(url, contextRelative, http10Compatible);
        this.contextRelative = contextRelative;
        this.http10Compatible = http10Compatible;
        setExposePathVariables(false);
    }

    /**
     * Create a new RedirectViewWithServletMapping with the given URL.
     * @param url the URL to redirect to
     * @param contextRelative whether to interpret the given URL as
     * relative to the current ServletContext
     * @param http10Compatible whether to stay compatible with HTTP 1.0 clients
     * @param exposeModelAttributes whether or not model attributes should be
     * exposed as query parameters
     */
    public RedirectViewWithServletMapping(String url, boolean contextRelative, boolean http10Compatible, boolean exposeModelAttributes) {
        super(url, contextRelative, http10Compatible);
        this.contextRelative = contextRelative;
        this.http10Compatible = http10Compatible;
        this.exposeModelAttributes = exposeModelAttributes;
        setExposePathVariables(false);
    }

    /**
     * Set whether to interpret a given URL that starts with a slash ("/")
     * as relative to the current ServletContext, i.e. as relative to the
     * web application root.
     * <p>Default is "false": A URL that starts with a slash will be interpreted
     * as absolute, i.e. taken as-is. If "true", the context path will be
     * prepended to the URL in such a case.
     * @see javax.servlet.http.HttpServletRequest#getContextPath
     */
    @Override
    public void setContextRelative(boolean contextRelative) {
        super.setContextRelative(contextRelative);
        this.contextRelative = contextRelative;
    }

    /**
     * Set whether to stay compatible with HTTP 1.0 clients.
     * <p>In the default implementation, this will enforce HTTP status code 302
     * in any case, i.e. delegate to {@code HttpServletResponse.sendRedirect}.
     * Turning this off will send HTTP status code 303, which is the correct
     * code for HTTP 1.1 clients, but not understood by HTTP 1.0 clients.
     * <p>Many HTTP 1.1 clients treat 302 just like 303, not making any
     * difference. However, some clients depend on 303 when redirecting
     * after a POST request; turn this flag off in such a scenario.
     * @see javax.servlet.http.HttpServletResponse#sendRedirect
     */
    @Override
    public void setHttp10Compatible(boolean http10Compatible) {
        super.setHttp10Compatible(http10Compatible);
        this.http10Compatible = http10Compatible;
    }

    /**
     * Set the {@code exposeModelAttributes} flag which denotes whether
     * or not model attributes should be exposed as HTTP query parameters.
     * <p>Defaults to {@code true}.
     */
    @Override
    public void setExposeModelAttributes(final boolean exposeModelAttributes) {
        super.setExposeModelAttributes(exposeModelAttributes);
        this.exposeModelAttributes = exposeModelAttributes;
    }

    /**
     * Set the encoding scheme for this view.
     * <p>Default is the request's encoding scheme
     * (which is ISO-8859-1 if not specified otherwise).
     */
    @Override
    public void setEncodingScheme(String encodingScheme) {
        super.setEncodingScheme(encodingScheme);
        this.encodingScheme = encodingScheme;
    }

    /**
     * Whether to treat the redirect URL as a URI template.
     * Set this flag to {@code false} if the redirect URL contains open
     * and close curly braces "{", "}" and you don't want them interpreted
     * as URI variables.
     * <p>Defaults to {@code true}.
     */
    @Override
    public void setExpandUriTemplateVariables(boolean expandUriTemplateVariables) {
        super.setExpandUriTemplateVariables(expandUriTemplateVariables);
        this.expandUriTemplateVariables = expandUriTemplateVariables;
    }

    /**
     * Convert model to request parameters and redirect to the given URL.
     * @see #appendQueryProperties
     * @see #sendRedirect
     */
    @Override
    protected void renderMergedOutputModel(Map<String, Object> model, HttpServletRequest request,
            HttpServletResponse response) throws IOException {

        String targetUrl = createTargetUrlWithServletMapping(model, request);
        targetUrl = updateTargetUrl(targetUrl, model, request, response);

        FlashMap flashMap = RequestContextUtils.getOutputFlashMap(request);
        if (!CollectionUtils.isEmpty(flashMap)) {
            UriComponents uriComponents = UriComponentsBuilder.fromUriString(targetUrl).build();
            flashMap.setTargetRequestPath(uriComponents.getPath());
            flashMap.addTargetRequestParams(uriComponents.getQueryParams());
            FlashMapManager flashMapManager = RequestContextUtils.getFlashMapManager(request);
            if (flashMapManager == null) {
                throw new IllegalStateException("FlashMapManager not found despite output FlashMap having been set");
            }
            flashMapManager.saveOutputFlashMap(flashMap, request, response);
        }

        sendRedirect(request, response, targetUrl, this.http10Compatible);
    }

    /**
     * Create the target URL by checking if the redirect string is a URI template first,
     * expanding it with the given model, and then optionally appending simple type model
     * attributes as query String parameters.
     */
    protected String createTargetUrlWithServletMapping(Map<String, Object> model, HttpServletRequest request)
            throws UnsupportedEncodingException {

        // Prepare target URL.
        StringBuilder targetUrl = new StringBuilder();
        if (this.contextRelative && getUrl().startsWith("/")) {
            // Do not apply context path to relative URLs.
            targetUrl.append(request.getContextPath());
            targetUrl.append(request.getServletPath());
        }
        targetUrl.append(getUrl());

        String enc = this.encodingScheme;
        if (enc == null) {
            enc = request.getCharacterEncoding();
        }
        if (enc == null) {
            enc = WebUtils.DEFAULT_CHARACTER_ENCODING;
        }

        if (this.expandUriTemplateVariables && StringUtils.hasText(targetUrl)) {
            Map<String, String> variables = getCurrentRequestUriVariables(request);
            targetUrl = replaceUriTemplateVariables(targetUrl.toString(), model, variables, enc);
        }
        if (isPropagateQueryProperties()) {
            appendCurrentQueryParams(targetUrl, request);
        }
        if (this.exposeModelAttributes) {
            appendQueryProperties(targetUrl, model, enc);
        }

        return targetUrl.toString();
    }

    @SuppressWarnings("unchecked")
    private Map<String, String> getCurrentRequestUriVariables(HttpServletRequest request) {
        String name = HandlerMapping.URI_TEMPLATE_VARIABLES_ATTRIBUTE;
        Map<String, String> uriVars = (Map<String, String>) request.getAttribute(name);
        return (uriVars != null) ? uriVars : Collections.<String, String> emptyMap();
    }

}
