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
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.AuthorizationCode;
import com.vmware.identity.openidconnect.common.GrantType;
import com.vmware.identity.openidconnect.common.ParseException;

/**
 * @author Yehia Zayour
 */
public final class AuthorizationCodeGrant extends AuthorizationGrant {
    private static final GrantType GRANT_TYPE = GrantType.AUTHORIZATION_CODE;

    private final AuthorizationCode authzCode;
    private final URI redirectUri;

    public AuthorizationCodeGrant(AuthorizationCode authzCode, URI redirectUri) {
        super(GRANT_TYPE);

        Validate.notNull(authzCode, "authzCode");
        Validate.notNull(redirectUri, "redirectUri");

        this.authzCode = authzCode;
        this.redirectUri = redirectUri;
    }

    public AuthorizationCode getAuthorizationCode() {
        return this.authzCode;
    }

    public URI getRedirectURI() {
        return this.redirectUri;
    }

    @Override
    public Map<String, String> toParameters() {
        Map<String, String> parameters = new HashMap<String, String>();
        parameters.put("grant_type", GRANT_TYPE.getValue());
        parameters.put("code", this.authzCode.getValue());
        parameters.put("redirect_uri", this.redirectUri.toString());
        return parameters;
    }

    public static AuthorizationCodeGrant parse(Map<String, String> parameters) throws ParseException {
        Validate.notNull(parameters, "parameters");

        GrantType grantType = GrantType.parse(ParameterMapUtils.getString(parameters, "grant_type"));
        if (grantType != GRANT_TYPE) {
            throw new ParseException("unexpected grant_type: " + grantType.getValue());
        }

        AuthorizationCode authzCode = new AuthorizationCode(ParameterMapUtils.getString(parameters, "code"));
        URI redirectUri = ParameterMapUtils.getURI(parameters, "redirect_uri");

        return new AuthorizationCodeGrant(authzCode, redirectUri);
    }
}