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
package com.vmware.identity.rest.core.server.authorization.request;

import java.security.cert.Certificate;
import java.util.Collection;
import java.util.EnumSet;
import java.util.List;

import javax.ws.rs.container.ContainerRequestContext;

import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.RoleMapper;
import com.vmware.identity.rest.core.server.authorization.exception.InsufficientRoleException;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidTokenException;
import com.vmware.identity.rest.core.server.authorization.token.AccessToken;
import com.vmware.identity.rest.core.server.authorization.token.AccessTokenBuilder;
import com.vmware.identity.rest.core.server.authorization.token.TokenInfo;
import com.vmware.identity.rest.core.server.authorization.token.TokenStyle;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;
import com.vmware.identity.rest.core.server.authorization.token.extractor.AccessTokenBodyExtractor;
import com.vmware.identity.rest.core.server.authorization.token.extractor.AccessTokenExtractor;
import com.vmware.identity.rest.core.server.authorization.token.extractor.AccessTokenHeaderExtractor;
import com.vmware.identity.rest.core.server.authorization.token.extractor.AccessTokenQueryExtractor;
import com.vmware.identity.rest.core.server.authorization.token.jwt.bearer.JWTBearerTokenBuilder;
import com.vmware.identity.rest.core.server.authorization.token.jwt.hok.JWTHoKTokenBuilder;
import com.vmware.identity.rest.core.server.authorization.token.saml.SAMLTokenBuilder;
import com.vmware.identity.rest.core.server.authorization.verifier.AccessTokenVerifier;
import com.vmware.identity.rest.core.server.authorization.verifier.jwt.bearer.JWTBearerTokenVerifier;
import com.vmware.identity.rest.core.server.authorization.verifier.jwt.hok.JWTHoKTokenVerifier;
import com.vmware.identity.rest.core.server.authorization.verifier.saml.SAMLTokenVerifier;
import com.vmware.identity.rest.core.server.exception.ServerException;
import com.vmware.identity.rest.core.server.util.ClientFactory;
import com.vmware.identity.rest.core.server.util.PathParameters;
import com.vmware.identity.rest.core.util.StringManager;

/**
 * Represents a single resource access request
 */
public class ResourceAccessRequest {

    private TokenStyle style;
    private TokenType type;
    private AccessToken token;
    private AccessTokenVerifier verifier;
    private boolean secure;
    private RoleMapper roleMapper;

    private StringManager sm;

    /**
     * Constructor for creating ResourceAccessRequest.
     * <p>
     * All requests should be parsed from a {@link ContainerRequestContext} by {@link #fromRequestContext(ContainerRequestContext)}.
     *
     * @param style the style of token that was used for the request
     * @param type the type of token that was used for the request
     * @param token the token that was used in this request
     * @param verifier a verifier object for use in token verification
     * @param secure a flag indicating whether the request is secure
     */
    public ResourceAccessRequest(TokenStyle style, TokenType type, AccessToken token, AccessTokenVerifier verifier, boolean secure, RoleMapper roleMapper) {
        this.style = style;
        this.type = type;
        this.token = token;
        this.verifier = verifier;
        this.secure = secure;
        this.roleMapper = roleMapper;
        this.sm = StringManager.getManager(Config.LOCALIZATION_PACKAGE_NAME);
    }

    /**
     * Returns the token style used in this <tt>ResourceAccessRequest</tt>.
     *
     * @return the {@link TokenStyle} that represents the token style used for this request
     */
    public TokenStyle getStyle() {
        return style;
    }

    /**
     * Returns the token type used in this <tt>ResourceAccessRequest</tt>.
     *
     * @return the {@link TokenType} that represents the token type used for this request
     */
    public TokenType getType() {
        return type;
    }

    /**
     * Returns the access token used in this <tt>ResourceAccessRequest</tt>.
     *
     * @return the {@link AccessToken} that represents the token used for this request
     */
    public AccessToken getToken() {
        return token;
    }

    /**
     * Returns the security state of this <tt>ResourceAccessRequest</tt>.
     *
     * @return boolean indicating whether the request is secure or not.
     */
    public boolean isSecure() {
        return secure;
    }

    /**
     * Returns the role mapper of this <tt>ResourceAccessRequest</tt>.
     *
     * @return role mapper used for mapping roles to groups
     */
    public RoleMapper getRoleMapper() {
        return roleMapper;
    }

    /**
     * Verifies the access token for this request.
     *
     * @return <tt>true</tt> if the token was successfully verified.
     * @throws InvalidTokenException if the token could not be verified
     * @throws ServerException if an error occurred preventing the token from being verified
     */
    public void verify() throws InvalidTokenException, ServerException {
         verifier.verify(token);
    }

    /**
     * Validates the contents of the access token.
     *
     * @throws InvalidTokenException if the token could not be validated
     */
    public void validateContents() throws InvalidTokenException {
        if (token.getSubject() == null) {
            throw new InvalidTokenException(sm.getString("auth.ite.bad.subject"));
        }
    }

    /**
     * Checks the token's role or groups against the required role.
     *
     * <p>If the token's role field is not null, we assume that to be the
     * sole arbiter of the token's role. If the token's role field is null,
     * then we check against the group listing offered by the token.
     *
     * @param requiredRole the role to ensure that the token is valid for
     */
    public void validateRole(Role requiredRole) throws InsufficientRoleException {
        if (token.getRole() != null) {
            checkRoleField(requiredRole);
        } else {
            checkGroupsField(requiredRole);
        }
    }

    private void checkRoleField(Role requiredRole) throws InsufficientRoleException {
        Role role = Role.findByRoleName(token.getRole());

        if (role == null) {
            throw new InsufficientRoleException(sm.getString("auth.ise.bad.role", token.getRole()));
        } else if (!role.is(requiredRole)) {
            throw new InsufficientRoleException(sm.getString("auth.ise.wrong.role", requiredRole));
        }
    }

    private void checkGroupsField(Role requiredRole) throws InsufficientRoleException {
        List<String> groupList = token.getGroupList();
        if (groupList != null && !groupList.isEmpty()) {
            Role[] roles = Role.values();

            // Start at the highest role and work our way down until we've gone too low
            int i = roles.length - 1;

            Role role = roles[i];
            while (role.is(requiredRole)) {
                if (checkIfGroupExists(groupList, roleMapper.getRoleGroup(role).getGroupNetbios())) {
                    return;
                }
                role = roles[--i];
            }
        }

        throw new InsufficientRoleException(sm.getString("auth.ise.wrong.role", requiredRole));
    }

    private boolean checkIfGroupExists(List<String> groupList, String group) {
        boolean groupExists = false;
        for (String groupName : groupList) {
            if (groupName.equalsIgnoreCase(group)) {
                groupExists = true;
            }
        }
        return groupExists;
    }

    /**
     * Construct a ResourceAccessRequest from a {@link ContainerRequestContext}.
     *
     * @param context the request to parse from
     * @return the request object constructed from the specified context or null if no token exists
     */
    public static ResourceAccessRequest fromRequestContext(ContainerRequestContext context) throws ServerException {
        boolean secure = isRequestSecure(context);

        CasIdmClient client = ClientFactory.getClient();

        TokenInfo info = getTokenInfo(context);
        // There were no tokens to be found, we can leave without building anything
        if (info == null) {
            return null;
        }

        AccessTokenBuilder builder = getAccessTokenBuilder(info.getType());
        AccessToken token = builder.build(info);

        String tenant = getTenant(context, client);
        long skew = getSkew(tenant, client);
        Certificate cert = getSigningCert(tenant, client);

        AccessTokenVerifier verifier = getAccessTokenVerifier(context, info, skew, cert);


        RoleMapper roleMapper = new RoleMapper(getSystemDomainOfTenant(tenant, client), getSystemDomainOfSystemTenant(client));

        return new ResourceAccessRequest(info.getStyle(), info.getType(), token, verifier, secure, roleMapper);
    }

    /**
     * Determine whether the request is secure by checking the header and the URI scheme.
     *
     * @param context the request to check the security of
     * @return true if the header is marked with "X-SSL-SECURE=true" or the scheme is "https", false otherwise
     */
    private static boolean isRequestSecure(ContainerRequestContext context) {
        String scheme = context.getUriInfo().getRequestUri().getScheme();
        String header = context.getHeaderString(Config.X_SSL_SECURE_HEADER);

        if (header != null && header.equals("true")) {
            return true;
        }

        if (scheme != null) {
            return scheme.equalsIgnoreCase("https");
        }

        return false;
    }

    /**
     * Fetch the {@link TokenInfo} from a {@link ContainerRequestContext}.
     *
     * @param context the request to parse from
     * @return the token info constructed from the specified context or null if one does not exist
     */
    private static TokenInfo getTokenInfo(ContainerRequestContext context) {
        for (TokenStyle style : TokenStyle.values()) {
            AccessTokenExtractor ex = getAccessTokenExtractor(context, style);

            if (ex.exists()) {
                return ex.extract();
            }
        }

        return null;
    }

    /**
     * Get the {@link AccessTokenExtractor} for a specific {@link TokenStyle}.
     *
     * @param context the request to extract from
     * @param style the token style to create an extractor for
     * @return the extractor for the token style
     */
    private static AccessTokenExtractor getAccessTokenExtractor(ContainerRequestContext context, TokenStyle style) {
        switch(style) {
        case HEADER:
            return new AccessTokenHeaderExtractor(context, Config.ACCESS_TOKEN_HEADER);
        case QUERY:
            return new AccessTokenQueryExtractor(context, Config.ACCESS_TOKEN_PARAMETER, Config.TOKEN_TYPE_PARAMETER, Config.TOKEN_SIGNATURE_PARAMETER);
        case BODY:
            return new AccessTokenBodyExtractor(context, Config.ACCESS_TOKEN_PARAMETER, Config.TOKEN_TYPE_PARAMETER, Config.TOKEN_SIGNATURE_PARAMETER);
        default:
            throw new IllegalArgumentException("Invalid TokenStyle: '" + style.toString() + "'");
        }
    }

    /**
     * Get the {@link AccessTokenBuilder} for a specific {@link TokenType}.
     *
     * @param type the token type to create a builder for
     * @return the builder for the token type
     * @throws ServerException
     */
    private static AccessTokenBuilder getAccessTokenBuilder(TokenType type) throws ServerException {
        switch(type) {
        case BEARER:
            return new JWTBearerTokenBuilder(Config.JWT_TYPE_FIELD, Config.JWT_ROLE_FIELD, Config.JWT_GROUPS_FIELD);
        case HOK:
            return new JWTHoKTokenBuilder(Config.JWT_TYPE_FIELD, Config.JWT_ROLE_FIELD, Config.JWT_GROUPS_FIELD, Config.JWT_HOK_FIELD);
        case SAML:
            return new SAMLTokenBuilder();
        default:
            throw new IllegalArgumentException("Invalid TokenType: '" + type.toString() + "'");
        }
    }

    /**
     * Get the {@link AccessTokenVerifier} for a specific {@link TokenType}.
     *
     * @param context the request to verify with
     * @param info the token info that we are creating a verifier with
     * @param tenant the tenant this request is targeted for
     * @param client the IDM client for retrieving the signing cert
     * @return the verifier for the token type
     * @throws ServerException server error preventing retrieval of the default tenant name or signing cert
     */
    private static AccessTokenVerifier getAccessTokenVerifier(ContainerRequestContext context, TokenInfo info, long skew, Certificate cert) throws ServerException {
        switch(info.getType()) {
        case BEARER:
            return new JWTBearerTokenVerifier(skew, cert.getPublicKey());
        case HOK:
            return new JWTHoKTokenVerifier(info.getSignature(), context, skew, cert.getPublicKey());
        case SAML:
            return new SAMLTokenVerifier(info.getSignature(), context, skew, cert);
        default:
            throw new IllegalArgumentException("Invalid TokenType: '" + info.getType().toString() + "'");
        }
    }

    /**
     * Fetch the tenant using the tenant specified in the request URI.
     *
     * @param context the request to fetch the URI from
     * @param client the IDM client to communicate with
     * @return the tenant corresponding to the request URI
     * @throws ServerException if there is a server error preventing the retrieval of the tenant name
     */
    private static String getTenant(ContainerRequestContext context, CasIdmClient client) throws ServerException {
        String tenant = context.getUriInfo().getPathParameters().getFirst(PathParameters.TENANT_NAME);

        if (tenant == null) {
            try {
                tenant = client.getSystemTenant();
            } catch (Exception e) {
                throw new ServerException("An error occurred with the IDM client", e);
            }
        }

        return tenant;
    }

    /**
     * Fetch the clock tolerance for a tenant.
     *
     * @param tenant the tenant to fetch the clock tolerance of
     * @param client the IDM client to communicate with
     * @return the clock tolerance of the tenant in milliseconds
     * @throws ServerException if there is a server error preventing the retrieval of the clock tolerance
     */
    private static long getSkew(String tenant, CasIdmClient client) throws ServerException {
        try {
            return client.getClockTolerance(tenant);
        } catch (Exception e) {
            throw new ServerException("An error occurred with the IDM client", e);
        }
    }

    /**
     * Fetch the signing certificate for a tenant
     *
     * @param tenant the tenant to get the signing certificates for
     * @param client the IDM client to communicate with
     * @return the signing certificate for <tt>tenant</tt>
     * @throws ServerException if there is a server error preventing the retrieval of the certificate
     */
    private static Certificate getSigningCert(String tenant, CasIdmClient client) throws ServerException {
        List<Certificate> certList;
        try {
            certList = client.getTenantCertificate(tenant);
        } catch (Exception e) {
            throw new ServerException("An error occurred with the IDM client", e);
        }

        if (certList == null || certList.isEmpty()) {
            throw new ServerException("Unable to retrieve a signing certificate from tenant '" + tenant + "'");
        }

        return certList.get(0);
    }

    /**
     * Fetch the system domain for a tenant
     *
     * @param tenant the tenant to get the system domain of
     * @param client the IDM client to communicate with
     * @return the system domain for <tt>tenant</tt>
     * @throws ServerException if there is a server error preventing the retrieval of the system domain
     */
    private static String getSystemDomainOfTenant(String tenant, CasIdmClient client) throws ServerException {
        String systemDomain = null;
        try {
            Collection<IIdentityStoreData> stores = client.getProviders(tenant, EnumSet.of(DomainType.SYSTEM_DOMAIN));
            if ((stores != null) && (stores.size() > 0)) {
                systemDomain = stores.iterator().next().getName();
            }
        } catch (Exception e) {
            throw new ServerException("An error occurred with the IDM client", e);
        }

        return systemDomain;
    }

    /**
     * Fetch the system tenant
     *
     * @param client the IDM client to communicate with
     * @return the name of the system tenant
     * @throws ServerException if there is a server error preventing the retrieval of the system tenant
     */
    private static String getSystemTenant(CasIdmClient client) throws ServerException {
        String systemTenant = null;
        try {
            systemTenant = client.getSystemTenant();
        } catch (Exception e) {
            throw new ServerException("An error occurred with the IDM client", e);
        }

        return systemTenant;
    }

    /**
     * Utility function for fetching the system domain of the system tenant
     *
     * @param client the IDM client to communicate with
     * @return the name of the system tenant
     * @throws ServerException if there is a server error preventing the retrieval of the system tenant's system domain
     */
    private static String getSystemDomainOfSystemTenant(CasIdmClient client) throws ServerException {
        return getSystemDomainOfTenant(getSystemTenant(client), client);
    }

}
