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

import com.vmware.identity.idm.ServiceEndpoint;
import com.vmware.identity.rest.idm.data.ServiceEndpointDTO;

/**
 * Mapper utility to map objects from {@link ServiceEndpoint} to {@link ServiceEndpointDTO} and vice-versa.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class ServiceEndPointMapper {

    public static ServiceEndpoint getServiceEndPoint(ServiceEndpointDTO endpointDTO) {
        return new ServiceEndpoint(endpointDTO.getName(), endpointDTO.getEndpoint(), endpointDTO.getBinding());
    }

    public static Collection<ServiceEndpoint> getServiceEndPoints(Collection<ServiceEndpointDTO> endpointDTOs) {
        Collection<ServiceEndpoint> serviceEndpoints = new ArrayList<ServiceEndpoint>();
        for(ServiceEndpointDTO endpointDTO : endpointDTOs){
            serviceEndpoints.add(getServiceEndPoint(endpointDTO));
        }
        return serviceEndpoints;
    }

    public static ServiceEndpointDTO getServiceEndPointDTO(ServiceEndpoint endpoint) {
        ServiceEndpointDTO svcEndpointDTO = null;
        if (endpoint != null) {
            svcEndpointDTO = ServiceEndpointDTO.builder()
                                               .withBinding(endpoint.getBinding())
                                               .withEndpoint(endpoint.getEndpoint())
                                               .withName(endpoint.getName())
                                               .build();
        }
        return svcEndpointDTO;
    }

    public static Collection<ServiceEndpointDTO> getServiceEndPointDTOs(Collection<ServiceEndpoint> endpoints) {
        Collection<ServiceEndpointDTO> endPointDTOs = null;
        if (endpoints != null) {
            endPointDTOs = new ArrayList<ServiceEndpointDTO>();
            for (ServiceEndpoint endpoint : endpoints) {
                endPointDTOs.add(getServiceEndPointDTO(endpoint));
            }

        }
        return endPointDTOs;
    }

}
