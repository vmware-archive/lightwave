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

import java.io.IOException;
import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

import org.apache.http.client.ClientProtocolException;
import org.apache.http.conn.ssl.NoopHostnameVerifier;
import org.apache.http.conn.ssl.TrustStrategy;
import org.apache.http.ssl.SSLContextBuilder;
import org.apache.log4j.Logger;

import com.vmware.identity.rest.core.client.AccessToken;
import com.vmware.identity.rest.core.client.HostRetriever;
import com.vmware.identity.rest.core.client.SimpleHostRetriever;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.idm.client.IdmClient;

/**
 * The {@code SampleBase} class contains the implementation for getting the access token and creating the IDM client for REST API calls. All sample
 * classes inherit this IDM client for REST calls.
 * 
 * @author abapat
 *
 */
public class SampleBase {
	protected Logger log = Logger.getLogger(this.getClass());
	protected final String HOST;
	protected final String DOMAIN;

	private final String TENANT;
	private final String ADMIN_USERNAME;
	private final String ADMIN_PASSWORD;

	protected IdmClient client;

	/**
	 * Gets access credentials from the configuration file and initializes the IDM client.
	 * 
	 * @throws KeyManagementException if an error occurs when making SSL request.
	 * @throws NoSuchAlgorithmException if an error occurs when making SSL request.
	 * @throws KeyStoreException if an error occurs building making SSL request.
	 * @throws ClientProtocolException in case of an http protocol error.
	 * @throws ClientException if a client side error occurs.
	 * @throws IOException if an error occurs when reading response.
	 */
	protected SampleBase()
			throws KeyManagementException, NoSuchAlgorithmException, KeyStoreException, ClientProtocolException, ClientException, IOException {
		HOST = Config.getProperty(Config.HOSTNAME);
		DOMAIN = Config.getProperty(Config.DOMAIN);
		TENANT = Config.getProperty(Config.TENANT);
		ADMIN_USERNAME = Config.getProperty(Config.ADMIN_USERNAME);
		ADMIN_PASSWORD = Config.getProperty(Config.ADMIN_PASSWORD);

		client = createClient(HOST, TENANT, ADMIN_USERNAME, ADMIN_PASSWORD);
	}

	/**
	 * Sets up a client with a simple host over HTTPS, a no-op hostname verifier, and a trust strategy that always trusts the server's certificates.
	 * Initializes the IDM client with an Access Token from the configuration file credentials.
	 * 
	 * @param host the host of the remote REST server.
	 * @param tenant the tenant to get access token from.
	 * @param username the username to authenticate with.
	 * @param password the password of user to authenticate with.
	 * @return the IDM client to use for REST requests
	 * @throws KeyManagementException if an error occurs when making SSL request.
	 * @throws NoSuchAlgorithmException if an error occurs when making SSL request.
	 * @throws KeyStoreException if an error occurs building making SSL request.
	 * @throws ClientProtocolException in case of an http protocol error.
	 * @throws ClientException if a client side error occurs.
	 * @throws IOException if an error occurs when reading response.
	 */
	private IdmClient createClient(String host, String tenant, String username, String password)
			throws KeyManagementException, NoSuchAlgorithmException, KeyStoreException, ClientProtocolException, ClientException, IOException {
		HostRetriever hostRetriever = new SimpleHostRetriever(host, true);
		IdmClient client = new IdmClient(hostRetriever, NoopHostnameVerifier.INSTANCE,
				new SSLContextBuilder().loadTrustMaterial(null, new TrustStrategy() {

					@Override
					public boolean isTrusted(X509Certificate[] chain, String authType) throws CertificateException {
						return true;
					}
				}).build());

		String token = getAccessToken(host, tenant, username, password);

		client.setToken(new AccessToken(token, AccessToken.Type.JWT));
		return client;
	}

	/**
	 * Gets the Access Token from {@code OIDCClient}.
	 * 
	 * @param host the host of the remote REST server.
	 * @param tenant the tenant to get access token from.
	 * @param username the username to authenticate with.
	 * @param password the password of user to authenticate with.
	 * @return the access token.
	 * @throws KeyManagementException if an error occurs when making SSL request.
	 * @throws NoSuchAlgorithmException if an error occurs when making SSL request.
	 * @throws KeyStoreException if an error occurs building making SSL request.
	 * @throws ClientProtocolException in case of an http protocol error.
	 * @throws ClientException if a client side error occurs.
	 * @throws IOException if an error occurs when reading response.
	 */
	private String getAccessToken(String host, String tenant, String username, String password)
			throws KeyManagementException, NoSuchAlgorithmException, KeyStoreException, ClientProtocolException, ClientException, IOException {
		OIDCClient client = new OIDCClient(host, NoopHostnameVerifier.INSTANCE, new SSLContextBuilder().loadTrustMaterial(null, new TrustStrategy() {

			@Override
			public boolean isTrusted(X509Certificate[] chain, String authType) throws CertificateException {
				return true;
			}
		}).build());
		String s = client.getAccessToken(tenant, username, password);
		client.close();
		return s;

	}
}
