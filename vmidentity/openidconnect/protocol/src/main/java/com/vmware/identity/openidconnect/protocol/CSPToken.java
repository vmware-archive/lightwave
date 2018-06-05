/*
 *  Copyright (c) 2018 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.openidconnect.protocol;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.common.Nonce;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.TokenClass;

public class CSPToken extends FederationToken {

    private static final String CLAIM_CONTEXT_NAME = "context_name";
    private static final String CLAIM_USER_NAME = "username";
    private static final String CLAIM_DOMAIN = "domain";
    private static final String CLAIM_EMAIL = "email";
    private static final String CLAIM_PERMS = "perms";

    private String orgId;
    private String username;
    private String domain;
    private String emailId;
    private Collection<String> permissions;
    private Nonce nonce;

    public CSPToken(TokenClass tokenClass, SignedJWT signedJwt) throws ParseException {
        super(tokenClass, signedJwt);

        JWTClaimsSet claims = JWTUtils.getClaimsSet(signedJwt);
        this.orgId = JWTUtils.getString(claims, tokenClass, CLAIM_CONTEXT_NAME);
        this.username = JWTUtils.getString(claims, tokenClass, CLAIM_USER_NAME);
        this.domain = JWTUtils.getString(claims, tokenClass, CLAIM_DOMAIN);
        if (claims.getClaims().containsKey(CLAIM_EMAIL)) {
            this.emailId = JWTUtils.getString(claims, tokenClass, CLAIM_EMAIL);
        }
        if (claims.getClaims().containsKey(CLAIM_PERMS)) {
            String[] permissions = JWTUtils.getStringArray(claims, tokenClass, CLAIM_PERMS);
            this.permissions = Arrays.asList(permissions);
        } else {
            this.permissions = Collections.emptySet();
        }
        Nonce nonce = null;
        if (claims.getClaims().containsKey("nonce")) {
            nonce = new Nonce(JWTUtils.getString(claims, tokenClass, "nonce"));
        }
        this.nonce = nonce;
    }

    @Override
    public String getTenant() {
        return this.orgId;
    }

    @Override
    public String getUsername() {
        return this.username;
    }

    @Override
    public String getDomain() {
        return this.domain;
    }

    public String getEmailAddress() {
        return this.emailId;
    }

    @Override
    public Collection<String> getPermissions() {
        return this.permissions;
    }

    @Override
    public Nonce getNonce() {
        return this.nonce;
    }
}
