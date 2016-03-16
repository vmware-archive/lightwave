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
package com.vmware.identity.rest.idm.server.test.integration.util.data;

import java.io.IOException;
import java.security.cert.CertificateException;
import java.util.Arrays;
import java.util.List;

import com.vmware.identity.idm.OIDCClient;
import com.vmware.identity.rest.idm.data.OIDCClientMetadataDTO;

/**
 *  Data generator for OIDCClient APIs
 */
public class OIDCClientDataGenerator {

    public static final List<String> REDIRECT_URIS = Arrays.asList("https://www.vmware.com/redirect1", "https://www.vmware.com/redirect2");
    public static final String TOKEN_ENDPOINT_AUTH_METHOD = "private_key_jwt";
    public static final String TOKEN_ENDPOINT_AUTH_SIGNING_ALG = "RS256";
    public static final String ID_TOKEN_SIGNED_RESPONSE_ALG = "RS256";
    public static final List<String> POST_LOGOUT_REDIRECT_URIS = Arrays.asList("https://www.vmware.com/postlogoutredirect1");
    public static final String LOGOUT_URI = "https://www.vmware.com/logout";
    public static final String CERT_SUBJECT_DN = "OU=mID-2400d17e-d4f4-4753-98fd-fb9ecbf098ae,C=US,DC=local,DC=vsphere,CN=oidc-client-123";
    public static final Long AUTHN_REQUEST_CLIENT_ASSERTION_LIFETIME_MS = new Long(1234L);

    public static OIDCClient generateOIDCClient(String clientId) throws CertificateException, IOException {
        return new OIDCClient.Builder(clientId).
                redirectUris(REDIRECT_URIS).
                tokenEndpointAuthMethod(TOKEN_ENDPOINT_AUTH_METHOD).
                tokenEndpointAuthSigningAlg(TOKEN_ENDPOINT_AUTH_SIGNING_ALG).
                idTokenSignedResponseAlg(ID_TOKEN_SIGNED_RESPONSE_ALG).
                postLogoutRedirectUris(POST_LOGOUT_REDIRECT_URIS).
                logoutUri(LOGOUT_URI).
                certSubjectDN(CERT_SUBJECT_DN).build();
    }

    public static OIDCClientMetadataDTO generateOIDCClientMetadataDTO() throws CertificateException, IOException {
        return new OIDCClientMetadataDTO.Builder().
                withRedirectUris(REDIRECT_URIS).
                withTokenEndpointAuthMethod(TOKEN_ENDPOINT_AUTH_METHOD).
                withPostLogoutRedirectUris(POST_LOGOUT_REDIRECT_URIS).
                withLogoutUri(LOGOUT_URI).
                withCertSubjectDN(CERT_SUBJECT_DN).
                withAuthnRequestClientAssertionLifetimeMS(AUTHN_REQUEST_CLIENT_ASSERTION_LIFETIME_MS).build();
    }
}
