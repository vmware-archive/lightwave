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

package com.vmware.identity.openidconnect.sample;

import java.util.concurrent.ConcurrentHashMap;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.client.OIDCTokens;
import com.vmware.identity.openidconnect.common.SessionID;

/**
 * @author Yehia Zayour
 */
public class SessionManager {
    private final ConcurrentHashMap<SessionID, OIDCTokens> map;

    public SessionManager() {
        this.map = new ConcurrentHashMap<SessionID, OIDCTokens>();
    }

    public void add(SessionID sessionId, OIDCTokens tokens) {
        Validate.notNull(sessionId, "sessionId");
        Validate.notNull(tokens, "tokens");
        if (this.map.contains(sessionId)) {
            throw new IllegalArgumentException("already has this sessionId: " + sessionId);
        }
        this.map.put(sessionId, tokens);
    }

    public OIDCTokens remove(SessionID sessionId) {
        Validate.notNull(sessionId, "sessionId");
        return this.map.remove(sessionId);
    }

    public OIDCTokens get(SessionID sessionId) {
        Validate.notNull(sessionId, "sessionId");
        return this.map.get(sessionId);
    }
}