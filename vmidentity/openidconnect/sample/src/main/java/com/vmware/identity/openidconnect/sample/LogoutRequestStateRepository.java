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

import com.vmware.identity.openidconnect.client.State;

/**
 * @author Yehia Zayour
 */
public class LogoutRequestStateRepository {
    private final ConcurrentHashMap<State, String> map;

    public LogoutRequestStateRepository() {
        this.map = new ConcurrentHashMap<State, String>();
    }

    public void add(State state, String userName) {
        this.map.put(state, userName);
    }

    public String getUserName(State state) {
        return this.map.get(state);
    }

    public void remove(State state) {
        this.map.remove(state);
    }

    public boolean contains(State state) {
        return this.map.containsKey(state);
    }
}
