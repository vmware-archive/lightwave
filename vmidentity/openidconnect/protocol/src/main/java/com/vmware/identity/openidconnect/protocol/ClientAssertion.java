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

package com.vmware.identity.openidconnect.protocol;

import java.net.URI;
import java.security.interfaces.RSAPrivateKey;
import java.util.Date;
import java.util.Map;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.JWTID;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.TokenClass;

/**
 * @author Yehia Zayour
 */
public final class ClientAssertion extends ClientIssuedAssertion {
    private static final TokenClass TOKEN_CLASS = TokenClass.CLIENT_ASSERTION;

    private final SignedJWT signedJwt;

    private ClientAssertion(SignedJWT signedJwt) throws ParseException {
        super(TOKEN_CLASS, signedJwt);

        this.signedJwt = signedJwt;
    }

    public ClientAssertion(
            RSAPrivateKey privateKey,
            JWTID jwtId,
            ClientID clientId,
            URI endpoint,
            Date issueTime) throws JOSEException {
        super(TOKEN_CLASS, jwtId, clientId.getValue(), endpoint, issueTime);

        Validate.notNull(privateKey, "privateKey");

        JWTClaimsSet.Builder claimsBuilder = super.claimsBuilder();
        this.signedJwt = JWTUtils.signClaimsSet(claimsBuilder.build(), privateKey);
    }

    @Override
    protected SignedJWT getSignedJWT() {
        return this.signedJwt;
    }

    public static ClientAssertion parse(Map<String, String> parameters) throws ParseException {
        Validate.notNull(parameters, "parameters");
        return new ClientAssertion(ParameterMapUtils.getSignedJWT(parameters, "client_assertion"));
    }

    public static ClientAssertion parse(String signedJwtString) throws ParseException {
        Validate.notEmpty(signedJwtString, "signedJwtString");
        return new ClientAssertion(JWTUtils.parseSignedJWT(signedJwtString));
    }

    public static ClientAssertion parse(SignedJWT signedJwt) throws ParseException {
        Validate.notNull(signedJwt, "signedJwt");
        return new ClientAssertion(signedJwt);
    }
}