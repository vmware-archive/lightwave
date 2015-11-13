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

import com.vmware.identity.idm.Attribute;
import com.vmware.identity.rest.idm.data.AttributeDTO;

/**
 * Mapper utility to map objects from {@link Attribute} to {@link AttributeDTO} and vice-versa.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class AttributeMapper {

    public static AttributeDTO getAttributeDTO(Attribute attribute){
          return AttributeDTO.builder()
                             .withName(attribute.getName())
                             .withFriendlyName(attribute.getFriendlyName())
                             .withNameFormat(attribute.getNameFormat())
                             .build();
    }

    public static Attribute getAttribute(AttributeDTO attributeDTO) {
        return new Attribute(attributeDTO.getName(), attributeDTO.getNameFormat(), attributeDTO.getFriendlyName());
    }

    public static Collection<AttributeDTO> getAttributeDTOs(Collection<Attribute> attributes) {
        Collection<AttributeDTO> attributeDTOs = new ArrayList<AttributeDTO>();
        for (Attribute attribute : attributes) {
            attributeDTOs.add(getAttributeDTO(attribute));
        }
        return attributeDTOs;
    }

    public static Collection<Attribute> getAttributes(Collection<AttributeDTO> attributeDTOs) {
        Collection<Attribute> attributes = new ArrayList<Attribute>();
        for (AttributeDTO attributeDTO : attributeDTOs) {
            attributes.add(getAttribute(attributeDTO));
        }
        return attributes;
    }

}
