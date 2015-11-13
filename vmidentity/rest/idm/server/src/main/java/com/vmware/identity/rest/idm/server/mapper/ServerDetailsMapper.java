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

import com.vmware.identity.idm.VmHostData;
import com.vmware.identity.rest.idm.data.ServerDetailsDTO;

/**
 * VmHost object mapper that maps from {@link VmHostData.class} to {@link ServerDetailsDTO.class} and vice-versa.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class ServerDetailsMapper {

    /**
     * Map object of type {@link VmHostData.class} to {@link ServerDetailsDTO}
     */
    public static ServerDetailsDTO getServerDetailsDTO(VmHostData serverInfo) {
        return ServerDetailsDTO.builder()
                               .withHostname(serverInfo.getHostName())
                               .withDomainController(serverInfo.isDomainController())
                               .build();
    }

    public static Collection<ServerDetailsDTO> getServerDetailsDTOs(Collection<VmHostData> servers) {
        Collection<ServerDetailsDTO> serversList = new ArrayList<ServerDetailsDTO>();
        for (VmHostData server : servers) {
            serversList.add(getServerDetailsDTO(server));
        }
        return serversList;
    }
}
