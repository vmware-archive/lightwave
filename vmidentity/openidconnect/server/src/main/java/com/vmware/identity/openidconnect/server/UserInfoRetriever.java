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

package com.vmware.identity.openidconnect.server;

import java.util.Collection;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang3.Validate;

import com.nimbusds.oauth2.sdk.OAuth2Error;
import com.nimbusds.oauth2.sdk.Scope;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.KnownSamlAttributes;

/**
 * @author Yehia Zayour
 */
public class UserInfoRetriever {
    private final IdmClient idmClient;

    public UserInfoRetriever(IdmClient idmClient) {
        this.idmClient = idmClient;
    }

    public UserInformation retrieveUserInfo(User user, Scope scope) throws ServerException {
        Validate.notNull(user, "user");
        Validate.notNull(scope, "scope");

        if (!isEnabled(user)) {
            throw new ServerException(OAuth2Error.ACCESS_DENIED.setDescription("user has been disabled or deleted"));
        }

        List<String> groupMembership = null;
        String adminServerRole = null;
        String givenName = null;
        String familyName = null;

        if (user instanceof PersonUser) {
            com.vmware.identity.idm.PersonUser idmPersonUser;
            try {
                idmPersonUser = this.idmClient.findPersonUser(user.getTenant(), user.getPrincipalId());
            } catch (Exception e) {
                throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("idm error while retrieving person user"), e);
            }
            if (idmPersonUser == null) {
                throw new ServerException(OAuth2Error.INVALID_REQUEST.setDescription("person user with specified id not found"));
            }
            givenName = idmPersonUser.getDetail().getFirstName();
            familyName = idmPersonUser.getDetail().getLastName();
        }

        if (scope.contains(ScopeValue.ID_TOKEN_GROUPS.getName()) || scope.contains(ScopeValue.ACCESS_TOKEN_GROUPS.getName())) {
            groupMembership = computeGroupMembership(user);
        }

        if (scope.contains(ScopeValue.RESOURCE_SERVER_ADMIN_SERVER.getName())) {
            if (groupMembership == null) {
                groupMembership = computeGroupMembership(user);
            }
            adminServerRole = computeAdminServerRole(user.getTenant(), groupMembership);
        }

        return new UserInformation(groupMembership, adminServerRole, givenName, familyName);
    }

    public boolean isMemberOfActAsGroup(SolutionUser solutionUser) throws ServerException {
        Validate.notNull(solutionUser, "solutionUser");
        try {
            return this.idmClient.isMemberOfSystemGroup(solutionUser.getTenant(), solutionUser.getPrincipalId(), "ActAsUsers");
        } catch (Exception e) {
            throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("idm error while checking is member of ActAsUsers"), e);
        }
    }

    private boolean isEnabled(User user) throws ServerException {
        boolean enabled;
        try {
            enabled = this.idmClient.isActive(user.getTenant(), user.getPrincipalId());
        } catch (InvalidPrincipalException e) {
            enabled = false;
        } catch (Exception e) {
            throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("idm error while checking isActive status"), e);
        }
        return enabled;
    }

    private List<String> computeGroupMembership(User user) throws ServerException {
        Collection<Attribute> attributes = new HashSet<Attribute>();
        attributes.add(new Attribute(KnownSamlAttributes.ATTRIBUTE_USER_GROUPS));
        Collection<AttributeValuePair> attributeValuePairs;
        try {
            attributeValuePairs = this.idmClient.getAttributeValues(
                    user.getTenant(),
                    user.getPrincipalId(),
                    attributes);
        } catch (Exception e) {
            throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("idm error while retrieving group membership"), e);
        }
        Object[] pairsArray = attributeValuePairs.toArray();
        assert pairsArray.length == 1;
        AttributeValuePair attributeValuePair = (AttributeValuePair) pairsArray[0];
        return attributeValuePair.getValues();
    }

    private String computeAdminServerRole(String tenant, List<String> groupMembership) throws ServerException {
        String systemDomainName = getSystemDomainName(tenant).toLowerCase() + "\\";
        Set<String> groupsSet = new HashSet<String>();
        for (String group : groupMembership) {
            groupsSet.add(group.toLowerCase());
        }

        String role;
        if (groupsSet.contains(systemDomainName + "administrators")) {
            role = "Administrator";
        } else if (groupsSet.contains(systemDomainName + "systemconfiguration.administrators")) {
            role = "ConfigurationUser";
        } else if (groupsSet.contains(systemDomainName + "users")) {
            role = "RegularUser";
        } else {
            role = "GuestUser";
        }
        return role;
    }

    private String getSystemDomainName(String tenant) throws ServerException {
        String systemDomainName = "";
        Collection<IIdentityStoreData> identityStores;
        try {
            identityStores = this.idmClient.getProviders(tenant, EnumSet.of(DomainType.SYSTEM_DOMAIN));
        } catch (Exception e) {
            throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("idm error while retrieving system domain"), e);
        }
        if ((identityStores != null) && (identityStores.size() > 0)) {
            systemDomainName = identityStores.iterator().next().getName();
        }
        return systemDomainName;
    }
}
