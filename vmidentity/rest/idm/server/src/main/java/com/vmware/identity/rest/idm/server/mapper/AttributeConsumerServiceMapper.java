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

import com.vmware.identity.idm.AttributeConsumerService;
import com.vmware.identity.rest.idm.data.AttributeConsumerServiceDTO;

/**
 * Mapper utility to map objects from {@link AttributeConsumerService} to {@link AttributeConsumerServiceDTO} and vice-versa.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class AttributeConsumerServiceMapper {

    /**
     * Map {@link AttributeConsumerServiceDTO} object to {@link AttributeConsumerService} object
     */
    public static AttributeConsumerService getAttributeCs(AttributeConsumerServiceDTO assertion) {
        AttributeConsumerService attributeConsumerService = null;
        if (assertion != null) {
            attributeConsumerService = new AttributeConsumerService(assertion.getName());
            attributeConsumerService.setIndex(assertion.getIndex());
            attributeConsumerService.setAttributes(AttributeMapper.getAttributes(assertion.getAttributes()));
        }
        return attributeConsumerService;
    }

    /**
     * Map collection of {@link AttributeConsumerServiceDTO} objects to {@link AttributeConsumerService} objects
     */

    public static Collection<AttributeConsumerService> getAttributeCsConfigs(Collection<AttributeConsumerServiceDTO> assertions) {
        Collection<AttributeConsumerService> acsConfigs = null;
        if (assertions != null) {
            acsConfigs = new ArrayList<AttributeConsumerService>();
            for (AttributeConsumerServiceDTO assertion : assertions) {
                acsConfigs.add(getAttributeCs(assertion));
            }
        }
        return acsConfigs;
    }

    /**
     * Map {@link AttributeConsumerService} object to {@link AttributeConsumerServiceDTO} object
     */
    public static AttributeConsumerServiceDTO getAttributeCsDTO(AttributeConsumerService acs) {
        return AttributeConsumerServiceDTO.builder()
                                          .withIndex(acs.getIndex())
                                          .withName(acs.getName())
                                          .withAttributes(AttributeMapper.getAttributeDTOs(acs.getAttributes()))
                                          .build();
    }

    /**
     * Map collection of {@link AttributeConsumerService} objects to {@link AttributeConsumerServiceDTO} objects
     */
    public static Collection<AttributeConsumerServiceDTO> getAttributeCsDTOs(Collection<AttributeConsumerService> acsConfigs){
        Collection<AttributeConsumerServiceDTO> acsDTOs = new ArrayList<AttributeConsumerServiceDTO>();
        for(AttributeConsumerService acsConfig : acsConfigs){
            acsDTOs.add(getAttributeCsDTO(acsConfig));
        }
        return acsDTOs;
    }

}
