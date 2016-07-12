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
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code SolutionUserDTO} class contains the information that makes up a Solution User.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = SolutionUserDTO.Builder.class)
public class SolutionUserDTO extends DTO {

    private final String name;
    private final String domain;
    private final String description;
    private final PrincipalDTO alias;
    private final CertificateDTO certificate;
    private final Boolean disabled;
    private final String objectId;

    /**
     * Construct a {@code SolutionUserDTO} with its various fields.
     *
     * @param name the name of the solution user.
     * @param domain the domain of the solution user.
     * @param description the description of the solution user.
     * @param alias an optional alias for the solution user.
     * @param certificate the certificate associated with the solution user.
     * @param disabled a flag indicating whether the solution user is disabled or not.
     * @param objectId the object identifier for the solution user.
     */
    public SolutionUserDTO(String name, String domain, String description, PrincipalDTO alias, CertificateDTO certificate, Boolean disabled, String objectId) {
        this.name = name;
        this.domain = domain;
        this.description = description;
        this.alias = alias;
        this.certificate = certificate;
        this.disabled = disabled;
        this.objectId = objectId;
    }

    /**
     * Get the solution user's name.
     *
     * @return the solution user's name.
     */
    public String getName() {
        return name;
    }

    /**
     * Get the solution user's domain.
     *
     * @return the solution user's domain.
     */
    public String getDomain() {
        return domain;
    }

    /**
     * Get the solution user's description.
     *
     * @return the solution user's description.
     */
    public String getDescription() {
        return description;
    }

    /**
     * Get the solution user's alias.
     *
     * @return the solution user's alias.
     */
    public PrincipalDTO getAlias() {
        return alias;
    }

    /**
     * Get the solution user's certificate.
     *
     * @return the solution user's certificate.
     */
    public CertificateDTO getCertificate() {
        return certificate;
    }

    /**
     * Check if the solution user is disabled or not.
     *
     * @return {@code true} if the solution user is disabled, {@code false} otherwise.
     */
    public Boolean isDisabled() {
        return disabled;
    }

    /**
     * Get the solution user's object identifier.
     *
     * @return the solution user's object identifier.
     */
    public String getObjectId() {
        return objectId;
    }

    /**
     * Creates an instance of the {@link SolutionUserDTO.Builder} class.
     *
     * @return a new {@code SolutionUserDTO.Builder}.
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
        private String domain;
        private String description;
        private PrincipalDTO alias;
        private CertificateDTO certificate;
        private Boolean disabled;
        private String objectId;

        public Builder withName(String name) {
            this.name = name;
            return this;
        }

        public Builder withDomain(String domain) {
            this.domain = domain;
            return this;
        }

        public Builder withDescription(String description) {
            this.description = description;
            return this;
        }

        public Builder withAlias(PrincipalDTO alias) {
            this.alias = alias;
            return this;
        }

        public Builder withCertificate(CertificateDTO certificate) {
            this.certificate = certificate;
            return this;
        }

        public Builder withDisabled(Boolean disabled) {
            this.disabled = disabled;
            return this;
        }

        public Builder withObjectId(String objectId) {
            this.objectId = objectId;
            return this;
        }

        public SolutionUserDTO build() {
            return new SolutionUserDTO(name, domain, description, alias, certificate, disabled, objectId);
        }
    }

}
