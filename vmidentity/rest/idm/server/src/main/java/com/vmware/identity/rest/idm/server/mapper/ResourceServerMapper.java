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

import org.apache.commons.lang.Validate;

import com.vmware.identity.idm.ResourceServer;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.ResourceServerDTO;

/**
 * Mapper utility to map objects from {@link ResourceServer} to {@link ResourceServerDTO} and vice-versa.
 *
 * @author Yehia Zayour
 */
public class ResourceServerMapper {

    public static ResourceServer getResourceServer(ResourceServerDTO resourceServerDTO) {
        Validate.notNull(resourceServerDTO, "resourceServerDTO");

        try {
            return new ResourceServer.Builder(resourceServerDTO.getName()).groupFilter(resourceServerDTO.getGroupFilter()).build();
        } catch (Exception e) {
            throw new DTOMapperException("cannot convert from dto to base idm object", e);
        }
    }

    public static ResourceServerDTO getResourceServerDTO(ResourceServer resourceServer) {
        Validate.notNull(resourceServer, "resourceServer");
        return ResourceServerDTO.builder().
                withName(resourceServer.getName()).
                withGroupFilter(resourceServer.getGroupFilter()).build();
    }

    public static Collection<ResourceServerDTO> getResourceServerDTOs(Collection<ResourceServer> resourceServers) {
        Validate.notNull(resourceServers, "resourceServers");
        Collection<ResourceServerDTO> resourceServerDTOs = new ArrayList<ResourceServerDTO>();
        for (ResourceServer resourceServer : resourceServers) {
            resourceServerDTOs.add(getResourceServerDTO(resourceServer));
        }
        return resourceServerDTOs;
    }
}
