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

import com.vmware.identity.openidconnect.common.AuthenticationRequest;
import com.vmware.identity.openidconnect.common.SessionID;

/**
 * @author Yehia Zayour
 */
public class AuthorizationCodeEntry {
    private final PersonUser personUser;
    private final SessionID sessionId;
    private final AuthenticationRequest authnRequest;

    public AuthorizationCodeEntry(
            PersonUser personUser,
            SessionID sessionId,
            AuthenticationRequest authnRequest) {
        Validate.notNull(personUser, "personUser");
        Validate.notNull(sessionId, "sessionId");
        Validate.notNull(authnRequest, "authnRequest");

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

    public AuthenticationRequest getAuthnRequest() {
        return this.authnRequest;
    }
}
