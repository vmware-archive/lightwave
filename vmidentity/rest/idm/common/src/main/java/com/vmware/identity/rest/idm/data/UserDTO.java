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
 * The {@code UserDTO} class contains the information that makes up a User. This can be
 * considered analogous to a user in Active Directory or LDAP.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder=UserDTO.Builder.class)
public class UserDTO extends DTO {

    private final String name;
    private final String domain;
    private final PrincipalDTO alias;
    private final UserDetailsDTO details;
    private final Boolean disabled;
    private final Boolean locked;
    private final String objectId;
    private final PasswordDetailsDTO passwordDetails;

    /**
     * Construct a {@code UserDTO} with its various fields.
     *
     * @param name the name of the user.
     * @param domain the domain of the user.
     * @param alias an optional alias for the user.
     * @param details the details for the user.
     * @param disabled a flag indicating whether the user is disabled or not.
     * @param locked a flag indicating whether the user is locked or not.
     * @param passwordDetails the password details for the user, for use when creating a new user.
     * @param objectId the object identifier for the user.
     */
    public UserDTO(String name, String domain, PrincipalDTO alias, UserDetailsDTO details, Boolean disabled, Boolean locked, PasswordDetailsDTO passwordDetails, String objectId) {
        this.name = name;
        this.domain = domain;
        this.alias = alias;
        this.details = details;
        this.disabled = disabled;
        this.locked = locked;
        this.passwordDetails = passwordDetails;
        this.objectId = objectId;
    }

    /**
     * Get the name of the user.
     *
     * @return the user's name.
     */
    public String getName() {
        return name;
    }

    /**
     * Get the domain of the user.
     *
     * @return the user's domain.
     */
    public String getDomain() {
        return domain;
    }

    /**
     * Get the user's alias.
     *
     * @return the alias of the user.
     */
    public PrincipalDTO getAlias() {
        return alias;
    }

    /**
     * Get the user's details.
     *
     * @return the details of the user.
     */
    public UserDetailsDTO getDetails() {
        return details;
    }

    /**
     * Check if the user is disabled.
     *
     * @return {@code true} if the user is disabled, {@code false} otherwise.
     */
    public Boolean isDisabled() {
        return disabled;
    }

    /**
     * Check if the user is locked.
     *
     * @return {@code true} if the user is locked, {@code false} otherwise.
     */
    public Boolean isLocked() {
        return locked;
    }

    /**
     * Get the password details for the user. For use when creating a new user.
     *
     * @return the password details of the user.
     */
    public PasswordDetailsDTO getPasswordDetails() {
        return passwordDetails;
    }

    /**
     * Get the user's object identifier.
     *
     * @return the object identifier of the user.
     */
    public String getObjectId() {
        return objectId;
    }

    /**
     * Creates an instance of the {@link UserDTO.Builder} class.
     *
     * @return a new {@code UserDTO.Builder}.
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
        private PrincipalDTO alias;
        private UserDetailsDTO details;
        private Boolean disabled;
        private Boolean locked;
        private PasswordDetailsDTO passwordDetails;
        private String objectId;

        public Builder withName(String name) {
            this.name = name;
            return this;
        }

        public Builder withDomain(String domain) {
            this.domain = domain;
            return this;
        }

        public Builder withAlias(PrincipalDTO alias) {
            this.alias = alias;
            return this;
        }

        public Builder withDetails(UserDetailsDTO details) {
            this.details = details;
            return this;
        }

        public Builder withDisabled(Boolean disabled) {
            this.disabled = disabled;
            return this;
        }

        public Builder withLocked(Boolean locked) {
            this.locked = locked;
            return this;
        }

        public Builder withPasswordDetails(PasswordDetailsDTO passwordDetails) {
            this.passwordDetails = passwordDetails;
            return this;
        }

        public Builder withObjectId(String objectId) {
            this.objectId = objectId;
            return this;
        }

        public UserDTO build() {
            return new UserDTO(name, domain, alias, details, disabled, locked, passwordDetails, objectId);
        }
    }

}
