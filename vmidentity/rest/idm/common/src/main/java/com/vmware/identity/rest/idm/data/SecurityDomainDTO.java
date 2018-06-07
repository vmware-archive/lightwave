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
 * The {@code SecurityDomainDTO} class represents an domain in one of identity sources
 * {@link IdentityProviderDTO}.
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = SecurityDomainDTO.Builder.class)
public class SecurityDomainDTO extends DTO {

    private final String name;
    private final String alias;

    /**
     * Construct an {@code SecurityDomainDTO} with a name, and alias.
     *
     * @param name name of the domain
     * @param alias alias of the domain
     */
    public SecurityDomainDTO(String name, String alias) {
        this.name = name;
        this.alias = alias;
    }

    /**
     * Get the name of the domain.
     *
     * @return the name.
     */
    public String getName() {
        return this.name;
    }

    /**
     * Get the alias name of the domain.
     *
     * @return alias of the domain.
     */
    public String getAlias() {
        return this.alias;
    }

    /**
     * Create an instance of the {@link SecurityDomainDTO.Builder} class.
     *
     * @return a new {@code SecurityDomainDTO.Builder}.
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
        private String alias;

        public Builder withName(String name) {
            this.name = name;
            return this;
        }

        public Builder withAlias(String alias) {
            this.alias = alias;
            return this;
        }

        public SecurityDomainDTO build() {
            return new SecurityDomainDTO(name, alias);
        }
    }
}
