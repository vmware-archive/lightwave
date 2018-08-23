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

package com.vmware.identity.websso.test.util;

import com.vmware.identity.websso.test.util.common.SAMLConstants;
import com.vmware.identity.websso.test.util.common.SSOConstants;

import org.apache.http.Header;
import org.apache.http.HttpResponse;
import org.apache.http.client.protocol.HttpClientContext;
import org.apache.http.cookie.Cookie;
import org.apache.http.impl.client.BasicCookieStore;
import org.apache.http.impl.cookie.BasicClientCookie;

import java.net.HttpCookie;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import junit.framework.Assert;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ResponseState {
    protected static final Logger log = LoggerFactory.getLogger(ResponseState.class);

    BasicCookieStore responseCookies;
    String responseName;
    Map<String, List<String>> responseHeaders;
    int statusCode;
    String responseContent;
    String statusReason;

    ResponseState(String responseName) {
        this.responseName = responseName;
        responseHeaders = new HashMap<String, List<String>>();
        responseCookies = new BasicCookieStore();
    }

    public String getLocation() {
      List<String> values = getHeaderValue(SSOConstants.HDR_LOCATION_KEY);
      if (values == null || values.isEmpty()) {
        return null;
      } else {
        return values.get(0);
      }
    }

    public List<String> getHeaderValue(String headerName) {
        return responseHeaders.get(headerName.toLowerCase());
    }

    public int saveResponseHeaders(Header[] responseHdrs) {
        for (int i = 0; i < responseHdrs.length; ++i) {
          String key = responseHdrs[i].getName().toLowerCase();
          List<String> value = responseHeaders.get(key);
          if (value == null) {
            value = new ArrayList<>();
            responseHeaders.put(key, value);
          }
          value.add(responseHdrs[i].getValue());
        }
        return responseHeaders.size();
    }

    public void setStatusCode(int statusCode) {
        this.statusCode = statusCode;
    }

    public int getStatusCode() {
        return statusCode;
    }

    public void setStatusReason(String reason) {
        this.statusReason = reason;
    }

    public String getStatusReason() {
        return statusReason;
    }

    public List<Cookie> getAllCookies() {
        return responseCookies != null ? responseCookies.getCookies() : null;
    }

    public Cookie getCookie(String cookieName) {
        Cookie result = null;
        if (responseCookies != null) {
            for (Cookie cookie : responseCookies.getCookies()) {
                if (cookieName.equalsIgnoreCase(cookie.getName())) {
                    result = cookie;
                    break;
                }
            }
        }
        return result;
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();

        sb.append(String.format("Response Status=%d Reason=%s\n", statusCode,
                                (statusReason == null) ? statusReason : "(null)"
        ));
        if (responseHeaders.size() > 0) {
            for (Map.Entry<String, List<String>> entry : responseHeaders.entrySet()) {
              for (String value : entry.getValue()) {
                sb.append(String.format("Response header [%s]:[%s]\n", entry.getKey(), value));
              }
            }
        }
        if (responseContent != null && !responseContent.isEmpty()) {
            sb.append("-------------------------------------------------\nResponse Body:\n\n");
            String[] lines = responseContent.split(System.getProperty("line.separator"));
            for (int i = 0; i < 10 && i < lines.length; ++i) {
                sb.append(lines[i]);
            }
        }
        if (responseCookies != null && responseCookies.getCookies().size() > 0) {
            for (Cookie cookie : responseCookies.getCookies()) {
                sb.append(String.format("Cookie[%s]=%s\n", cookie.getName(), cookie.getValue()));
            }
        } else {
            sb.append("Response has no cookies\n");
        }
        return sb.toString();
    }

    public void buildFromHttpResponse(HttpResponse response, HttpClientContext context) throws IllegalStateException {

        saveResponseHeaders(response.getAllHeaders());
        setStatusCode(response.getStatusLine().getStatusCode());
        setStatusReason(response.getStatusLine().getReasonPhrase());

        for (Map.Entry<String, List<String>> entry : responseHeaders.entrySet()) {
            if (!entry.getKey().equalsIgnoreCase(SAMLConstants.CookieHeaderName)) {
                continue;
            }
            List<String> cookieHeaderValues = entry.getValue();
            if (cookieHeaderValues != null) {
              for (String cookieHeaderValue : cookieHeaderValues) {
                // Sample Cookie format
                // CastleSession<domain name>=_1fc64145138421310ca7ad1f2d520d4b; Path=/; Secure; HttpOnly
                List<HttpCookie> cookies = HttpCookie.parse(cookieHeaderValue);
                Assert.assertNotNull("Could not parse cookies", cookies);
                BasicClientCookie cookie = new BasicClientCookie(cookies.get(0).getName(), cookies.get(0).getValue());
                responseCookies.addCookie(cookie);
              }
            }
        }
    }

    public void setResponseContent(String responseContent) {
        this.responseContent = responseContent;
    }

    public String getResponseContent() {
        return responseContent;
    }
}
