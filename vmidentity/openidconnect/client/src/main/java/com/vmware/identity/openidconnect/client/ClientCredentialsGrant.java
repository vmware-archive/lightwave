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

package com.vmware.identity.openidconnect.client;

/**
 * Client Credentials Grant
 *
 * @author Jun Sun
 */
public class ClientCredentialsGrant extends AuthorizationGrant {

    /**
     * Constructor
     */
    public ClientCredentialsGrant() {}

    /* (non-Javadoc)
     * @see com.vmware.identity.openidconnect.client.AuthorizationGrant#toNimbusAuthorizationGrant()
     */
    @Override
    com.nimbusds.oauth2.sdk.AuthorizationGrant toNimbusAuthorizationGrant() {
        return new com.nimbusds.oauth2.sdk.ClientCredentialsGrant();
    }
}
