/*
 *
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
 *
 */

/**
 * VMware Identity Service
 *
 * Identity Provider Interface
 *
 * @author:  Sriram Nambakam <snambakam@vmware.com>
 *
 * @version: 1.0
 * @since:   2011-12-7
 *
 */

package com.vmware.identity.idm.server.provider;

import java.util.Collection;
import java.util.Set;

import javax.security.auth.login.LoginException;

import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SearchResult;
import com.vmware.identity.idm.SecurityDomain;

public interface IIdentityProvider
{
    String getName();
    String getDomain();
    String getAlias();
    Set<String> getRegisteredUpnSuffixes();

    Collection<SecurityDomain> getDomains();

    PrincipalId
    authenticate(PrincipalId principal, String password) throws LoginException;

    Collection<AttributeValuePair>
    getAttributes(
        PrincipalId           principalId,
        Collection<Attribute> attributes
        ) throws Exception;

    PersonUser findUser(PrincipalId id) throws Exception;

    PersonUser findUserByObjectId(String userObjectId) throws Exception;

    Set<PersonUser> findUsers(String searchString, String domainName, int limit) throws Exception;

    Set<PersonUser> findUsersByName(String searchString, String domainName, int limit) throws Exception;

    Set<PersonUser> findUsersInGroup(PrincipalId groupId, String searchString, int limit) throws Exception;

    Set<PersonUser> findUsersByNameInGroup(PrincipalId groupId, String searchString, int limit) throws Exception;

    Set<PersonUser> findDisabledUsers(String searchString, int limit) throws Exception;

    Set<PersonUser> findLockedUsers(String searchString, int limit) throws Exception;

    PrincipalGroupLookupInfo findDirectParentGroups(PrincipalId principalId) throws Exception;

    PrincipalGroupLookupInfo findNestedParentGroups(PrincipalId userId) throws Exception;

    Group findGroup(PrincipalId groupId) throws Exception;

    Group findGroupByObjectId(String groupObjectId) throws Exception;

    Set<Group> findGroups(String searchString, String domainName, int limit) throws Exception;

    Set<Group> findGroupsByName(String searchString, String domainName, int limit) throws Exception;

    Set<Group> findGroupsInGroup(PrincipalId groupId, String searchString, int limit) throws Exception;

    Set<Group> findGroupsByNameInGroup(PrincipalId groupId, String searchString, int limit) throws Exception;

    SearchResult find(String searchString, String domainName, int limit) throws Exception;

    SearchResult findByName(String searchString, String domainName, int limit) throws Exception;

    boolean IsActive(PrincipalId id) throws Exception;

    void checkUserAccountFlags(PrincipalId principalId) throws IDMException;

    /**
     * Search by attribute to return one user.
     * 
     * @param attributeName
     * @param attributeValue
     * @return
     * @throws Exception
     *             no principal found or can not uniquely map to a principal.
     */
    PrincipalId findActiveUser(String attributeName, String attributeValue) throws Exception;

    /**
     * new version of findActiveUser that can return more than one user account.
     * This allows using attribute that does not guarantee uniquely identify
     * user.
     *
     * @param attributeName
     * @param attributeValue
     * @param userDomain
     *            The actual domain to search.
     * @param additionalAttribute
     *            Additional attribute to retrieve
     * @return
     * @throws Exception
     */
    UserSet findActiveUsersInDomain(String attributeName, String attributeValue
            , String userDomain, String additionalAttribute)
            throws Exception;

    /**
     * @return attribute name mapped to UserAttributePrincipalName
     */
    String getStoreUPNAttributeName();

    /**
     * LDAP attribute name that to be mapped to user name hint provided in client certificate authentication
     * @return
     * @throws IDMException
     */
    String getStoreUserHintAttributeName() throws IDMException;
    /**
     * @return mapping user certificate using principal name field in SAN to account's UPN attribute.
     */
    boolean getCertificateMappingUseUPN();
}

