/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
 *
 */
package com.vmware.identity.rest.idm.samples;

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
import com.vmware.identity.rest.core.client.SimpleHostRetriever;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.util.ObjectMapperSingleton;

/**
 * The {@code OIDCClient} class contains the implementation for getting an access token from Open ID Connect endpoint.
 * 
 * @author abapat
 *
 */
public class OIDCClient extends BaseClient {

	/**
	 * Constructs a client with a variety of parameters. The {@code host} parameter is loaded into a {@link SimpleHostRetriever} using the default
	 * port (-1), assuming that the connection will be sure.
	 *
	 * @param host the hostname of the remote REST server.
	 * @param verifier a {@link HostnameVerifier} for use when verifying the SSL certificate hostname.
	 * @param sslContext a {@link SSLContext} that defines the trust relationship with the SSL certificates.
	 */
	public OIDCClient(String host, HostnameVerifier verifier, SSLContext sslContext) {
		super(host, verifier, sslContext);
	}

	private static final String ACCESS_TOKEN_KEY = "access_token";
	private static final String TOKEN_TENANT_URI_STRING = "/openidconnect/token/%s";

	/**
	 * Accesses /openidconnect endpoint and gets access token
	 * 
	 * @param tenant
	 * @param username
	 * @param password
	 * @return the access token
	 * @throws ClientException if a client side error occurs.
	 * @throws ClientProtocolException in case of an http protocol error.
	 * @throws IOException if an error occurs when reading response.
	 */
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
					TypeReference<HashMap<String, String>> ref = new TypeReference<HashMap<String, String>>() {
					};
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

	/**
	 * Gets the content from an input stream
	 * 
	 * @param in InputStream
	 * @return the content from input stream
	 * @throws IOException if an error occurs when reading InputStream.
	 */
	private static String getContents(InputStream in) throws IOException {
		BufferedReader reader = new BufferedReader(new InputStreamReader(in));
		StringBuilder sb = new StringBuilder();

		String line;
		while ((line = reader.readLine()) != null) {
			sb.append(line);
		}
		reader.close();
		return sb.toString();
	}
}
