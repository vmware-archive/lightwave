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
 * The {@code AssertionConsumerServiceDTO} class represents the information of an
 * assertion consumer in a {@link RelyingPartyDTO}.
 * 
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = AssertionConsumerServiceDTO.Builder.class)
public class AssertionConsumerServiceDTO extends DTO {

    private final String name;
    private final String endpoint;
    private final String binding;
    public final Integer index;

    /**
     * Construct an {@code AssertionConsumerServiceDTO} with a name, endpoint, binding, and index.
     *
     * @param name name of the service.
     * @param endpoint endpoint where the service is registered.
     * @param binding the SAML binding.
     * @param index the SAML index of this service.
     */
    public AssertionConsumerServiceDTO(String name, String endpoint, String binding, Integer index) {
        this.name = name;
        this.endpoint = endpoint;
        this.binding = binding;
        this.index = index;
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
     * Get the endpoint of the service.
     *
     * @return the endpoint.
     */
    public String getEndpoint() {
        return endpoint;
    }

    /**
     * Get the SAML binding of the service.
     *
     * @return the binding.
     */
    public String getBinding() {
        return binding;
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
     * Create an instance of the {@link AssertionConsumerServiceDTO.Builder}
     * class.
     *
     * @return a new {@code AssertionConsumerServiceDTO.Builder}.
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
        private String endpoint;
        private String binding;
        public Integer index;

        public Builder withName(String name) {
            this.name = name;
            return this;
        }

        public Builder withEndpoint(String endpoint) {
            this.endpoint = endpoint;
            return this;
        }

        public Builder withBinding(String binding) {
            this.binding = binding;
            return this;
        }

        public Builder withIndex(Integer index) {
            this.index = index;
            return this;
        }

        public AssertionConsumerServiceDTO build() {
            return new AssertionConsumerServiceDTO(name, endpoint, binding, index);
        }
    }

}
