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

import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.util.Collection;
import java.util.Date;
import java.util.List;

import com.nimbusds.jose.JOSEException;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.Issuer;
import com.vmware.identity.openidconnect.common.JWTID;
import com.vmware.identity.openidconnect.common.Nonce;
import com.vmware.identity.openidconnect.common.Scope;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.Subject;
import com.vmware.identity.openidconnect.common.TokenClass;
import com.vmware.identity.openidconnect.common.TokenType;
import com.vmware.identity.openidconnect.protocol.AccessToken;
import com.vmware.identity.openidconnect.protocol.IDToken;
import com.vmware.identity.openidconnect.protocol.ServerIssuedToken;

/**
 * @author Yehia Zayour
 */
public final class JWTBuilder {
    private final RSAPrivateKey privateKey;
    private final TokenClass tokenClass;
    private TokenType tokenType;
    private JWTID jwtId;
    private Issuer issuer;
    private Subject subject;
    private List<String> audience;
    private Date issueTime;
    private Date expirationTime;
    private Scope scope;
    private String tenant;
    private ClientID clientId;
    private SessionID sessionId;
    private RSAPublicKey holderOfKey;
    private Subject actAs;
    private Nonce nonce;
    private Collection<String> groups;
    private String givenName;       // id_token only
    private String familyName;      // id_token only

    private JWTBuilder(RSAPrivateKey privateKey, TokenClass tokenClass) {
        this.privateKey = privateKey;
        this.tokenClass = tokenClass;
    }

    public static JWTBuilder idTokenBuilder(RSAPrivateKey privateKey) {
        return new JWTBuilder(privateKey, TokenClass.ID_TOKEN);
    }

    public static JWTBuilder accessTokenBuilder(RSAPrivateKey privateKey) {
        return new JWTBuilder(privateKey, TokenClass.ACCESS_TOKEN);
    }

    public JWTBuilder tokenType(TokenType tokenType) {
        this.tokenType = tokenType;
        return this;
    }

    public JWTBuilder jwtId(JWTID jwtId) {
        this.jwtId = jwtId;
        return this;
    }

    public JWTBuilder issuer(Issuer issuer) {
        this.issuer = issuer;
        return this;
    }

    public JWTBuilder subject(Subject subject) {
        this.subject = subject;
        return this;
    }

    public JWTBuilder audience(List<String> audience) {
        this.audience = audience;
        return this;
    }

    public JWTBuilder issueTime(Date issueTime) {
        this.issueTime = issueTime;
        return this;
    }

    public JWTBuilder expirationTime(Date expirationTime) {
        this.expirationTime = expirationTime;
        return this;
    }

    public JWTBuilder scope(Scope scope) {
        this.scope = scope;
        return this;
    }

    public JWTBuilder tenant(String tenant) {
        this.tenant = tenant;
        return this;
    }

    public JWTBuilder clientId(ClientID clientId) {
        this.clientId = clientId;
        return this;
    }

    public JWTBuilder sessionId(SessionID sessionId) {
        this.sessionId = sessionId;
        return this;
    }

    public JWTBuilder holderOfKey(RSAPublicKey holderOfKey) {
        this.holderOfKey = holderOfKey;
        return this;
    }

    public JWTBuilder actAs(Subject actAs) {
        this.actAs = actAs;
        return this;
    }

    public JWTBuilder nonce(Nonce nonce) {
        this.nonce = nonce;
        return this;
    }

    public JWTBuilder groups(Collection<String> groups) {
        this.groups = groups;
        return this;
    }

    public JWTBuilder givenName(String givenName) {
        if (this.tokenClass != TokenClass.ID_TOKEN) {
            throw new IllegalStateException("givenName only applies to id_token");
        }
        this.givenName = givenName;
        return this;
    }

    public JWTBuilder familyName(String familyName) {
        if (this.tokenClass != TokenClass.ID_TOKEN) {
            throw new IllegalStateException("familyName only applies to id_token");
        }
        this.familyName = familyName;
        return this;
    }

    public String build() {
        ServerIssuedToken token;

        if (this.tokenClass == TokenClass.ID_TOKEN) {
            try {
                token = new IDToken(
                        this.privateKey,
                        this.tokenType,
                        this.jwtId,
                        this.issuer,
                        this.subject,
                        this.audience,
                        this.issueTime,
                        this.expirationTime,
                        this.scope,
                        this.tenant,
                        this.clientId,
                        this.sessionId,
                        this.holderOfKey,
                        this.actAs,
                        this.nonce,
                        this.groups,
                        this.givenName,
                        this.familyName);
            } catch (JOSEException e) {
                throw new IllegalArgumentException("failed to sign id_token");
            }
        } else if (this.tokenClass == TokenClass.ACCESS_TOKEN) {
            try {
                token = new AccessToken(
                        this.privateKey,
                        this.tokenType,
                        this.jwtId,
                        this.issuer,
                        this.subject,
                        this.audience,
                        this.issueTime,
                        this.expirationTime,
                        this.scope,
                        this.tenant,
                        this.clientId,
                        this.sessionId,
                        this.holderOfKey,
                        this.actAs,
                        this.nonce,
                        this.groups,
                        null /* adminServerRole */);
            } catch (JOSEException e) {
                throw new IllegalArgumentException("failed to sign access_token");
            }
        } else {
            throw new IllegalStateException("unexpected tokenClass: " + this.tokenClass);
        }

        return token.serialize();
    }
}