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
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code UpdateAttributesMapDTO} class represents an attribute map change.
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = UpdateAttributesMapDTO.Builder.class)
public class UpdateAttributesMapDTO extends DTO {

    private final Map<String, String> add;
    private final Collection<String> remove;

    /**
     * Construct an {@code UpdateAttributesDTO}.
     *
     * @param name name of the attribute
     * @param friendlyName a human-readable name of the attribute
     * @param nameFormat the format of the name.
     */
    public UpdateAttributesMapDTO(Map<String, String> add, Collection<String> remove) {
        this.add = add;
        this.remove = remove;
    }

    /**
     * Get Attributes mappings to add.
     *
     * @return the name.
     */
    public Map<String, String> getAdd() {
        return this.add;
    }

    /**
     * Get attributes to remove from attribute mappings.
     *
     * @return
     */
    public Collection<String> getRemove() {
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
        private Map<String,String> add;
        private List<String> remove;

        public Builder() {
            this.add = new HashMap<String,String>();
            this.remove = new ArrayList<String>();
        }

        public Builder withAdd(Map<String,String> map) {
            this.add.putAll(map);
            return this;
        }

        public Builder withRemove(Collection<String> names) {
            this.remove.addAll(names);
            return this;
        }

        public Builder withAddMapping(String key, String value) {
            this.add.put(key, value);
            return this;
        }

        public Builder withRemoveAttribute(String name) {
            this.remove.add(name);
            return this;
        }

        public UpdateAttributesMapDTO build() {
            return new UpdateAttributesMapDTO(this.add, this.remove);
        }
    }

}
