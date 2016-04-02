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

import java.util.Set;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code ResourceServerDTO} class contains the details of a Resource Server
 *
 * @author Yehia Zayour
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = ResourceServerDTO.Builder.class)
public class ResourceServerDTO extends DTO {

    private final String name;
    private final Set<String> groupFilter;

    /**
     * Constructs an {@code ResourceServerDTO} with a name and group filter.
     *
     * @param name identifier for the resource server.
     * @param groupFilter group filter for the resource server.
     */
    public ResourceServerDTO(String name, Set<String> groupFilter) {
        this.name = name;
        this.groupFilter = groupFilter;
    }

    /**
     * Get the name for the resource server.
     *
     * @return the name for the resource server.
     */
    public String getName() {
        return this.name;
    }

    /**
     * Get the group filter for the resource server.
     *
     * @return the group filter for the resource server.
     */
    public Set<String> getGroupFilter() {
        return this.groupFilter;
    }

    /**
     * Creates an instance of the {@link ResourceServerDTO.Builder} class.
     *
     * @return a new {@code ResourceServerDTO.Builder}.
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
        private Set<String> groupFilter;

        public Builder withName(String name) {
            this.name = name;
            return this;
        }

        public Builder withGroupFilter(Set<String> groupFilter) {
            this.groupFilter = groupFilter;
            return this;
        }

        public ResourceServerDTO build() {
            return new ResourceServerDTO(name, groupFilter);
        }
    }
}
