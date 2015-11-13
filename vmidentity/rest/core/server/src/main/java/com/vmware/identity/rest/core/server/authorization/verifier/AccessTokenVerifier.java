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
package com.vmware.identity.rest.core.server.authorization.verifier;

import com.vmware.identity.rest.core.server.authorization.exception.InvalidRequestException;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidTokenException;
import com.vmware.identity.rest.core.server.authorization.token.AccessToken;
import com.vmware.identity.rest.core.server.exception.ServerException;

/**
 * An object that can be used to verify an access token.
 */
public interface AccessTokenVerifier {

    /**
     * Verifies an access token.
     *
     * @return <tt>true</tt> if the token was successfully verified.
     * @throws InvalidTokenException if the token could not be verified
     * @throws InvalidRequestException if information was missing for verification
     * @throws ServerException if an error occurred preventing the token from being verified
     */
    public void verify(AccessToken token) throws InvalidTokenException, InvalidRequestException, ServerException;

}
