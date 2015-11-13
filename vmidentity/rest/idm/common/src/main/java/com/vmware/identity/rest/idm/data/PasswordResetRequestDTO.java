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
 * The {@code PasswordResetRequestDTO} class contains the parameters necessary for changing
 * or resetting a user's password.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = PasswordResetRequestDTO.Builder.class)
public class PasswordResetRequestDTO extends DTO {

    private final String currentPassword;
    private final String newPassword;

    /**
     * Create a {@code PasswordResetRequestDTO} with the user's current password and new
     * password.
     *
     * @param currentPassword the user's current password. This is usually only necessary
     *  when the user is changing their password.
     * @param newPassword the user's new password.
     */
    public PasswordResetRequestDTO(String currentPassword, String newPassword) {
        this.currentPassword = currentPassword;
        this.newPassword = newPassword;
    }

    /**
     * Get the user's current password.
     *
     * @return the user's current password.
     */
    public String getCurrentPassword() {
        return currentPassword;
    }

    /**
     * Get the user's new password.
     *
     * @return the user's new password.
     */
    public String getNewPassword() {
        return newPassword;
    }

    /**
     * Creates an instance of the {@link PasswordResetRequestDTO.Builder} class.
     *
     * @return a new {@code PasswordResetRequestDTO.Builder}.
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

        private String currentPassword;
        private String newPassword;

        public Builder withCurrentPassword(String currentPassword) {
            this.currentPassword = currentPassword;
            return this;
        }

        public Builder withNewPassword(String newPassword) {
            this.newPassword = newPassword;
            return this;
        }

        public PasswordResetRequestDTO build() {
            return new PasswordResetRequestDTO(currentPassword, newPassword);
        }
    }

}
