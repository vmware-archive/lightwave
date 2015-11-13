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

import com.vmware.identity.idm.Tenant;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.TenantDTO;

/**
 * Mapper for tenant object
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 *
 */
public class TenantMapper {

    public static TenantDTO getTenantDTO(Tenant tenant) {
        return TenantDTO.builder()
                .withName(tenant.getName())
                .withLongName(tenant._longName)
                .withKey(tenant._tenantKey)
                .withIssuer(tenant._issuerName)
                .withGuid(tenant._guid)
                .build();
    }

    public static Tenant getTenant(TenantDTO tenantDTO) throws DTOMapperException {
        try {
            return new Tenant(tenantDTO.getName(), tenantDTO.getLongName(), tenantDTO.getKey());
        } catch (Exception e) {
            throw new DTOMapperException("Unable to convert TenantDTO to Tenant");
        }
    }

}
