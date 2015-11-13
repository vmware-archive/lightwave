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

import java.security.cert.X509Certificate;

import org.apache.commons.lang3.Validate;

import com.nimbusds.oauth2.sdk.OAuth2Error;
import com.vmware.identity.idm.GSSResult;
import com.vmware.identity.idm.IDMLoginException;
import com.vmware.identity.idm.PrincipalId;

/**
 * @author Yehia Zayour
 */
public class PersonUserAuthenticator {
    private final IdmClient idmClient;

    public PersonUserAuthenticator(IdmClient idmClient) {
        this.idmClient = idmClient;
    }

    public PersonUser authenticate(
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
            throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("idm error while authenticating username/password"), e);
        }
        return new PersonUser(principalId, tenant);
    }

    public PersonUser authenticate(
            String tenant,
            X509Certificate[] tlsCertChain) throws InvalidCredentialsException, ServerException {
        Validate.notEmpty(tenant, "tenant");
        Validate.notNull(tlsCertChain, "tlsCertChain");

        PrincipalId principalId;
        try {
            principalId = this.idmClient.authenticate(tenant, tlsCertChain);
        } catch (IDMLoginException e) {
            throw new InvalidCredentialsException(e);
        } catch (Exception e) {
            throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("idm error while authenticationg tls cert"), e);
        }
        return new PersonUser(principalId, tenant);
    }

    public GSSResult authenticate(
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
            throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("idm error while authenticationg gss ticket"), e);
        }
        return result;
    }
}
