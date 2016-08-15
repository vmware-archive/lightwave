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
 * The {@code UserDetailsDTO} class contains detailed information about a {@link UserDTO}.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = UserDetailsDTO.Builder.class)
public class UserDetailsDTO extends DTO {

    private final String email;
    private final String upn;
    private final String firstName;
    private final String lastName;
    private final String description;

    /**
     * Construct a {@code UserDetailsDTO} with its various fields.
     *
     * @param email the email address of the user.
     * @param upn the user principal name of the user.
     * @param firstName the first name of the user.
     * @param lastName the last name of the user.
     * @param description the description of the user.
     */
    public UserDetailsDTO(String email, String upn, String firstName, String lastName, String description) {
        this.email = email;
        this.upn = upn;
        this.firstName = firstName;
        this.lastName = lastName;
        this.description = description;
    }

    /**
     * Get the email address of the user.
     *
     * @return the email address of the user.
     */
    public String getEmail() {
        return email;
    }

    /**
     * Get the user principal name of the user.
     *
     * @return the user principal name of the user.
     */
    public String getUPN() {
        return upn;
    }

    /**
     * Get the first name of the user.
     *
     * @return the first name of the user.
     */
    public String getFirstName() {
        return firstName;
    }

    /**
     * Get the last name of the user.
     *
     * @return the last name of the user.
     */
    public String getLastName() {
        return lastName;
    }

    /**
     * Get the description of the user.
     *
     * @return the description of the user.
     */
    public String getDescription() {
        return description;
    }

    /**
     * Creates an instance of the {@link UserDetailsDTO.Builder} class.
     *
     * @return a new {@code UserDetailsDTO.Builder}.
     */
    public static Builder builder() {
        return new UserDetailsDTO.Builder();
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
        private String email;
        private String upn;
        private String firstName;
        private String lastName;
        private String description;

        public Builder withEmail(String emailAddress) {
            this.email = emailAddress;
            return this;
        }

        public Builder withUPN(String userPrincipalName) {
            this.upn = userPrincipalName;
            return this;
        }

        public Builder withFirstName(String firstName) {
            this.firstName = firstName;
            return this;
        }

        public Builder withLastName(String lastName) {
            this.lastName = lastName;
            return this;
        }

        public Builder withDescription(String description) {
            this.description = description;
            return this;
        }

        public UserDetailsDTO build() {
            return new UserDetailsDTO(email, upn,
                    firstName, lastName, description);
        }
    }

}