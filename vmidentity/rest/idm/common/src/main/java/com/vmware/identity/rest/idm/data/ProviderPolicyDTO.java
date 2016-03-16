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
 * The {@code ProviderPolicyDTO} class describes the provider policy configuration for a tenant.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = ProviderPolicyDTO.Builder.class)
public class ProviderPolicyDTO extends DTO {

    private final String defaultProvider;
    private final String defaultProviderAlias;
    private final Boolean providerSelectionEnabled;

    /**
     * Construct a {@code ProviderPolicyDTO} with a default provider.
     *
     * @param defaultProvider the default provider used by the tenant.
     * @param defaultProviderAlias the default provider alias of the tenant.
     * @param providerSelectionEnabled true/false if idp selection for a tenant to be enabled/disabled.
     */
    public ProviderPolicyDTO(String defaultProvider, String defaultProviderAlias, Boolean providerSelectionEnabled) {
        this.defaultProvider = defaultProvider;
        this.defaultProviderAlias = defaultProviderAlias;
        this.providerSelectionEnabled = providerSelectionEnabled;
    }

    /**
     * Get the default provider for the tenant.
     *
     * @return the name of the default provider.
     */
    public String getDefaultProvider() {
        return defaultProvider;
    }

    /**
     * Get the default provider alias of the tenant.
     *
     * @return the default provider alias of the tenant.
     */
    public String getDefaultProviderAlias() {
        return defaultProviderAlias;
    }

    /**
     * Check if idp selection is enabled.
     *
     * @return true/false if idp selection is enabled/disabled.
     */
    public Boolean isProviderSelectionEnabled() {
        return providerSelectionEnabled;
    }

    /**
     * Creates an instance of the {@link ProviderPolicyDTO.Builder} class.
     *
     * @return a new {@code ProviderPolicyDTO.Builder}.
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
        private String defaultProvider;
        private String defaultProviderAlias;
        private Boolean providerSelectionEnabled;

        public Builder withDefaultProvider(String defaultProvider) {
            this.defaultProvider = defaultProvider;
            return this;
        }

        public Builder withDefaultProviderAlias(String defaultProviderAlias) {
            this.defaultProviderAlias = defaultProviderAlias;
            return this;
        }

        public Builder withProviderSelectionEnabled(Boolean providerSelectionEnabled) {
            this.providerSelectionEnabled = providerSelectionEnabled;
            return this;
        }

        public ProviderPolicyDTO build() {
            return new ProviderPolicyDTO(defaultProvider, defaultProviderAlias, providerSelectionEnabled);
        }
    }

}
