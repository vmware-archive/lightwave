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

import com.vmware.identity.openidconnect.common.TokenType;

/**
 * Token Spec
 *
 * @author Jun Sun
 */
public final class TokenSpec {

    public static final TokenSpec EMPTY = new TokenSpec(new Builder());

    private final boolean refreshTokenRequested;
    private final GroupMembershipType idTokenGroupsRequested;
    private final GroupMembershipType accessTokenGroupsRequested;
    private final List<String> resourceServers;

    private TokenSpec(Builder builder) {
        this.refreshTokenRequested = builder.refreshTokenRequested;
        this.idTokenGroupsRequested = builder.idTokenGroupsRequested;
        this.accessTokenGroupsRequested = builder.accessTokenGroupsRequested;
        this.resourceServers = builder.resourceServers;
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
     * Get id token group requested
     *
     * @return                          GroupMembershipType
     */
    public GroupMembershipType idTokenGroupsRequested() {
        return this.idTokenGroupsRequested;
    }

    /**
     * Get access token group requested
     *
     * @return                          GroupMembershipType
     */
    public GroupMembershipType accessTokenGroupsRequested() {
        return this.accessTokenGroupsRequested;
    }

    /**
     * Get resource servers
     *
     * @return                          A list of resource servers
     */
    public List<String> getResourceServers() {
        return this.resourceServers;
    }

    /**
     * Builder for TokenSpec class
     */
    public static class Builder {
        private boolean refreshTokenRequested;
        private GroupMembershipType idTokenGroupsRequested;
        private GroupMembershipType accessTokenGroupsRequested;
        private List<String> resourceServers;

        /**
         * Constructor
         */
        public Builder() {
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
         * @param idTokenGroupsRequested        GroupMembershipType
         * @return                              Builder object
         */
        public Builder idTokenGroups(GroupMembershipType idTokenGroupsRequested) {
            this.idTokenGroupsRequested = idTokenGroupsRequested;
            return this;
        }

        /**
         * Set access token group request flag
         *
         * @param accessTokenGroupsRequested    GroupMembershipType
         * @return                              Builder object
         */
        public Builder accessTokenGroups(GroupMembershipType accessTokenGroupsRequested) {
            this.accessTokenGroupsRequested = accessTokenGroupsRequested;
            return this;
        }

        /**
         * Set resource servers
         *
         * @param resourceServers                A list of resource servers
         * @return                              Builder object
         */
        public Builder resourceServers(List<String> resourceServers) {
            this.resourceServers = resourceServers;
            return this;
        }

        /**
         * Build TokenSpec object
         *
         * @return                              Builder object
         */
        public TokenSpec build() {
            return new TokenSpec(this);
        }
    }
}