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
 * The {@code TenantDTO} class contains the basic information that makes up a tenant in IDM.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder=TenantDTO.Builder.class)
public class TenantDTO extends DTO {

    private final String name;
    private final String longName;
    private final String key;
    private final String guid;
    private final String issuer;
    private final TenantCredentialsDTO credentials;
    private final String username;
    private final String password;

    /**
     * Construct a {@code TenantDTO} with its various fields.
     *
     * @param name the name of the tenant.
     * @param longName the long-form name of the tenant.
     * @param key the tenant key.
     * @param guid a generated unique identifier for the tenant.
     * @param issuer the tenant issuer.
     * @param tenantCredentials the credentials associated with the tenant.
     * @param username the default tenant administrator's username for use when
     *  creating a new tenant.
     * @param password the default tenant administrator's password for use when
     *  creating a new tenant.
     */
    public TenantDTO(String name, String longName, String key, String guid, String issuer, TenantCredentialsDTO tenantCredentials, String username, String password) {
        this.name = name;
        this.longName = longName;
        this.key = key;
        this.guid = guid;
        this.issuer = issuer;
        this.credentials = tenantCredentials;
        this.username = username;
        this.password = password;
    }

    /**
     * Get the name of the tenant.
     *
     * @return the name of the tenant.
     */
    public String getName() {
        return name;
    }

    /**
     * Get the long-form name of the tenant.
     *
     * @return the long-form name of the tenant.
     */
    public String getLongName() {
        return longName;
    }

    /**
     * Get the tenant key.
     *
     * @return the tenant key.
     */
    public String getKey() {
        return key;
    }

    /**
     * Get the generated unique identifier of the tenant.
     *
     * @return the tenant's generated unique identifier.
     */
    public String getGuid() {
        return guid;
    }

    /**
     * Get the issuer name of the tenant.
     *
     * @return the issuer name of the tenant.
     */
    public String getIssuer() {
        return issuer;
    }

    /**
     * Get the tenant's credentials.
     *
     * @return the tenant's credentials.
     */
    public TenantCredentialsDTO getCredentials() {
        return credentials;
    }

    /**
     * Get the tenant's default administrator's username. For use when constructing
     * a new tenant.
     *
     * @return the tenant's default administrator's username.
     */
    public String getUsername() {
        return username;
    }

    /**
     * Get the tenant's default administrator's password. For use when constructing
     * a new tenant.
     *
     * @return the tenant's default administrator's password.
     */
    public String getPassword() {
        return password;
    }

    /**
     * Creates an instance of the {@link TenantDTO.Builder} class.
     *
     * @return a new {@code TenantDTO.Builder}.
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
        private String longName;
        private String key;
        private String guid;
        private String issuer;
        private TenantCredentialsDTO tenantCredentials;
        private String username;
        private String password;

        public Builder withName(String name) {
            this.name = name;
            return this;
        }

        public Builder withLongName(String longName) {
            this.longName = longName;
            return this;
        }

        public Builder withKey(String key) {
            this.key = key;
            return this;
        }

        public Builder withGuid(String guid) {
            this.guid = guid;
            return this;
        }

        public Builder withIssuer(String issuer) {
            this.issuer = issuer;
            return this;
        }

        public Builder withCredentials(TenantCredentialsDTO credentialsDTO) {
            this.tenantCredentials = credentialsDTO;
            return this;
        }

        public Builder withUsername(String username){
            this.username = username;
            return this;
        }

        public Builder withPassword(String password){
            this.password = password;
            return this;
        }

        public TenantDTO build() {
            return new TenantDTO(name, longName, key, guid, issuer, tenantCredentials, username, password);
        }
    }

}
