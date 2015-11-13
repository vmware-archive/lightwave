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
 * The {@code GroupDTO} class contains the information that makes up a Group. This can be
 * considered analogous to a group in Active Directory or LDAP.
 * 
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = GroupDTO.Builder.class)
public class GroupDTO extends DTO {

    private final String name;
    private final String domain;
    private final GroupDetailsDTO details;
    private final PrincipalDTO alias;
    private final String objectId;

    /**
     * Construct a {@code GroupDTO} with its various fields.
     *
     * @param name the name of the group.
     * @param domain the domain of the group.
     * @param groupDetailsDTO the details for the group.
     * @param alias an optional alias for the group.
     * @param objectId the object identifier for the group.
     */
    public GroupDTO(String name, String domain, GroupDetailsDTO groupDetailsDTO, PrincipalDTO alias, String objectId) {
        this.name = name;
        this.domain = domain;
        this.details = groupDetailsDTO;
        this.alias = alias;
        this.objectId = objectId;
    }

    /**
     * Get the group's name.
     *
     * @return the group's name.
     */
    public String getName() {
        return name;
    }

    /**
     * Get the group's domain.
     *
     * @return the group's domain.
     */
    public String getDomain() {
        return domain;
    }

    /**
     * Get the group's details.
     *
     * @return the group's details.
     */
    public GroupDetailsDTO getDetails() {
        return details;
    }

    /**
     * Get the group's alias.
     *
     * @return the group's alias.
     */
    public PrincipalDTO getAlias() {
        return alias;
    }

    /**
     * Get the group's object identifier.
     *
     * @return the group's object identifier.
     */
    public String getObjectId() {
        return objectId;
    }

    /**
     * Creates an instance of the {@link GroupDTO.Builder} class.
     *
     * @return a new {@code GroupDTO.Builder}.
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
        private GroupDetailsDTO details;
        private PrincipalDTO alias;
        private String objectId;

        public Builder withName(String name) {
            this.name = name;
            return this;
        }

        public Builder withDomain(String domain) {
            this.domain = domain;
            return this;
        }

        public Builder withDetails(GroupDetailsDTO groupDetails) {
            this.details = groupDetails;
            return this;
        }

        public Builder withAlias(PrincipalDTO alias) {
            this.alias = alias;
            return this;
        }

        public Builder withObjectId(String objectId) {
            this.objectId = objectId;
            return this;
        }

        public GroupDTO build() {
            return new GroupDTO(name, domain, details, alias, objectId);
        }
    }

}
