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
 * The {@code AuthenticationType} enum represents several known
 * authentication types for use with {@link IdentityProviderDTO}.
 */
public enum AuthenticationType {
    /**
     * A combination of username and password will be used for authentication.
     */
    PASSWORD,

    /**
     * Kerberos credentials will be used to authentication against the external server
     * if it is supported.
     * <p>
     * This method is supported as long as the external server supports Kerberos binds.
     */
    USE_KERBEROS,

    /**
     * Used internally only for LDAP binding with VMware Directory.
     */
    SRP
}
