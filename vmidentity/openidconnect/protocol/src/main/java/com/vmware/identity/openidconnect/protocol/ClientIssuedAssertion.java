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
import java.util.Arrays;
import java.util.Date;
import java.util.Objects;

import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.Issuer;
import com.vmware.identity.openidconnect.common.JWTID;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.Subject;
import com.vmware.identity.openidconnect.common.TokenClass;
import com.vmware.identity.openidconnect.common.TokenType;

/**
 * @author Yehia Zayour
 */
public abstract class ClientIssuedAssertion extends JWTToken {
    private final URI targetEndpoint;

    protected ClientIssuedAssertion(TokenClass tokenClass, SignedJWT signedJwt) throws ParseException {
        super(tokenClass, signedJwt);

        if (!Objects.equals(super.getIssuer().getValue(), super.getSubject().getValue())) {
            throw new ParseException(ErrorObject.invalidClient(tokenClass.getValue() + " issuer and subject must be the same"));
        }

        if (this.getAudience().size() != 1) {
            throw new ParseException(ErrorObject.invalidClient(tokenClass.getValue() + " audience should be single-valued"));
        }

        try {
            this.targetEndpoint = URIUtils.parseURI(super.getAudience().get(0));
        } catch (ParseException e) {
            throw new ParseException(ErrorObject.invalidClient(tokenClass.getValue() + " audience should be a valid URI"), e);
        }
    }

    protected ClientIssuedAssertion(
            TokenClass tokenClass,
            JWTID jwtId,
            String issuerAndSubject,
            URI targetEndpoint,
            Date issueTime) {
        super(
                tokenClass,
                TokenType.BEARER,
                jwtId,
                new Issuer(issuerAndSubject),
                new Subject(issuerAndSubject),
                Arrays.asList(targetEndpoint.toString()),
                issueTime);
        this.targetEndpoint = targetEndpoint;
    }

    public URI getTargetEndpoint() {
        return this.targetEndpoint;
    }

    public String validate(
            long assertionLifetimeMs,
            URI requestUri,
            long clockToleranceMS) {
        // if we are behind rhttp proxy, the requestUri will have http scheme instead of https
        URI httpsRequestUri = URIUtils.changeSchemeComponent(requestUri, "https");
        if (!URIUtils.areEqual(httpsRequestUri, this.targetEndpoint)) {
            return String.format("%s audience does not match request URI", this.getTokenClass().getValue());
        }

        Date now = new Date();
        Date issueTime = this.getIssueTime();
        Date lowerBound = new Date(now.getTime() - clockToleranceMS - assertionLifetimeMs);
        Date upperBound = new Date(now.getTime() + clockToleranceMS);
        if (issueTime.before(lowerBound) || issueTime.after(upperBound)) {
            return String.format("stale_%s", this.getTokenClass().getValue());
        }

        return null;
    }
}