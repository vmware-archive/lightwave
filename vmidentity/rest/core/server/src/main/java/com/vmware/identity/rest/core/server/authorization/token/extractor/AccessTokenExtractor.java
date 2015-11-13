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
package com.vmware.identity.rest.core.server.authorization.token.extractor;

import com.vmware.identity.rest.core.server.authorization.exception.InvalidRequestException;
import com.vmware.identity.rest.core.server.authorization.token.TokenInfo;

/**
 * An object that extracts access token information from some supplied
 * input data.
 */
public interface AccessTokenExtractor {

    /**
     * Determine whether the extractor can find a token match.
     *
     * @return true if a token exists, false otherwise
     */
    public boolean exists();

    /**
     * Extract a TokenInfo object from the request.
     *
     * @param context request to extract from
     * @return the extracted token info
     * @throws InvalidRequestException if extraction failed
     * due to an invalid request object
     */
    public TokenInfo extract() throws InvalidRequestException;

}
