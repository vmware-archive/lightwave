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
package com.vmware.identity.rest.idm.data;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code TokenPolicyDTO} class contains the details that describe a tenant's
 * token policy.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = TokenPolicyDTO.Builder.class)
public class TokenPolicyDTO extends DTO {

    private final Long clockToleranceMillis;
    private final Integer delegationCount;
    private final Long maxBearerTokenLifeTimeMillis;
    private final Long maxHOKTokenLifeTimeMillis;
    private final Long maxBearerRefreshTokenLifeTimeMillis;
    private final Long maxHOKRefreshTokenLifeTimeMillis;
    private final Integer renewCount;

    /**
     * Construct a {@code TokenPolicyDTO} with its various fields.
     *
     * @param clockToleranceMillis the clock tolerance of a token, in milliseconds. The clock
     *  tolerance is the time difference between the token issuer and the token consumer.
     * @param delegationCount the maximum number of times a token issued by the tenant
     *  may be delegated.
     * @param maxBearerTokenLifeTimeMillis the maximum lifetime, in milliseconds, of a
     *  bearer token issued by the tenant.
     * @param maxHOKTokenLifeTimeMillis the maximum lifetime, in milliseconds, of a
     *  holder-of-key token issued by the tenant.
     * @param maxBearerRefreshTokenLifeTimeMillis the maximum lifetime, in milliseconds,
     *  of a bearer refresh token issued by the tenant.
     * @param maxHOKRefreshTokenLifeTimeMillis the maximum lifetime, in milliseconds,
     *  of a holder-of-key refresh token issued by the tenant.
     * @param renewCount the maximum number of times a token issued by the tenant may be renewed.
     */
    public TokenPolicyDTO(Long clockToleranceMillis, Integer delegationCount,
            Long maxBearerTokenLifeTimeMillis, Long maxHOKTokenLifeTimeMillis,
            Long maxBearerRefreshTokenLifeTimeMillis, Long maxHOKRefreshTokenLifeTimeMillis,
            Integer renewCount) {
        this.clockToleranceMillis = clockToleranceMillis;
        this.delegationCount = delegationCount;
        this.maxBearerTokenLifeTimeMillis = maxBearerTokenLifeTimeMillis;
        this.maxHOKTokenLifeTimeMillis = maxHOKTokenLifeTimeMillis;
        this.maxBearerRefreshTokenLifeTimeMillis = maxBearerRefreshTokenLifeTimeMillis;
        this.maxHOKRefreshTokenLifeTimeMillis = maxHOKRefreshTokenLifeTimeMillis;
        this.renewCount = renewCount;
    }

    /**
     * Get the clock tolerance of a token. The clock tolerance is the time difference between the
     * token issuer and the token consumer.
     *
     * @return the clock tolerance in milliseconds.
     */
    public Long getClockToleranceMillis() {
        return clockToleranceMillis;
    }

    /**
     * Get the maximum number of times a token issued by the tenant may be delegated.
     *
     * @return the delegation count.
     */
    public Integer getDelegationCount() {
        return delegationCount;
    }

    /**
     * Get the maximum lifetime, in milliseconds, of a bearer token issued by the tenant.
     *
     * @return the maximum lifetime of a bearer token in milliseconds.
     */
    public Long getMaxBearerTokenLifeTimeMillis() {
        return maxBearerTokenLifeTimeMillis;
    }

    /**
     * Get the maximum lifetime, in milliseconds, of a holder-of-key token issued by the tenant.
     *
     * @return the maximum lifetime of a holder-of-key token in milliseconds.
     */
    public Long getMaxHOKTokenLifeTimeMillis() {
        return maxHOKTokenLifeTimeMillis;
    }

    /**
     * Get the maximum lifetime, in milliseconds, of a bearer refresh token issued by the tenant.
     *
     * @return the maximum lifetime of a bearer refresh token in milliseconds
     */
    public Long getMaxBearerRefreshTokenLifeTimeMillis() {
        return maxBearerRefreshTokenLifeTimeMillis;
    }

    /**
     * Get the maximum lifetime, in milliseconds, of a holder-of-key refresh token issued by
     * the tenant.
     *
     * @return the maximum lifetime of a holder-of-key refresh token in milliseconds.
     */
    public Long getMaxHOKRefreshTokenLifeTimeMillis() {
        return maxHOKRefreshTokenLifeTimeMillis;
    }

    /**
     * Get the maximum number of times a token issued by the tenant may be renewed.
     *
     * @return the maximum number of times a token issued by the tenant may be renewed.
     */
    public Integer getRenewCount() {
        return renewCount;
    }

    /**
     * Creates an instance of the {@link TokenPolicyDTO.Builder} class.
     *
     * @return a new {@code TokenPolicyDTO.Builder}.
     */
    public static Builder builder() {
        return new Builder();
    }

    /**
     * The JSON POJO Builder for this class. The builder class is meant mostly for
     * usage when constructing the object from its JSON string and thus only accepts
     * content for the canonical fields of the JSON representation. Other constructors
     * may exist that provide greater convenience.
     */
    @JsonIgnoreProperties(ignoreUnknown=true)
    @JsonPOJOBuilder
    public static class Builder {

        private Long clockToleranceMillis;
        private Integer delegationCount;
        private Long maxBearerTokenLifeTimeMillis;
        private Long maxHOKTokenLifeTimeMillis;
        private Long maxBearerRefreshTokenLifeTimeMillis;
        private Long maxHOKRefreshTokenLifeTimeMillis;
        private Integer renewCount;

        public Builder withClockToleranceMillis(Long clockToleranceMillis) {
            this.clockToleranceMillis = clockToleranceMillis;
            return this;
        }

        public Builder withDelegationCount(Integer delegationCount) {
            this.delegationCount = delegationCount;
            return this;
        }

        public Builder withMaxBearerTokenLifeTimeMillis(Long maxBearerTokenLifeTimeMillis) {
            this.maxBearerTokenLifeTimeMillis = maxBearerTokenLifeTimeMillis;
            return this;
        }

        public Builder withMaxHOKTokenLifeTimeMillis(Long maxHOKTokenLifeTimeMillis) {
            this.maxHOKTokenLifeTimeMillis = maxHOKTokenLifeTimeMillis;
            return this;
        }

        public Builder withMaxBearerRefreshTokenLifeTimeMillis(Long maxBearerRefreshTokenLifetimeMillis) {
            this.maxBearerRefreshTokenLifeTimeMillis = maxBearerRefreshTokenLifetimeMillis;
            return this;
        }

        public Builder withMaxHOKRefreshTokenLifeTimeMillis(Long maxHOKRefreshTokenLifetimeMillis) {
            this.maxHOKRefreshTokenLifeTimeMillis = maxHOKRefreshTokenLifetimeMillis;
            return this;
        }

        public Builder withRenewCount(Integer renewCount) {
            this.renewCount = renewCount;
            return this;
        }

        public TokenPolicyDTO build() {
            return new TokenPolicyDTO(
                    clockToleranceMillis,
                    delegationCount,
                    maxBearerTokenLifeTimeMillis,
                    maxHOKTokenLifeTimeMillis,
                    maxBearerRefreshTokenLifeTimeMillis,
                    maxHOKRefreshTokenLifeTimeMillis,
                    renewCount);
        }
    }

}
