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

/**
 * The {@code MemberType} enum contains the known types of members for use
 * when adding or removing members from a group or when performing searches
 * on the tenant.
 *
 * @author Balaji Boggaram Ramanarayan
 */
public enum MemberType {

    /**
     * Users
     *
     * @see UserDTO
     */
    USER,

    /**
     * Groups
     *
     * @see GroupDTO
     */
    GROUP,

    /**
     * Solution Users
     *
     * @see SolutionUserDTO
     */
    SOLUTIONUSER,

    /**
     * All available member types.
     */
    ALL
}
