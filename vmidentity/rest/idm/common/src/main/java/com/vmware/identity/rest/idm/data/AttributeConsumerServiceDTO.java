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

import java.util.Collection;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code AttributeConsumerServiceDTO} class represents the information of an
 * attribute consumer in a {@link RelyingPartyDTO}.
 * 
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Halls
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = AttributeConsumerServiceDTO.Builder.class)
public class AttributeConsumerServiceDTO extends DTO {

    private final String name;
    private final Integer index;
    private final Collection<AttributeDTO> attributes;

    /**
     * Construct an {@code AttributeConsumerServiceDTO} with a name, index,
     * and collection of attributes.
     *
     * @param name name of the service.
     * @param index the SAML index of this service.
     * @param attributes the collection of attributes.
     */
    public AttributeConsumerServiceDTO(String name, Integer index, Collection<AttributeDTO> attributes) {
        this.name = name;
        this.index = index;
        this.attributes = attributes;
    }

    /**
     * Get the name of the service.
     *
     * @return the name.
     */
    public String getName() {
        return name;
    }

    /**
     * Get the SAML index of the service.
     *
     * @return the index.
     */
    public Integer getIndex() {
        return index;
    }

    /**
     * Get the collection of attributes for the service.
     *
     * @return the attributes.
     */
    public Collection<AttributeDTO> getAttributes() {
        return attributes;
    }

    /**
     * Create an instance of the {@link AttributeConsumerServiceDTO.Builder}
     * class.
     *
     * @return a new {@code AttributeConsumerServiceDTO.Builder}.
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
        private Integer index;
        private Collection<AttributeDTO> attributes;

        public Builder withName(String name) {
            this.name = name;
            return this;
        }

        public Builder withIndex(Integer index) {
            this.index = index;
            return this;
        }

        public Builder withAttributes(Collection<AttributeDTO> attributes) {
            this.attributes = attributes;
            return this;
        }

        public AttributeConsumerServiceDTO build() {
            return new AttributeConsumerServiceDTO(name, index, attributes);
        }
    }

}
