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
package com.vmware.identity.rest.idm.data.attributes;

import com.vmware.identity.rest.idm.data.AuthenticationPolicyDTO;
import com.vmware.identity.rest.idm.data.BrandPolicyDTO;
import com.vmware.identity.rest.idm.data.LockoutPolicyDTO;
import com.vmware.identity.rest.idm.data.PasswordPolicyDTO;
import com.vmware.identity.rest.idm.data.TokenPolicyDTO;

/**
 * The {@code TenantConfigType} enum contains the known types of configuration policies
 * for use when retrieving a tenant configuration.
 *
 * @author Balaji Boggaram Ramanarayan
 */
public enum TenantConfigType {

    /**
     * Retrieve all tenant configuration policies
     */
    ALL,

    /**
     * Retrieve only the tenant lockout policy.
     *
     * @see LockoutPolicyDTO
     */
    LOCKOUT,

    /**
     * Retrieve only the tenant password policy.
     *
     * @see PasswordPolicyDTO
     */
    PASSWORD,

    /**
     * Retrieve only the tenant provider policy.
     *
     * @see PasswordPolicyDTO
     */
    PROVIDER,

    /**
     * Retrieve only the tenant token policy.
     *
     * @see TokenPolicyDTO
     */
    TOKEN,

    /**
     * Retrieve only the tenant brand policy.
     *
     * @see BrandPolicyDTO
     */
    BRAND,

    /**
     * Retrieve only the tenant authentication policy.
     *
     * @see AuthenticationPolicyDTO
     */
    AUTHENTICATION
}
