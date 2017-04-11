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
package com.vmware.identity.rest.idm.client;

import static com.vmware.identity.rest.core.client.RequestExecutor.execute;
import static com.vmware.identity.rest.core.client.RequestExecutor.executeAndReturnList;
import static com.vmware.identity.rest.core.client.URIFactory.buildURI;

import java.io.IOException;
import java.net.URI;
import java.util.List;
import java.util.Map;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.methods.HttpGet;

import com.vmware.identity.rest.core.client.BaseClient;
import com.vmware.identity.rest.core.client.ClientResource;
import com.vmware.identity.rest.core.client.RequestFactory;
import com.vmware.identity.rest.core.client.URIFactory;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.core.client.methods.HttpDeleteWithBody;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.data.CertificateChainDTO;
import com.vmware.identity.rest.idm.data.PrivateKeyDTO;
import com.vmware.identity.rest.idm.data.TenantCredentialsDTO;
import com.vmware.identity.rest.idm.data.attributes.CertificateGranularity;
import com.vmware.identity.rest.idm.data.attributes.CertificateScope;

/**
 * The {@code CertificateResource} is effectively a container that gathers all of the
 * commands related to a tenant's certificates. These commands are all gathered under
 * the {@value #CERTIFICATE_URI} endpoint on the REST server.
 */
public class CertificateResource extends ClientResource {

    private static final String CERTIFICATE_URI = "/idm/tenant/%s/certificates";
    private static final String PRIVATE_KEY_URI = CERTIFICATE_URI + "/privatekey";

    private static final String CERTIFICATE_POST_URI = "/idm/post/tenant/%s/certificates";
    private static final String PRIVATE_KEY_POST_URI = CERTIFICATE_POST_URI + "/privatekey";

    public CertificateResource(BaseClient parent) {
        super(parent);
    }

    /**
     * Request the list of certificates associated with a tenant.
     *
     * <p><b>Required Role:</b> {@code anonymous}.
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
    public List<CertificateChainDTO> get(String tenant, CertificateScope scope, CertificateGranularity granularity) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        Map<String, Object> params = URIFactory.buildParameters("scope", scope,
                                                                "granularity", granularity);
        URI uri = buildURI(parent.getHostRetriever(), CERTIFICATE_URI, tenant, params);

        HttpGet get = RequestFactory.createGetRequest(uri);
        return executeAndReturnList(parent.getClient(), get, CertificateChainDTO.class);
    }

    /**
     * Delete a tenant's certificate by way of the certificate fingerprint.
     *
     * <p><b>Required Role:</b> {@code administrator}.
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
    public void delete(String tenant, String fingerprint) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        Map<String, Object> params = URIFactory.buildParameters("fingerprint", fingerprint);

        URI uri = buildURI(parent.getHostRetriever(), CERTIFICATE_URI, tenant, params);

        HttpDeleteWithBody delete = RequestFactory.createDeleteRequest(uri, parent.getToken());
        execute(parent.getClient(), delete);
    }

    /**
     * Request the private key associated with a tenant.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant name of the tenant to request the private key from.
     * @return a private key.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public PrivateKeyDTO getPrivateKey(String tenant) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), PRIVATE_KEY_POST_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return execute(parent.getClient(), post, PrivateKeyDTO.class);
    }

    /**
     * Set the tenant's private key and issuer certificate credentials.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant name of the tenant to replace the credentials of.
     * @param credentials the credentials to use.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void setCredentials(String tenant, TenantCredentialsDTO credentials) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), PRIVATE_KEY_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken(), credentials);
        execute(parent.getClient(), post);
    }

}
