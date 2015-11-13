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

import java.util.Collection;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code SearchResultDTO} class contains the results of a search. The results
 * a separated into three collections: domains, groups, and users.
 * 
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder=SearchResultDTO.Builder.class)
public class SearchResultDTO extends DTO {

    private final Collection<UserDTO> users;
    private final Collection<GroupDTO> groups;
    private final Collection<SolutionUserDTO> solutionUsers;

    /**
     * Construct a {@code SearchResultDTO} with its various collections.
     *
     * @param users the collection of users resulting from the search.
     * @param groups the collection of groups resulting from the search.
     * @param solutionUsers the collection of solution users resulting from the search.
     */
    public SearchResultDTO(Collection<UserDTO> users, Collection<GroupDTO> groups, Collection<SolutionUserDTO> solutionUsers) {
        this.users = users;
        this.groups = groups;
        this.solutionUsers = solutionUsers;
    }

    /**
     * Get the collection of users found as a result of the search.
     *
     * @return a collection of {@link UserDTO}.
     */
    public Collection<UserDTO> getUsers() {
        return users;
    }

    /**
     * Get the collection of groups found as a result of the search.
     *
     * @return a collection of {@link GroupDTO}.
     */
    public Collection<GroupDTO> getGroups() {
        return groups;
    }

    /**
     * Get the collection of solution users found as a result of the search.
     *
     * @return a collection of {@link SolutionUserDTO}.
     */
    public Collection<SolutionUserDTO> getSolutionUsers() {
        return solutionUsers;
    }

    /**
     * Returns {@code true} if the search result is empty.
     *
     * @return {@code true} if the search result is empty.
     */
    @JsonIgnore
    public boolean isEmpty() {
        if (users != null && !users.isEmpty()) {
            return false;
        }

        if (groups != null && !groups.isEmpty()) {
            return false;
        }

        if (solutionUsers != null && !solutionUsers.isEmpty()) {
            return false;
        }

        return true;
    }

    /**
     * Creates an instance of the {@link SearchResultDTO.Builder} class.
     *
     * @return a new {@code SearchResultDTO.Builder}.
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

        private Collection<UserDTO> users;
        private Collection<GroupDTO> groups;
        private Collection<SolutionUserDTO> solutionUsers;

        public Builder withUsers(Collection<UserDTO> users) {
            this.users = users;
            return this;
        }

        public Builder withGroups(Collection<GroupDTO> groups) {
            this.groups = groups;
            return this;
        }

        public Builder withSolutionUsers(Collection<SolutionUserDTO> solutionUsers) {
            this.solutionUsers = solutionUsers;
            return this;
        }

        public SearchResultDTO build() {
            return new SearchResultDTO(users, groups, solutionUsers);
        }
    }

}
