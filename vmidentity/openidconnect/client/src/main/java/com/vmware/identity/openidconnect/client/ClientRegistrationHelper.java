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

package com.vmware.identity.openidconnect.client;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.security.KeyStore;
import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

import net.minidev.json.JSONArray;
import net.minidev.json.JSONObject;
import net.minidev.json.JSONValue;

import org.apache.commons.lang3.Validate;

import com.nimbusds.oauth2.sdk.ParseException;
import com.nimbusds.oauth2.sdk.http.HTTPRequest;
import com.nimbusds.oauth2.sdk.http.HTTPResponse;

/**
 * OIDC client registration helper class
 *
 * @author Jun Sun
 */
public class ClientRegistrationHelper {
    private final URL restAdminOIDCClientURL;
    private final KeyStore keyStore;

    private ClientRegistrationHelper(Builder builder) {
        this.restAdminOIDCClientURL = builder.restAdminOIDCClientURL;
        this.keyStore = builder.keyStore;
    }

    /**
     * Builder for ClientRegistrationHelper
     */
    public static class Builder {
        private URL restAdminOIDCClientURL;
        private KeyStore keyStore;
        private final String domainControllerFQDN;
        private int domainControllerPort = OIDCClientUtils.DEFAULT_OP_PORT;
        private String tenant = OIDCClientUtils.DEFAULT_TENANT;

        /**
         * Constructor
         *
         * @param domainControllerFQDN      Domain controller FQDN which runs service.
         */
        public Builder(String domainControllerFQDN) {
            Validate.notEmpty(domainControllerFQDN, "domainControllerFQDN");

            this.domainControllerFQDN = domainControllerFQDN;
        }

        /**
         * Set domain controller port
         *
         * @param domainControllerPort      Domain controller Port which runs service.
         * @return Builder
         */
        public Builder domainControllerPort(int domainControllerPort) {
            Validate.isTrue(domainControllerPort > 0, "domainControllerPort");

            this.domainControllerPort = domainControllerPort;
            return this;
        }

        /**
         * Set tenant
         *
         * @param tenant                    Tenant.
         * @return Builder
         */
        public Builder tenant(String tenant) {
            Validate.notEmpty(tenant, "tenant");

            this.tenant = tenant;
            return this;
        }

        /**
         * Set key store
         *
         * @param keyStore                  Key store which contains server SSL certificate.
         * @return Builder
         */
        public Builder keyStore(KeyStore keyStore) {
            Validate.notNull(keyStore, "keyStore");

            this.keyStore = keyStore;
            return this;
        }

        /**
         * Build ClientRegistrationHelper
         *
         * @return ClientRegistrationHelper
         */
        public ClientRegistrationHelper build() {
            if (this.keyStore == null) {
                this.keyStore = VecsKeyStore.getInstance();
            }

            StringBuilder sb = new StringBuilder();
            sb.append("https://");
            sb.append(this.domainControllerFQDN);
            sb.append(":");
            sb.append(String.valueOf(this.domainControllerPort));
            sb.append("/idm/tenant/");
            sb.append(this.tenant);
            sb.append("/oidcclient/");
            try {
                this.restAdminOIDCClientURL = new URL(sb.toString());
            } catch (MalformedURLException e) {
                throw new IllegalArgumentException("Failed to build tenantized Admin server URI: " + e.getMessage(), e);
            }

            return new ClientRegistrationHelper(this);
        }
    }

    /**
     * Register an OIDC client
     *
     * @param accessToken                       Access token required by the registration operation.
     * @param accessTokenType                   Type of the access token.
     * @param redirectURIs                      Redirect URIs to be registered.
     * @param logoutURI                         Logout URI to be registered.
     * @param postLogoutRedirectURIs            Post redirect URIs to be registered.
     * @param clientAuthenticationMethod        Client authentication method.
     * @param certSubjectDN                     Certificate subject distinguished name.
     * @return                                  Client information for the registered client
     * @throws OIDCClientException              Client side exception.
     * @throws SSLConnectionException           SSL connection exception.
     * @throws AdminServerException             Admin server side exception.
     */
    public ClientInformation registerClient(
            AccessToken accessToken,
            TokenType accessTokenType,
            Set<URI> redirectURIs,
            URI logoutURI,
            Set<URI> postLogoutRedirectURIs,
            ClientAuthenticationMethod clientAuthenticationMethod,
            String certSubjectDN) throws OIDCClientException, SSLConnectionException, AdminServerException {
        Validate.notNull(accessToken, "accessToken");
        Validate.notNull(accessTokenType, "accessTokenType");
        Validate.notEmpty(redirectURIs, "redirectURIs");
        Validate.notNull(clientAuthenticationMethod, "clientAuthenticationMethod");
        if (ClientAuthenticationMethod.PRIVATE_KEY_JWT.equals(clientAuthenticationMethod)) {
            Validate.notEmpty(certSubjectDN, "certSubjectDN");
        }

        if (accessTokenType.equals(TokenType.HOK)) {
            throw new UnsupportedOperationException("Holder of the key access token is not supported in client registration.");
        }

        try {
            HTTPRequest httpRequest = new HTTPRequest(HTTPRequest.Method.POST, this.restAdminOIDCClientURL);
            httpRequest.setQuery(buildOIDCClientMetadataJSONObject(
                    redirectURIs,
                    logoutURI,
                    postLogoutRedirectURIs,
                    clientAuthenticationMethod,
                    certSubjectDN).toString());
            httpRequest.setContentType("application/json");

            HTTPResponse httpResponse = AdminServerHelper.processHTTPRequest(
                    httpRequest,
                    accessToken,
                    accessTokenType,
                    this.keyStore);

            return convertToClientInformation(httpResponse.getContentAsJSONObject());
        } catch (ParseException e) {
            throw new OIDCClientException("Exception caught during client registration: " + e.getMessage(), e);
        }
    }

    /**
     * Get all OIDC clients in a tenant
     *
     * @param accessToken                       Access token required by the registration operation.
     * @param accessTokenType                   Type of the access token.
     * @return                                  A collection of OIDC clients
     * @throws OIDCClientException              Client side exception.
     * @throws SSLConnectionException           SSL connection exception.
     * @throws AdminServerException             Admin server side exception.
     */
    public Collection<ClientInformation> getAllClients(
            AccessToken accessToken,
            TokenType accessTokenType) throws OIDCClientException, SSLConnectionException, AdminServerException {
        Validate.notNull(accessToken, "accessToken");
        Validate.notNull(accessTokenType, "accessTokenType");

        if (accessTokenType.equals(TokenType.HOK)) {
            throw new UnsupportedOperationException("Holder of the key access token is not supported in client registration.");
        }

        HTTPRequest httpRequest = new HTTPRequest(HTTPRequest.Method.GET, this.restAdminOIDCClientURL);

        HTTPResponse httpResponse = AdminServerHelper.processHTTPRequest(
                httpRequest,
                accessToken,
                accessTokenType,
                this.keyStore);

        JSONArray jsonArray = (JSONArray) JSONValue.parse(httpResponse.getContent());

        Set<ClientInformation> clientInformations = new HashSet<ClientInformation>();
        for (Object oidcClientDTO : jsonArray) {
            clientInformations.add(convertToClientInformation((JSONObject) oidcClientDTO));
        }
        return clientInformations;
    }

    /**
     * Get a client by client ID
     *
     * @param accessToken                       Access token required by the registration operation.
     * @param accessTokenType                   Type of the access token.
     * @param clientID                          Client ID for a registered client
     * @return                                  A client matches the input client ID
     * @throws OIDCClientException              Client side exception.
     * @throws SSLConnectionException           SSL connection exception.
     * @throws AdminServerException             Admin server side exception.
     */
    public ClientInformation getClient(
            AccessToken accessToken,
            TokenType accessTokenType,
            ClientID clientID) throws OIDCClientException, SSLConnectionException, AdminServerException {
        Validate.notNull(accessToken, "accessToken");
        Validate.notNull(accessTokenType, "accessTokenType");
        Validate.notNull(clientID, "clientID");

        if (accessTokenType.equals(TokenType.HOK)) {
            throw new UnsupportedOperationException("Holder of the key access token is not supported in client registration.");
        }

        try {
            HTTPRequest httpRequest = new HTTPRequest(HTTPRequest.Method.GET, buildEndpointWithClientID(this.restAdminOIDCClientURL, clientID));

            HTTPResponse httpResponse = AdminServerHelper.processHTTPRequest(
                    httpRequest,
                    accessToken,
                    accessTokenType,
                    this.keyStore);

            return convertToClientInformation(httpResponse.getContentAsJSONObject());
        } catch (ParseException e) {
            throw new OIDCClientException("Exception caught during client registration: " + e.getMessage(), e);
        }
    }

    /**
     * Delete a client
     *
     * @param accessToken                       Access token required by the registration operation.
     * @param accessTokenType                   Type of the access token.
     * @param clientID                          Client ID for a registered client
     * @throws OIDCClientException              Client side exception.
     * @throws SSLConnectionException           SSL connection exception.
     * @throws AdminServerException             Admin server side exception.
     */
    public void deleteClient(
            AccessToken accessToken,
            TokenType accessTokenType,
            ClientID clientID) throws OIDCClientException, SSLConnectionException, AdminServerException {
        Validate.notNull(accessToken, "accessToken");
        Validate.notNull(accessTokenType, "accessTokenType");
        Validate.notNull(clientID, "clientID");

        if (accessTokenType.equals(TokenType.HOK)) {
            throw new UnsupportedOperationException("Holder of the key access token is not supported in client registration.");
        }

        HTTPRequest httpRequest = new HTTPRequest(HTTPRequest.Method.DELETE, buildEndpointWithClientID(this.restAdminOIDCClientURL, clientID));

        AdminServerHelper.processHTTPRequest(
                httpRequest,
                accessToken,
                accessTokenType,
                this.keyStore);
    }

    /**
     * Update a client
     *
     * @param accessToken                       Access token required by the registration operation.
     * @param accessTokenType                   Type of the access token.
     * @param clientID                          Client ID for a registered client
     * @param redirectURIs                      Redirect URIs to be registered.
     * @param logoutURI                         Logout URI to be registered.
     * @param postLogoutRedirectURIs            Post redirect URIs to be registered.
     * @param clientAuthenticationMethod        Client authentication method.
     * @param certSubjectDN                     Certificate subject distinguished name.
     * @return                                  Client information for the updated client
     * @throws OIDCClientException              Client side exception.
     * @throws SSLConnectionException           SSL connection exception.
     * @throws AdminServerException             Admin server side exception.
     */
    public ClientInformation updateClient(
            AccessToken accessToken,
            TokenType accessTokenType,
            ClientID clientID,
            Set<URI> redirectURIs,
            URI logoutURI,
            Set<URI> postLogoutRedirectURIs,
            ClientAuthenticationMethod clientAuthenticationMethod,
            String certSubjectDN) throws OIDCClientException, SSLConnectionException, AdminServerException {
        Validate.notNull(accessToken, "accessToken");
        Validate.notNull(accessTokenType, "accessTokenType");
        Validate.notNull(clientID, "clientID");
        Validate.notEmpty(redirectURIs, "redirectURIs");
        Validate.notNull(clientAuthenticationMethod, "clientAuthenticationMethod");
        if (ClientAuthenticationMethod.PRIVATE_KEY_JWT.equals(clientAuthenticationMethod)) {
            Validate.notEmpty(certSubjectDN, "certSubjectDN");
        }

        if (accessTokenType.equals(TokenType.HOK)) {
            throw new UnsupportedOperationException("Holder of the key access token is not supported in client registration.");
        }

        try {
            HTTPRequest httpRequest = new HTTPRequest(HTTPRequest.Method.PUT, buildEndpointWithClientID(this.restAdminOIDCClientURL, clientID));
            httpRequest.setQuery(buildOIDCClientMetadataJSONObject(
                    redirectURIs,
                    logoutURI,
                    postLogoutRedirectURIs,
                    clientAuthenticationMethod,
                    certSubjectDN).toString());
            httpRequest.setContentType("application/json");

            HTTPResponse httpResponse = AdminServerHelper.processHTTPRequest(
                    httpRequest,
                    accessToken,
                    accessTokenType,
                    this.keyStore);

            return convertToClientInformation(httpResponse.getContentAsJSONObject());
        } catch (ParseException e) {
            throw new OIDCClientException("Exception caught during client registration: " + e.getMessage(), e);
        }
    }

    private JSONObject buildOIDCClientMetadataJSONObject(
            Set<URI> redirectURIs,
            URI logoutURI,
            Set<URI> postLogoutRedirectURIs,
            ClientAuthenticationMethod clientAuthenticationMethod,
            String certSubjectDN) {
        Validate.notEmpty(redirectURIs, "redirectURIs");
        Validate.notNull(clientAuthenticationMethod, "clientAuthenticationMethod");
        if (ClientAuthenticationMethod.PRIVATE_KEY_JWT.equals(clientAuthenticationMethod)) {
            Validate.notEmpty(certSubjectDN, "certSubjectDN");
        }

        JSONObject jsonObject= new JSONObject();
        JSONArray uris = new JSONArray();
        for (URI uri : redirectURIs) {
            uris.add(uri.toString());
        }
        jsonObject.put("redirectUris", uris);
        jsonObject.put("tokenEndpointAuthMethod", clientAuthenticationMethod.getValue());
        uris = new JSONArray();
        if (postLogoutRedirectURIs != null && !postLogoutRedirectURIs.isEmpty()) {
            for (URI uri : postLogoutRedirectURIs) {
                uris.add(uri.toString());
            }
            jsonObject.put("postLogoutRedirectUris", uris);
        }
        if (logoutURI != null) {
            jsonObject.put("logoutUri", logoutURI.toString());
        }
        if (certSubjectDN != null) {
            jsonObject.put("certSubjectDN", certSubjectDN);
        }

        return jsonObject;
    }

    private ClientInformation convertToClientInformation(JSONObject jsonObject) throws OIDCClientException {
        Validate.notEmpty(jsonObject, "jsonObject");

        try {
            JSONObject oidcclientMetadataDTO = (JSONObject) jsonObject.get("oidcclientMetadataDTO");

            Set<URI> redirectUriSet = new HashSet<URI>();
            JSONArray jsonArray = (JSONArray) oidcclientMetadataDTO.get("redirectUris");
            for (Object uri : jsonArray) {
                redirectUriSet.add(new URI((String) uri));
            }

            Set<URI> postLogoutRedirectUriSet = null;
            jsonArray = (JSONArray) oidcclientMetadataDTO.get("postLogoutRedirectUris");
            if (jsonArray != null) {
                postLogoutRedirectUriSet = new HashSet<URI>();
                for (Object uri : jsonArray) {
                    postLogoutRedirectUriSet.add(new URI((String) uri));
                }
            }

            return new ClientInformation(
                    new ClientID((String) jsonObject.get("clientId")),
                    redirectUriSet,
                    ClientAuthenticationMethod.getClientAuthenticationMethod((String) oidcclientMetadataDTO.get("tokenEndpointAuthMethod")),
                    postLogoutRedirectUriSet,
                    (oidcclientMetadataDTO.get("logoutUri") == null) ? null : new URI((String) oidcclientMetadataDTO.get("logoutUri")),
                    (String) oidcclientMetadataDTO.get("certSubjectDN"));
        } catch (URISyntaxException e) {
            throw new OIDCClientException("Exception caught during converting client information: " + e.getMessage(), e);
        }
    }

    private URL buildEndpointWithClientID(URL tenantizedAdminServerURL, ClientID clientID) throws OIDCClientException {
        Validate.notNull(tenantizedAdminServerURL, "tenantizedAdminServerURL");
        Validate.notNull(clientID, "clientID");

        StringBuilder sb = new StringBuilder(tenantizedAdminServerURL.toString());
        sb.append(clientID.getValue());
        try {
            return new URL(sb.toString());
        } catch (MalformedURLException e) {
            throw new OIDCClientException("Failed to build endpoint with clientID: " + e.getMessage(), e);
        }
    }
}
