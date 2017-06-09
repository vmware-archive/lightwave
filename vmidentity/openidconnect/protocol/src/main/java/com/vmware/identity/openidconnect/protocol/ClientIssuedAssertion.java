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
        // if we are behind rhttp proxy, we will receive the request as http instead of https, also the port number might be non-443
        if (!Objects.equals(
                URIUtils.changePortComponent(URIUtils.changeSchemeComponent(requestUri, "https"), 443),
                URIUtils.changePortComponent(this.targetEndpoint, 443))) {
            return String.format("%s audience does not match request URI", this.getTokenClass().getValue());
        }

        Date now = new Date();
        Date issueTime = this.getIssueTime();
        Date expirationTime = new Date(issueTime.getTime() + assertionLifetimeMs);
        Date notBefore = new Date(issueTime.getTime() - clockToleranceMS);
        Date notAfter = new Date(expirationTime.getTime() + clockToleranceMS);
        if (now.before(notBefore)) {
            return String.format("%s is not yet valid", this.getTokenClass().getValue());
        }
        if (now.after(notAfter)) {
            return String.format("%s has expired", this.getTokenClass().getValue());
        }
        return null;
    }
}