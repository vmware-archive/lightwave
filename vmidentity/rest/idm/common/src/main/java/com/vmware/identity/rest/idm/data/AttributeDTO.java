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
 * The {@code AttributeDTO} class represents an attribute in a
 * {@link AttributeConsumerServiceDTO}.
 * 
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = AttributeDTO.Builder.class)
public class AttributeDTO extends DTO {

    private final String name;
    private final String friendlyName;
    private final String nameFormat;

    /**
     * Construct an {@code AttributeDTO} with a name, friendly name, and format.
     *
     * @param name name of the attribute
     * @param friendlyName a human-readable name of the attribute
     * @param nameFormat the format of the name.
     */
    public AttributeDTO(String name, String friendlyName, String nameFormat) {
        this.name = name;
        this.friendlyName = friendlyName;
        this.nameFormat = nameFormat;
    }

    /**
     * Get the name of the attribute.
     *
     * @return the name.
     */
    public String getName() {
        return name;
    }

    /**
     * Get the human-readable name of the attribute.
     *
     * @return the human-readable name.
     */
    public String getFriendlyName() {
        return friendlyName;
    }

    /**
     * Get the name format of the attribute.
     *
     * @return the name format.
     */
    public String getNameFormat() {
        return nameFormat;
    }

    /**
     * Create an instance of the {@link AttributeDTO.Builder} class.
     *
     * @return a new {@code AttributeDTO.Builder}.
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
        private String friendlyName;
        private String nameFormat;

        public Builder withName(String name) {
            this.name = name;
            return this;
        }

        public Builder withFriendlyName(String friendlyName) {
            this.friendlyName = friendlyName;
            return this;
        }

        public Builder withNameFormat(String nameFormat) {
            this.nameFormat = nameFormat;
            return this;
        }

        public AttributeDTO build() {
            return new AttributeDTO(name, friendlyName, nameFormat);
        }
    }

}
