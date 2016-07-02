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
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.lang3.StringUtils;
import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.GrantType;
import com.vmware.identity.openidconnect.common.ParseException;

/**
 * @author Yehia Zayour
 */
public final class SecurIDGrant extends AuthorizationGrant {
    private static final GrantType GRANT_TYPE = GrantType.SECURID;

    private final String username;
    private final String passcode;
    private final String sessionId;

    public SecurIDGrant(String username, String passcode, String sessionId) {
        super(GRANT_TYPE);

        Validate.notEmpty(username, "username");
        Validate.notEmpty(passcode, "passcode");
        // nullable sessionId

        this.username = username;
        this.passcode = passcode;
        this.sessionId = sessionId;
    }

    public String getUsername() {
        return this.username;
    }

    public String getPasscode() {
        return this.passcode;
    }

    public String getSessionID() {
        return this.sessionId;
    }

    @Override
    public Map<String, String> toParameters() {
        Map<String, String> parameters = new HashMap<String, String>();
        parameters.put("grant_type", GRANT_TYPE.getValue());
        parameters.put("username", this.username);
        parameters.put("passcode", this.passcode);
        if (!StringUtils.isEmpty(this.sessionId)) {
            parameters.put("session_id", Base64Utils.encodeToString(this.sessionId));
        }
        return parameters;
    }

    public static SecurIDGrant parse(Map<String, String> parameters) throws ParseException {
        Validate.notNull(parameters, "parameters");

        GrantType grantType = GrantType.parse(ParameterMapUtils.getString(parameters, "grant_type"));
        if (grantType != GRANT_TYPE) {
            throw new ParseException("unexpected grant_type: " + grantType.getValue());
        }

        String username = ParameterMapUtils.getString(parameters, "username");
        String passcode = ParameterMapUtils.getString(parameters, "passcode");
        String sessionId = null;
        if (parameters.containsKey("session_id")) {
            sessionId = Base64Utils.decodeToString(ParameterMapUtils.getString(parameters, "session_id"));
        }

        return new SecurIDGrant(username, passcode, sessionId);
    }
}