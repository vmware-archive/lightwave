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

import java.io.IOException;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang3.Validate;

import com.nimbusds.oauth2.sdk.ErrorObject;
import com.nimbusds.oauth2.sdk.Response;
import com.nimbusds.oauth2.sdk.SerializeException;
import com.nimbusds.oauth2.sdk.http.HTTPResponse;

/**
 * @author Yehia Zayour
 */
public class HttpResponse {
    public static int SC_BAD_REQUEST           = HttpServletResponse.SC_BAD_REQUEST;
    public static int SC_UNAUTHORIZED          = HttpServletResponse.SC_UNAUTHORIZED;
    public static int SC_INTERNAL_SERVER_ERROR = HttpServletResponse.SC_INTERNAL_SERVER_ERROR;

    private final Response response;
    private final ServerException serverException;
    private final Header[] headers;
    private final Cookie[] cookies;

    private HttpResponse(
            Response response,
            ServerException serverException,
            Header[] headers,
            Cookie[] cookies) {
        this.response = response;
        this.serverException = serverException;
        this.headers = headers;
        this.cookies = cookies;
    }

    public static HttpResponse success(Response response) {
        Validate.notNull(response, "response");
        return new HttpResponse(response, (ServerException) null, (Header[]) null, (Cookie[]) null);
    }

    public static HttpResponse success(Response response, Cookie... cookies) {
        Validate.notNull(response, "response");
        return new HttpResponse(response, (ServerException) null, (Header[]) null, cookies);
    }

    public static HttpResponse error(ServerException serverException) {
        Validate.notNull(serverException, "serverException");
        return new HttpResponse((Response) null, serverException, serverException.getHeaders(), (Cookie[]) null);
    }

    public void applyTo(HttpServletResponse httpServletResponse) throws SerializeException, IOException {
        Validate.notNull(httpServletResponse, "httpServletResponse");

        if (this.cookies != null) {
            for (Cookie cookie : this.cookies) {
                if (cookie != null) {
                    httpServletResponse.addCookie(cookie);
                }
            }
        }

        if (this.headers != null) {
            for (Header header : this.headers) {
                if (header != null) {
                    httpServletResponse.addHeader(header.getName(), header.getValue());
                }
            }
        }

        if (this.response != null) {
            HTTPResponse nimbusHttpResponse = this.response.toHTTPResponse(); // throws SerializeException
            nimbusHttpResponse.applyTo(httpServletResponse); // throws IOException
        }

        if (this.serverException != null) {
            ErrorObject error = this.serverException.getErrorObject();
            httpServletResponse.sendError(error.getHTTPStatusCode(), error.getCode() + ": " + error.getDescription());
        }
    }
}
