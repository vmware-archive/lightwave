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

import com.vmware.identity.idm.AssertionConsumerService;
import com.vmware.identity.rest.idm.data.AssertionConsumerServiceDTO;

/**
 * Mapper utility to map objects from {@link AssertionConsumerService} to {@link AssertionConsumerServiceDTO} and vice-versa.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class AssertionConsumerServiceMapper {

    /**
     * Map {@link AssertionConsumerServiceDTO} object to {@link AssertionConsumerService} object
     */
    public static AssertionConsumerService getAssertionCs(AssertionConsumerServiceDTO assertion) {
        AssertionConsumerService assertionConsumerService = null;
        if (assertion != null) {
            assertionConsumerService = new AssertionConsumerService(assertion.getName());
            assertionConsumerService.setBinding(assertion.getBinding());
            assertionConsumerService.setEndpoint(assertion.getEndpoint());
            assertionConsumerService.setIndex(assertion.getIndex());
        }
        return assertionConsumerService;
    }

    /**
     * Maps collection of {@link AssertionConsumerServiceDTO} object to {@link AssertionConsumerService} objects
     */
    public static Collection<AssertionConsumerService> getAssertionCsConfigs(Collection<AssertionConsumerServiceDTO> assertions) {
        Collection<AssertionConsumerService> acsConfigs = null;
        if (assertions != null) {
            acsConfigs = new ArrayList<AssertionConsumerService>();
            for (AssertionConsumerServiceDTO assertion : assertions) {
                acsConfigs.add(getAssertionCs(assertion));
            }
        }
        return acsConfigs;
    }

    /**
     * Map {@link AssertionConsumerService} object to {@link AssertionConsumerServiceDTO}.
     */
    public static AssertionConsumerServiceDTO getAssertionCsDTO(AssertionConsumerService acs) {
        return AssertionConsumerServiceDTO.builder()
                                          .withBinding(acs.getBinding())
                                          .withEndpoint(acs.getEndpoint())
                                          .withIndex(acs.getIndex())
                                          .withName(acs.getName())
                                          .build();
    }

    /**
     * Map collection of {@link AssertionConsumerService} objects to {@link AssertionConsumerServiceDTO} objects.
     */
    public static Collection<AssertionConsumerServiceDTO> getAssertionCsDTOs(Collection<AssertionConsumerService> acsConfigs){
        Collection<AssertionConsumerServiceDTO> acsDTOs = new ArrayList<AssertionConsumerServiceDTO>();
        for(AssertionConsumerService acsConfig : acsConfigs){
            acsDTOs.add(getAssertionCsDTO(acsConfig));
        }
        return acsDTOs;
    }

}
