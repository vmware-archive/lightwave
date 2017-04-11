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

import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;

import org.apache.commons.lang3.Validate;
import org.apache.http.NameValuePair;
import org.apache.http.client.utils.URIBuilder;
import org.apache.http.message.BasicNameValuePair;

import com.vmware.identity.openidconnect.common.ParseException;

/**
 * @author Yehia Zayour
 */
public final class URIUtils {
    private URIUtils() {
    }

    public static URI changePathComponent(URI uri, String path) {
        Validate.notNull(uri, "uri");
        Validate.notEmpty(path, "path");

        URIBuilder uriBuilder = new URIBuilder(uri);
        uriBuilder.setPath(path);
        try {
            return uriBuilder.build();
        } catch (URISyntaxException e) {
            throw new IllegalArgumentException("failed to change uri path component", e);
        }
    }

    public static URI changeHostComponent(URI uri, String host) {
        Validate.notNull(uri, "uri");
        Validate.notEmpty(host, "host");

        URIBuilder uriBuilder = new URIBuilder(uri);
        uriBuilder.setHost(host);
        try {
            return uriBuilder.build();
        } catch (URISyntaxException e) {
            throw new IllegalArgumentException("failed to change uri host component", e);
        }
    }

    public static URI changeSchemeComponent(URI uri, String scheme) {
        Validate.notNull(uri, "uri");
        Validate.notEmpty(scheme, "scheme");

        URIBuilder uriBuilder = new URIBuilder(uri);
        uriBuilder.setScheme(scheme);
        try {
            return uriBuilder.build();
        } catch (URISyntaxException e) {
            throw new IllegalArgumentException("failed to change uri scheme component", e);
        }
    }

    public static URI changePortComponent(URI uri, int port) {
        Validate.notNull(uri, "uri");

        URIBuilder uriBuilder = new URIBuilder(uri);
        uriBuilder.setPort(port);
        try {
            return uriBuilder.build();
        } catch (URISyntaxException e) {
            throw new IllegalArgumentException("failed to change port component", e);
        }
    }

    public static URI appendQueryParameter(URI uri, String parameterName, String parameterValue) {
        Validate.notNull(uri, "uri");
        Validate.notEmpty(parameterName, "parameterName");
        Validate.notEmpty(parameterValue, "parameterValue");

        URIBuilder uriBuilder = new URIBuilder(uri);
        uriBuilder.addParameter(parameterName, parameterValue);
        try {
            return uriBuilder.build();
        } catch (URISyntaxException e) {
            throw new IllegalArgumentException("failed to add parameter to uri", e);
        }
    }

    public static URI appendQueryParameters(URI uri, Map<String, String> parameters) {
        Validate.notNull(uri, "uri");
        Validate.notNull(parameters, "parameters");

        List<NameValuePair> pairs = new ArrayList<NameValuePair>(parameters.size());
        for (Map.Entry<String, String> entry : parameters.entrySet()) {
            pairs.add(new BasicNameValuePair(entry.getKey(), entry.getValue()));
        }

        URIBuilder uriBuilder = new URIBuilder(uri);
        uriBuilder.addParameters(pairs);
        try {
            return uriBuilder.build();
        } catch (URISyntaxException e) {
            throw new IllegalArgumentException("failed to add parameters to uri", e);
        }
    }

    public static URI appendFragmentParameters(URI uri, Map<String, String> parameters) {
        Validate.notNull(uri, "uri");
        Validate.notNull(parameters, "parameters");

        URI uriWithQuery = appendQueryParameters(uri, parameters);
        String uriWithFragmentString = uriWithQuery.toString().replace('?', '#');
        try {
            return URIUtils.parseURI(uriWithFragmentString);
        } catch (ParseException e) {
            throw new IllegalArgumentException("failed to convert uri query string to fragment", e);
        }
    }

    public static URI from(HttpServletRequest httpServletRequest) {
        Validate.notNull(httpServletRequest, "httpServletRequest");

        try {
            return new URI(httpServletRequest.getRequestURL().toString());
        } catch (URISyntaxException e) {
            throw new IllegalArgumentException("failed to construct uri off of HttpServletRequest", e);
        }
    }

    public static URI parseURI(String uriString) throws ParseException {
        Validate.notEmpty(uriString, "uriString");

        URI uri;
        try {
            uri = new URI(uriString);
        } catch (URISyntaxException e) {
            throw new ParseException("failed to parse uri", e);
        }

        if (uri.getScheme() == null) {
            throw new ParseException("uri must have a scheme");
        }

        return uri;
    }
}