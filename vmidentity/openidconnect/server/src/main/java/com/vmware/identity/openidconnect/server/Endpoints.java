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

package com.vmware.identity.openidconnect.server;

import java.net.URI;

import com.vmware.identity.openidconnect.protocol.URIUtils;

/**
 * @author Yehia Zayour
 */
public class Endpoints {
    public static final String BASE = "/openidconnect";
    public static final String AUTHENTICATION = "/oidc/authorize";
    public static final String AUTHENTICATION_CAC_RPROXY = "/oidc/cac_rproxy";
    public static final String AUTHENTICATION_CAC_TOMCAT = "/oidc/cac_tomcat";
    public static final String TOKEN = "/token";
    public static final String LOGOUT = "/logout";
    public static final String JWKS = "/jwks";
    public static final String METADATA = "/.well-known/openid-configuration";

    // choose cac endpoint to return to login form based on how the request arrived
    // https means request came directly to tomcat, http means it went through reverse proxy
    public static String authenticationCacForLoginForm(URI requestUri) {
        return ("https").equalsIgnoreCase(requestUri.getScheme()) ?
                AUTHENTICATION_CAC_TOMCAT :
                AUTHENTICATION_CAC_RPROXY;
    }

    // the client_assertion audience is expected to be the authorization endpoint (/oidc/authorize), but
    // login using TLS client cert (smart card / CAC) will arrive at the cac endpoint instead (/oidc/cac), so
    // we do this uri transformation to allow the client_assertion to pass validation
    public static URI normalizeAuthenticationRequestURI(URI requestUri) {
        URI result;
        if (requestUri.getPath().contains(AUTHENTICATION_CAC_RPROXY)) {
            result = URIUtils.changePathComponent(requestUri, requestUri.getPath().replace(AUTHENTICATION_CAC_RPROXY, AUTHENTICATION));
        } else if (requestUri.getPath().contains(AUTHENTICATION_CAC_TOMCAT)) {
            result = URIUtils.changePathComponent(requestUri, requestUri.getPath().replace(AUTHENTICATION_CAC_TOMCAT, AUTHENTICATION));
        } else {
            result = requestUri;
        }
        return result;
    }
}