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
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.nimbusds.jose.JWSAlgorithm;
import com.nimbusds.oauth2.sdk.ParseException;
import com.nimbusds.oauth2.sdk.auth.ClientAuthenticationMethod;
import com.nimbusds.oauth2.sdk.id.ClientID;
import com.nimbusds.openid.connect.sdk.rp.OIDCClientInformation;
import com.nimbusds.openid.connect.sdk.rp.OIDCClientMetadata;
import com.vmware.identity.idm.OIDCClient;
import com.vmware.identity.idm.OIDCClient.Builder;

/**
 * @author Jun Sun
 */
public class OIDCClientUtils {

    // Convert OIDCClientInformation to OIDCClient
    // The semantic of OIDC client fields can be found in the following links:
    // OAUTH: https://tools.ietf.org/html/rfc6749#section-2
    // OIDC: http://openid.net/specs/openid-connect-registration-1_0.html#ClientMetadata
    //       http://openid.net/specs/openid-connect-registration-1_0.html#RegistrationResponse
    public static OIDCClient convertToOIDCClient(OIDCClientInformation oidcClientInformation) throws ParseException {

        OIDCClient oidcClient = null;

        if (oidcClientInformation.getID() != null) {
            Builder oidcClientBuilder = new Builder(oidcClientInformation.getID().toString());

            if (oidcClientInformation.getOIDCMetadata() != null) {
                OIDCClientMetadata oidcClientMetadata = oidcClientInformation.getOIDCMetadata();

                if (oidcClientMetadata.getRedirectionURIs() != null) {
                    oidcClientBuilder.redirectUris(convertSetToStringList(oidcClientMetadata.getRedirectionURIs()));
                }

                if (oidcClientMetadata.getTokenEndpointAuthMethod() != null) {
                    oidcClientBuilder.tokenEndpointAuthMethod(oidcClientMetadata.getTokenEndpointAuthMethod().toString());
                }

                if (oidcClientMetadata.getTokenEndpointAuthJWSAlg() != null) {
                    oidcClientBuilder.tokenEndpointAuthSigningAlg(oidcClientMetadata.getTokenEndpointAuthJWSAlg().toString());
                }

                if (oidcClientMetadata.getIDTokenJWSAlg() != null) {
                    oidcClientBuilder.idTokenSignedResponseAlg(oidcClientMetadata.getIDTokenJWSAlg().toString());
                }

                if (oidcClientMetadata.getPostLogoutRedirectionURIs() != null) {
                    oidcClientBuilder.postLogoutRedirectUris(convertSetToStringList(oidcClientMetadata.getPostLogoutRedirectionURIs()));
                }

                if (oidcClientMetadata.getCustomField("logout_uri") != null) {
                    oidcClientBuilder.logoutUri((String) oidcClientMetadata.getCustomField("logout_uri"));
                }

                if (oidcClientMetadata.getCustomField("cert_subject_dn") != null) {
                    oidcClientBuilder.certSubjectDN((String) oidcClientMetadata.getCustomField("cert_subject_dn"));
                }
            }
            oidcClient = oidcClientBuilder.build();
        }
        return oidcClient;
    }

    private static <T> List<String> convertSetToStringList(Set<T> set) {
        List<String> list = null;
        if (!set.isEmpty()) {
            list = new ArrayList<String>();
            for (T entry : set) {
                list.add(entry.toString());
            }
        }
        return list;
    }

    // Convert OIDCClient to OIDCClientInformation
    // The semantic of OIDC client fields can be found in the following links:
    // OAUTH: https://tools.ietf.org/html/rfc6749#section-2
    // OIDC: http://openid.net/specs/openid-connect-registration-1_0.html#ClientMetadata
    //       http://openid.net/specs/openid-connect-registration-1_0.html#RegistrationResponse
    public static OIDCClientInformation convertToOIDCClientInformation(OIDCClient oidcClient) throws ParseException, URISyntaxException, java.text.ParseException {

        ClientID clientID = null;
        if (oidcClient.getClientId() != null) {
            clientID = new ClientID(oidcClient.getClientId());
        }

        OIDCClientMetadata oidcClientMetadata = new OIDCClientMetadata();

        Set<URI> redirectURIs = null;
        if (oidcClient.getRedirectUris() != null) {
            redirectURIs = new HashSet<URI>();
            for (String s : oidcClient.getRedirectUris()) {
                redirectURIs.add(new URI(s));
            }
        }
        oidcClientMetadata.setRedirectionURIs(redirectURIs);

        if (oidcClient.getTokenEndpointAuthMethod() != null) {
            oidcClientMetadata.setTokenEndpointAuthMethod(ClientAuthenticationMethod.parse(oidcClient.getTokenEndpointAuthMethod()));
        }

        if (oidcClient.getTokenEndpointAuthSigningAlg() != null) {
            oidcClientMetadata.setTokenEndpointAuthJWSAlg(JWSAlgorithm.parse(oidcClient.getTokenEndpointAuthSigningAlg()));
        }

        if (oidcClient.getIdTokenSignedResponseAlg() != null) {
            oidcClientMetadata.setIDTokenJWSAlg(JWSAlgorithm.parse(oidcClient.getIdTokenSignedResponseAlg()));
        }

        Set<URI> logoutURIs = null;
        if (oidcClient.getPostLogoutRedirectUris() != null) {
            logoutURIs = new HashSet<URI>();
            for (String s : oidcClient.getPostLogoutRedirectUris()) {
                logoutURIs.add(new URI(s));
            }
        }
        oidcClientMetadata.setPostLogoutRedirectionURIs(logoutURIs);

        if (oidcClient.getLogoutUri() != null) {
            oidcClientMetadata.setCustomField("logout_uri", oidcClient.getLogoutUri());
        }

        if (oidcClient.getCertSubjectDN() != null) {
            oidcClientMetadata.setCustomField("cert_subject_dn", oidcClient.getCertSubjectDN());
        }

        return new OIDCClientInformation(clientID, null, oidcClientMetadata, null, null, null);
    }
}
