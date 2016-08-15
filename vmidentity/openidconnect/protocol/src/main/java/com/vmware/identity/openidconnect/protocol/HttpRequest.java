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

package com.vmware.identity.openidconnect.protocol;

import java.io.UnsupportedEncodingException;
import java.net.URI;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;

import org.apache.commons.lang3.Validate;
import org.apache.http.NameValuePair;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.methods.HttpRequestBase;
import org.apache.http.message.BasicNameValuePair;

/**
 * @author Yehia Zayour
 */
public final class HttpRequest {
    public enum Method {
        GET,
        POST;
    }

    private final Method method;
    private final URI uri;
    private final Map<String, String> parameters;
    private final HttpServletRequest httpServletRequest;

    private HttpRequest(
            Method method,
            URI uri,
            Map<String, String> parameters,
            HttpServletRequest httpServletRequest) {
        this.method = method;
        this.uri = uri;
        this.parameters = parameters;
        this.httpServletRequest = httpServletRequest;
    }

    public static HttpRequest createGetRequest(URI uri) {
        Validate.notNull(uri, "uri");
        return new HttpRequest(
                Method.GET,
                uri,
                Collections.<String, String>emptyMap(),
                (HttpServletRequest) null);
    }

    public static HttpRequest createGetRequest(URI baseUri, Map<String, String> parameters) {
        Validate.notNull(baseUri, "baseUri");
        Validate.notNull(parameters, "parameters");
        URI uriWithQuery = URIUtils.appendQueryParameters(baseUri, parameters);
        return new HttpRequest(
                Method.GET,
                uriWithQuery,
                parameters,
                (HttpServletRequest) null);
    }

    public static HttpRequest createPostRequest(URI uri, Map<String, String> parameters) {
        Validate.notNull(uri, "uri");
        Validate.notNull(parameters, "parameters");
        return new HttpRequest(
                Method.POST,
                uri,
                parameters,
                (HttpServletRequest) null);
    }

    public static HttpRequest from(HttpServletRequest httpServletRequest) {
        Validate.notNull(httpServletRequest, "httpServletRequest");

        Method method;
        if (httpServletRequest.getMethod().equalsIgnoreCase("POST")) {
            method = Method.POST;
        } else if (httpServletRequest.getMethod().equalsIgnoreCase("GET")) {
            method = Method.GET;
        } else {
            throw new IllegalArgumentException("unsupported http request method: " + httpServletRequest.getMethod());
        }

        URI uri = URIUtils.from(httpServletRequest);

        Map<String, String> parameters = new HashMap<String, String>();
        for (Map.Entry<String, String[]> entry : httpServletRequest.getParameterMap().entrySet()) {
            if (entry.getValue().length != 1) {
                throw new IllegalArgumentException("HttpServletRequest parameter map must have a single entry for every key. " + entry);
            }
            parameters.put(entry.getKey(), entry.getValue()[0]);
        }

        return new HttpRequest(
                method,
                uri,
                parameters,
                httpServletRequest);
    }

    public Method getMethod() {
        return this.method;
    }

    public URI getURI() {
        return this.uri;
    }

    public Map<String, String> getParameters() {
        return this.parameters;
    }

    public String getHeaderValue(String headerName) {
        Validate.notEmpty(headerName, "headerName");
        return (this.httpServletRequest == null) ? null : this.httpServletRequest.getHeader(headerName);
    }

    public String getCookieValue(String cookieName) {
        Validate.notEmpty(cookieName, "cookieName");

        if (this.httpServletRequest != null && this.httpServletRequest.getCookies() != null) {
            for (Cookie cookie : this.httpServletRequest.getCookies()) {
                if (cookie.getName().equals(cookieName)) {
                    return cookie.getValue();
                }
            }
        }
        return null;
    }

    public List<X509Certificate> getClientCertificateChain() {
        List<X509Certificate> result = null;
        if (this.httpServletRequest != null) {
            X509Certificate[] certArray = (X509Certificate[]) this.httpServletRequest.getAttribute("javax.servlet.request.X509Certificate");
            if (certArray != null) {
                result = Collections.unmodifiableList(Arrays.asList(certArray));
            }
        }
        return result;
    }

    public HttpRequestBase toHttpTask() {
        if (this.method == Method.GET) {
            return new HttpGet(this.uri.toString());
        }

        assert this.method == Method.POST;

        List<NameValuePair> nameValuePairs = new ArrayList<NameValuePair>();
        for (Map.Entry<String, String> entry : this.parameters.entrySet()) {
            nameValuePairs.add(new BasicNameValuePair(entry.getKey(), entry.getValue()));
        }

        UrlEncodedFormEntity form;
        try {
            form = new UrlEncodedFormEntity(nameValuePairs, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw new IllegalStateException("failed to construct UrlEncodedFormEntity", e);
        }

        HttpPost httpPost = new HttpPost(this.uri.toString());
        httpPost.setEntity(form);
        httpPost.setHeader("Content-Type", "application/x-www-form-urlencoded");
        return httpPost;
    }
}