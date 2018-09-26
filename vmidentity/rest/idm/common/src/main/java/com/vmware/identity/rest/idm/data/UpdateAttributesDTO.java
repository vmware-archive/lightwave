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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code UpdateAttributesDTO} class represents an attribute.
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = UpdateAttributesDTO.Builder.class)
public class UpdateAttributesDTO extends DTO {

    private final List<AttributeDTO> add;
    private final List<String> remove;

    /**
     * Construct an {@code UpdateAttributesDTO}.
     *
     * @param name name of the attribute
     * @param friendlyName a human-readable name of the attribute
     * @param nameFormat the format of the name.
     */
    public UpdateAttributesDTO(List<AttributeDTO> add, List<String> remove) {
        this.add = add;
        this.remove = remove;
    }

    /**
     * Get Attributes to add.
     *
     * @return the name.
     */
    public List<AttributeDTO> getAdd() {
        return this.add;
    }

    /**
     * Get attributes to remove.
     *
     * @return the human-readable name.
     */
    public List<String> getRemove() {
        return this.remove;
    }

    /**
     * Create an instance of the {@link UpdateAttributesDTO.Builder} class.
     *
     * @return a new {@code UpdateAttributesDTO.Builder}.
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
        private List<AttributeDTO> add;
        private List<String> remove;

        public Builder() {
            this.add = new ArrayList<AttributeDTO>();
            this.remove = new ArrayList<String>();
        }

        public Builder withAdd(Collection<AttributeDTO> attrs) {
            this.add.addAll(attrs);
            return this;
        }

        public Builder withRemove(Collection<String> names) {
            this.remove.addAll(names);
            return this;
        }

        public Builder withAddAttribute(AttributeDTO attr) {
            this.add.add(attr);
            return this;
        }

        public Builder withRemoveAttribute(String name) {
            this.remove.add(name);
            return this;
        }

        public UpdateAttributesDTO build() {
            return new UpdateAttributesDTO(this.add, this.remove);
        }
    }

}
