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

import com.vmware.identity.rest.idm.data.IdentityProviderDTO;

/**
 * The {@code IdentityProviderType} enum represents the known provider types
 * for use with {@link IdentityProviderDTO}.
 *
 * @author Balaji Boggaram Ramanarayan
 */
public enum IdentityProviderType {

    /**
     * An Active Directory identity provider.
     */
    IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY,

    /**
     * A LDAP identity provider.
     */
    IDENTITY_STORE_TYPE_LDAP,

    /**
     * A Local OS identity provider.
     */
    IDENTITY_STORE_TYPE_LOCAL_OS,

    /**
     * An Active Directory identity provider using an LDAP protocol.
     */
    IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING
}
