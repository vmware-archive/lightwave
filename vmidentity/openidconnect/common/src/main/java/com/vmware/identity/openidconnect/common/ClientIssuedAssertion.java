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

package com.vmware.identity.openidconnect.common;

import java.net.URI;
import java.util.Arrays;
import java.util.Date;
import java.util.Objects;

import com.nimbusds.jwt.SignedJWT;

/**
 * @author Yehia Zayour
 */
public abstract class ClientIssuedAssertion extends JWTToken {
    protected ClientIssuedAssertion(TokenClass tokenClass, SignedJWT signedJwt) throws ParseException {
        super(tokenClass, signedJwt);

        if (!Objects.equals(super.getIssuer().getValue(), super.getSubject().getValue())) {
            throw new ParseException(ErrorObject.invalidClient(super.getTokenClass().getValue() + " issuer and subject must be the same"));
        }
    }

    protected ClientIssuedAssertion(
            TokenClass tokenClass,
            JWTID jwtId,
            String issuerAndSubject,
            URI endpoint,
            Date issueTime) {
        super(
                tokenClass,
                TokenType.BEARER,
                jwtId,
                new Issuer(issuerAndSubject),
                new Subject(issuerAndSubject),
                Arrays.asList(endpoint.toString()),
                issueTime);
    }
}