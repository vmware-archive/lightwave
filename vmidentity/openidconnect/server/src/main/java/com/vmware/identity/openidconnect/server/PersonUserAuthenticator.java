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

package com.vmware.identity.openidconnect.server;

import java.net.URI;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPublicKey;
import java.util.Collections;
import java.util.List;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JOSEException;
import com.vmware.identity.idm.GSSResult;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IDMLoginException;
import com.vmware.identity.idm.IDMSecureIDNewPinException;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.RSAAMResult;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.protocol.PersonUserAssertion;

/**
 * @author Yehia Zayour
 */
public class PersonUserAuthenticator {
    private final CasIdmClient idmClient;

    public PersonUserAuthenticator(CasIdmClient idmClient) {
        Validate.notNull(idmClient, "idmClient");
        this.idmClient = idmClient;
    }

    public PersonUser authenticateByPassword(
            String tenant,
            String username,
            String password) throws InvalidCredentialsException, ServerException {
        Validate.notEmpty(tenant, "tenant");
        Validate.notEmpty(username, "username");
        Validate.notEmpty(password, "password");

        PrincipalId principalId;
        try {
            principalId = this.idmClient.authenticate(tenant, username, password);
        } catch (IDMLoginException e) {
            throw new InvalidCredentialsException(e);
        } catch (Exception e) {
            throw new ServerException(ErrorObject.serverError("idm error while authenticating username/password"), e);
        }
        return new PersonUser(principalId, tenant);
    }

    public PersonUser authenticateByPersonUserCertificate(
            String tenant,
            X509Certificate personUserCertificate,
            PersonUserAssertion personUserAssertion,
            long assertionLifetimeMs,
            URI requestUri,
            long clockToleranceMS) throws InvalidCredentialsException, ServerException {
        Validate.notEmpty(tenant, "tenant");
        Validate.notNull(personUserCertificate, "personUserCertificate");
        Validate.notNull(personUserAssertion, "personUserAssertion");
        Validate.isTrue(assertionLifetimeMs > 0, "assertionLifetimeMs > 0");
        Validate.notNull(requestUri, "requestUri");
        Validate.isTrue(clockToleranceMS >= 0, "clockToleranceMS >= 0");

        if (!("RSA").equals(personUserCertificate.getPublicKey().getAlgorithm())) {
            throw new ServerException(ErrorObject.invalidGrant("certificate should use RSA for signature"));
        }

        try {
            if (!personUserAssertion.hasValidSignature((RSAPublicKey) personUserCertificate.getPublicKey())) {
                throw new ServerException(ErrorObject.invalidGrant("person_user_assertion has an invalid signature"));
            }
        } catch (JOSEException e) {
            throw new ServerException(ErrorObject.serverError("error while verifying person_user_assertion signature"), e);
        }

        String errorDescription = personUserAssertion.validate(assertionLifetimeMs, requestUri, clockToleranceMS);
        if (errorDescription != null) {
            throw new ServerException(ErrorObject.invalidGrant(errorDescription));
        }

        return authenticateByPersonUserCertificate(tenant, Collections.singletonList(personUserCertificate));
    }

    public PersonUser authenticateByPersonUserCertificate(
            String tenant,
            List<X509Certificate> personUserCertificateChain) throws InvalidCredentialsException, ServerException {
        Validate.notEmpty(tenant, "tenant");
        Validate.notNull(personUserCertificateChain, "personUserCertificateChain");

        PrincipalId principalId;
        try {
            principalId = this.idmClient.authenticate(tenant, personUserCertificateChain.toArray(new X509Certificate[0]), null);
        } catch (IDMLoginException e) {
            throw new InvalidCredentialsException(e);
        } catch (Exception e) {
            throw new ServerException(ErrorObject.serverError("idm error while authenticating person user cert"), e);
        }
        return new PersonUser(principalId, tenant);
    }

    public GSSResult authenticateByGssTicket(
            String tenant,
            String contextId,
            byte[] gssTicket) throws InvalidCredentialsException, ServerException {
        Validate.notEmpty(tenant, "tenant");
        Validate.notEmpty(contextId, "contextId");
        Validate.notNull(gssTicket, "gssTicket");

        GSSResult result;
        try {
            result = this.idmClient.authenticate(tenant, contextId, gssTicket);
        } catch (IDMLoginException e) {
            throw new InvalidCredentialsException(e);
        } catch (Exception e) {
            throw new ServerException(ErrorObject.serverError("idm error while authenticationg gss ticket"), e);
        }
        return result;
    }

    public RSAAMResult authenticateBySecurID(
            String tenant,
            String username,
            String passcode,
            String sessionId) throws InvalidCredentialsException, ServerException, IDMSecureIDNewPinException {
        Validate.notEmpty(tenant, "tenant");
        Validate.notEmpty(username, "username");
        Validate.notEmpty(passcode, "passcode");
        // nullable sessionId

        try {
            return this.idmClient.authenticateRsaSecurId(tenant, sessionId, username, passcode);
        } catch (IDMSecureIDNewPinException e) {
            throw e;
        } catch (IDMException e) {
            throw new InvalidCredentialsException(e);
        } catch (Exception e) {
            throw new ServerException(ErrorObject.serverError("idm error while authentication securid token"), e);
        }
    }
}
