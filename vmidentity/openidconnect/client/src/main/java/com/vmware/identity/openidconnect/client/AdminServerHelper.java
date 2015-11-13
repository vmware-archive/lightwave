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

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.URL;
import java.security.KeyStore;
import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;

import net.minidev.json.JSONObject;

import org.apache.commons.lang3.Validate;

import sun.misc.BASE64Encoder;
import sun.security.provider.X509Factory;

import com.nimbusds.oauth2.sdk.ParseException;
import com.nimbusds.oauth2.sdk.http.HTTPRequest;
import com.nimbusds.oauth2.sdk.http.HTTPResponse;

/**
 * Admin server helper class
 *
 * @author Jun Sun
 */
public class AdminServerHelper {
    private final URL adminServerBaseURL;
    private final String tenant;
    private final KeyStore keyStore;

    private AdminServerHelper(Builder builder) {
        this.adminServerBaseURL = builder.adminServerBaseURL;
        this.tenant = builder.tenant;
        this.keyStore = builder.keyStore;
    }

    /**
     * Builder for AdminServerHelper
     */
    public static class Builder {
        private URL adminServerBaseURL;
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
         * Build AdminServerHelper
         *
         * @return AdminServerHelper
         */
        public AdminServerHelper build() {
            this.adminServerBaseURL = OIDCClientUtils.buildBaseUrl(
                    this.domainControllerFQDN,
                    this.domainControllerPort);

            if (this.keyStore == null) {
                this.keyStore = VecsKeyStore.getInstance();
            }

            return new AdminServerHelper(this);
        }
    }

    /**
     * Create a solution user
     *
     * @param accessToken                       Access token required by the registration operation.
     * @param accessTokenType                   Type of the access token.
     * @param solutionUserName                  Solution user name.
     * @param clientCertificate                 Certificate of solution user.
     * @throws OIDCClientException              Client side exception.
     * @throws SSLConnectionException           SSL connection exception.
     * @throws AdminServerException             Admin server side exception.
     */
    public void createSolutionUser(
            AccessToken accessToken,
            TokenType accessTokenType,
            String solutionUserName,
            X509Certificate clientCertificate) throws OIDCClientException, SSLConnectionException, AdminServerException {

        URL adminServerSolutionUserURL = OIDCClientUtils.buildEndpointUrl(
                this.adminServerBaseURL,
                String.format("/idm/tenant/%s/solutionusers", this.tenant));
        HTTPRequest httpRequest = new HTTPRequest(HTTPRequest.Method.POST, adminServerSolutionUserURL);
        JSONObject certificateJSONObject = new JSONObject();
        certificateJSONObject.put("encoded", convertToBase64PEMString(clientCertificate));
        JSONObject solutionUserJSONObject = new JSONObject();
        solutionUserJSONObject.put("name", solutionUserName);
        solutionUserJSONObject.put("domain", this.tenant);
        solutionUserJSONObject.put("certificate", certificateJSONObject);
        httpRequest.setQuery(solutionUserJSONObject.toString());
        try {
            httpRequest.setContentType("application/json");
        } catch (ParseException e) {
            throw new OIDCClientException("HTTP request failed to set content type: " + e.getMessage(), e);
        }

        processHTTPRequest(
                httpRequest,
                accessToken,
                accessTokenType,
                this.keyStore);
    }

    /**
     * Add solution user to ActAs user group
     *
     * @param accessToken                       Access token required by the registration operation.
     * @param accessTokenType                   Type of the access token.
     * @param solutionUserUPN                   UPN of solution user.
     * @throws OIDCClientException              Client side exception.
     * @throws SSLConnectionException           SSL connection exception.
     * @throws AdminServerException             Admin server side exception.
     */
    public void addSolutionUserToActAsUsersGroup(
            AccessToken accessToken,
            TokenType accessTokenType,
            String solutionUserUPN) throws OIDCClientException, SSLConnectionException, AdminServerException {

        String actAsUsersGroup = "ActAsUsers" + "@" + this.tenant;

        URL adminServerAddMembersToGroupURL = OIDCClientUtils.buildEndpointUrl(
                this.adminServerBaseURL,
                String.format("/idm/tenant/%s/groups/%s/members?type=user&members=%s", // ideally type should be "solutionuser", but it is not supported by REST admin, so use "user" for now.
                        this.tenant,
                        actAsUsersGroup,
                        solutionUserUPN));
        HTTPRequest httpRequest = new HTTPRequest(HTTPRequest.Method.PUT, adminServerAddMembersToGroupURL);
        try {
            httpRequest.setContentType("application/x-www-form-urlencoded");
        } catch (ParseException e) {
            throw new OIDCClientException("HTTP request failed to set content type: " + e.getMessage(), e);
        }

        processHTTPRequest(
                httpRequest,
                accessToken,
                accessTokenType,
                this.keyStore);
    }

    private String convertToBase64PEMString(X509Certificate x509Certificate) throws OIDCClientException {

        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        BASE64Encoder encoder = new BASE64Encoder();
        try{
            byteArrayOutputStream.write(X509Factory.BEGIN_CERT.getBytes());
            byteArrayOutputStream.write("\n".getBytes());
            encoder.encodeBuffer(x509Certificate.getEncoded(), byteArrayOutputStream);
            byteArrayOutputStream.write(X509Factory.END_CERT.getBytes());
            byteArrayOutputStream.write("\n".getBytes());
        } catch (IOException | CertificateEncodingException e) {
            throw new OIDCClientException("Failed to convert certificate: " + e.getMessage(), e);
        }
        return byteArrayOutputStream.toString();
    }

    static HTTPResponse processHTTPRequest(
            HTTPRequest httpRequest,
            AccessToken accessToken,
            TokenType accessTokenType,
            KeyStore keyStore) throws OIDCClientException, SSLConnectionException, AdminServerException {
        Validate.notNull(httpRequest, "httpRequest");

        httpRequest.setAuthorization(String.format("%s %s", accessTokenType.getValue(), accessToken.getValue()));
        HTTPResponse httpResponse = OIDCClientUtils.sendSecureRequest(httpRequest, keyStore);
        try {
            if (httpResponse.getStatusCode() != 200 && httpResponse.getStatusCode() != 204) {
                throw convertToAdminServerException(httpResponse.getStatusCode(), httpResponse.getContentAsJSONObject());
            }
        } catch (ParseException e) {
            throw new OIDCClientException("Exception caught during exception conversion: " + e.getMessage(), e);
        }
        return httpResponse;
    }

    static AdminServerException convertToAdminServerException(int httpStatusCode, JSONObject jsonObject) {
        Validate.notEmpty(jsonObject, "jsonObject");

        return new AdminServerException(
                httpStatusCode,
                (String) jsonObject.get("error"),
                (String) jsonObject.get("details"),
                (String) jsonObject.get("cause"));
    }
}
