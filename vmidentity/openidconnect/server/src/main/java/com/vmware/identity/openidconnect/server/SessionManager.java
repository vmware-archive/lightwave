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

import com.nimbusds.openid.connect.sdk.rp.OIDCClientInformation;
import com.vmware.identity.openidconnect.common.SessionID;

/**
 * @author Yehia Zayour
 */
public class SessionManager {
    private static final long LIFE_TIME_MS = 1000L * 60 * 60 * 8; // 8 hours
    private final SlidingWindowMap<SessionID, SessionEntry> map;

    public SessionManager() {
        this.map = new SlidingWindowMap<SessionID, SessionEntry>(LIFE_TIME_MS);
    }

    public synchronized void add(SessionID sessionId, PersonUser personUser, OIDCClientInformation client) {
        Validate.notNull(sessionId, "sessionId");
        Validate.notNull(personUser, "personUser");
        Validate.notNull(client, "client");

        this.map.add(sessionId, new SessionEntry(personUser, client));
    }

    public synchronized SessionEntry update(SessionID sessionId, OIDCClientInformation client) {
        Validate.notNull(sessionId, "sessionId");
        Validate.notNull(client, "client");

        SessionEntry entry = this.map.get(sessionId);
        if (entry != null) {
            entry.add(client);
        }
        return entry;
    }

    public synchronized SessionEntry remove(SessionID sessionId) {
        Validate.notNull(sessionId, "sessionId");
        return this.map.remove(sessionId);
    }

    public synchronized SessionEntry get(SessionID sessionId) {
        Validate.notNull(sessionId, "sessionId");
        return this.map.get(sessionId);
    }
}
