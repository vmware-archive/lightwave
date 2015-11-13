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
package com.vmware.identity.rest.core.data;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;

/**
 * The {@code CredentialsDTO} class represents user credentials in a JSON format intended for
 * consumption by REST SSO classes.
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = CredentialsDTO.Builder.class)
public class CredentialsDTO extends DTO {

    private final String username;
    private final String password;

    /**
     * Construct a {@code CredentialsDTO} from a username and password.
     *
     * @param username
     * @param password
     */
    public CredentialsDTO(String username, String password) {
        this.username = username;
        this.password = password;
    }

    /**
     * Get the username represented by this {@code CredentialsDTO}.
     *
     * @return the username represented by this {@code CredentialsDTO}.
     */
    public String getUsername() {
        return username;
    }

    /**
     * Get the password represented by this {@code CredentialsDTO}.
     *
     * @return the password represented by this {@code CredentialsDTO}.
     */
    public String getPassword() {
        return password;
    }

    /**
     * Create an instance of the {@link CredentialsDTO.Builder} class.
     *
     * @return a new {@code CredentialsDTO.Builder}.
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
        private String username;
        private String password;

        public Builder withUsername(String username) {
            this.username = username;
            return this;
        }

        public Builder withPassword(String password) {
            this.password = password;
            return this;
        }

        public CredentialsDTO build() {
            return new CredentialsDTO(username, password);
        }
    }

}
