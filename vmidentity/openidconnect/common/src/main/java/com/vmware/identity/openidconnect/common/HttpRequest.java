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

package com.vmware.identity.openidconnect.common;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URL;
import java.util.HashMap;
import java.util.Map;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;

import org.apache.commons.lang3.Validate;

/**
 * @author Yehia Zayour
 */
public class HttpRequest {
    private final HttpServletRequest httpServletRequest;
    private final Map<String, String> parameters;
    private final URL requestUrl;
    private final URI requestUri;

    private HttpRequest(
            HttpServletRequest httpServletRequest,
            Map<String, String> parameters,
            URL requestUrl,
            URI requestUri) {
        this.httpServletRequest = httpServletRequest;
        this.parameters = parameters;
        this.requestUrl = requestUrl;
        this.requestUri = requestUri;
    }

    public static HttpRequest create(HttpServletRequest httpServletRequest) {
        Validate.notNull(httpServletRequest, "httpServletRequest");

        Map<String, String> parameters = new HashMap<String, String>();
        for (Map.Entry<String, String[]> entry : httpServletRequest.getParameterMap().entrySet()) {
            parameters.put(entry.getKey(), entry.getValue()[0]); // just take the first value
        }

        String requestUrlString = httpServletRequest.getRequestURL().toString();
        URL requestUrl;
        try {
            requestUrl = new URL(requestUrlString);
        } catch (MalformedURLException e) {
            // this should not happen since the URL is formed off of httpServletRequest.getRequestURL()
            throw new IllegalArgumentException(e);
        }
        URI requestUri = URI.create(requestUrlString);

        return new HttpRequest(httpServletRequest, parameters, requestUrl, requestUri);
    }

    public Map<String, String> getParameters() {
        return this.parameters;
    }

    public URL getRequestUrl() {
        return this.requestUrl;
    }

    public URI getRequestUri() {
        return this.requestUri;
    }

    public String getCookieValue(String cookieName) {
        Validate.notEmpty(cookieName, "cookieName");

        String value = null;
        Cookie[] cookies = this.httpServletRequest.getCookies();
        if (cookies != null) {
            for (Cookie cookie : cookies) {
                if (cookie.getName().equals(cookieName)) {
                    value = cookie.getValue();
                    break;
                }
            }
        }
        return value;
    }
}
