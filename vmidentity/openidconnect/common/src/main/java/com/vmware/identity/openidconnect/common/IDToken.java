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

import java.text.ParseException;

import com.nimbusds.jose.Header;
import com.nimbusds.jose.util.Base64URL;
import com.nimbusds.jwt.JWT;
import com.nimbusds.jwt.ReadOnlyJWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.nimbusds.oauth2.sdk.id.Identifier;

/**
 * @author Yehia Zayour
 */
public class IDToken extends Identifier implements JWT {
    private static final long serialVersionUID = 2015_05_04L;

    private final SignedJWT signedJwt;

    public IDToken(SignedJWT signedJwt) {
        super(signedJwt.serialize());
        this.signedJwt = signedJwt;
    }

    public SignedJWT getSignedJWT() {
        return this.signedJwt;
    }

    @Override
    public Header getHeader() {
        return this.signedJwt.getHeader();
    }

    @Override
    public ReadOnlyJWTClaimsSet getJWTClaimsSet() throws ParseException {
        return this.signedJwt.getJWTClaimsSet();
    }

    @Override
    public Base64URL[] getParsedParts() {
        return this.signedJwt.getParsedParts();
    }

    @Override
    public String getParsedString() {
        return this.signedJwt.getParsedString();
    }

    @Override
    public String serialize() {
        return this.signedJwt.serialize();
    }

    @Override
    public boolean equals(Object other) {
        boolean areEqual = false;
        if (other instanceof IDToken) {
            IDToken otherToken = (IDToken) other;
            areEqual = otherToken.signedJwt.equals(this.signedJwt);
        }
        return areEqual;
    }

    @Override
    public int hashCode() {
        return super.hashCode(); // use Identifier.hashCode
    }
}
