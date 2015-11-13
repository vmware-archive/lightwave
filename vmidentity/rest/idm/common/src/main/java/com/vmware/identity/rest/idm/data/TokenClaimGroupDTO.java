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

import java.util.List;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code TokenClaimGroupDTO} contains the information that makes up a token claim
 * group mapping for an external identity provider.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonDeserialize(builder=TokenClaimGroupDTO.Builder.class)
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
public class TokenClaimGroupDTO extends DTO {

    private final String claimName;
    private final String claimValue;
    private final List<String> groups;

    /**
     * Construct a {@code TokenClaimGroupDTO} with a name, value, and list of groups.
     *
     * @param claimName the name of the claim.
     * @param claimValue the value of the claim.
     * @param groups the list of groups associated with the claim group.
     */
    public TokenClaimGroupDTO(String claimName, String claimValue, List<String> groups) {
        this.claimName = claimName;
        this.claimValue = claimValue;
        this.groups = groups;
    }

    /**
     * Get the name of the claim.
     *
     * @return the name of the claim.
     */
    public String getClaimName() {
        return this.claimName;
    }

    /**
     * Get the value of the claim.
     *
     * @return the value of the claim.
     */
    public String getClaimValue() {
        return this.claimValue;
    }

    /**
     * Get the list of groups associated with the claim group.
     *
     * @return a list of groups.
     */
    public List<String> getGroups() {
        return this.groups;
    }

    /**
     * Creates an instance of the {@link TokenClaimGroupDTO.Builder} class.
     *
     * @return a new {@code TokenClaimGroupDTO.Builder}.
     */
    public static Builder builder() {
        return new TokenClaimGroupDTO.Builder();
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
        private String claimName;
        private String claimValue;
        private List<String> groups;

        public Builder withClaimName(String claimName) {
            this.claimName = claimName;
            return this;
        }

        public Builder withClaimValue(String claimValue) {
            this.claimValue = claimValue;
            return this;
        }

        public Builder withGroups(List<String> groups) {
            this.groups = groups;
            return this;
        }

        public TokenClaimGroupDTO build() {
            return new TokenClaimGroupDTO(claimName, claimValue, groups);
        }
    }

}
