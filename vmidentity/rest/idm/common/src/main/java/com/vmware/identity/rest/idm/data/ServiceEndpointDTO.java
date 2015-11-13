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
 * The {@code ServiceEndpointDTO} represents an endpoint used for
 * external identity provider services (e.g. single-sign-on or single-log-off).
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonDeserialize(builder=ServiceEndpointDTO.Builder.class)
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
public class ServiceEndpointDTO extends DTO {

    private final String name;
    private final String endpoint;
    private final String binding;

    /**
     * Construct a {@code ServiceEndpointDTO} with a name, endpoint, and binding type.
     *
     * @param name name of the service endpoint.
     * @param endpoint the uri of the service endpoint.
     * @param binding the HTTP binding for the service endpoint.
     */
    public ServiceEndpointDTO(String name, String endpoint, String binding) {
        this.name = name;
        this.endpoint = endpoint;
        this.binding = binding;
    }

    /**
     * Get the name of the service.
     *
     * @return the name of the service.
     */
    public String getName() {
        return this.name;
    }

    /**
     * Get the endpoint of the service.
     *
     * @return the endpoint for this service.
     */
    public String getEndpoint() {
        return this.endpoint;
    }

    /**
     * Get the HTTP binding for the service.
     *
     * @return a string representing the HTTP binding of the service.
     */
    public String getBinding() {
        return this.binding;
    }

    /**
     * Creates an instance of the {@link ServiceEndpointDTO.Builder} class.
     *
     * @return a new {@code ServiceEndpointDTO.Builder}.
     */
    public static Builder builder() {
        return new ServiceEndpointDTO.Builder();
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

        public ServiceEndpointDTO build() {
            return new ServiceEndpointDTO(name, endpoint, binding);
        }
    }

}
