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
package com.vmware.identity.rest.afd.server.mapper;

import com.vmware.identity.idm.ActiveDirectoryJoinInfo;
import com.vmware.identity.idm.ActiveDirectoryJoinInfo.JoinStatus;
import com.vmware.identity.rest.afd.data.ActiveDirectoryJoinInfoDTO;
import com.vmware.identity.rest.afd.data.attributes.ADJoinStatus;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;


/**
 * Maps AD join info from IDM based {@link ActiveDirectoryJoinInfo} to REST based
 * {@link ActiveDirectoryJoinInfoDTO}
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class ActiveDirectoryInfoMapper {

    public static ActiveDirectoryJoinInfoDTO getActiveDirectoryDTO(ActiveDirectoryJoinInfo adJoinInfo) {
        return ActiveDirectoryJoinInfoDTO.builder().withAlias(adJoinInfo.getAlias()).withDn(adJoinInfo.getDn()).withName(adJoinInfo.getName()).withStatus(mapJoinStatusFromIDMToREST(adJoinInfo.getJoinStatus())).build();
    }

    /**
     * Maps join status of active directory form IDM to RESTful and viceversa. The need of having
     * this mapper is because of longer and redundant enum constants in IDM.
     */
    private static String mapJoinStatusFromIDMToREST(JoinStatus joinStatus) {
        if (joinStatus == JoinStatus.ACTIVE_DIRECTORY_JOIN_STATUS_UNKNOWN) {
            return ADJoinStatus.UNKNOWN.name();
        } else if (joinStatus == JoinStatus.ACTIVE_DIRECTORY_JOIN_STATUS_WORKGROUP) {
            return ADJoinStatus.WORKGROUP.name();
        } else if (joinStatus == JoinStatus.ACTIVE_DIRECTORY_JOIN_STATUS_DOMAIN) {
            return ADJoinStatus.DOMAIN.name();
        } else {
            throw new DTOMapperException("Failed to map AD join status :" + joinStatus.name());
        }
    }
}
