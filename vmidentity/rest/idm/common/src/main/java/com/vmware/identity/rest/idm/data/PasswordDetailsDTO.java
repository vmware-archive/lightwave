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
 * The {@code PasswordDetailsDTO} class contains details about a {@link UserDTO}'s password.
 * While the password field can be sent from a client to the server (e.g. while creating a user),
 * the server should never populate the field while returning information.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = PasswordDetailsDTO.Builder.class)
public class PasswordDetailsDTO extends DTO {

    private final String password;
    private final Long lastSet;
    private final Long lifetime;

    /**
     * Create a {@code PasswordDetailsDTO} with its various details.
     *
     * @param password the password to set for a user.
     * @param passwordLastSet the timestamp at which the password was last set.
     * @param passwordLifetime the lifetime of the password.
     */
    public PasswordDetailsDTO(String password, Long passwordLastSet, Long passwordLifetime) {
        this.password = password;
        this.lastSet = passwordLastSet;
        this.lifetime = passwordLifetime;
    }

    /**
     * Get the password from the password details. When retrieving information from the server
     * this value should always be null. The field should only be populated when sending data
     * to the server.
     *
     * @return the password.
     */
    public String getPassword() {
        return password;
    }

    /**
     * Get the timestamp at which the password was last set.
     *
     * @return the timestamp indicating when the password was last set.
     */
    public Long getLastSet() {
        return lastSet;
    }

    /**
     * Get the lifetime of the password.
     *
     * @return the lifetime of the password.
     */
    public Long getLifetime() {
        return lifetime;
    }

    /**
     * Creates an instance of the {@link PasswordDetailsDTO.Builder} class.
     *
     * @return a new {@code PasswordDetailsDTO.Builder}.
     */
    public static Builder builder() {
        return new PasswordDetailsDTO.Builder();
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
        private String password;
        private Long lastSet;
        private Long lifetime;

        public Builder withPassword(String password) {
            this.password = password;
            return this;
        }

        public Builder withLastSet(Long passwordLastSet) {
            this.lastSet = passwordLastSet;
            return this;
        }

        public Builder withLifetime(Long passwordLifetime) {
            this.lifetime = passwordLifetime;
            return this;
        }

        public PasswordDetailsDTO build() {
            return new PasswordDetailsDTO(password, lastSet, lifetime);
        }

    }

}
