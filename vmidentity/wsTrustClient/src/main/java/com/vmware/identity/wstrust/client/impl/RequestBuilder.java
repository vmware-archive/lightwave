/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *   use this file except in compliance with the License.  You may obtain a copy
 *   of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, without
 *   warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */
package com.vmware.identity.wstrust.client.impl;

/**
 * Implementations of this interface will serve as WS-Trust 1.4 request builders
 * responsible for SSO client -> STS server communication
 */
interface RequestBuilder {

    /**
     * Creates a STS request.
     *
     * @return SoapMessage ready to be passed to STS
     * @throws ParserException
     */
    public SoapMessage createRequest() throws ParserException;

    /**
     * Returns the amount of time in seconds that requests requests created by
     * this builder are valid
     *
     * <p>
     * RequestBuilder implementations can set a hard-coded, configurable or
     * adaptive request validity interval
     *
     * @return a positive number of seconds
     */
    public int getRequestValidityInSeconds();
}
