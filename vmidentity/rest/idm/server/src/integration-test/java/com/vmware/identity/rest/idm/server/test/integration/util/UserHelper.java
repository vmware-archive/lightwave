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

import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.server.util.PrincipalUtil;

/**
 * Person user utility which helps calling IDM directly. This helper is mostly used in two phases while running user resource integration tests :
 * <li> Preparing test set-up - Before running integration tests </li>
 * <li> Cleaning up set-up - After running integration tests
 *
 * @author Balaji Boggaram Ramanarayan
 */
public class UserHelper {

    private static final String USER_PASSWORD = "Admin!23";
    private CasIdmClient idmClient;

    public UserHelper(CasIdmClient client) {
        this.idmClient = client;
    }

    public void createUser(String tenantName, PersonUser user) throws Exception {
        idmClient.addPersonUser(tenantName, user.getId().getName(), user.getDetail(), USER_PASSWORD.toCharArray());
    }

    public void deleteUser(String tenantName, String userName) throws Exception {
        if (userName.indexOf("@") != -1) {
            userName = PrincipalUtil.fromName(userName).getName();
        }
        idmClient.deletePrincipal(tenantName, userName);
    }

    public void deleteUsers(String tenantName, List<String> users) throws Exception {
        for (String user : users) {
            deleteUser(tenantName, user);
        }
    }

    public PersonUser findUser(String tenantName, String userName) throws Exception {
        PrincipalId user = new PrincipalId(userName, tenantName);
        return idmClient.findPersonUser(tenantName, user);
    }

    public void addUserToGroup(String tenant, String userName, String groupName) throws Exception {
        PrincipalId userID = new PrincipalId(userName, tenant);
        idmClient.addUserToGroup(tenant, userID, groupName);
    }

}
