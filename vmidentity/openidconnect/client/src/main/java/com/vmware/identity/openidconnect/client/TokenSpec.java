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

import java.util.List;

import org.apache.commons.lang3.Validate;

/**
 * Token Spec
 *
 * @author Jun Sun
 */
public class TokenSpec {

    private final TokenType tokenType;
    private final boolean refreshTokenRequested;
    private final boolean idTokenGroupsRequested;
    private final boolean accessTokenGroupsRequested;
    private final List<String> resouceServers;
    private final List<String> additionalScopeValues;

    private TokenSpec(Builder builder) {
        this.tokenType = builder.tokenType;
        this.refreshTokenRequested = builder.refreshTokenRequested;
        this.idTokenGroupsRequested = builder.idTokenGroupsRequested;
        this.accessTokenGroupsRequested = builder.accessTokenGroupsRequested;
        this.resouceServers = builder.resouceServers;
        this.additionalScopeValues = builder.additionalScopeValues;
    }

    /**
     * Get token type
     *
     * @return                          Token type
     */
    public TokenType getTokenType() {
        return this.tokenType;
    }

    /**
     * Get refresh token request flag
     *
     * @return                          Boolean, refresh token request flag
     */
    public boolean isRefreshTokenRequested() {
        return this.refreshTokenRequested;
    }

    /**
     * Get id token group request flag
     *
     * @return                          Boolean, id token group request flag
     */
    public boolean isIdTokenGroupsRequested() {
        return this.idTokenGroupsRequested;
    }

    /**
     * Get access token group request flag
     *
     * @return                          Boolean, access token group request flag
     */
    public boolean isAccessTokenGroupsRequested() {
        return this.accessTokenGroupsRequested;
    }

    /**
     * Get resource servers
     *
     * @return                          A list of resource servers
     */
    public List<String> getResouceServers() {
        return this.resouceServers;
    }

    /**
     * Get additional scope values
     *
     * @return                          Additional scope values
     */
    public List<String> getAdditionalScopeValues() {
        return this.additionalScopeValues;
    }

    /**
     * Builder for TokenSpec class
     */
    public static class Builder {
        private final TokenType tokenType;
        private boolean refreshTokenRequested;
        private boolean idTokenGroupsRequested;
        private boolean accessTokenGroupsRequested;
        private List<String> resouceServers;
        private List<String> additionalScopeValues;

        /**
         * Constructor
         *
         * @param tokenType                     Token type
         */
        public Builder(TokenType tokenType) {
            this.tokenType = tokenType;
        }

        /**
         * Set refresh token request flag
         *
         * @param refreshTokenRequested         Boolean, refresh token request flag
         * @return                              Builder object
         */
        public Builder refreshToken(boolean refreshTokenRequested) {
            this.refreshTokenRequested = refreshTokenRequested;
            return this;
        }

        /**
         * Set id token group request flag
         *
         * @param idTokenGroupsRequested        Boolean, id token group request flag
         * @return                              Builder object
         */
        public Builder idTokenGroups(boolean idTokenGroupsRequested) {
            this.idTokenGroupsRequested = idTokenGroupsRequested;
            return this;
        }

        /**
         * Set access token group request flag
         *
         * @param accessTokenGroupsRequested    Boolean, access token group request flag
         * @return                              Builder object
         */
        public Builder accessTokenGroups(boolean accessTokenGroupsRequested) {
            this.accessTokenGroupsRequested = accessTokenGroupsRequested;
            return this;
        }

        /**
         * Set resource servers
         *
         * @param resouceServers                A list of resource servers
         * @return                              Builder object
         */
        public Builder resouceServers(List<String> resouceServers) {
            this.resouceServers = resouceServers;
            return this;
        }

        /**
         * Set additional scope values
         *
         * @param additionalScopeValues         additional scope values
         * @return                              Builder object
         */
        public Builder additionalScopeValues(List<String> additionalScopeValues) {
            this.additionalScopeValues = additionalScopeValues;
            return this;
        }

        /**
         * Build TokenSpec object
         *
         * @return                              Builder object
         */
        public TokenSpec build() {
            Validate.notNull(this.tokenType, "tokenType");

            return new TokenSpec(this);
        }
    }
}
