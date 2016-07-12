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
package com.vmware.directory.rest.common.data;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code PrincipalDTO} class describes the base details of a principal
 * (e.g. a user or group).
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder=PrincipalDTO.Builder.class)
public class PrincipalDTO extends DTO {

    private final String name;
    private final String domain;

    /**
     * Construct a {@code PrincipalDTO} with a name and domain.
     *
     * @param name name of the principal.
     * @param domain domain of the principal.
     */
    public PrincipalDTO(String name, String domain) {
        this.name = name;
        this.domain = domain;
    }

    /**
     * Get the principal's name.
     *
     * @return the principal's name.
     */
    public String getName() {
        return name;
    }

    /**
     * Get the principal's domain.
     *
     * @return the principal's domain.
     */
    public String getDomain() {
        return domain;
    }

    /**
     * Creates an instance of the {@link PrincipalDTO.Builder} class.
     *
     * @return a new {@code PrincipalDTO.Builder}.
     */
    public static Builder builder() {
        return new Builder();
    }

    public boolean equals(Object object) {
        if (object == this) {
            return true;
        }

        if (object == null) {
            return false;
        }

        if (!(object instanceof PrincipalDTO)) {
            return false;
        }

        PrincipalDTO principal = (PrincipalDTO) object;

        boolean nameEquals = true;
        if (name != null && principal.getName() != null) {
            nameEquals = name.equals(principal.getName());
        }

        boolean domainEquals = true;
        if (nameEquals && domain != null && principal.getDomain() != null) {
            domainEquals = domain.equals(principal.getDomain());
        }

        return nameEquals && domainEquals;
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
        private String domain;

        public Builder withName(String name) {
            this.name = name;
            return this;
        }

        public Builder withDomain(String domain) {
            this.domain = domain;
            return this;
        }

        public PrincipalDTO build() {
            return new PrincipalDTO(name, domain);
        }
    }

}