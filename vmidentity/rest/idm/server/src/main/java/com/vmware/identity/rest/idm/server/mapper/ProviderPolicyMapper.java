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
package com.vmware.identity.rest.idm.server.mapper;

import java.util.Collection;

import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.ProviderPolicyDTO;

/**
 * Mapper for provider policy that be set on identity providers such as AD, OpenLDAP etc
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 *
 */
public class ProviderPolicyMapper {

    public static ProviderPolicyDTO getProviderPolicyDTO(Collection<String> defaultProviders, String defaultProviderAlias, boolean providerSelectionEnabled) {
        if (defaultProviders == null || defaultProviders.size() > 1) {
            // We never expect more than one default identity providers set per tenant.
            throw new DTOMapperException("Failed to map provider policy");
        }

        String defaultProvider = null;

        if (!defaultProviders.isEmpty()) {
            defaultProvider = defaultProviders.iterator().next();
        }

        return ProviderPolicyDTO.builder().withDefaultProvider(defaultProvider)
                                          .withDefaultProviderAlias(defaultProviderAlias)
                                          .withProviderSelectionEnabled(new Boolean(providerSelectionEnabled)).build();
    }

}
