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

import java.util.Map;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.GrantType;
import com.vmware.identity.openidconnect.common.ParseException;

/**
 * @author Yehia Zayour
 */
public abstract class AuthorizationGrant {
    private final GrantType grantType;

    public AuthorizationGrant(GrantType grantType) {
        Validate.notNull(grantType, "grantType");
        this.grantType = grantType;
    }

    public GrantType getGrantType() {
        return this.grantType;
    }

    public abstract Map<String, String> toParameters();

    public static AuthorizationGrant parse(Map<String, String> parameters) throws ParseException {
        Validate.notNull(parameters, "parameters");

        GrantType grantType = GrantType.parse(ParameterMapUtils.getString(parameters, "grant_type"));

        AuthorizationGrant authzGrant;
        switch (grantType) {
            case AUTHORIZATION_CODE:
                authzGrant = AuthorizationCodeGrant.parse(parameters);
                break;
            case PASSWORD:
                authzGrant = PasswordGrant.parse(parameters);
                break;
            case SOLUTION_USER_CREDENTIALS:
                authzGrant = SolutionUserCredentialsGrant.parse(parameters);
                break;
            case CLIENT_CREDENTIALS:
                authzGrant = ClientCredentialsGrant.parse(parameters);
                break;
            case PERSON_USER_CERTIFICATE:
                authzGrant = PersonUserCertificateGrant.parse(parameters);
                break;
            case GSS_TICKET:
                authzGrant = GSSTicketGrant.parse(parameters);
                break;
            case SECURID:
                authzGrant = SecurIDGrant.parse(parameters);
                break;
            case REFRESH_TOKEN:
                authzGrant = RefreshTokenGrant.parse(parameters);
                break;
            default:
                throw new IllegalStateException("unrecognized grant type: " + grantType.getValue());
        }
        return authzGrant;
    }
}