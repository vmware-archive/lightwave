/*
 *
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
 *
 */
package com.vmware.identity.idm;

import java.io.Serializable;
import java.util.Set;

public class SearchResult implements Serializable {

    /**
     * serial version id
     */
    private static final long serialVersionUID = 326008584840731220L;
    private Set<PersonUser> personUsers;
    private Set<SolutionUser> solutionUsers;
    private Set<Group> groups;

    public SearchResult(Set<PersonUser> pUsers, Set<SolutionUser> sUsers,
            Set<Group> grps) {
        this.personUsers = pUsers;
        this.solutionUsers = sUsers;
        this.groups = grps;
    }

    /**
     * Retrieve the matching solution users
     *
     * @return the solution users found; {@code not-null} value
     */
    public Set<SolutionUser> getSolutionUsers() {
        return this.solutionUsers;
    }

    /**
     * Retrieve the matching person users
     *
     * @return person users found; {@code not-null} value
     */
    public Set<PersonUser> getPersonUsers() {
        return this.personUsers;
    }

    /**
     * Retrieve the matching groups
     *
     * @return the groups found; {@code not-null} value
     */
    public Set<Group> getGroups() {
        return this.groups;
    }
}
