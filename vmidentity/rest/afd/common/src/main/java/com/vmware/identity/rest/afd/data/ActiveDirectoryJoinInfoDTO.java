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
 * The {@code ActiveDirectoryJoinInfoDTO} class represents the information of the
 * Active Directory join.
 */
@JsonDeserialize(builder=ActiveDirectoryJoinInfoDTO.Builder.class)
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
public class ActiveDirectoryJoinInfoDTO extends DTO {

    private final String alias;
    private final String dn;
    private final String name;
    private final String status;

    /**
     * Construct an {@code ActiveDirectoryJoinInfoDTO} with an alias, dn, name, and status.
     *
     * @param alias the alias of the Active Directory join.
     * @param dn the domain name of the Active Directory join.
     * @param name the name of the Active Directory join.
     * @param status the status of the Active Directory join.
     */
    public ActiveDirectoryJoinInfoDTO(String alias, String dn, String name, String status) {
        this.alias = alias;
        this.dn = dn;
        this.name = name;
        this.status = status;
    }

    /**
     * Get the alias of the Active Directory join.
     *
     * @return the alias.
     */
    public String getAlias() {
        return alias;
    }

    /**
     * Get the domain name of the Active Directory join.
     *
     * @return the domain.
     */
    public String getDn() {
        return dn;
    }

    /**
     * Get the name of the Active Directory join.
     *
     * @return the name.
     */
    public String getName() {
        return name;
    }

    /**
     * Get the status of the Active Directory join.
     *
     * @return the status.
     */
    public String getStatus() {
        return status;
    }

    /**
     * Create an instance of the {@link ActiveDirectoryJoinInfoDTO.Builder}
     * class.
     *
     * @return a new {@code ActiveDirectoryJoinInfoDTO.Builder}.
     */
    public static Builder builder() {
        return new ActiveDirectoryJoinInfoDTO.Builder();
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
        private String alias;
        private String dn;
        private String domainName;
        private String status;

        public Builder withAlias(String alias) {
            this.alias = alias;
            return this;
        }

        public Builder withDn(String dn) {
            this.dn = dn;
            return this;
        }

        public Builder withName(String domainName) {
            this.domainName = domainName;
            return this;
        }

        public Builder withStatus(String status) {
            this.status = status;
            return this;
        }

        public ActiveDirectoryJoinInfoDTO build() {
            return new ActiveDirectoryJoinInfoDTO(alias, dn, domainName, status);
        }
    }
}
