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

package com.vmware.identity.idm.server.provider;
import java.util.List;
import java.util.Set;

import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.LockoutPolicy;
import com.vmware.identity.idm.PasswordPolicy;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SolutionDetail;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.idm.VmHostData;

public interface ISystemDomainIdentityProvider extends IIdentityProvider
{
    byte[] getUserHashedPassword(PrincipalId principal) throws Exception;

    PrincipalId
    addServicePrincipal(
            String         accountName,
            SolutionDetail detail
    ) throws Exception;

    SolutionUser findServicePrincipal(String accountName) throws Exception;

    SolutionUser findServicePrincipalInExternalTenant(String accountName) throws Exception;

    public Set<SolutionUser> findServicePrincipals(String searchString) throws Exception;

    public Set<SolutionUser> findServicePrincipalsInExternalTenant(String searchString) throws Exception;

    public SolutionUser findServicePrincipalByCertDn(String subjectDN) throws Exception;

    public SolutionUser findServicePrincipalByCertDnInExternalTenant(String subjectDN) throws Exception;

    public Set<SolutionUser> findServicePrincipalsInGroup(String groupName, String searchString)
            throws Exception;

    public Set<SolutionUser> findDisabledServicePrincipals(String searchString) throws Exception;

    public Set<SolutionUser> findDisabledServicePrincipalsInExternalTenant(String searchString) throws Exception;

    PrincipalId addUser(String accoutName,
                        PersonDetail detail, char[] password) throws Exception;

    /**
     * Adds a regular/jit user to the system domain.
     * If extIdpEntityId is non-null, this user is a jit user.
     * extUserId must be provided for jit user provisioning.
     *
     * @param accountName Account name of regular/jit user. required, non-null,
     * @param detail Detailed information about the user. required, non-null.
     * @param password User's password
     * @param extIdpEntityId ExternalIDP entity ID. If it is non-null, add jit user.
     * @param extUserId External User's ID. Required attribute for jit user.
     * @return Principal id of the regular user after it has been created.
     * @throws Exception
     */
    PrincipalId addUser(String accoutName,
            PersonDetail detail, char[] password, String extIdpEntityId, String extUserId) throws Exception;

    PrincipalId addUser(String accountName, PersonDetail detail,
                        byte[] hashedPassword, String hashingAlgorithm) throws Exception;

    boolean enableUserAccount(PrincipalId userId) throws Exception;

    boolean disableUserAccount(PrincipalId userId) throws Exception;

    boolean unlockUserAccount(PrincipalId userId) throws Exception;

    PrincipalId addGroup(String groupName,
                         GroupDetail detail) throws Exception;

    boolean addUserToGroup(PrincipalId userId, String groupName)
            throws Exception;

    boolean addGroupToGroup(PrincipalId groupId, String groupName)
            throws Exception;

    boolean removeFromGroup(PrincipalId principalId, String groupName)
            throws Exception;

    void deletePrincipal(String accountName) throws Exception;

    void deleteJitUsers(String extIdpEntityId) throws Exception;

    PrincipalId updatePersonUserDetail(String accountName, PersonDetail detail)
            throws Exception;

    PrincipalId updateServicePrincipalDetail(String accountName,
            SolutionDetail detail) throws Exception;

    PrincipalId updateServicePrincipalDetailInExternalTenant(String accountName,
            SolutionDetail detail) throws Exception;

    PrincipalId updateGroupDetail(String groupName, GroupDetail detail)
            throws Exception;

    void resetUserPassword(String accountName, char[] newPassword)
            throws Exception;

    void resetUserPassword(String accountName, char[] currentPassword,
            char[] newPassword) throws Exception;

    PasswordPolicy getPasswordPolicy() throws Exception;

    void setPasswordPolicy(PasswordPolicy policy) throws Exception;

    LockoutPolicy getLockoutPolicy() throws Exception;

    void setLockoutPolicy(LockoutPolicy policy) throws Exception;

    boolean registerExternalIDPUser(String fspUserDN) throws Exception;

    boolean removeExternalIDPUser(String fspUserDN) throws Exception;

    boolean isObjectIdCandidate(String candidate);

    String getObjectId(String candidate);

    String getObjectIdName(String objectId);

    PrincipalId findExternalIDPUserRegistration(PrincipalId id) throws Exception;

    PrincipalId findActiveUser( String attributeName, String attributeValue ) throws Exception;

    String getMappingSamlAttributeForGroupMembership();

    Group getEveryoneGroup();

    String getDomainId() throws Exception;

    String getSiteId() throws Exception;

    boolean doesContainerExist(String containerName) throws Exception;

    void addContainer(String containerName) throws Exception;

    //GroupName is in domainFqdn\\groupName
    List<String> findGroupsForFsps(List<String> fspIds) throws Exception;

    List<Group> findGroupObjectsForFsps(List<String> fspIds) throws Exception;

    List<VmHostData> getComputers(boolean getDCOnly) throws Exception;
}
