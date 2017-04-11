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

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.List;

import org.apache.http.HttpException;
import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpUriRequest;
import org.apache.http.impl.client.CloseableHttpClient;

import com.fasterxml.jackson.databind.JavaType;
import com.vmware.identity.rest.core.client.exceptions.GeneralRequestException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.core.client.exceptions.client.BadRequestException;
import com.vmware.identity.rest.core.client.exceptions.client.ForbiddenException;
import com.vmware.identity.rest.core.client.exceptions.client.UnauthorizedException;
import com.vmware.identity.rest.core.client.exceptions.client.NotFoundException;
import com.vmware.identity.rest.core.client.exceptions.server.InternalServerErrorException;
import com.vmware.identity.rest.core.client.exceptions.server.NotImplementedException;
import com.vmware.identity.rest.core.data.ErrorInfo;
import com.vmware.identity.rest.core.util.ObjectMapperSingleton;

public class RequestExecutor {

    public static void execute(CloseableHttpClient client, HttpUriRequest request) throws ClientProtocolException, WebApplicationException, HttpException, IOException {
        executeInternal(client, request, null);
    }

    public static <T> T execute(CloseableHttpClient client, HttpUriRequest request, Class<T> valueType) throws ClientProtocolException, WebApplicationException, HttpException, IOException {
        return executeInternal(client, request, ObjectMapperSingleton.getInstance().getTypeFactory().constructType(valueType));
    }

    public static <T> List<T> executeAndReturnList(CloseableHttpClient client, HttpUriRequest request, Class<T> valueType) throws ClientProtocolException, WebApplicationException, HttpException, IOException {
        return executeInternal(client, request, ObjectMapperSingleton.getInstance().getTypeFactory().constructCollectionType(List.class, valueType));
    }

    private static <T> T executeInternal(CloseableHttpClient client, HttpUriRequest request, JavaType type) throws ClientProtocolException, WebApplicationException, HttpException, IOException {
        CloseableHttpResponse response = client.execute(request);

        try {
            int statusCode = response.getStatusLine().getStatusCode();
            if (statusCode == HttpStatus.SC_OK || statusCode == HttpStatus.SC_NO_CONTENT) {
                return handleSuccess(response, type);
            } else {
                throw handleError(response, response.getStatusLine().getStatusCode());
            }
        } finally {
            response.close();
        }
    }

    private static <T> T handleSuccess(HttpResponse response, JavaType type) {
        T object = null;

        if (type != null) {
            try {
                if (response != null && response.getEntity() != null && response.getEntity().getContent() != null) {
                    object = ObjectMapperSingleton.getInstance().<T>readValue(response.getEntity().getContent(), type);
                }
            } catch (IOException e) {
               throw new IllegalArgumentException("An error occurred unmarshalling the server's response", e);
            }
        }

        return object;
    }

    private static HttpException handleError(HttpResponse response, int statusCode) throws UnsupportedOperationException, IOException {
        String content = consumeContent(response.getEntity().getContent());

        try {
            ErrorInfo error = ObjectMapperSingleton.getInstance().<ErrorInfo>readValue(content, ErrorInfo.class);
            return handleKnownError(statusCode, error);
        } catch (IOException e) {
            return new GeneralRequestException(statusCode, "An error occurred unmarshalling the server's error response", content, e);
        }
    }

    private static WebApplicationException handleKnownError(int statusCode, ErrorInfo error) {
        switch (statusCode) {
        case HttpStatus.SC_BAD_REQUEST:
            return new BadRequestException(error);
        case HttpStatus.SC_UNAUTHORIZED:
            return new UnauthorizedException(error);
        case HttpStatus.SC_FORBIDDEN:
            return new ForbiddenException(error);
        case HttpStatus.SC_NOT_FOUND:
            return new NotFoundException(error);
        case HttpStatus.SC_INTERNAL_SERVER_ERROR:
            return new InternalServerErrorException(error);
        case HttpStatus.SC_NOT_IMPLEMENTED:
            return new NotImplementedException(error);
        default:
            throw new IllegalArgumentException("An unknown error status occurred: '" + statusCode + "'");
        }
    }

    private static String consumeContent(InputStream in) throws IOException {
        ByteArrayOutputStream out = new ByteArrayOutputStream(in.available());

        final byte[] data = new byte[8196];
        int len = 0;

        while ((len = in.read(data)) > 0) {
            out.write(data, 0, len);
        }

        return out.toString();
    }

}
