package com.vmware.directory.rest.common.data;

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

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code TenantConfigurationDTO} class contains a number of configuration policies for a
 * given tenant.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = TenantConfigurationDTO.Builder.class)
public class TenantConfigurationDTO extends DTO {

    private final PasswordPolicyDTO passwordPolicy;
    private final LockoutPolicyDTO lockoutPolicy;

    /**
     * Construct a {@code TenantConfigurationDTO} with its various policies.
     *
     * @param passwordPolicy the password policy for a tenant.
     * @param lockoutPolicy the lockout policy for a tenant.
     * @param tokenPolicy the token policy for a tenant.
     * @param providerPolicy the provider policy for a tenant.
     * @param brandPolicy the brand policy for a tenant.
     * @param authenticationPolicy the authentication policy for a tenant.
     */
    public TenantConfigurationDTO(PasswordPolicyDTO passwordPolicy, LockoutPolicyDTO lockoutPolicy) {
        this.passwordPolicy = passwordPolicy;
        this.lockoutPolicy = lockoutPolicy;
    }

    /**
     * Get the password policy.
     *
     * @return the password policy.
     */
    public PasswordPolicyDTO getPasswordPolicy() {
        return passwordPolicy;
    }

    /**
     * Get the lockout policy.
     *
     * @return the lockout policy.
     */
    public LockoutPolicyDTO getLockoutPolicy() {
        return lockoutPolicy;
    }

    /**
     * Create an instance of the {@link TenantConfigurationDTO.Builder} class.
     *
     * @return a new {@code TenantConfigurationDTO.Builder}.
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

        private PasswordPolicyDTO passwordPolicy;
        private LockoutPolicyDTO lockoutPolicy;

        public Builder withPasswordPolicy(PasswordPolicyDTO passwordPolicy) {
            this.passwordPolicy = passwordPolicy;
            return this;
        }

        public Builder withLockoutPolicy(LockoutPolicyDTO lockoutPolicy) {
            this.lockoutPolicy = lockoutPolicy;
            return this;
        }

        public TenantConfigurationDTO build() {
            return new TenantConfigurationDTO(passwordPolicy, lockoutPolicy);
        }
    }

}

