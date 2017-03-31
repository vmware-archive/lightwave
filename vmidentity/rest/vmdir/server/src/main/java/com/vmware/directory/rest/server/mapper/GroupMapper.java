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

import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.PrincipalDTO;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;

/**
 * Group object mapper that maps {@link GroupDTO} to {@link Group} and vice-versa.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public final class GroupMapper {

    public static GroupDTO getGroupDTO(Group group) {
        PrincipalDTO alias = null;
        if (group.getAlias() != null) {
            alias = new PrincipalDTO(group.getAlias().getName(), group.getAlias().getDomain());
        }

        return GroupDTO.builder()
                .withName(group.getName())
                .withDomain(group.getDomain())
                .withDetails(GroupDetailsMapper.getGroupDetailsDTO(group.getDetail()))
                .withAlias(alias)
                .withObjectId(group.getObjectId())
                .build();
    }

    public static Set<GroupDTO> getGroupDTOs(Collection<Group> groups) {
        Set<GroupDTO> groupDTOs = new HashSet<GroupDTO>();
        for(Group group : groups) {
            groupDTOs.add(getGroupDTO(group));
        }
        return groupDTOs;
    }

    public static Group getGroup(GroupDTO groupDTO) throws DTOMapperException {
        PrincipalId groupId = new PrincipalId(groupDTO.getName(), groupDTO.getDomain());
        PrincipalId groupAlias = null;

        try {
            if (groupDTO.getAlias() != null) {
                groupAlias = new PrincipalId(groupDTO.getAlias().getName(), groupDTO.getAlias().getDomain());
            }

            GroupDetail groupDetail = new GroupDetail(groupDTO.getDetails().getDescription());

            return new Group(groupId, groupAlias, null, groupDetail);
        } catch (Exception e) {
            throw new DTOMapperException("Unable to convert GroupDTO to Group", e);
        }
    }
}
