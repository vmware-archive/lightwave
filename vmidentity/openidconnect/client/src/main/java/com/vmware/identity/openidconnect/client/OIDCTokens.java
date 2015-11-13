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

/**
 * OIDC Tokens, contain all tokens
 *
 * @author Jun Sun
 */
public class OIDCTokens {

    private final AccessToken accessToken;
    private final IDToken idToken;
    private final RefreshToken refreshToken;

    /**
     * Constructor
     *
     * @param accessToken       access token
     * @param idToken           id token
     * @param refreshToken      refresh token
     */
    OIDCTokens(AccessToken accessToken, IDToken idToken, RefreshToken refreshToken) {
        Validate.notNull(idToken, "idToken");

        this.accessToken = accessToken;
        this.idToken = idToken;
        this.refreshToken = refreshToken;
    }

    /**
     * Get access token
     *
     * @return                  access token
     */
    public AccessToken getAccessToken() {
        return this.accessToken;
    }

    /**
     * Get id token
     *
     * @return                  id token
     */
    public IDToken getIdToken() {
        return this.idToken;
    }

    /**
     * Get refresh token
     *
     * @return                  refresh token
     */
    public RefreshToken getRefreshToken() {
        return this.refreshToken;
    }
}
