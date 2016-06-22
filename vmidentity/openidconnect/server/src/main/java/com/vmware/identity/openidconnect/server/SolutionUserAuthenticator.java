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
import java.util.Date;
import java.util.Objects;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JOSEException;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.URIUtils;
import com.vmware.identity.openidconnect.protocol.ClientAssertion;
import com.vmware.identity.openidconnect.protocol.ClientIssuedAssertion;
import com.vmware.identity.openidconnect.protocol.SolutionUserAssertion;


/**
 * @author Yehia Zayour
 */
public class SolutionUserAuthenticator {
    private final CasIdmClient idmClient;

    public SolutionUserAuthenticator(CasIdmClient idmClient) {
        Validate.notNull(idmClient, "idmClient");
        this.idmClient = idmClient;
    }

    public SolutionUser authenticateBySolutionUserAssertion(
            SolutionUserAssertion solutionUserAssertion,
            long solutionUserAssertionLifetimeMs,
            URI requestUri,
            TenantInfo tenantInfo) throws ServerException {
        Validate.notNull(solutionUserAssertion, "solutionUserAssertion");
        Validate.isTrue(solutionUserAssertionLifetimeMs > 0, "solutionUserAssertionLifetimeMs should be positive");
        Validate.notNull(requestUri, "requestUri");
        Validate.notNull(tenantInfo, "tenantInfo");
        return authenticateByAssertion(solutionUserAssertion, solutionUserAssertionLifetimeMs, requestUri, tenantInfo, (ClientInfo) null);
    }

    public SolutionUser authenticateByClientAssertion(
            ClientAssertion clientAssertion,
            long clientAssertionLifetimeMs,
            URI requestUri,
            TenantInfo tenantInfo,
            ClientInfo clientInfo) throws ServerException {
        Validate.notNull(clientAssertion, "clientAssertion");
        Validate.isTrue(clientAssertionLifetimeMs > 0, "clientAssertionLifetimeMs should be positive");
        Validate.notNull(requestUri, "requestUri");
        Validate.notNull(tenantInfo, "tenantInfo");
        Validate.notNull(clientInfo, "clientInfo");
        return authenticateByAssertion(clientAssertion, clientAssertionLifetimeMs, requestUri, tenantInfo, clientInfo);
    }

    private SolutionUser authenticateByAssertion(
            ClientIssuedAssertion assertion,
            long assertionLifetimeMs,
            URI requestUri,
            TenantInfo tenantInfo,
            ClientInfo clientInfo) throws ServerException {
        SolutionUser solutionUser;

        validateAssertion(assertion, assertionLifetimeMs, requestUri, tenantInfo);

        String certSubjectDn;
        if (assertion instanceof ClientAssertion) {
            certSubjectDn = clientInfo.getCertSubjectDN();
            if (certSubjectDn == null) {
                throw new ServerException(ErrorObject.invalidClient("client authn failed because client did not register a cert"));
            }
        } else {
            certSubjectDn = assertion.getIssuer().getValue();
        }

        solutionUser = retrieveSolutionUser(tenantInfo.getName(), certSubjectDn);

        try {
            if (!assertion.hasValidSignature(solutionUser.getPublicKey())) {
                throw new ServerException(ErrorObject.invalidClient(String.format("%s has an invalid signature", assertion.getTokenClass().getValue())));
            }
        } catch (JOSEException e) {
            throw new ServerException(ErrorObject.serverError(String.format("error while verifying %s signature", assertion.getTokenClass().getValue())), e);
        }

        return solutionUser;
    }

    private SolutionUser retrieveSolutionUser(String tenant, String certSubjectDn) throws ServerException {
        com.vmware.identity.idm.SolutionUser idmSolutionUser;
        try {
            idmSolutionUser = this.idmClient.findSolutionUserByCertDn(tenant, certSubjectDn);
        } catch (Exception e) {
            throw new ServerException(ErrorObject.serverError("idm error while retrieving solution user"), e);
        }

        if (idmSolutionUser == null || idmSolutionUser.getId() == null || idmSolutionUser.getCert() == null) {
            throw new ServerException(ErrorObject.invalidRequest("solution user with specified cert subject dn not found"));
        }

        if (idmSolutionUser.isDisabled()) {
            throw new ServerException(ErrorObject.accessDenied("solution user has been disabled or deleted"));
        }

        Date now = new Date();
        if (now.before(idmSolutionUser.getCert().getNotBefore()) || now.after(idmSolutionUser.getCert().getNotAfter())) {
            throw new ServerException(ErrorObject.accessDenied("cert has expired"));
        }

        return new SolutionUser(idmSolutionUser.getId(), tenant, idmSolutionUser.getCert());
    }

    private static void validateAssertion(
            ClientIssuedAssertion assertion,
            long assertionLifetimeMs,
            URI requestUri,
            TenantInfo tenantInfo) throws ServerException {
        // if we are behind rhttp proxy, the requestUri will have http scheme instead of https
        URI httpsRequestUri = URIUtils.changeSchemeComponent(requestUri, "https");
        boolean validAudience =
                assertion.getAudience().size() == 1 &&
                Objects.equals(assertion.getAudience().get(0), httpsRequestUri.toString());
        if (!validAudience) {
            throw new ServerException(ErrorObject.invalidClient(String.format("%s audience does not match request URI", assertion.getTokenClass().getValue())));
        }

        Date now = new Date();
        Date issueTime = assertion.getIssueTime();
        Date lowerBound = new Date(now.getTime() - tenantInfo.getClockToleranceMs() - assertionLifetimeMs);
        Date upperBound = new Date(now.getTime() + tenantInfo.getClockToleranceMs());
        if (issueTime.before(lowerBound) || issueTime.after(upperBound)) {
            String message = (assertion instanceof ClientAssertion) ?
                    "stale_client_assertion" :
                    "stale_solution_user_assertion";
            throw new ServerException(ErrorObject.invalidClient(message));
        }
    }
}