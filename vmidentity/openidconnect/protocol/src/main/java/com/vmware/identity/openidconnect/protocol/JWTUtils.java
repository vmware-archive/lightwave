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

import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.util.Date;
import java.util.Objects;

import net.minidev.json.JSONObject;

import org.apache.commons.lang3.StringUtils;
import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jose.JWSAlgorithm;
import com.nimbusds.jose.JWSHeader;
import com.nimbusds.jose.JWSSigner;
import com.nimbusds.jose.crypto.RSASSASigner;
import com.nimbusds.jose.jwk.JWK;
import com.nimbusds.jose.jwk.JWKSet;
import com.nimbusds.jose.jwk.KeyType;
import com.nimbusds.jose.jwk.KeyUse;
import com.nimbusds.jose.jwk.RSAKey;
import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.TokenClass;

/**
 * @author Yehia Zayour
 */
public final class JWTUtils {
    private JWTUtils() {
    }

    public static SignedJWT signClaimsSet(JWTClaimsSet claims, RSAPrivateKey privateKey) throws JOSEException {
        Validate.notNull(claims, "claims");
        Validate.notNull(privateKey, "privateKey");

        SignedJWT signedJWT = new SignedJWT(new JWSHeader(JWSAlgorithm.RS256), claims);
        JWSSigner signer = new RSASSASigner(privateKey);
        signedJWT.sign(signer);
        return signedJWT;
    }

    public static Date getIssueTime(JWTClaimsSet claims, TokenClass tokenClass) throws ParseException {
        Validate.notNull(claims, "claims");
        Validate.notNull(tokenClass, "tokenClass");

        Date result = claims.getIssueTime();
        if (result == null) {
            throw new ParseException(String.format("%s is missing %s claim", tokenClass.getValue(), "iat"));
        }
        return result;
    }

    public static Date getExpirationTime(JWTClaimsSet claims, TokenClass tokenClass) throws ParseException {
        Validate.notNull(claims, "claims");
        Validate.notNull(tokenClass, "tokenClass");

        Date result = claims.getExpirationTime();
        if (result == null) {
            throw new ParseException(String.format("%s is missing %s claim", tokenClass.getValue(), "exp"));
        }
        return result;
    }

    public static String getString(JWTClaimsSet claims, TokenClass tokenClass, String key) throws ParseException {
        Validate.notNull(claims, "signedJwt");
        Validate.notNull(tokenClass, "tokenClass");
        Validate.notEmpty(key, "key");

        String result;
        try {
            result = claims.getStringClaim(key);
        } catch (java.text.ParseException e) {
            throw new ParseException(String.format("%s has non-string %s claim", tokenClass.getValue(), key), e);
        }

        if (StringUtils.isEmpty(result)) {
            throw new ParseException(String.format("%s is missing %s claim", tokenClass.getValue(), key));
        }

        return result;
    }

    public static String[] getStringArray(JWTClaimsSet claims, TokenClass tokenClass, String key) throws ParseException {
        Validate.notNull(claims, "signedJwt");
        Validate.notNull(tokenClass, "tokenClass");
        Validate.notEmpty(key, "key");

        String[] result;
        try {
            result = claims.getStringArrayClaim(key);
        } catch (java.text.ParseException e) {
            throw new ParseException(String.format("%s has non-string-array %s claim", tokenClass.getValue(), key), e);
        }

        if (result == null) {
            throw new ParseException(String.format("%s is missing %s claim", tokenClass.getValue(), key));
        }

        return result;
    }

    public static JWKSet getJWKSet(JWTClaimsSet claims, TokenClass tokenClass, String key) throws ParseException {
        Validate.notNull(claims, "signedJwt");
        Validate.notNull(tokenClass, "tokenClass");
        Validate.notEmpty(key, "key");

        Object jwkSetObject = claims.getClaim(key);

        if (jwkSetObject == null) {
            throw new ParseException(String.format("%s is missing %s claim", tokenClass.getValue(), key));
        }

        if (!(jwkSetObject instanceof JSONObject)) {
            throw new ParseException(String.format("%s has non-json %s claim", tokenClass.getValue(), key));
        }

        try {
            return JWKSet.parse((JSONObject) jwkSetObject);
        } catch (java.text.ParseException e) {
            throw new ParseException(String.format("%s has non-jwkset %s claim", tokenClass.getValue(), key), e);
        }
    }

    public static RSAPublicKey getPublicKey(JWKSet jwkSet) throws ParseException {
        Validate.notNull(jwkSet, "jwkSet");

        if (jwkSet.getKeys() == null || jwkSet.getKeys().size() != 1) {
            throw new ParseException("JWKSet must contain a single JWK");
        }
        JWK jwk = jwkSet.getKeys().get(0);

        if (
                !Objects.equals(jwk.getAlgorithm(), JWSAlgorithm.RS256) ||
                !Objects.equals(jwk.getKeyType(),   KeyType.RSA) ||
                !Objects.equals(jwk.getKeyUse(),    KeyUse.SIGNATURE)) {
            throw new ParseException("JWK is not of the correct type");
        }
        RSAKey rsaKey = (RSAKey) jwk;

        try {
            return rsaKey.toRSAPublicKey();
        } catch (JOSEException e) {
            throw new ParseException("failed to convert RSAKey to RSAPublicKey", e);
        }
    }

    public static JWTClaimsSet getClaimsSet(SignedJWT signedJwt) throws ParseException {
        Validate.notNull(signedJwt, "signedJwt");

        try {
            return signedJwt.getJWTClaimsSet();
        } catch (java.text.ParseException e) {
            throw new ParseException("failed to extract ClaimsSet from SignedJWT", e);
        }
    }

    public static SignedJWT parseSignedJWT(String signedJwtString) throws ParseException {
        Validate.notEmpty(signedJwtString, "signedJwtString");

        try {
            return SignedJWT.parse(signedJwtString);
        } catch (java.text.ParseException e) {
            throw new ParseException("failed to parse SignedJWT", e);
        }
    }
}