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
import java.util.Date;
import java.util.Map;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jose.JWSAlgorithm;
import com.nimbusds.jose.JWSHeader;
import com.nimbusds.jose.util.Base64URL;
import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.common.JWTID;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.PersonUserAssertionSigner;
import com.vmware.identity.openidconnect.common.TokenClass;

/**
 * @author Yehia Zayour
 */
public final class PersonUserAssertion extends ClientIssuedAssertion {
    private static final TokenClass TOKEN_CLASS = TokenClass.PERSON_USER_ASSERTION;

    private final SignedJWT signedJwt;

    private PersonUserAssertion(SignedJWT signedJwt) throws ParseException {
        super(TOKEN_CLASS, signedJwt);

        this.signedJwt = signedJwt;
    }

    public PersonUserAssertion(
            PersonUserAssertionSigner signer,
            JWTID jwtId,
            String certSubjectDN,
            URI tokenEndpoint,
            Date issueTime) throws JOSEException {
        super(TOKEN_CLASS, jwtId, certSubjectDN, tokenEndpoint, issueTime);

        Validate.notNull(signer, "signer");

        JWTClaimsSet.Builder claimsBuilder = super.claimsBuilder();

        SignedJWT unsignedJwt = new SignedJWT(new JWSHeader(JWSAlgorithm.RS256), claimsBuilder.build());
        byte[] signature = signer.signUsingRS256(unsignedJwt.getSigningInput());

        try {
            this.signedJwt = new SignedJWT(
                    unsignedJwt.getHeader().toBase64URL(),
                    unsignedJwt.getPayload().toBase64URL(),
                    Base64URL.encode(signature));
        } catch (java.text.ParseException e) {
            throw new JOSEException("failed to create SignedJWT", e);
        }
    }

    @Override
    protected SignedJWT getSignedJWT() {
        return this.signedJwt;
    }

    public static PersonUserAssertion parse(Map<String, String> parameters) throws ParseException {
        Validate.notNull(parameters, "parameters");
        return new PersonUserAssertion(ParameterMapUtils.getSignedJWT(parameters, "person_user_assertion"));
    }
}