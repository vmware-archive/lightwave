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

package com.vmware.identity.openidconnect.client;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.State;

/**
 * @author Jun Sun
 * @author Yehia Zayour
 */
public final class AuthenticationTokensResponse {
    private final State state;
    private final OIDCTokens tokens;

    AuthenticationTokensResponse(State state, OIDCTokens tokens) {
        Validate.notNull(state, "state");
        Validate.notNull(tokens, "tokens");
        this.state = state;
        this.tokens = tokens;
    }

    public State getState() {
        return this.state;
    }

    public OIDCTokens getTokens() {
        return this.tokens;
    }
}