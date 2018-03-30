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

import java.util.ArrayList;
import java.util.Collection;

import com.vmware.identity.idm.SecurityDomain;
import com.vmware.identity.rest.idm.data.SecurityDomainDTO;

/**
 * Mapper utility to map objects from {@link SecurityDomain} to {@link SecurityDomainDTO} and vice-versa.
 *
 */
public class SecurityDomainMapper {

    public static SecurityDomainDTO getSecurityDomainDTO(SecurityDomain secDomain){
          return SecurityDomainDTO.builder()
                             .withName(secDomain.getName())
                             .withAlias(secDomain.getAlias())
                             .build();
    }

    public static SecurityDomain getSecurityDomain(SecurityDomainDTO secDomainDTO) {
        return new SecurityDomain(secDomainDTO.getName(), secDomainDTO.getAlias());
    }

    public static Collection<SecurityDomainDTO> getSecurityDomainDTOs(Collection<SecurityDomain> secDomains) {
        Collection<SecurityDomainDTO> secDomainDTOs = new ArrayList<SecurityDomainDTO>();
        for (SecurityDomain secDomain : secDomains) {
            secDomainDTOs.add(getSecurityDomainDTO(secDomain));
        }
        return secDomainDTOs;
    }

    public static Collection<SecurityDomain> getAttributes(Collection<SecurityDomainDTO> secDomainDTOs) {
        Collection<SecurityDomain> secDomains = new ArrayList<SecurityDomain>();
        for (SecurityDomainDTO secDomainDTO : secDomainDTOs) {
            secDomains.add(getSecurityDomain(secDomainDTO));
        }
        return secDomains;
    }

}
