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
package com.vmware.identity.rest.afd.data;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code ActiveDirectoryJoinRequestDTO} class represents the information
 * necessary for performing an Active Directory join request.
 */
@JsonDeserialize(builder = ActiveDirectoryJoinRequestDTO.Builder.class)
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
public class ActiveDirectoryJoinRequestDTO extends DTO {

    private final String username;
    private final String password;
    private final String domain;
    private final String ou;

    /**
     * Construct an {@code ActiveDirectoryJoinRequestDTO} from a username, password, domain
     * and organizational unit.
     *
     * @param username the username with which to perform the request.
     * @param password the password for the user that will perform the request.
     * @param domain the Active Directory domain to join.
     * @param ou the organizational unit.
     */
    public ActiveDirectoryJoinRequestDTO(String username, String password, String domain, String ou) {
        this.username = username;
        this.password = password;
        this.domain = domain;
        this.ou = ou;
    }

    /**
     * Get the username this request is created for.
     *
     * @return the username.
     */
    public String getUsername() {
        return username;
    }

    /**
     * Get the password used by this request.
     *
     * @return the password.
     */
    public String getPassword() {
        return password;
    }

    /**
     * Get the domain this request will join.
     *
     * @return the domain.
     */
    public String getDomain() {
        return domain;
    }

    /**
     * Get the organizational unit of the join request.
     *
     * @return the organizational unit.
     */
    public String getOU() {
        return ou;
    }

    /**
     * Create an instance of the {@link ActiveDirectoryJoinRequestDTO.Builder}
     * class.
     *
     * @return a new {@code ActiveDirectoryJoinRequestDTO.Builder}.
     */
    public static Builder builder() {
        return new ActiveDirectoryJoinRequestDTO.Builder();
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
        private String domain;
        private String ou;

        public Builder withUsername(String username) {
            this.username = username;
            return this;
        }

        public Builder withPassword(String password) {
            this.password = password;
            return this;
        }

        public Builder withDomain(String domain) {
            this.domain = domain;
            return this;
        }

        public Builder withOU(String ou) {
            this.ou = ou;
            return this;
        }

        public ActiveDirectoryJoinRequestDTO build() {
            return new ActiveDirectoryJoinRequestDTO(username, password, domain, ou);
        }
    }
}
