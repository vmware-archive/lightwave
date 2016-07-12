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

import com.vmware.identity.openidconnect.common.AuthorizationCode;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.protocol.AuthenticationRequest;

/**
 * @author Yehia Zayour
 */
public class AuthorizationCodeManager {
    private static final long LIFETIME_MS = 10 * 60 * 1000L; // 10 minutes (allow for clock skew)
    private final SlidingWindowMap<AuthorizationCode, Entry> map;

    public AuthorizationCodeManager() {
        this.map = new SlidingWindowMap<AuthorizationCode, Entry>(LIFETIME_MS);
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

        this.map.add(authzCode, new Entry(personUser, sessionId, authnRequest));
    }

    public synchronized Entry remove(AuthorizationCode authzCode) {
        Validate.notNull(authzCode, "authzCode");
        return this.map.remove(authzCode);
    }

    public static class Entry {
        private final PersonUser personUser;
        private final SessionID sessionId;
        private final AuthenticationRequest authnRequest;

        private Entry(
                PersonUser personUser,
                SessionID sessionId,
                AuthenticationRequest authnRequest) {
            this.personUser = personUser;
            this.sessionId = sessionId;
            this.authnRequest = authnRequest;
        }

        public PersonUser getPersonUser() {
            return this.personUser;
        }

        public SessionID getSessionId() {
            return this.sessionId;
        }

        public AuthenticationRequest getAuthenticationRequest() {
            return this.authnRequest;
        }
    }
}