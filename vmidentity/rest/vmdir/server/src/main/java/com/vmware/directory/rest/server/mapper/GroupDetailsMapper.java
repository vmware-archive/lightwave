/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.directory.rest.server.mapper;


import com.vmware.directory.rest.common.data.GroupDetailsDTO;
import com.vmware.identity.idm.GroupDetail;

/**
 * Object mapper mapping from {@link GroupDetailsDTO} to {@link GroupDetail} and vice-versa.
 * This mapper helps in transforming objects which can be serializer/de-serializer friendly.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class GroupDetailsMapper {

    /**
     * Map object from {@link GroupDetailsDTO} to {@link GroupDetail}
     */
    public static GroupDetail getGroupDetails(GroupDetailsDTO groupDetailsDTO) {
        return new GroupDetail(groupDetailsDTO.getDescription());
    }

    /**
     * Map object from {@link GroupDetail} to {@link GroupDetailsDTO}
     */
    public static GroupDetailsDTO getGroupDetailsDTO(GroupDetail groupDetail){
        return GroupDetailsDTO.builder()
                              .withDescription(groupDetail.getDescription())
                              .build();
    }

}
