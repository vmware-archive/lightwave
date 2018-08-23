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
import com.vmware.identity.websso.test.util.processor.request.IRequestProcessor;
import com.vmware.identity.websso.test.util.processor.request.RequestType;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.apache.http.client.ClientProtocolException;
import org.apache.http.Header;
import org.apache.http.HttpMessage;
import org.apache.http.RequestLine;
import org.apache.http.client.utils.URIBuilder;
import org.apache.http.impl.client.BasicCookieStore;
import org.apache.http.impl.cookie.BasicClientCookie;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.cookie.Cookie;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.Map.Entry;

public class RequestState {
    private static final Logger log =
        LoggerFactory.getLogger(RequestState.class.getName());

    Map<String, String> queryString;
    Map<String, String> formVars;
    Map<String, String> requestHeaders;

    BasicCookieStore cookieStore = null;

    String requestName;
    String requestPath;
    String hostName;
    String tenantName;
    URI requestURI;
    RequestLine requestLine;
    RequestType verb = RequestType.GET;
    IRequestProcessor updater;

    RequestState() {
        HttpClientUtils.disableInfoLevel(RequestState.class.getName());
        queryString = new HashMap<String, String>();
        formVars = new HashMap<String, String>();
        requestHeaders = new HashMap<String, String>();
        cookieStore = new BasicCookieStore();
        updater = null;
    }

    public RequestState(String requestName) {
        this();
        this.requestName = requestName;
    }

    // This will construct request from previous response and request pair
    public RequestState(
        String requestName, RequestState prevRequest, ResponseState respState
    ) throws URISyntaxException { // Build a request from previous response
        // If the previous response contains a location header then the
      // request will be populated with headers
        this(requestName);
        if (respState.getLocation() != null) {
            setRequestURI(new URI(respState.getLocation()));
        } else if (prevRequest.getRequestURI() != null) {
            setRequestURI(prevRequest.getRequestURI());
        }
        List<Cookie> cookies = respState.getAllCookies();
        if (cookies != null) {
            addCookies(cookies);
        }
    }

    public void addCookie(String cookieName, String cookieValue) {
        cookieStore.addCookie(new BasicClientCookie(cookieName, cookieValue));
    }

    // Add cookies to the request that is created
    public void addCookies(List<Cookie> cookies) {
        for (Cookie cookie : cookies) {
            BasicClientCookie requestCookie =
                new BasicClientCookie(cookie.getName(), cookie.getValue());
            requestCookie.setDomain(this.hostName);
            requestCookie.setPath(this.requestPath);

            cookieStore.addCookie(requestCookie);
        }
    }

    public void addRequestUpdater(IRequestProcessor updater) {
        this.updater = updater;
    }

    public void setRequestURI(URI initialURI) {
        requestURI = initialURI;
        hostName = initialURI.getHost();
        requestPath = initialURI.getPath();
    }

    public void setRequestURI(String initialURI) throws URISyntaxException {
        this.setRequestURI(new URI(initialURI));
    }

    public void setVerb(RequestType verb) {
        this.verb = verb;
    }

    public URI getRequestURI() {
        return requestURI;
    }

    private void buildPostBody(HttpPost httppost)
        throws UnsupportedEncodingException {
        List<BasicNameValuePair> formParams =
            new ArrayList<BasicNameValuePair>();
        Iterator<Entry<String, String>> it = formVars.entrySet().iterator();
        for (Map.Entry<String, String> entry : formVars.entrySet()) {

            formParams
                .add(new BasicNameValuePair(entry.getKey(), entry.getValue()));

        } httppost.setEntity(new UrlEncodedFormEntity(formParams));
    }

    private void buildRequestPath() {
        StringBuilder sb = new StringBuilder();
        sb.append(SAMLConstants.SSOSTS_Path); sb.append("/");
        sb.append(tenantName); requestPath = sb.toString();
    }

    private void saveRequestData(HttpMessage httpRequest) {
        Header[] requestHdrs = httpRequest.getAllHeaders();
        for (int i = 0; i < requestHdrs.length; ++i) {
            requestHeaders
                .put(requestHdrs[i].getName(), requestHdrs[i].getValue());

        }

        if (httpRequest instanceof HttpGet) {
            HttpGet get = (HttpGet) httpRequest;
            requestLine = get.getRequestLine(); setRequestURI(get.getURI());
        }
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(String.format("Request: [%s]\n", requestName));
        sb.append(String.format("RequestURI: [%s]\n", requestURI));
        if (requestLine != null) {
            sb.append(String.format("RequestLine: [%s %s %s]\n",
                                    requestLine.getMethod(),
                                    requestLine.getUri(),
                                    requestLine.getProtocolVersion()
            ));
        }

        if (requestHeaders.size() > 0) {
            sb.append("Request Headers ----------\n");

            for (Map.Entry<String, String> entry : requestHeaders.entrySet())
                sb.append(String.format("Request header [%s]:[%s]\n",
                                        entry.getKey(), entry.getValue()
                ));

        }
        if (formVars.size() > 0) {
            sb.append("Request Form Vars ----------\n");
            for (Map.Entry<String, String> entry : formVars.entrySet())
                sb.append(String.format("Form Variable  [%s]=[%s]\n",
                                        entry.getKey(), entry.getValue()
                ));
        }
        if (queryString.size() > 0) {
            sb.append("Request QueryString --------\n");
            for (Map.Entry<String, String> entry : queryString.entrySet())
                sb.append(String
                              .format("QueryString [%s]=[%s]\n", entry.getKey(),
                                      entry.getValue()
                              ));
        }
        if (cookieStore.getCookies().size() > 0) {
            sb.append("Request cookies -------------\n");
            for (Cookie cookie : cookieStore.getCookies()) {
                sb.append(String.format("Cookie [%s]=[%s]\n", cookie.getName(),
                                        cookie.getValue()
                ));
            }
        }
        return sb.toString();
    }

    public static RequestState buildFromHttpRequest(
        String requestName, HttpMessage httpRequest
    ) {
        RequestState req = new RequestState(requestName);
        req.saveRequestData(httpRequest); return req;
    }

    public void addRequestHeader(String name, String value) {
        requestHeaders.put(name, value);
    }

    public void addFormVariable(String name, String value) {
        formVars.put(name, value);

    }

    public void setFormVars(Map<String, String> formVars) {
        this.formVars = formVars;
    }

    public void setQueryParams(Map<String, String> qsParams) {
        this.queryString = qsParams;
    }

    private void populateRequest(HttpMessage message) {
        buildRequestURI();
        for (Map.Entry<String, String> entry : requestHeaders.entrySet()) {
            message.addHeader(entry.getKey(), entry.getValue());
        }
    }

    private void buildRequestURI() {
        return;
    }

    public HttpGet buildGetRequest(TestHttpClient httpClient)
        throws URISyntaxException {
        if (verb != RequestType.GET) {
            throw new UnsupportedOperationException();
        }
        if (updater != null) {
            updater.updateRequestState(this);
        }
        HttpGet httpGet = new HttpGet(requestURI);
        populateRequest(httpGet);

        httpClient.setCookieStore(cookieStore);

        return httpGet;
    }

    public HttpPost buildPostRequest(TestHttpClient httpClient, Map<String, String> queryString)
        throws URISyntaxException {
        if (verb != RequestType.POST) {
            throw new UnsupportedOperationException();
        }
        if (updater != null) {
            updater.updateRequestState(this);
        }
        URIBuilder builder = new URIBuilder(requestURI);
        for (Map.Entry<String, String> entry : queryString.entrySet()) {
            builder.setParameter(entry.getKey(), entry.getValue());
        }
        HttpPost httpPost = new HttpPost(builder.build());
        populateRequest(httpPost);

        httpClient.setCookieStore(cookieStore);

        return httpPost;
    }

    public ReqRespPair executeRequest(TestHttpClient httpClient)
        throws IOException {
        CloseableHttpResponse response = null;
        ResponseState respState = null;
        long reqStartTime = 0, reqEndTime = 0;
        try {
            switch (verb) {
                case GET:
                    HttpGet httpget = buildGetRequest(httpClient);

                    reqStartTime = System.currentTimeMillis();
                    response = httpClient.execute(httpget);
                    reqEndTime = System.currentTimeMillis();

                    break;
                case POST:
                    HttpPost httpPost = buildPostRequest(httpClient, queryString);

                    buildPostBody(httpPost);
                    reqStartTime = System.currentTimeMillis();
                    response = httpClient.execute(httpPost);
                    reqEndTime = System.currentTimeMillis();

                    break;
                default:
                    throw new UnsupportedOperationException(
                        "Invalid RequestType verb" + verb.toString()
                    );
            }
            String responseContent = HttpClientUtils.getResponseContent(response);
            respState = new ResponseState(String.format("%s_Response", requestName));
            respState.buildFromHttpResponse(response, httpClient.getHttpClientContext());
            respState.setResponseContent(responseContent);
        } catch (ClientProtocolException exc) {
            HttpClientUtils.logException(log, exc);
            log.error("RequestState: " + this.toString());
        } catch (IOException exc) {
            HttpClientUtils.logException(log, exc);
            log.error("RequestState: " + this.toString());
        } catch (Exception exc) {
            HttpClientUtils.logException(log, exc);
            log.error("RequestState: " + this.toString());
        } finally {
            if (response != null) {
                response.close();
            }
        }
        return respState == null? null : new ReqRespPair(this, respState, reqEndTime - reqStartTime);
    }
}
