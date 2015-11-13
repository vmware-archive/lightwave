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

package com.vmware.identity.openidconnect.server;

import java.net.URI;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Assert;
import org.junit.Test;

import com.nimbusds.jose.JWSAlgorithm;
import com.nimbusds.oauth2.sdk.auth.ClientAuthenticationMethod;
import com.nimbusds.oauth2.sdk.id.ClientID;
import com.nimbusds.openid.connect.sdk.rp.OIDCClientInformation;
import com.nimbusds.openid.connect.sdk.rp.OIDCClientMetadata;
import com.vmware.identity.idm.OIDCClient;
import com.vmware.identity.idm.OIDCClient.Builder;

/**
 * @author Jun Sun
 */
public class OIDCClientUtilsTest {

    @Test(expected=IllegalArgumentException.class)
    public void testConvertNullValues() throws Exception {

        String clientID = "client";

        // test ConvertToOIDCClientInformation
        OIDCClient oidcClient = new Builder(clientID).build();
        OIDCClientInformation actualOIDCClientInformation = OIDCClientUtils.convertToOIDCClientInformation(oidcClient);
        OIDCClientInformation expectedOIDCClientInformation = new OIDCClientInformation(
                new ClientID(clientID),
                null,
                new OIDCClientMetadata(),
                null,
                null,
                null);
        assertEqualsOIDCClientInformation(expectedOIDCClientInformation, actualOIDCClientInformation);

        // test ConvertToOIDCClient
        OIDCClientInformation oidcClientInformation = new OIDCClientInformation(
                new ClientID(clientID),
                null,
                new OIDCClientMetadata(),
                null,
                null,
                null);
        OIDCClient actualOIDCClient = OIDCClientUtils.convertToOIDCClient(oidcClientInformation);
        OIDCClient expectedOIDCClient = new Builder(clientID).build();
        assertEqualsOIDCClient(expectedOIDCClient, actualOIDCClient);
    }

    @Test
    public void testConvertFullValues() throws Exception {

        // Build oidcClient object
        String clientID = "client";
        List<String> redirectUris = Arrays.asList("https://www.vmware.com/redirect1", "https://www.vmware.com/redirect2");
        String tokenEndpointAuthMethod = "private_key_jwt";
        String tokenEndpointAuthSigningAlg = "RS256";
        String idTokenSignedResponseAlg = "RS256";
        List<String> postLogoutRedirectUris = Arrays.asList("https://www.vmware.com/postlogoutredirect1", "https://www.vmware.com/postlogoutredirect2");
        String logoutUri = "https://www.vmware.com/logout";
        String certSubDN = "OU=mID-2400d17e-d4f4-4753-98fd-fb9ecbf098ae,C=US,DC=local,DC=vsphere,CN=oidc-client-123";

        Builder oidcClientBuilder = new Builder(clientID);
        oidcClientBuilder.redirectUris(redirectUris);
        oidcClientBuilder.tokenEndpointAuthMethod(tokenEndpointAuthMethod);
        oidcClientBuilder.tokenEndpointAuthSigningAlg(tokenEndpointAuthSigningAlg);
        oidcClientBuilder.idTokenSignedResponseAlg(idTokenSignedResponseAlg);
        oidcClientBuilder.postLogoutRedirectUris(postLogoutRedirectUris);
        oidcClientBuilder.logoutUri(logoutUri);
        oidcClientBuilder.certSubjectDN(certSubDN);
        OIDCClient oidcClient = oidcClientBuilder.build();

        // Build corresponding oidcClientInformation object
        OIDCClientMetadata oidcClientMetadata = new OIDCClientMetadata();
        Set<URI> redirectURIs = new HashSet<URI>(Arrays.asList(new URI("https://www.vmware.com/redirect1"), new URI("https://www.vmware.com/redirect2")));
        ClientAuthenticationMethod authMethod = ClientAuthenticationMethod.PRIVATE_KEY_JWT;
        JWSAlgorithm authJWSAlg = JWSAlgorithm.RS256;
        JWSAlgorithm idTokenJWSAlg = JWSAlgorithm.RS256;
        Set<URI> logoutURIs = new HashSet<URI>(Arrays.asList(new URI("https://www.vmware.com/postlogoutredirect1"), new URI("https://www.vmware.com/postlogoutredirect2")));

        oidcClientMetadata.setRedirectionURIs(redirectURIs);
        oidcClientMetadata.setTokenEndpointAuthMethod(authMethod);
        oidcClientMetadata.setTokenEndpointAuthJWSAlg(authJWSAlg);
        oidcClientMetadata.setIDTokenJWSAlg(idTokenJWSAlg);
        oidcClientMetadata.setPostLogoutRedirectionURIs(logoutURIs);
        oidcClientMetadata.setCustomField("logout_uri", logoutUri);
        oidcClientMetadata.setCustomField("cert_subject_dn", certSubDN);

        // test ConvertToOIDCClientInformation
        OIDCClientInformation actualOIDCClientInformation = OIDCClientUtils.convertToOIDCClientInformation(oidcClient);
        OIDCClientInformation expectedOIDCClientInformation = new OIDCClientInformation(
                new ClientID(clientID),
                null,
                oidcClientMetadata,
                null,
                null,
                null);
        assertEqualsOIDCClientInformation(expectedOIDCClientInformation, actualOIDCClientInformation);

        // test ConvertToOIDCClient
        OIDCClientInformation oidcClientInformation = new OIDCClientInformation(
                new ClientID(clientID),
                null,
                oidcClientMetadata,
                null,
                null,
                null);
        OIDCClient actualOIDCClient = OIDCClientUtils.convertToOIDCClient(oidcClientInformation);
        OIDCClient expectedOIDCClient = oidcClient;
        assertEqualsOIDCClient(expectedOIDCClient, actualOIDCClient);
    }

    private void assertEqualsOIDCClientInformation(OIDCClientInformation expected, OIDCClientInformation actual) {
        Assert.assertEquals(expected.getID(), actual.getID());

        OIDCClientMetadata expectedMetadata = expected.getOIDCMetadata();
        OIDCClientMetadata actualMetadata = actual.getOIDCMetadata();
        Assert.assertEquals(expectedMetadata.getRedirectionURIs(), actualMetadata.getRedirectionURIs());
        Assert.assertEquals(expectedMetadata.getTokenEndpointAuthMethod(), actualMetadata.getTokenEndpointAuthMethod());
        Assert.assertEquals(expectedMetadata.getTokenEndpointAuthJWSAlg(), actualMetadata.getTokenEndpointAuthJWSAlg());
        Assert.assertEquals(expectedMetadata.getIDTokenJWSAlg(), actualMetadata.getIDTokenJWSAlg());
        Assert.assertEquals(expectedMetadata.getPostLogoutRedirectionURIs(), actualMetadata.getPostLogoutRedirectionURIs());
        Assert.assertEquals(expectedMetadata.getCustomField("logout_uri"), actualMetadata.getCustomField("logout_uri"));
        Assert.assertEquals(expectedMetadata.getCustomField("cert_subject_dn"), actualMetadata.getCustomField("cert_subject_dn"));
    }

    private void assertEqualsOIDCClient(OIDCClient expected, OIDCClient actual) {
        Assert.assertEquals(expected.getClientId(), actual.getClientId());

        if (expected.getRedirectUris() != null) {
            Assert.assertEquals(new HashSet<String>(expected.getRedirectUris()), new HashSet<String>(actual.getRedirectUris()));
        } else {
            Assert.assertNull(actual.getRedirectUris());
        }
        Assert.assertEquals(expected.getTokenEndpointAuthMethod(), actual.getTokenEndpointAuthMethod());
        Assert.assertEquals(expected.getTokenEndpointAuthSigningAlg(), actual.getTokenEndpointAuthSigningAlg());
        Assert.assertEquals(expected.getIdTokenSignedResponseAlg(), actual.getIdTokenSignedResponseAlg());
        if (expected.getPostLogoutRedirectUris() != null) {
            Assert.assertEquals(new HashSet<String>(expected.getPostLogoutRedirectUris()), new HashSet<String>(actual.getPostLogoutRedirectUris()));
        } else {
            Assert.assertNull(actual.getPostLogoutRedirectUris());
        }
        Assert.assertEquals(expected.getLogoutUri(), actual.getLogoutUri());
        Assert.assertEquals(expected.getCertSubjectDN(), actual.getCertSubjectDN());
    }
}
