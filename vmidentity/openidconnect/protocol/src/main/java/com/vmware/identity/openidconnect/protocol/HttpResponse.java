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

import java.io.IOException;
import java.io.PrintWriter;
import java.net.URI;
import java.util.ArrayList;
import java.util.List;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletResponse;

import net.minidev.json.JSONObject;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.StatusCode;

/**
 * @author Yehia Zayour
 */
public final class HttpResponse {
    private final StatusCode statusCode;
    private final String htmlContent;
    private final JSONObject jsonContent;
    private final URI redirectTarget;
    private final ErrorObject errorObject;
    private final List<Header> headers;
    private final List<Cookie> cookies;

    private HttpResponse(
            StatusCode statusCode,
            String htmlContent,
            JSONObject jsonContent,
            URI redirectTarget,
            ErrorObject errorObject) {
        this.statusCode = statusCode;
        this.htmlContent = htmlContent;
        this.jsonContent = jsonContent;
        this.redirectTarget = redirectTarget;
        this.errorObject = errorObject;
        this.headers = new ArrayList<Header>();
        this.cookies = new ArrayList<Cookie>();
    }

    public static HttpResponse createErrorResponse(ErrorObject errorObject) {
        Validate.notNull(errorObject, "errorObject");
        return new HttpResponse(
                errorObject.getStatusCode(),
                null, // htmlContent
                (JSONObject) null,
                (URI) null,
                errorObject);
    }

    public static HttpResponse createJsonResponse(ErrorObject errorObject) {
        Validate.notNull(errorObject, "errorObject");
        return new HttpResponse(
                errorObject.getStatusCode(),
                null, // htmlContent
                ErrorObjectMapper.toJSONObject(errorObject),
                (URI) null,
                (ErrorObject) null);
    }

    public static HttpResponse createJsonResponse(StatusCode statusCode, JSONObject jsonContent) {
        Validate.notNull(statusCode, "statusCode");
        Validate.notNull(jsonContent, "jsonContent");
        return new HttpResponse(
                statusCode,
                null, // htmlContent
                jsonContent,
                (URI) null,
                (ErrorObject) null);
    }

    public static HttpResponse createHtmlResponse(StatusCode statusCode, String htmlContent) {
        Validate.notNull(statusCode, "statusCode");
        Validate.notEmpty(htmlContent, "htmlContent");
        return new HttpResponse(
                statusCode,
                htmlContent,
                (JSONObject) null,
                (URI) null,
                (ErrorObject) null);
    }

    public static HttpResponse createRedirectResponse(URI redirectTarget) {
        Validate.notNull(redirectTarget, "redirectTarget");
        return new HttpResponse(
                StatusCode.FOUND,
                null, // htmlContent
                (JSONObject) null,
                redirectTarget,
                (ErrorObject) null);
    }

    public void applyTo(HttpServletResponse httpServletResponse) throws IOException {
        Validate.notNull(httpServletResponse, "httpServletResponse");

        for (Cookie cookie : this.cookies) {
            httpServletResponse.addCookie(cookie);
        }

        for (Header header : this.headers) {
            httpServletResponse.addHeader(header.getName(), header.getValue());
        }

        if (this.errorObject != null) {
            httpServletResponse.sendError(
                    this.errorObject.getStatusCode().getValue(),
                    this.errorObject.getErrorCode().getValue() + ": " + this.errorObject.getDescription());
        } else if (this.redirectTarget != null) {
            httpServletResponse.sendRedirect(this.redirectTarget.toString());
        } else if (this.htmlContent != null) {
            httpServletResponse.setStatus(this.statusCode.getValue());
            httpServletResponse.setHeader("Cache-Control", "no-cache, no-store");
            httpServletResponse.setHeader("Pragma", "no-cache");
            httpServletResponse.setContentType("text/html;charset=UTF-8");
            PrintWriter writer = httpServletResponse.getWriter();
            writer.print(this.htmlContent);
            writer.close();
        } else if (this.jsonContent != null) {
            httpServletResponse.setStatus(this.statusCode.getValue());
            httpServletResponse.setHeader("Cache-Control", "no-store");
            httpServletResponse.setHeader("Pragma", "no-cache");
            httpServletResponse.setContentType("application/json;charset=UTF-8");
            PrintWriter writer = httpServletResponse.getWriter();
            writer.print(this.jsonContent.toString());
            writer.close();
        } else {
            throw new IllegalStateException("unexpected");
        }
   }

    public StatusCode getStatusCode() {
        return this.statusCode;
    }

    public String getHtmlContent() {
        return this.htmlContent;
    }

    public JSONObject getJsonContent() {
        return this.jsonContent;
    }

    public URI getRedirectTarget() {
        return this.redirectTarget;
    }

    public void addHeader(Header header) {
        Validate.notNull(header, "header");
        this.headers.add(header);
    }

    public void addCookie(Cookie cookie) {
        Validate.notNull(cookie, "cookie");
        this.cookies.add(cookie);
    }
}