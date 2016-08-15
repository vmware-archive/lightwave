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
package com.vmware.identity.rest.core.server.authorization.token.jwt.bearer;

import java.text.ParseException;

import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidTokenException;
import com.vmware.identity.rest.core.server.authorization.token.AccessToken;
import com.vmware.identity.rest.core.server.authorization.token.AccessTokenBuilder;
import com.vmware.identity.rest.core.server.authorization.token.TokenInfo;
import com.vmware.identity.rest.core.util.StringManager;

/**
 * An implementation of a bearer token builder using the JSON Web Token format.
 *
 * @see <a href="http://jwt.io/">JSON Web Token Format</a>
 */
public class JWTBearerTokenBuilder implements AccessTokenBuilder {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(JWTBearerTokenBuilder.class);

    protected String tokenTypeField;
    protected String roleField;
    protected String groupsField;
    protected StringManager sm;

    public JWTBearerTokenBuilder(String tokenTypeField, String roleField, String groupsField) {
        this.tokenTypeField = tokenTypeField;
        this.roleField = roleField;
        this.groupsField = groupsField;
        this.sm = StringManager.getManager(Config.LOCALIZATION_PACKAGE_NAME);
    }

    public String getTokenTypeField() {
        return tokenTypeField;
    }

    public String getRoleField() {
        return roleField;
    }

    public String getGroupsField() {
        return groupsField;
    }

    @Override
    public AccessToken build(TokenInfo info) throws InvalidTokenException {
        try {
            SignedJWT jwt = SignedJWT.parse(info.getToken());
            return new JWTBearerToken(jwt, tokenTypeField, roleField, groupsField);
        } catch (ParseException e) {
            log.error("Error parsing the JWT Bearer Token", e);
            throw new InvalidTokenException(sm.getString("auth.ite.parse.malformed"));
        }
    }

}
