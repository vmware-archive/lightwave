/*
 *
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
 *
 */

package com.vmware.identity.idm.server.config.directory;

import java.util.Arrays;
import java.util.List;

import com.vmware.identity.idm.OIDCClient;
import com.vmware.identity.idm.OIDCClient.Builder;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.interop.ldap.LdapValue;

// The semantic of OIDC client fields can be found in the following links:
// OAUTH: https://tools.ietf.org/html/rfc6749#section-2
// OIDC: http://openid.net/specs/openid-connect-registration-1_0.html#ClientMetadata
//       http://openid.net/specs/openid-connect-registration-1_0.html#RegistrationResponse
public final class OIDCClientLdapObject extends BaseLdapObjectBase<OIDCClient, OIDCClient.Builder> {

    private static OIDCClientLdapObject _instance = new OIDCClientLdapObject();

    public static OIDCClientLdapObject getInstance() {
        return _instance;
    }

    private static final String OBJECT_CLASS = "vmwOidcRelyingParty";

    private static final String PROPERTY_NAME = CN;
    private static final String PROPERTY_OIDC_CLIENT_ID = "vmwOidcClientID";
    public static final String PROPERTY_OIDC_REDIRECT_URIS = "vmwOidcRedirectURIs";
    private static final String PROPERTY_OIDC_TOKEN_ENDPOINT_AUTH_METHOD = "vmwOidcTokenEndpointAuthMethod";
    private static final String PROPERTY_OIDC_TOKEN_ENDPOINT_JWS_ALG = "vmwOidcTokenEndpointJWSAlg";
    private static final String PROPERTY_OIDC_ID_TOKEN_JWS_ALG = "vmwOidcIDTokenJWSAlg";
    public static final String PROPERTY_OIDC_POST_LOGOUT_REDIRECT_URI = "vmwOidcPostLogoutRedirectURI";
    public static final String PROPERTY_OIDC_LOGOUT_URI = "vmwOidcLogoutURI";
    public static final String PROPERTY_OIDC_CERT_SUB_DN = "vmwOidcCertSubDN";
    public static final String PROPERTY_OIDC_AUTHN_REQUEST_CLIENT_ASSERTION_LIFETIME_MS = "vmwOidcAuthnRequestClientAssertionLifetimeMS";

    @SuppressWarnings("unchecked")
    private OIDCClientLdapObject() {
        super(OBJECT_CLASS, new PropertyMapperMetaInfoBase[] {
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_NAME,
                        0,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                throw new IllegalStateException("property is not settable.");
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                String s = oidcClient.getClientId();
                                return ServerUtils.getLdapValue(s);
                            }
                        },
                        false // cannot update in ldap
                ),
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_OIDC_CLIENT_ID,
                        -1,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                // no op
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                String s = oidcClient.getClientId();
                                return ServerUtils.getLdapValue(s);
                            }
                        },
                        false // cannot update in ldap
                ),
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_OIDC_REDIRECT_URIS,
                        -1,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                ValidateUtil.validateNotNull(builder, "builder");
                                builder.redirectUris(convertToList(ServerUtils.getMultiStringValue(value)));
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                List<String> s = oidcClient.getRedirectUris();
                                return ServerUtils.getLdapValue(s);
                            }
                        },
                        true
                ),
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_OIDC_TOKEN_ENDPOINT_AUTH_METHOD,
                        -1,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                ValidateUtil.validateNotNull(builder, "builder");
                                builder.tokenEndpointAuthMethod(ServerUtils.getStringValue(value));
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                String s = oidcClient.getTokenEndpointAuthMethod();
                                return ServerUtils.getLdapValue(s);
                            }
                        },
                        true
                ),
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_OIDC_TOKEN_ENDPOINT_JWS_ALG,
                        -1,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                ValidateUtil.validateNotNull(builder, "builder");
                                builder.tokenEndpointAuthSigningAlg(ServerUtils.getStringValue(value));
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                String s = oidcClient.getTokenEndpointAuthSigningAlg();
                                return ServerUtils.getLdapValue(s);
                            }
                        },
                        true
                ),
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_OIDC_ID_TOKEN_JWS_ALG,
                        -1,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                ValidateUtil.validateNotNull(builder, "builder");
                                builder.idTokenSignedResponseAlg(ServerUtils.getStringValue(value));
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                String s = oidcClient.getIdTokenSignedResponseAlg();
                                return ServerUtils.getLdapValue(s);
                            }
                        },
                        true
                ),
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_OIDC_POST_LOGOUT_REDIRECT_URI,
                        -1,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                ValidateUtil.validateNotNull(builder, "builder");
                                builder.postLogoutRedirectUris(convertToList(ServerUtils.getMultiStringValue(value)));
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                List<String> s = oidcClient.getPostLogoutRedirectUris();
                                return ServerUtils.getLdapValue(s);
                            }
                        },
                        true
                ),
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_OIDC_LOGOUT_URI,
                        -1,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                ValidateUtil.validateNotNull(builder, "builder");
                                builder.logoutUri(ServerUtils.getStringValue(value));
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                String s = oidcClient.getLogoutUri();
                                return ServerUtils.getLdapValue(s);
                            }
                        },
                        true
                ),
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_OIDC_CERT_SUB_DN,
                        -1,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                ValidateUtil.validateNotNull(builder, "builder");
                                builder.certSubjectDN(ServerUtils.getStringValue(value));
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                String s = oidcClient.getCertSubjectDN();
                                return ServerUtils.getLdapValue(s);
                            }
                        },
                        true
                ),
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_OIDC_AUTHN_REQUEST_CLIENT_ASSERTION_LIFETIME_MS,
                        -1,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                ValidateUtil.validateNotNull(builder, "builder");
                                long longValue = (value == null) ? 0L : ServerUtils.getNativeLongValue(value);
                                builder.authnRequestClientAssertionLifetimeMS(longValue);
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                return ServerUtils.getLdapValue(oidcClient.getAuthnRequestClientAssertionLifetimeMS());
                            }
                        },
                        true
                ) });
    }

    @Override
    protected Builder createObject(List<LdapValue[]> ctorParams) {
        if ((ctorParams == null) || (ctorParams.size() != 1)) {
            throw new IllegalArgumentException("ctorParams");
        }

        return new Builder(ServerUtils.getStringValue(ctorParams.get(0)));
    }

    private static List<String> convertToList(String[] s) {
        if (s != null) {
            return Arrays.asList(s);
        } else {
            return null;
        }
    }

    @Override
    protected OIDCClient createFinalObject(Builder object) {
        return object.build();
    };
}
