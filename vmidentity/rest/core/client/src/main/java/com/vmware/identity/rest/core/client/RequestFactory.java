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
package com.vmware.identity.rest.core.client;

import java.net.URI;
import java.security.InvalidKeyException;
import java.security.PrivateKey;
import java.security.SignatureException;
import java.util.Date;

import org.apache.http.client.methods.HttpEntityEnclosingRequestBase;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.methods.HttpPut;
import org.apache.http.client.methods.HttpUriRequest;
import org.apache.http.entity.ContentType;
import org.apache.http.entity.StringEntity;
import org.apache.http.message.BasicHeader;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.methods.HttpDeleteWithBody;
import com.vmware.identity.rest.core.util.ObjectMapperSingleton;
import com.vmware.identity.rest.core.util.RequestSigner;

public class RequestFactory {

    private final static String DATE_HEADER = "Date";
    private final static ContentType APPLICATION_JSON_UTF8 = ContentType.create(ContentType.APPLICATION_JSON.getMimeType(), "UTF-8");
    private final static ContentType APPLICATION_XML_UTF8 = ContentType.create(ContentType.APPLICATION_XML.getMimeType(), "UTF-8");

    public static HttpGet createGetRequest(URI uri) {
        return prepareRequest(new HttpGet(uri), null);
    }

    public static HttpGet createGetRequest(URI uri, AccessToken token) {
        return prepareRequest(new HttpGet(uri), token);
    }

    public static HttpPost createPostRequest(URI uri, AccessToken token) throws ClientException, JsonProcessingException {
        return prepareRequest(new HttpPost(uri), token, null);
    }

    public static HttpPost createPostRequest(URI uri, AccessToken token, Object entity) throws ClientException, JsonProcessingException {
        return prepareRequest(new HttpPost(uri), token, entity);
    }

    public static HttpPost createPostRequestWithXml(URI uri, AccessToken token, String xml) throws ClientException {
        return prepareRequestWithXml(new HttpPost(uri), token, xml);
    }

    public static HttpPut createPutRequest(URI uri, AccessToken token) throws ClientException, JsonProcessingException {
        return prepareRequest(new HttpPut(uri), token, null);
    }

    public static HttpPut createPutRequest(URI uri, AccessToken token, Object entity) throws ClientException, JsonProcessingException {
        return prepareRequest(new HttpPut(uri), token, entity);
    }

    public static HttpDeleteWithBody createDeleteRequest(URI uri, AccessToken token) throws ClientException, JsonProcessingException {
        return prepareRequest(new HttpDeleteWithBody(uri), token, null);
    }

    public static HttpDeleteWithBody createDeleteRequest(URI uri, AccessToken token, Object entity) throws ClientException, JsonProcessingException {
        return prepareRequest(new HttpDeleteWithBody(uri), token, entity);
    }

    private static <T extends HttpUriRequest> T prepareRequest(T request, AccessToken token) {
        if (token != null) {
            request.setHeader(new BasicHeader("Authorization", token.getType().getIdentifier() + " " + token.getToken()));
        }

        return request;
    }

    private static <T extends HttpEntityEnclosingRequestBase> T prepareRequest(T request, AccessToken token, Object entity) throws ClientException, JsonProcessingException {
        Date date = new Date();
        request.setHeader(DATE_HEADER, RequestSigner.getHttpFormattedDate(date));

        StringBuilder builder = new StringBuilder()
            .append("access_token=").append(token.getToken())
            .append("&").append("token_type=").append(token.getType().getIdentifier());

        String jsonEntity = null;
        if (entity != null) {
            jsonEntity = ObjectMapperSingleton.getInstance().writer().writeValueAsString(entity);
        }

        if (token.getType() == AccessToken.Type.JWT_HOK || token.getType() == AccessToken.Type.SAML_HOK) {
            try {
                String signedRequest = sign(request, jsonEntity, date, token.getPrivateKey());
                builder.append("&").append("token_signature=").append(signedRequest);
            } catch (InvalidKeyException | SignatureException e) {
                throw new ClientException("An error occurred while signing the request", e);
            }
        }

        if (entity != null) {
            builder.append("&").append(jsonEntity);
        }

        request.setEntity(new StringEntity(builder.toString(), APPLICATION_JSON_UTF8));

        return request;
    }

    private static <T extends HttpEntityEnclosingRequestBase> T prepareRequestWithXml(T request, AccessToken token, String xml) throws ClientException {
        Date date = new Date();
        request.setHeader(DATE_HEADER, RequestSigner.getHttpFormattedDate(date));
        if (token != null) {
            request.setHeader(new BasicHeader("Authorization", token.getType().getIdentifier() + " " + token.getToken()));
        }

        if (xml != null) {
            StringEntity entity = new StringEntity(xml, APPLICATION_XML_UTF8);
            request.setEntity(entity);
        }

        return request;
    }

    private static String sign(HttpEntityEnclosingRequestBase request, String entity, Date date, PrivateKey privateKey) throws InvalidKeyException, SignatureException {
        String stringToSign = RequestSigner.createSigningString(
                request.getMethod(),
                RequestSigner.computeMD5(entity),
                APPLICATION_JSON_UTF8.toString(),
                date,
                request.getURI());

        return RequestSigner.sign(stringToSign, privateKey);
    }

}
