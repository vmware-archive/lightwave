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

import com.vmware.identity.openidconnect.common.Nonce;
import com.vmware.identity.openidconnect.common.State;

/**
 * @author Yehia Zayour
 */
public class AuthenticationRequestTracker {
    private final ConcurrentHashMap<State, Nonce> map;

    public AuthenticationRequestTracker() {
        this.map = new ConcurrentHashMap<State, Nonce>();
    }

    public void add(State state, Nonce nonce) {
        Validate.notNull(state, "state");
        Validate.notNull(nonce, "nonce");
        if (this.map.contains(state)) {
            throw new IllegalArgumentException("already has this state: " + state);
        }
        this.map.put(state, nonce);
    }

    public Nonce remove(State state) {
        Validate.notNull(state, "state");
        return this.map.remove(state);
    }

    public Nonce get(State state) {
        Validate.notNull(state, "state");
        return this.map.get(state);
    }
}