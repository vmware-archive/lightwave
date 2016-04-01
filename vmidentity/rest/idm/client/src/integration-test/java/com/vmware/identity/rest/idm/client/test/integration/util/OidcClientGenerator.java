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
package com.vmware.identity.rest.idm.client.test.integration.util;

import java.util.Arrays;
import java.util.List;

import com.vmware.identity.rest.idm.data.OIDCClientMetadataDTO;

public class OidcClientGenerator {

    private static final List<String> REDIRECT_URIS = Arrays.asList("https://www.vmware.com/redirect1", "https://www.vmware.com/redirect2");
    private static final String TOKEN_ENDPOINT_AUTH_METHOD = "private_key_jwt";
    private static final List<String> POST_LOGOUT_REDIRECT_URIS = Arrays.asList("https://www.vmware.com/postlogoutredirect1");
    private static final String LOGOUT_URI = "https://www.vmware.com/logout";
    private static final String OIDC_CERT_SUBJECT_DN = "OU=mID-2400d17e-d4f4-4753-98fd-fb9ecbf098ae,C=US,DC=local,DC=vsphere,CN=oidc-client-123";
    private static final Long AUTHN_REQUEST_CLIENT_ASSERTION_LIFETIME_MS = new Long(1234L);

    public static OIDCClientMetadataDTO generateOIDCClientMetadata() {
        return new OIDCClientMetadataDTO.Builder()
            .withRedirectUris(REDIRECT_URIS)
            .withTokenEndpointAuthMethod(TOKEN_ENDPOINT_AUTH_METHOD)
            .withPostLogoutRedirectUris(POST_LOGOUT_REDIRECT_URIS)
            .withLogoutUri(LOGOUT_URI)
            .withCertSubjectDN(OIDC_CERT_SUBJECT_DN)
            .withAuthnRequestClientAssertionLifetimeMS(AUTHN_REQUEST_CLIENT_ASSERTION_LIFETIME_MS)
            .build();
    }
}
