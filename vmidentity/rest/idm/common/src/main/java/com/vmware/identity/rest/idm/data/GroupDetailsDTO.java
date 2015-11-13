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
 * The {@code GroupDetailsDTO} class contains detailed information about a {@link GroupDTO}.
 * 
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = GroupDetailsDTO.Builder.class)
public class GroupDetailsDTO extends DTO {

    private final String description;

    /**
     * Construct a {@code GroupDTO} with a description.
     *
     * @param description the description of the group.
     */
    public GroupDetailsDTO(String description) {
        this.description = description;
    }

    /**
     * Get the description of the group.
     *
     * @return the description of the group.
     */
    public String getDescription() {
        return this.description;
    }

    /**
     * Creates an instance of the {@link GroupDetailsDTO.Builder} class.
     *
     * @return a new {@code GroupDetailsDTO.Builder}.
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

        public Builder withDescription(String description) {
            this.description = description;
            return this;
        }

        public GroupDetailsDTO build() {
            return new GroupDetailsDTO(description);
        }
    }

}
