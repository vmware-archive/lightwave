/*
 *  Copyright (c) 2018 VMware, Inc.  All Rights Reserved.
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

import com.vmware.identity.openidconnect.common.State;

public class FederationAuthenticationRequestTracker {

    private static final long REQUEST_STATE_LIFETIME_MS = 5 * 60 * 1000L; // 5 minutes is sufficient for a round trip to external IDP
    private final SlidingWindowMap<State, FederationRelayState> map;

    public FederationAuthenticationRequestTracker() {
        this.map = new SlidingWindowMap<State, FederationRelayState>(REQUEST_STATE_LIFETIME_MS);
    }

    public synchronized void add(State state, FederationRelayState relayState) {
        Validate.notNull(state, "state must not be null");
        Validate.notNull(relayState, "relay state must not be null.");
        Validate.validState(state.equals(relayState.getState()), "state and relay state does not match.");
        this.map.add(state, relayState);
    }

    public synchronized FederationRelayState remove(State state) {
        Validate.notNull(state, "state");
        return this.map.remove(state);
    }

    public synchronized FederationRelayState get(State state) {
        Validate.notNull(state, "state");
        return this.map.get(state);
    }
}
