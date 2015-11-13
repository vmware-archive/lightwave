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
 * The {@code LockoutPolicyDTO} class describes the lockout policy configuration for a tenant.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = LockoutPolicyDTO.Builder.class)
public class LockoutPolicyDTO extends DTO {

    private final String description;
    private final Long failedAttemptIntervalSec;
    private final Integer maxFailedAttempts;
    private final Long autoUnlockIntervalSec;

    /**
     * Construct a {@code LockoutPolicyDTO} with its various details.
     *
     * @param description a description of the lockout policy.
     * @param failedAttemptIntervalSec the time window, in seconds, during which repeated
     *  login failures may trigger a lockout.
     * @param maxFailedAttempts the maximum number of failed login attempts allowed before
     *  lockout occurs.
     * @param autoUnlockIntervalSec the amount of time, in seconds, that the account will
     *  remain locked. A value of 0 (zero) will mean the account can only be unlocked by
     *  way of an administrator.
     */
    public LockoutPolicyDTO(String description, Long failedAttemptIntervalSec, Integer maxFailedAttempts, Long autoUnlockIntervalSec) {
        this.description = description;
        this.failedAttemptIntervalSec = failedAttemptIntervalSec;
        this.maxFailedAttempts = maxFailedAttempts;
        this.autoUnlockIntervalSec = autoUnlockIntervalSec;
    }

    /**
     * Get the detailed description of the lockout policy.
     *
     * @return the description for the lockout policy.
     */
    public String getDescription() {
        return description;
    }

    /**
     * Get the time window, in seconds, during which repeated login failures may
     * trigger a lockout.
     *
     * @return a positive number indicating the failed attempt interval.
     */
    public Long getFailedAttemptIntervalSec() {
        return failedAttemptIntervalSec;
    }

    /**
     * Get the maximum number of failed login attempts allowed before lockout occurs.
     *
     * @return a positive number indicating the maximum number of failed login attempts.
     */
    public Integer getMaxFailedAttempts() {
        return maxFailedAttempts;
    }

    /**
     * Get the amount of time, in seconds, that an account will remain locked out.
     * A return value of 0 (zero), indicates that the account can only be unlocked
     * by way of an administrator.
     *
     * @return a non-negative number indicating the span of time for which an account
     *  will remain locked.
     */
    public Long getAutoUnlockIntervalSec() {
        return autoUnlockIntervalSec;
    }

    /**
     * Creates an instance of the {@link LockoutPolicyDTO.Builder} class.
     *
     * @return a new {@code LockoutPolicyDTO.Builder}.
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

        private String description;
        private Long failedAttemptIntervalSec;
        private Integer maxFailedAttempts;
        private Long autoUnlockIntervalSec;

        public Builder withDescription(String description) {
            this.description = description;
            return this;
        }

        public Builder withFailedAttemptIntervalSec(Long failedAttemptIntervalSec) {
            this.failedAttemptIntervalSec = failedAttemptIntervalSec;
            return this;
        }

        public Builder withMaxFailedAttempts(Integer maxFailedAttempts) {
            this.maxFailedAttempts = maxFailedAttempts;
            return this;
        }

        public Builder withAutoUnlockIntervalSec(Long autoUnlockIntervalSec) {
            this.autoUnlockIntervalSec = autoUnlockIntervalSec;
            return this;
        }

        public LockoutPolicyDTO build() {
            return new LockoutPolicyDTO(description, failedAttemptIntervalSec, maxFailedAttempts, autoUnlockIntervalSec);
        }
    }

}
