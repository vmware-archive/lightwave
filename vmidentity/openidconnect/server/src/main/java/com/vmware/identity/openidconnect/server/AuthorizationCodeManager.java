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

import org.apache.commons.lang3.Validate;

import com.nimbusds.oauth2.sdk.AuthorizationCode;
import com.vmware.identity.openidconnect.common.AuthenticationRequest;
import com.vmware.identity.openidconnect.common.SessionID;

/**
 * @author Yehia Zayour
 */
public class AuthorizationCodeManager {
    private static final long LIFE_TIME_MS = 1000L * 60 * 2; // 2 minutes
    private final SlidingWindowMap<AuthorizationCode, AuthorizationCodeEntry> map;

    public AuthorizationCodeManager() {
        this.map = new SlidingWindowMap<AuthorizationCode, AuthorizationCodeEntry>(LIFE_TIME_MS);
    }

    public synchronized void add(
            AuthorizationCode authzCode,
            PersonUser personUser,
            SessionID sessionId,
            AuthenticationRequest authnRequest) {
        Validate.notNull(authzCode, "authzCode");
        Validate.notNull(personUser, "personUser");
        Validate.notNull(sessionId, "sessionId");
        Validate.notNull(authnRequest, "authnRequest");

        this.map.add(authzCode, new AuthorizationCodeEntry(personUser, sessionId, authnRequest));
    }

    public synchronized AuthorizationCodeEntry remove(AuthorizationCode authzCode) {
        Validate.notNull(authzCode, "authzCode");
        return this.map.remove(authzCode);
    }
}
