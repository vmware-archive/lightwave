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
package com.vmware.identity.rest.idm.server;

/**
 * Path parameter constants
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public final class PathParameters {

    public static final String TENANT_NAME = "tenantName";
    public static final String TENANT_NAME_VAR = "/{tenantName}";

    public static final String USER_NAME = "userName";
    public static final String USER_NAME_VAR = "/{userName}";

    public static final String GROUP_NAME = "groupName";
    public static final String GROUP_NAME_VAR = "/{groupName}";

    public static final String PASSWORD = "password";

    public static final String SEARCH = "search";
}
