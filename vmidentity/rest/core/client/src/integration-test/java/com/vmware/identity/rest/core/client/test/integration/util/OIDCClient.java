/* **********************************************************************
 * Copyright 2015 VMware, Inc. All rights reserved.
 * *********************************************************************/
package com.vmware.identity.rest.core.client.test.integration.util;

import static com.vmware.identity.rest.core.client.URIFactory.buildURI;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URI;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.SSLContext;

import org.apache.http.HttpEntity;
import org.apache.http.HttpStatus;
import org.apache.http.NameValuePair;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.utils.URLEncodedUtils;
import org.apache.http.message.BasicNameValuePair;

import com.fasterxml.jackson.core.type.TypeReference;
import com.vmware.identity.rest.core.client.BaseClient;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.util.ObjectMapperSingleton;

public class OIDCClient extends BaseClient {

    private static final String ACCESS_TOKEN_KEY = "access_token";
    private static final String TOKEN_TENANT_URI_STRING = "/openidconnect/token/%s";

    public OIDCClient(String host, HostnameVerifier verifier, SSLContext sslContext) {
        super(host, verifier, sslContext);
    }

    public String getAccessToken(String tenant, String username, String password) throws ClientException, ClientProtocolException, IOException {
        URI uri = buildURI(getHostRetriever(), TOKEN_TENANT_URI_STRING, tenant);

        HttpPost post = new HttpPost(uri);
        post.setHeader("Content-Type", URLEncodedUtils.CONTENT_TYPE);

        List<NameValuePair> formParams = new ArrayList<NameValuePair>();
        formParams.add(new BasicNameValuePair("grant_type", "password"));
        formParams.add(new BasicNameValuePair("username", username));
        formParams.add(new BasicNameValuePair("password", password));
        formParams.add(new BasicNameValuePair("scope", "openid offline_access id_groups at_groups rs_admin_server"));

        HttpEntity entity = new UrlEncodedFormEntity(formParams);
        post.setEntity(entity);

        CloseableHttpResponse response = getClient().execute(post);

        HashMap<String, String> tokens;
        try {
            int statusCode = response.getStatusLine().getStatusCode();
            if (statusCode == HttpStatus.SC_OK) {
                try {
                    TypeReference<HashMap<String, String>> ref = new TypeReference<HashMap<String,String>>() {};
                    tokens = ObjectMapperSingleton.getInstance().readValue(response.getEntity().getContent(), ref);
                } catch (IOException e) {
                    throw new IllegalArgumentException("An error occurred unmarshalling the response", e);
                }
            } else {
                String error = getContents(response.getEntity().getContent());
                throw new IllegalArgumentException("An error (" + statusCode + ") occurred when retrieving the access token from OIDC. " + error);
            }
        } finally {
            response.close();
        }

        return tokens.get(ACCESS_TOKEN_KEY);
    }

    private static String getContents(InputStream in) throws IOException {
        BufferedReader reader = new BufferedReader(new InputStreamReader(in));
        StringBuilder sb = new StringBuilder();

        String line;
        while((line = reader.readLine()) != null) {
            sb.append(line);
        }
        reader.close();
        return sb.toString();
    }
}
