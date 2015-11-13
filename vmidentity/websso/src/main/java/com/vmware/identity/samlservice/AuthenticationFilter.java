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
package com.vmware.identity.samlservice;

import com.vmware.identity.saml.PrincipalAttributesExtractorFactory;
import com.vmware.identity.saml.config.ConfigExtractorFactory;

/**
 * Authentication filter for our messages. Has following methods -
 *
 * preAuthenticate() - analyze request, throw if more data is required
 * authenticate() - attempt to perform actual authentication, throw if more data
 *  is required
 * getPrincipalAttributeExtractorFactory() - return
 *  PrincipalAttributeExtractorFactory object to get an attribute extractor per
 *  tenant from
 * getConfigExtractorFactory() - return ConfigExtractorFactory object to get a
 *  config extractor per tenant from, all the SAML authority configuration will
 *  be extracted from it
 */
public interface AuthenticationFilter<T> extends ProcessingFilter<T> {
    /**
     * Analyze object, throw if more data is required
     *
     * @param t
     * @throws SamlServiceException
     */
    void preAuthenticate(T t) throws SamlServiceException;

    /**
     * Attempt to perform actual authentication, throw if more data is required
     *
     * @param t
     * @throws SamlServiceException
     */
    void authenticate(T t) throws SamlServiceException;

    /**
     * Obtain matching principal attribute extractor factory object for this
     * auth method
     *
     * @param idmHostName
     * @return
     * @throws SamlServiceException
     */
    PrincipalAttributesExtractorFactory getPrincipalAttributeExtractorFactory(
            String idmHostName);

    /**
     * @param idmHostName
     * @return config extractor factory object for this host
     */
    ConfigExtractorFactory getConfigExtractorFactory(String idmHostName);
}
