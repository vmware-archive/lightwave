/*
 *  Copyright (c) 2018 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.openidconnect.server;

import java.util.Collection;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.TokenClaimAttribute;
import com.vmware.identity.idm.client.CasIdmClient;

public class FederatedIdentityProvider {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(FederatedIdentityProvider.class);

    private final CasIdmClient idmClient;
    private final String tenant;
    private static final String CLAIM_PERMS = "perms";

    public FederatedIdentityProvider(String tenant, CasIdmClient idmClient) {
        Validate.notNull(idmClient, "idm client");
        Validate.notEmpty(tenant, "tenant name");
        this.tenant = tenant;
        this.idmClient = idmClient;
    }

    public boolean isFederationUserActive(PrincipalId userId) {
        Validate.notNull(userId, "user id");

        try {
          return idmClient.isActive(tenant, userId);
        } catch (Exception ex) {
          return false;
        }
    }

    public PrincipalId provisionFederationUser(String issuer, PrincipalId userId) throws Exception {
        Validate.notNull(userId, "userId");
        Validate.notEmpty(issuer, "issuer");

        String systemDomain = idmClient.getProviders(tenant, EnumSet.of(DomainType.SYSTEM_DOMAIN)).iterator().next().getName();
        if (systemDomain == null || systemDomain.isEmpty()) {
            throw new IllegalStateException("The system domain is invalid");
        }
        idmClient.registerUpnSuffix(tenant, systemDomain, userId.getDomain());
        // generate a unique user name in order not to conflict with local users
        String userName = userId.getName() + "-" + userId.getDomain();
        PrincipalId internalUserId = idmClient.addJitUser(tenant, userName,
                new PersonDetail.Builder()
                .userPrincipalName(userId.getUPN())
                .description("A JIT user account created for federated IDP.")
                .build(),
                issuer,
                userId.getUPN());

        logger.info("JIT user {} successfully added to tenant {}.", userId.getUPN(), this.tenant);
        return internalUserId;
    }

    public void updateUserGroups(PrincipalId userId, Collection<String> permissions,
            Map<TokenClaimAttribute, List<String>> roleGroupMappings) throws Exception {
        Validate.notNull(userId, "userId");
        Validate.notNull(permissions, "permissions");
        Validate.notNull(roleGroupMappings, "roleGroupMappings");

        Collection<String> permissionGroupsFromToken = getGroupNamesFromPermissionRoles(permissions, roleGroupMappings);
        Collection<String> existingGroups = getGroupNames(idmClient.findDirectParentGroups(tenant, userId));
        Collection<String> groupsFromIDPConfig = new HashSet<>();
        for (List<String> value : roleGroupMappings.values()) {
            groupsFromIDPConfig.addAll(value);
        }

        // add user to the groups based on permissions of token
        for (String group : permissionGroupsFromToken) {
            if (!existingGroups.contains(group)) {
                try {
                    idmClient.addUserToGroup(tenant, userId, group);
                } catch (Exception e) {
                    logger.warn("Encountered an error while adding user {} to group {} in tenant {}", userId.getUPN(),
                            group, tenant, e);
                }
            }
        }

        // delete user from the groups based on permissions of token
        for (String group : groupsFromIDPConfig) {
            if (existingGroups.contains(group) && !permissionGroupsFromToken.contains(group)) {
                try {
                    idmClient.removeFromLocalGroup(tenant, userId, group);
                } catch (Exception e) {
                    logger.warn("Encountered an error while deleting user {} from group {} in tenant {}", userId.getUPN(),
                            group, tenant, e);
                }
            }
        }
    }

    private static Collection<String> getGroupNamesFromPermissionRoles(Collection<String> permissions, Map<TokenClaimAttribute, List<String>> roleGroupMappings) {
        Collection<String> groups = new HashSet<>();
        if (roleGroupMappings != null) {
            for (String perm : permissions) {
                TokenClaimAttribute claim = new TokenClaimAttribute(CLAIM_PERMS, perm);
                List<String> mappedGroups = roleGroupMappings.get(claim);
                if (mappedGroups != null) {
                    groups.addAll(mappedGroups);
                }
            }
        }
        return groups;
    }

    private static Collection<String> getGroupNames(Collection<Group> groups) {
        Collection<String> groupNames = new HashSet<>();
        if (groups != null) {
            for (Group group : groups) {
                groupNames.add(group.getName());
            }
        }
        groupNames.remove("Everyone");
        return groupNames;
    }
}
