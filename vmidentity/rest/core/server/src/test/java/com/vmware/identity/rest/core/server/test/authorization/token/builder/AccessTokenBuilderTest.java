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
package com.vmware.identity.rest.core.server.test.authorization.token.builder;

import static org.junit.Assert.assertEquals;

import java.text.ParseException;

import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.rest.core.server.authorization.token.AccessToken;
import com.vmware.identity.rest.core.server.authorization.token.AccessTokenBuilder;
import com.vmware.identity.rest.core.server.authorization.token.TokenInfo;
import com.vmware.identity.rest.core.server.authorization.token.TokenStyle;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;

public abstract class AccessTokenBuilderTest {

    protected static String RESOURCE_SERVER_AUDIENCE = "rs_admin_server";

    public AccessToken build(SignedJWT jwt, TokenStyle style, TokenType type, AccessTokenBuilder builder) throws ParseException {
        TokenInfo info = new TokenInfo(style, type, jwt.serialize(), null);

        AccessToken token = builder.build(info);

        assertEquals("Subject does not match", jwt.getJWTClaimsSet().getSubject(), token.getSubject());
        assertEquals("Issuer does not match", jwt.getJWTClaimsSet().getIssuer(), token.getIssuer());
        assertEquals("Issued At does not match", jwt.getJWTClaimsSet().getIssueTime(), token.getIssueTime());
        assertEquals("Expiration Time does not match", jwt.getJWTClaimsSet().getExpirationTime(), token.getExpirationTime());
        assertEquals("Audience is not contained in the token", jwt.getJWTClaimsSet().getAudience(), token.getAudience());

        return token;
    }

}
