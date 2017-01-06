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
import java.util.List;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;

import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.client.CertificateResource;
import com.vmware.identity.rest.idm.data.CertificateChainDTO;
import com.vmware.identity.rest.idm.data.PrivateKeyDTO;
import com.vmware.identity.rest.idm.data.TenantCredentialsDTO;
import com.vmware.identity.rest.idm.data.attributes.CertificateScope;
import com.vmware.identity.rest.idm.data.attributes.CertificateGranularity;
/**
 * Samples for using {@code CertificateResource}. The CertificateResource is a container that gathers all of the commands related to a tenant's
 * certificate.
 * 
 * @author abapat
 *
 */
public class CertificateSample extends SampleBase {
	private CertificateResource resource;

	/**
	 * Initializes IDM client and CertificateResource.
	 * 
	 * @throws KeyManagementException if an error occurs when making SSL request.
	 * @throws NoSuchAlgorithmException if an error occurs when making SSL request.
	 * @throws KeyStoreException if an error occurs building making SSL request.
	 * @throws ClientProtocolException in case of an http protocol error.
	 * @throws ClientException if a client side error occurs.
	 * @throws IOException if an error occurs when reading response.
	 */
	public CertificateSample()
			throws KeyManagementException, NoSuchAlgorithmException, KeyStoreException, ClientProtocolException, ClientException, IOException {
		super();
		resource = new CertificateResource(client);
	}

	/**
	 * Request the list of certificates associated with a tenant.
	 *
	 * <p>
	 * <b>Required Role:</b> {@code anonymous}.
	 *
	 * @param tenant the name of the tenant to request the certificates from.
	 * @param scope the scope of certificates to request.
	 * @return a list of certificate chains.
	 * @throws ClientException if a client side error occurs.
	 * @throws ClientProtocolException in case of an http protocol error.
	 * @throws WebApplicationException in the event of an application error.
	 * @throws HttpException if there was a generic error with the remote call.
	 * @throws IOException if there was an error with the IO stream.
	 */
	public List<CertificateChainDTO> getCertificateChain(String tenant, CertificateScope scope)
			throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException {
		return resource.get(tenant, scope, CertificateGranularity.CHAIN);
	}

	/**
	 * Delete a tenant's certificate by way of the certificate fingerprint.
	 *
	 * <p>
	 * <b>Required Role:</b> {@code administrator}.
	 *
	 * @see CertificateDTO#getFingerprint()
	 * @param tenant the name of the tenant to delete the certificate from.
	 * @param fingerprint the fingerprint of the certificate.
	 * @throws ClientException if a client side error occurs.
	 * @throws ClientProtocolException in case of an http protocol error.
	 * @throws WebApplicationException in the event of an application error.
	 * @throws HttpException if there was a generic error with the remote call.
	 * @throws IOException if there was an error with the IO stream.
	 */
	public void delete(String tenant, String fingerprint)
			throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException {
		resource.delete(tenant, fingerprint);
	}

	/**
	 * Request the private key associated with a tenant.
	 *
	 * <p>
	 * <b>Required Role:</b> {@code administrator}.
	 *
	 * @param tenant name of the tenant to request the private key from.
	 * @return a private key.
	 * @throws ClientException if a client side error occurs.
	 * @throws ClientProtocolException in case of an http protocol error.
	 * @throws WebApplicationException in the event of an application error.
	 * @throws HttpException if there was a generic error with the remote call.
	 * @throws IOException if there was an error with the IO stream.
	 */
	public PrivateKeyDTO getPrivateKey(String tenant)
			throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException {
		return resource.getPrivateKey(tenant);
	}

	/**
	 * Set the tenant's private key and issuer certificate credentials.
	 *
	 * <p>
	 * <b>Required Role:</b> {@code administrator}.
	 *
	 * @param tenant name of the tenant to replace the credentials of.
	 * @param credentials the credentials to use.
	 * @throws ClientException if a client side error occurs.
	 * @throws ClientProtocolException in case of an http protocol error.
	 * @throws WebApplicationException in the event of an application error.
	 * @throws HttpException if there was a generic error with the remote call.
	 * @throws IOException if there was an error with the IO stream.
	 */
	public void setCredentials(String tenant, TenantCredentialsDTO credentials)
			throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException {
		resource.setCredentials(tenant, credentials);
	}
}
