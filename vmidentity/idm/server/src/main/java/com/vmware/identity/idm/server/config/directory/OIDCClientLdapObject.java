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

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

import com.fasterxml.jackson.databind.ObjectMapper;
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

    private static ObjectMapper objectMapper = new ObjectMapper();

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
    private static final String PROPERTY_OIDC_CLIENT_SECRET = "vmwOidcClientSecret";
    private static final String PROPERTY_OIDC_CLIENT_AUTHORITIES = "vmwOidcClientAuthorities";
    private static final String PROPERTY_OIDC_CLIENT_RESOURCE_IDS = "vmwOidcResourceIds";
    private static final String PROPERTY_OIDC_CLIENT_SCOPES = "vmwOidcScopes";
    private static final String PROPERTY_OIDC_CLIENT_AUTO_APPROVE_SCOPES = "vmwOidcAutoApproveScopes";
    private static final String PROPERTY_OIDC_CLIENT_AUTHORIZED_GRANT_TYPES = "vmwOidcAuthorizedGrantTypes";
    private static final String PROPERTY_OIDC_CLIENT_ADDITIONAL_INFORMATION = "vmwOidcAdditionalInformation";

    private static final String REDIRECT_URI_EMPTY_SENTINEL = "N/A";

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
                                List<String> values = new ArrayList<String>(convertToList(ServerUtils.getMultiStringValue(value)));
                                // Remove all of the sentinels from our list
                                values.removeAll(Arrays.asList(REDIRECT_URI_EMPTY_SENTINEL));
                                builder.redirectUris(values);
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                List<String> s = oidcClient.getRedirectUris();
                                // vmwOidcRedirectURIs are a 'MUST' attribute, so we have to substitute in a sentinel
                                if (s == null || s.isEmpty()) {
                                    s = Arrays.asList(REDIRECT_URI_EMPTY_SENTINEL);
                                }
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
                ),
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_OIDC_CLIENT_SECRET,
                        -1,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                ValidateUtil.validateNotNull(builder, "builder");
                                String secret = ServerUtils.getStringValue(value);
                                builder.clientSecret(secret);
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                return ServerUtils.getLdapValue(oidcClient.getClientSecret());
                            }
                        },
                        true
                ),
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_OIDC_CLIENT_AUTHORITIES,
                        -1,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                ValidateUtil.validateNotNull(builder, "builder");
                                builder.authorities(convertToList(ServerUtils.getMultiStringValue(value)));
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                return ServerUtils.getLdapValue(oidcClient.getAuthorities());
                            }
                        },
                        true
                ),
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_OIDC_CLIENT_RESOURCE_IDS,
                        -1,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                ValidateUtil.validateNotNull(builder, "builder");
                                builder.resourceIds(convertToList(ServerUtils.getMultiStringValue(value)));
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                return ServerUtils.getLdapValue(oidcClient.getResourceIds());
                            }
                        },
                        true
                ),
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_OIDC_CLIENT_SCOPES,
                        -1,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                ValidateUtil.validateNotNull(builder, "builder");
                                builder.scopes(convertToList(ServerUtils.getMultiStringValue(value)));
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                return ServerUtils.getLdapValue(oidcClient.getScopes());
                            }
                        },
                        true
                ),
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_OIDC_CLIENT_AUTO_APPROVE_SCOPES,
                        -1,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                ValidateUtil.validateNotNull(builder, "builder");
                                builder.autoApproveScopes(convertToList(ServerUtils.getMultiStringValue(value)));
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                return ServerUtils.getLdapValue(oidcClient.getAutoApproveScopes());
                            }
                        },
                        true
                ),
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_OIDC_CLIENT_AUTHORIZED_GRANT_TYPES,
                        -1,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                ValidateUtil.validateNotNull(builder, "builder");
                                builder.authorizedGrantTypes(convertToList(ServerUtils.getMultiStringValue(value)));
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                return ServerUtils.getLdapValue(oidcClient.getAuthorizedGrantTypes());
                            }
                        },
                        true
                ),
                new PropertyMapperMetaInfoBase<OIDCClient, OIDCClient.Builder>(PROPERTY_OIDC_CLIENT_ADDITIONAL_INFORMATION,
                        -1,
                        true,
                        new IPropertyGetterSetterBase<OIDCClient, OIDCClient.Builder>() {
                            @Override
                            public void SetLdapValue(OIDCClient.Builder builder, LdapValue[] value) {
                                ValidateUtil.validateNotNull(builder, "builder");
                                String additionalInformation = ServerUtils.getStringValue(value);
                                try {
                                    builder.additionalInformation(convertFromJson(additionalInformation));
                                } catch (IllegalArgumentException e) {
                                    throw new IllegalArgumentException("Could not decode JSON for additional information: " + additionalInformation, e);
                                }
                            }

                            @Override
                            public LdapValue[] GetLdapValue(OIDCClient oidcClient) {
                                ValidateUtil.validateNotNull(oidcClient, "oidcClient");
                                try {
                                    return ServerUtils.getLdapValue(convertToJson(oidcClient.getAdditionalInformation()));
                                } catch (IllegalArgumentException e) {
                                    throw new IllegalArgumentException("Could not serialize additional information", e);
                                }
                            }
                        },
                        true
                )});
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

    @SuppressWarnings("unchecked")
    private static Map<String, Object> convertFromJson(String json) {
        try {
            if (json != null && json.length() > 0) {
                return objectMapper.readValue(json, Map.class);
            } else {
                return null;
            }
        } catch (IOException e) {
            throw new IllegalArgumentException(e);
        }
    }

    private static String convertToJson(Map<String, Object> map) {
        try {
            if (map != null && map.size() > 0) {
                return objectMapper.writeValueAsString(map);
            } else {
                return null;
            }
        } catch (IOException e) {
            throw new IllegalArgumentException(e);
        }
    }

    @Override
    protected OIDCClient createFinalObject(Builder object) {
        return object.build();
    };
}
