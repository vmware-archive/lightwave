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

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.PrincipalDTO;

/**
 * Mapper for Principal entity
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 *
 */
public class PrincipalMapper {

    public static PrincipalDTO getPrincipalDTO(PrincipalId principal) {
        return PrincipalDTO.builder()
                .withName(principal.getName())
                .withDomain(principal.getDomain())
                .build();
    }

    public static PrincipalId getPrincipal(PrincipalDTO principalDTO) throws DTOMapperException {
        try {
            return new PrincipalId(principalDTO.getName(), principalDTO.getDomain());
        } catch (Exception e) {
            throw new DTOMapperException("Unable to convert from PrincipalDTO to PrincipalId");
        }
    }

}
