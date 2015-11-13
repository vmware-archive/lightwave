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
package com.vmware.identity.rest.core.server.authorization.token;

import com.vmware.identity.rest.core.server.authorization.exception.InvalidTokenException;

/**
 * An object that can be used to build an access token from {@link TokenInfo}.
 */
public interface AccessTokenBuilder {

    /**
     * Builds an access token from a supplied {@link TokenInfo}.
     *
     * @param info the token info used to build the access token
     * @return an access token
     * @throws InvalidTokenException if there was a problem parsing the token from the token info
     */
    public AccessToken build(TokenInfo info) throws InvalidTokenException;

}
