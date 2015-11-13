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
package com.vmware.identity.rest.idm.server.test.integration.util;

import java.util.List;
import java.util.Set;

import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SearchCriteria;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.server.util.PrincipalUtil;


/**
 * Group utility which helps calling IDM directly. This helper is mostly used in two phases while running group resource integration tests :
 * <li> Preparing test set-up - Before running integration tests </li>
 * <li> Cleaning up set-up - After running integration tests
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class GroupHelper {

    private static final String GROUP_DESCRIPTION = "A group created for purpose of integration testing";
    private static final int MAX_PRINCIPALS_TO_FETCH = 1000;

    private CasIdmClient idmClient;

    public GroupHelper(CasIdmClient idmClient) {
        this.idmClient = idmClient;
    }

    public PrincipalId createGroup(String tenant, String groupName) throws Exception {
        GroupDetail groupDetail = new GroupDetail(GROUP_DESCRIPTION);
        return idmClient.addGroup(tenant, groupName, groupDetail);
    }

    public Group getGroup(String tenant, String groupName) throws Exception {
        return idmClient.findGroup(tenant, new PrincipalId(groupName, tenant));
    }

    public void deleteGroup(String tenant, String groupName) throws Exception {
        if (groupName.indexOf("@") != -1) {
            // Support UPN format
            groupName = PrincipalUtil.fromName(groupName).getName();
        }
        idmClient.deletePrincipal(tenant, groupName);
    }

    public void deleteGroups(String tenant, List<String> groups) throws Exception {
        for (String group : groups) {
            deleteGroup(tenant, group);
        }
    }

    public void addGroupToGroup(String tenant, String groupName, String targetGroup) throws Exception {
        PrincipalId groupID = new PrincipalId(groupName, tenant);
        idmClient.addGroupToGroup(tenant, groupID, targetGroup);
    }

    public Set<PersonUser> findPersonUsersInGroup(String tenant, String groupName, String searchString) throws Exception {
        PrincipalId groupID = new PrincipalId(groupName, tenant);
        SearchCriteria criteria = new SearchCriteria(searchString, tenant);
        return idmClient.findPersonUsersInGroup(tenant, groupID, criteria.getSearchString(), MAX_PRINCIPALS_TO_FETCH);
    }

    public Set<Group> findGroupsInGroup(String tenant, String groupName, String searchString) throws Exception {
        PrincipalId groupID = new PrincipalId(groupName, tenant);
        SearchCriteria criteria = new SearchCriteria(searchString, tenant);
        return idmClient.findGroupsInGroup(tenant, groupID, criteria.getSearchString(), MAX_PRINCIPALS_TO_FETCH);
    }


}
