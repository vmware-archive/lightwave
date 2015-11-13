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
 * The {@code BrandPolicyDTO} class represents the per-tenant branding details such as
 * the brand name and logon banner.
 * 
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = BrandPolicyDTO.Builder.class)
public class BrandPolicyDTO extends DTO {

    private final String name;
    private final String logonBannerTitle;
    private final String logonBannerContent;
    private final Boolean logonBannerCheckboxEnabled;
    private final Boolean logonBannerDisabled;

    /**
     * Construct a {@code BrandPolicyDTO} with a brand name, and logon banner details.
     *
     * @param name the brand name.
     * @param logonBannerTitle the logon banner title.
     * @param logonBannerContent the content of the logon banner.
     * @param logonBannerCheckboxEnabled enable or disable the checkbox on the logon banner.
     * @param logonBannerDisabled disable or enable the logon banner.
     */
    public BrandPolicyDTO(String name, String logonBannerTitle, String logonBannerContent,
            Boolean logonBannerCheckboxEnabled, Boolean logonBannerDisabled) {
        this.name = name;
        this.logonBannerTitle = logonBannerTitle; // can be null or empty
        this.logonBannerContent = logonBannerContent; // can be null or empty
        this.logonBannerCheckboxEnabled = logonBannerCheckboxEnabled;
        this.logonBannerDisabled = logonBannerDisabled;
    }

    /**
     * Get the brand name.
     *
     * @return the brand name.
     */
    public String getName() {
        return name;
    }

    /**
     * Get the logon banner's title.
     *
     * @return the logon banner's title.
     */
    public String getLogonBannerTitle() {
        return logonBannerTitle;
    }

    /**
     * Get the logon banner's content.
     *
     * @return the logon banner's content.
     */
    public String getLogonBannerContent() {
        return logonBannerContent;
    }

    /**
     * Check if the logon banner checkbox is enabled.
     *
     * @return {@code true} if the logon banner checkbox is enabled,
     * {@code false} if it is disabled.
     */
    public Boolean isLogonBannerCheckboxEnabled() {
        return logonBannerCheckboxEnabled;
    }

    /**
     * Check if the logon banner is disabled.
     *
     * @return {@code true} if the logon banner is disabled, {@code false} if it is enabled.
     *
     */
    public Boolean isLogonBannerDisabled() {
        return logonBannerDisabled;
    }

    /**
     * Create an instance of the {@link BrandPolicyDTO.Builder} class.
     *
     * @return a new {@code BrandPolicyDTO.Builder}.
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
        private String name;
        private String logonBannerTitle;
        private String logonBannerContent;
        private Boolean logonBannerCheckboxEnabled;
        private Boolean logonBannerDisabled;

        public Builder withName(String name) {
            this.name = name;
            return this;
        }

        public Builder withLogonBannerTitle(String logonBannerTitle) {
            this.logonBannerTitle = logonBannerTitle;
            return this;
        }

        public Builder withLogonBannerContent(String logonBannerContent) {
            this.logonBannerContent = logonBannerContent;
            return this;
        }

        public Builder withLogonBannerCheckboxEnabled(Boolean logonBannerCheckboxEnabled) {
            this.logonBannerCheckboxEnabled = logonBannerCheckboxEnabled;
            return this;
        }

        public Builder withLogonBannerDisabled(Boolean logonBannerDisabled) {
            this.logonBannerDisabled = logonBannerDisabled;
            return this;
        }

        public BrandPolicyDTO build() {
            return new BrandPolicyDTO(name, logonBannerTitle, logonBannerContent, logonBannerCheckboxEnabled, logonBannerDisabled);
        }
    }

}
