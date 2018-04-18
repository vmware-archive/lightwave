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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.KnownSamlAttributes;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.Scope;
import com.vmware.identity.openidconnect.common.ScopeValue;

/**
 * @author Yehia Zayour
 */
public class UserInfoRetriever {
    private final CasIdmClient idmClient;

    public UserInfoRetriever(CasIdmClient idmClient) {
        Validate.notNull(idmClient, "idmClient");
        this.idmClient = idmClient;
    }

    public UserInfo retrieveUserInfo(
            User user,
            Scope scope,
            Set<ResourceServerInfo> resourceServerInfos) throws ServerException {
        Validate.notNull(user, "user");
        Validate.notNull(scope, "scope");
        Validate.notNull(resourceServerInfos, "resourceServerInfos");

        if (!isEnabled(user)) {
            throw new ServerException(ErrorObject.accessDenied("user has been disabled or deleted"));
        }

        String upn = null;
        String givenName = null;
        String familyName = null;
        List<String> groupMembership = null;

        Collection<Attribute> attributes = new ArrayList<>();
        attributes.add(new Attribute(KnownSamlAttributes.ATTRIBUTE_USER_PRINCIPAL_NAME));

        if (user instanceof PersonUser) {
            attributes.add(new Attribute(KnownSamlAttributes.ATTRIBUTE_USER_FIRST_NAME));
            attributes.add(new Attribute(KnownSamlAttributes.ATTRIBUTE_USER_LAST_NAME));
        }

        if (
                scope.contains(ScopeValue.ID_TOKEN_GROUPS) ||
                scope.contains(ScopeValue.ID_TOKEN_GROUPS_FILTERED) ||
                scope.contains(ScopeValue.ACCESS_TOKEN_GROUPS) ||
                scope.contains(ScopeValue.ACCESS_TOKEN_GROUPS_FILTERED) ||
                scope.contains(ScopeValue.RESOURCE_SERVER_ADMIN_SERVER)) {
            attributes.add(new Attribute(KnownSamlAttributes.ATTRIBUTE_USER_GROUPS));
        }


        Collection<AttributeValuePair> attributeValuePairs = getAttributes(user, attributes);

        for (AttributeValuePair entry : attributeValuePairs) {
            String attributeName = entry.getAttrDefinition().getName();
            if (attributeName.equals(KnownSamlAttributes.ATTRIBUTE_USER_FIRST_NAME)) {
                givenName = entry.getValues().isEmpty() ? null : entry.getValues().get(0);
            } else if (attributeName.equals(KnownSamlAttributes.ATTRIBUTE_USER_LAST_NAME)) {
                familyName = entry.getValues().isEmpty() ? null : entry.getValues().get(0);
            } else if (attributeName.equals(KnownSamlAttributes.ATTRIBUTE_USER_GROUPS)) {
                groupMembership = entry.getValues();
            } else if (attributeName.equals(KnownSamlAttributes.ATTRIBUTE_USER_PRINCIPAL_NAME)) {
                ValidateUtil.validateNotEmpty(entry.getValues(), "subject upn");
                upn = entry.getValues().get(0);
            }
        }

        String adminServerRole = null;
        if (scope.contains(ScopeValue.RESOURCE_SERVER_ADMIN_SERVER)) {
            adminServerRole = computeAdminServerRole(user, groupMembership);
        }

        boolean filteredGroupsRequested =
                scope.contains(ScopeValue.ID_TOKEN_GROUPS_FILTERED) ||
                scope.contains(ScopeValue.ACCESS_TOKEN_GROUPS_FILTERED);

        boolean shouldComputeFilteredGroups = false;
        if (filteredGroupsRequested && !resourceServerInfos.isEmpty()) {
            boolean emptyFilterFound = false;
            for (ResourceServerInfo rsInfo : resourceServerInfos) {
                if (rsInfo.getGroupFilter().isEmpty()) {
                    emptyFilterFound = true;
                    break;
                }
            }
            if (!emptyFilterFound) {
                shouldComputeFilteredGroups = true;
            }
        }

        Set<String> groupMembershipFiltered = null;
        if (shouldComputeFilteredGroups) {
            groupMembershipFiltered = computeGroupMembershipFiltered(groupMembership, resourceServerInfos);
        }

        return new UserInfo(upn, groupMembership, groupMembershipFiltered, adminServerRole, givenName, familyName);
    }

    public boolean isMemberOfGroup(User user, String group) throws ServerException {
        Validate.notNull(user, "user");
        Validate.notEmpty(group, "group");
        try {
            return this.idmClient.isMemberOfSystemGroup(user.getTenant(), user.getPrincipalId(), group);
        } catch (Exception e) {
            throw new ServerException(ErrorObject.serverError("idm error while checking is member of system group"), e);
        }
    }

    private boolean isEnabled(User user) throws ServerException {
        boolean enabled;
        try {
            enabled = this.idmClient.isActive(user.getTenant(), user.getPrincipalId());
        } catch (InvalidPrincipalException e) {
            enabled = false;
        } catch (Exception e) {
            throw new ServerException(ErrorObject.serverError("idm error while checking isActive status"), e);
        }
        return enabled;
    }

    private Set<String> computeGroupMembershipFiltered(List<String> groupMembership, Set<ResourceServerInfo> resourceServerInfos) {
        // 1. result = {union of all filters}
        Set<String> result = new HashSet<String>();
        for (ResourceServerInfo rsInfo : resourceServerInfos) {
            assert !rsInfo.getGroupFilter().isEmpty();
            Set<String> groupFilterLowerCase = toLowerCase(rsInfo.getGroupFilter());
            result.addAll(groupFilterLowerCase);
        }

        // 2. result = intersection of {union of all filters} with groupMembership
        Set<String> groupMembershipLowerCase = toLowerCase(groupMembership);
        result.retainAll(groupMembershipLowerCase);
        return result;
    }

    private Collection<AttributeValuePair> getAttributes(User user, Collection<Attribute> attributes) throws ServerException {
        if (attributes == null || attributes.isEmpty()) {
            return Collections.emptyList();
        }

        ValidateUtil.validateNotNull(user, "user");
        try {
            Collection<AttributeValuePair> attributeValuePairs = this.idmClient.getAttributeValues(
                    user.getTenant(),
                    user.getPrincipalId(),
                    attributes);
            ValidateUtil.validateNotNull(attributeValuePairs, "attribute value pairs");
            return attributeValuePairs;
        } catch (Exception e) {
            throw new ServerException(ErrorObject.serverError("idm error while retrieving user attributes"), e);
        }
    }

    private String computeAdminServerRole(User user, List<String> groupMembership) throws ServerException {
        String systemDomainName = getSystemDomainName(user);
        String groupNamePrefix = systemDomainName.toLowerCase() + "\\";
        String systemTenantGroupNamePrefix = getSystemDomainName(getSystemTenantName()).toLowerCase() + "\\";
        Set<String> groupMembershipLowerCase = toLowerCase(groupMembership);

        String role;
        if (groupMembershipLowerCase.contains(groupNamePrefix + "administrators")) {
            role = "Administrator";
        } else if (groupMembershipLowerCase.contains(systemTenantGroupNamePrefix + "systemconfiguration.administrators")) {
            role = "ConfigurationUser";
        } else if (groupMembershipLowerCase.contains(systemTenantGroupNamePrefix + "tenantoperators")) {
            role = "TenantOperator";
        } else if (groupMembershipLowerCase.contains(groupNamePrefix + "trustedusers")) {
            role = "TrustedUser";
        } else if (groupMembershipLowerCase.contains(groupNamePrefix + "users")) {
            role = "RegularUser";
        } else {
            role = "GuestUser";
        }
        return role;
    }

    private String getSystemDomainName(User user) throws ServerException {
        return getSystemDomainName(user.getTenant());
    }

    private String getSystemTenantName() throws ServerException {
        String systemTenant;
        try {
            systemTenant = this.idmClient.getSystemTenant();
        } catch (Exception e) {
            throw new ServerException(ErrorObject.serverError("idm error while retrieving system tenant"), e);
        }

        return systemTenant;
    }

    private String getSystemDomainName(String tenant) throws ServerException {
        Collection<IIdentityStoreData> identityStores;
        try {
            identityStores = this.idmClient.getProviders(tenant, EnumSet.of(DomainType.SYSTEM_DOMAIN));
        } catch (Exception e) {
            throw new ServerException(ErrorObject.serverError("idm error while retrieving system domain"), e);
        }
        assert identityStores != null && identityStores.size() == 1;
        IIdentityStoreData identityStore = identityStores.iterator().next();
        return identityStore.getName();
    }

    private static Set<String> toLowerCase(Collection<String> collection) {
        Set<String> result = new HashSet<String>();
        if (collection != null) {
            for (String element : collection) {
                result.add(element.toLowerCase());
            }
        }
        return result;
    }
}
