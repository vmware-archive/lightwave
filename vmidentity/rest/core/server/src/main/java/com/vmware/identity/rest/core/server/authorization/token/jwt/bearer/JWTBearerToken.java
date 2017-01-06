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
package com.vmware.identity.rest.core.server.authorization.token.jwt.bearer;

import java.text.ParseException;
import java.util.Date;
import java.util.List;

import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.rest.core.server.authorization.token.AccessToken;

/**
 * An implementation of a bearer token using the JSON Web Token format.
 *
 * @see <a href="http://jwt.io/">JSON Web Token Format</a>
 */
public class JWTBearerToken implements AccessToken {

    private SignedJWT jwt;
    private JWTClaimsSet claims;
    private String tokenType;
    private String role;
    private List<String> groups;

    /**
     * Constructs a new {@link JWTBearerToken}.
     *
     * @param jwt the signed JWT object that represents this token
     * @param tokenTypeField the field that contains the token type
     * @param roleField the field that contains the role
     * @param groupsField the field that contains the groups
     * @throws ParseException if there is an error parsing information from the JWT
     */
    @SuppressWarnings("unchecked")
    public JWTBearerToken(SignedJWT jwt, String tokenTypeField, String roleField, String groupsField) throws ParseException {
        this.jwt = jwt;
        this.claims = jwt.getJWTClaimsSet();
        this.tokenType = (String) claims.getClaim(tokenTypeField);
        this.role = (String) claims.getClaim(roleField);
        this.groups = (List<String>) claims.getClaim(groupsField);
    }

    @Override
    public List<String> getAudience() {
        return claims.getAudience();
    }

    @Override
    public String getIssuer() {
        return claims.getIssuer();
    }

    @Override
    public String getRole() {
        return role;
    }

    @Override
    public List<String> getGroupList() {
        return groups;
    }

    @Override
    public Date getIssueTime() {
        return claims.getIssueTime();
    }

    @Override
    public Date getExpirationTime() {
        return claims.getExpirationTime();
    }

    @Override
    public String getSubject() {
        return claims.getSubject();
    }

    @Override
    public String getTokenType() {
        return tokenType;
    }

    /**
     * Fetch the {@link SignedJWT} object from the access token.
     *
     * @return the JWT object from the access token
     */
    public SignedJWT getJWT() {
        return jwt;
    }

    /**
     * Fetch the {@link ReadOnlyJWTClaimsSet} from the access token.
     *
     * @return the claim set from the access token
     */
    public JWTClaimsSet getClaims() {
        return claims;
    }

}
