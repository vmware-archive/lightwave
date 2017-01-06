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
package com.vmware.identity.idm.server.provider.vmwdirectory;

import java.util.List;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import javax.security.auth.login.LoginException;

import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.KnownSamlAttributes;
import com.vmware.identity.idm.LockoutPolicy;
import com.vmware.identity.idm.PasswordPolicy;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.Principal;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SearchResult;
import com.vmware.identity.idm.SecurityDomain;
import com.vmware.identity.idm.SolutionDetail;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.provider.PrincipalGroupLookupInfo;
import com.vmware.identity.idm.server.provider.vmwdirectory.VMwareDirectoryProvider;

public class SystemDomainAliasedProvider extends VMwareDirectoryProvider
{
    private final Map<String, String> _userAliases;
    private final Map<String, String> _userAliasesBackMap;

    public SystemDomainAliasedProvider(String tenantName, IIdentityStoreData store, Map<String, String> systemDomainUserAliases ) throws InvalidArgumentException
    {
        super(tenantName, store, true);

        ValidateUtil.validateNotEmpty(this.getStoreDataEx().getAlias(), "Alias must not be null.");

        Map<String, String> userMap = null;
        Map<String, String> userBackMap = null;
        if (systemDomainUserAliases != null )
        {
            userMap = new HashMap<String, String>(systemDomainUserAliases.size());
            userBackMap = new HashMap<String,String>(systemDomainUserAliases.size());
            for( String key : systemDomainUserAliases.keySet() )
            {
                if ( ServerUtils.isNullOrEmpty(key) == false )
                {
                    String value = systemDomainUserAliases.get(key);
                    if ( ServerUtils.isNullOrEmpty(value) == false )
                    {
                        if ( userMap.containsKey(key.toLowerCase()) )
                        {
                            throw new InvalidArgumentException(
                                 String.format("User mapping [%s] must be unique. It is case-insensitive.", key)
                            );
                        }
                        if ( userBackMap.containsKey(value.toLowerCase()) )
                        {
                            throw new InvalidArgumentException(
                                String.format("User mapping must be unique. Cannot map different accounts to the same one [%s].", value )
                            );
                        }
                    }
                    userMap.put(key.toLowerCase(), value );
                    userBackMap.put(value.toLowerCase(), key);
                }
            }
        }
        else
        {
            userMap = Collections.<String, String>emptyMap();
            userBackMap = Collections.<String, String>emptyMap();
        }

        this._userAliases = Collections.<String,String>unmodifiableMap(userMap);
        this._userAliasesBackMap = Collections.<String,String>unmodifiableMap(userBackMap);
    }

    @Override
    public String getName()
    {
        return super.getName();
    }

    @Override
    public String getAlias()
    {
        return this.getStoreDataEx().getAlias();
    }

    @Override
    public Set<String> getRegisteredUpnSuffixes()
    {
        return super.getRegisteredUpnSuffixes();
    }

    @Override
    public Collection<SecurityDomain> getDomains()
    {
        return super.getDomains();
    }

    @Override
    public PrincipalId authenticate(PrincipalId principal, String password)
            throws LoginException
    {
        return this.mapToAlias(
            super.authenticate(
                this.mapFromAlias(principal), password
            )
        );
    }

    @Override
    public Collection<AttributeValuePair> getAttributes(
            PrincipalId principalId, Collection<Attribute> attributes)
            throws Exception
    {
        Collection<AttributeValuePair> attrValues =
            super.getAttributes(
                this.mapFromAlias(principalId), attributes);
        if ( attrValues != null )
        {
            for(AttributeValuePair pair : attrValues )
            {
                if (KnownSamlAttributes.ATTRIBUTE_USER_GROUPS.equalsIgnoreCase(pair.getAttrDefinition().getName()))
                {
                    List<String> values = pair.getValues();
                    if ( values != null )
                    {
                        int count = values.size();
                        for( int i = 0; i < count; i++ )
                        {
                            String newVal = this.mapToAlias(values.get(i));
                            if ( newVal != values.get(i) )
                            {
                                values.add(newVal);
                            }
                        }
                    }
                }
            }
        }
        return attrValues;
    }

    @Override
    public PersonUser findUser(PrincipalId id) throws Exception
    {
        return this.<PersonUser>mapToAlias( super.findUser(this.mapFromAlias(id)) );
    }

    @Override
    public PersonUser findUserByObjectId(String userObjectId) throws Exception
    {
        return this.<PersonUser>mapToAlias(super.findUserByObjectId(userObjectId));
    }

    @Override
    public Set<PersonUser> findUsers(String searchString, String domainName, int limit)
            throws Exception
    {
        return this.<PersonUser>mapToAlias( super.findUsers(searchString, domainName, limit) );
    }

    @Override
    public Set<PersonUser> findUsersInGroup(
        PrincipalId groupId, String searchString, int limit) throws Exception
    {
        return this.<PersonUser>mapToAlias( super.findUsersInGroup(this.mapFromAlias(groupId), searchString, limit) );
    }

    @Override
    public Set<PersonUser> findDisabledUsers(String searchString, int limit)
            throws Exception
    {
        return this.<PersonUser>mapToAlias( super.findDisabledUsers(searchString, limit) );
    }

    @Override
    public Set<PersonUser> findLockedUsers(String searchString, int limit)
            throws Exception
    {
        return this.<PersonUser>mapToAlias( super.findLockedUsers(searchString, limit) );
    }

    @Override
    public PrincipalGroupLookupInfo findDirectParentGroups(PrincipalId principalId)
            throws Exception
    {
        PrincipalGroupLookupInfo info = super.findDirectParentGroups(this.mapFromAlias(principalId));
        return (info != null) ?
                new PrincipalGroupLookupInfo( this.<Group>mapToAlias(info.getGroups()), info.getPrincipalObjectId() ):
                null;
    }

    @Override
    public PrincipalGroupLookupInfo findNestedParentGroups(PrincipalId userId)
            throws Exception
    {
        PrincipalGroupLookupInfo info = super.findNestedParentGroups(this.mapFromAlias(userId));
        return (info != null) ?
               new PrincipalGroupLookupInfo( this.<Group>mapToAlias(info.getGroups()), info.getPrincipalObjectId() ):
               null;
    }

    @Override
    public Group findGroup(PrincipalId groupId) throws Exception
    {
        return this.<Group>mapToAlias(super.findGroup(this.mapFromAlias(groupId)));
    }

    @Override
    public Group findGroupByObjectId(String groupObjectId) throws Exception
    {
        return this.<Group>mapToAlias(super.findGroupByObjectId(groupObjectId));
    }

    @Override
    public Set<Group> findGroups(String searchString, String domainName, int limit)
            throws Exception
    {
        return this.<Group>mapToAlias(super.findGroups(searchString, domainName, limit));
    }

    @Override
    public Set<Group> findGroupsInGroup(PrincipalId groupId, String searchString, int limit)
            throws Exception
    {
        return this.<Group>mapToAlias(super.findGroupsInGroup(this.mapFromAlias(groupId), searchString, limit));
    }

    @Override
    public SearchResult find(String searchString, String domainName, int limit) throws Exception
    {
        return this.mapToAlias(super.find(searchString, domainName, limit));
    }

    @Override
    public boolean IsActive(PrincipalId id) throws Exception
    {
        return super.IsActive(this.mapFromAlias(id));
    }

    @Override
    public void checkUserAccountFlags(PrincipalId principalId)
            throws IDMException
    {
        super.checkUserAccountFlags(this.mapFromAlias(principalId));
    }

    @Override
    public byte[] getUserHashedPassword(PrincipalId principal) throws Exception
    {
        return super.getUserHashedPassword(this.mapFromAlias(principal));
    }

    @Override
    public PrincipalId addServicePrincipal(String accountName,
            SolutionDetail detail) throws Exception
    {
        return this.mapToAlias(super.addServicePrincipal(this.mapAccountNameFromAlias(accountName), detail));
    }

    @Override
    public SolutionUser findServicePrincipal(String accountName)
            throws Exception
    {
        return this.<SolutionUser>mapToAlias(super.findServicePrincipal(this.mapAccountNameFromAlias(accountName)));
    }

    @Override
    public SolutionUser findServicePrincipalInExternalTenant(String accountName)
            throws Exception
    {
        return this.<SolutionUser>mapToAlias(super.findServicePrincipalInExternalTenant(this.mapAccountNameFromAlias(accountName)));
    }

    @Override
    public Set<SolutionUser> findServicePrincipals(String searchString)
            throws Exception
    {
        return this.<SolutionUser>mapToAlias(super.findServicePrincipals(searchString));
    }

    @Override
    public Set<SolutionUser> findServicePrincipalsInExternalTenant(
            String searchString) throws Exception
    {
        return this.<SolutionUser>mapToAlias(super.findServicePrincipalsInExternalTenant(searchString));
    }

    @Override
    public SolutionUser findServicePrincipalByCertDn(String subjectDN)
            throws Exception
    {
        return this.<SolutionUser>mapToAlias(super.findServicePrincipalByCertDn(subjectDN));
    }

    @Override
    public SolutionUser findServicePrincipalByCertDnInExternalTenant(
            String subjectDN) throws Exception
    {
        return this.<SolutionUser>mapToAlias(super.findServicePrincipalByCertDnInExternalTenant(subjectDN));
    }

    @Override
    public Set<SolutionUser> findServicePrincipalsInGroup(String groupName,
            String searchString) throws Exception
    {
        return this.<SolutionUser>mapToAlias(super.findServicePrincipalsInGroup(this.mapAccountNameFromAlias(groupName), searchString));
    }

    @Override
    public Set<SolutionUser> findDisabledServicePrincipals(String searchString)
            throws Exception
    {
        return this.<SolutionUser>mapToAlias(super.findDisabledServicePrincipals(searchString));
    }

    @Override
    public Set<SolutionUser> findDisabledServicePrincipalsInExternalTenant(
            String searchString) throws Exception
    {
        return this.<SolutionUser>mapToAlias(super.findDisabledServicePrincipalsInExternalTenant(searchString));
    }

    @Override
    public PrincipalId addUser(String accoutName, PersonDetail detail,
            char[] password) throws Exception
    {
        return this.mapToAlias(super.addUser(this.mapAccountNameFromAlias(accoutName), detail, password));
    }

    @Override
    public PrincipalId addUser(String accountName, PersonDetail detail,
            byte[] hashedPassword, String hashingAlgorithm) throws Exception
    {
        return this.mapToAlias(super.addUser(this.mapAccountNameFromAlias(accountName), detail, hashedPassword, hashingAlgorithm));
    }

    @Override
    public boolean enableUserAccount(PrincipalId userId) throws Exception
    {
        return super.enableUserAccount(this.mapFromAlias(userId));
    }

    @Override
    public boolean disableUserAccount(PrincipalId userId) throws Exception
    {
        return super.disableUserAccount(this.mapFromAlias(userId));
    }

    @Override
    public boolean unlockUserAccount(PrincipalId userId) throws Exception
    {
        return super.unlockUserAccount(this.mapFromAlias(userId));
    }

    @Override
    public PrincipalId addGroup(String groupName, GroupDetail detail)
            throws Exception
    {
        return this.mapToAlias(super.addGroup(this.mapAccountNameFromAlias(groupName), detail));
    }

    @Override
    public boolean addUserToGroup(PrincipalId userId, String groupName)
            throws Exception
    {
        return super.addUserToGroup(this.mapFromAlias(userId), this.mapAccountNameFromAlias(groupName));
    }

    @Override
    public boolean addGroupToGroup(PrincipalId groupId, String groupName)
            throws Exception
    {
        return super.addGroupToGroup(this.mapFromAlias(groupId), this.mapAccountNameFromAlias(groupName));
    }

    @Override
    public boolean removeFromGroup(PrincipalId principalId, String groupName)
            throws Exception
    {
        return super.removeFromGroup(this.mapFromAlias(principalId), this.mapAccountNameFromAlias(groupName));
    }

    @Override
    public void deletePrincipal(String accountName) throws Exception
    {
        super.deletePrincipal(this.mapAccountNameFromAlias(accountName));
    }

    @Override
    public PrincipalId updatePersonUserDetail(String accountName,
            PersonDetail detail) throws Exception
    {
        return this.mapToAlias(super.updatePersonUserDetail(this.mapAccountNameFromAlias(accountName), detail));
    }

    @Override
    public PrincipalId updateServicePrincipalDetail(String accountName,
            SolutionDetail detail) throws Exception
    {
        return this.mapToAlias(super.updateServicePrincipalDetail(this.mapAccountNameFromAlias(accountName), detail));
    }

    @Override
    public PrincipalId updateServicePrincipalDetailInExternalTenant(
            String accountName, SolutionDetail detail) throws Exception
    {
        return this.mapToAlias(super.updateServicePrincipalDetailInExternalTenant(this.mapAccountNameFromAlias(accountName), detail));
    }

    @Override
    public PrincipalId updateGroupDetail(String groupName, GroupDetail detail)
            throws Exception
    {
        return this.mapToAlias(super.updateGroupDetail(this.mapAccountNameFromAlias(groupName), detail));
    }

    @Override
    public void resetUserPassword(String accountName, char[] newPassword)
            throws Exception
    {
        super.resetUserPassword(this.mapAccountNameFromAlias(accountName), newPassword);
    }

    @Override
    public void resetUserPassword(String accountName, char[] currentPassword,
            char[] newPassword) throws Exception
    {
        super.resetUserPassword(this.mapAccountNameFromAlias(accountName), currentPassword, newPassword);
    }

    @Override
    public PasswordPolicy getPasswordPolicy() throws Exception
    {
        return super.getPasswordPolicy();
    }

    @Override
    public void setPasswordPolicy(PasswordPolicy policy) throws Exception
    {
        super.setPasswordPolicy(policy);
    }

    @Override
    public LockoutPolicy getLockoutPolicy() throws Exception
    {
        return super.getLockoutPolicy();
    }

    @Override
    public void setLockoutPolicy(LockoutPolicy policy) throws Exception
    {
        super.setLockoutPolicy(policy);
    }

    @Override
    public boolean registerExternalIDPUser(String fspUserDN) throws Exception
    {
        return super.registerExternalIDPUser(fspUserDN);
    }

    @Override
    public boolean removeExternalIDPUser(String fspUserDN) throws Exception
    {
        return super.removeExternalIDPUser(fspUserDN);
    }

    @Override
    public boolean isObjectIdCandidate(String candidate)
    {
        return super.isObjectIdCandidate(candidate);
    }

    @Override
    public String getObjectId(String candidate)
    {
        return super.getObjectId(candidate);
    }

    @Override
    public String getObjectIdName(String objectId)
    {
        return super.getObjectIdName(objectId);
    }

    @Override
    public PrincipalId findExternalIDPUserRegistration(PrincipalId id)
            throws Exception
    {
        return this.mapToAlias(super.findExternalIDPUserRegistration(this.mapFromAlias(id)));
    }

    @Override
    public PrincipalId findActiveUser(String attributeName,
            String attributeValue) throws Exception
    {
        return super.findActiveUser(attributeName, attributeValue);
    }

    @Override
    public String getMappingSamlAttributeForGroupMembership()
    {
        return super.getMappingSamlAttributeForGroupMembership();
    }

    @Override
    public Group getEveryoneGroup()
    {
        return super.getEveryoneGroup();
    }

    @Override
    public List<String> findGroupsForFsps(List<String> fspIds) throws Exception
    {
        List<String> values = super.findGroupsForFsps(fspIds);
        if ( values != null )
        {
            int count = values.size();
            for( int i = 0; i < count; i++ )
            {
                String newVal = this.mapToAlias(values.get(i));
                if ( newVal != values.get(i) )
                {
                    values.add(newVal);
                }
            }
        }
        return values;
    }

    @Override
    public Set<PersonUser> findUsersByName(
        String searchString, String domainName, int limit)
    throws Exception
    {
        return this.<PersonUser>mapToAlias( super.findUsersByName(searchString, domainName, limit) );
    }

    @Override
    public Set<PersonUser> findUsersByNameInGroup(PrincipalId groupId,
        String searchString, int limit) throws Exception
    {
        return this.<PersonUser>mapToAlias( super.findUsersByNameInGroup(this.mapFromAlias(groupId), searchString, limit) );
    }

    @Override
    public Set<Group> findGroupsByName(String searchString, String domainName,
            int limit) throws Exception
    {
        return this.<Group>mapToAlias( super.findGroupsByName(searchString, domainName, limit) );
    }

    @Override
    public Set<Group> findGroupsByNameInGroup(PrincipalId groupId,
        String searchString, int limit) throws Exception
    {
        return this.<Group>mapToAlias( super.findGroupsByNameInGroup(this.mapFromAlias(groupId), searchString, limit) );
    }

    @Override
    public SearchResult findByName(String searchString, String domainName,
        int limit) throws Exception
    {
        return this.mapToAlias(super.findByName(searchString, domainName, limit) );
    }

    @Override
    protected PrincipalId getPrincipalAliasId(String accountName)
    {
        // we might need to map proper alias if the account is mapped...
        return ServerUtils.getPrincipalAliasId(this.mapAccountNameToAlias(accountName), this.getAlias());
    }

    @Override
    protected PrincipalId getPrincipalId(String upn, String accountName, String domainName)
    {
        // for backwards compatibility
        // we need to unify on eUpn for this provider
        // as VC/IS store permissions using old SYSTEM-DOMAIN
        // which is an alias and rely on lookups to return same data
        PrincipalId id = null;
        if ( ServerUtils.isNullOrEmpty(upn) == false )
        {
            String[] parts = upn.split("@");
            if (
                 ( parts.length == 2 ) &&
                 ( ServerUtils.isNullOrEmpty(parts[0]) == false ) &&
                 ( ServerUtils.isNullOrEmpty(parts[1]) == false ) )
            {
                id = new PrincipalId(parts[0], parts[1]);
            }
        }

        if ( id == null )
        {
            id = new PrincipalId( accountName, domainName );
        }
        return id;
    }

    private PrincipalId mapFromAlias(PrincipalId principal)
    {
        return mapAlias( principal, this.getAlias(), this.getDomain(), this._userAliases );
    }

    private PrincipalId mapToAlias(PrincipalId principal)
    {
        // by the new design this is a no-op
        // keeping possible hook in place for now, so if we need to switch later
        // it will be easier
        return principal;
    }

    private static PrincipalId mapAlias(
        PrincipalId principal, String matchFromDomain, String matchToDomain, Map<String, String> userMap)
    {
        PrincipalId id = principal;
        if ( principal != null )
        {
            // if the user name comes with the registered alias
            if ( principal.getDomain().equalsIgnoreCase(matchFromDomain) )
            {
                if ( userMap.containsKey(principal.getName().toLowerCase()) )
                {
                    // if also user name is a known alias, create a fully mapped user
                    id = new PrincipalId( userMap.get(principal.getName().toLowerCase()), matchToDomain );
                }
            }
        }
        return id;
    }

    private String mapAccountNameFromAlias(String accountName)
    {
        return mapAccountNameAlias(accountName, this._userAliases);
    }

    private String mapAccountNameToAlias(String accountName)
    {
        return mapAccountNameAlias(accountName, this._userAliasesBackMap);
    }

    private static String mapAccountNameAlias(String accountName, Map<String, String> lookupMap)
    {
        String mappedAccountName = accountName;
        if ( ServerUtils.isNullOrEmpty(accountName) == false )
        {
            if ( lookupMap.containsKey(accountName.toLowerCase()) )
            {
                // if also user name is a known alias, create a fully mapped user
                mappedAccountName = lookupMap.get(accountName.toLowerCase());
            }
        }
        return mappedAccountName;
    }

    private String mapToAlias(String principal)
    {
        return mapAlias(principal, this.getDomain(), this.getAlias(), this._userAliasesBackMap);
    }

    private static String mapAlias(
        String principal, String matchFromDomain, String matchToDomian, Map<String, String> userMap)
    {
        String mapped = null;
        if( principal != null )
        {
            // support either domain\\user or user@domain
            String formatString = null; // format string will assume param1 - user, param2 - domain
            String user = null;
            String domain = null;
            int separatorIndex = principal.indexOf(ValidateUtil.NETBIOS_SEPARATOR);
            if ( separatorIndex != -1 )
            {
                user = principal.substring(separatorIndex+1);
                domain = principal.substring(0, separatorIndex);
                formatString = String.format("%s%c%s", "%2$s", ValidateUtil.NETBIOS_SEPARATOR, "%1$s" );
            }
            else
            {
                separatorIndex = principal.indexOf(ValidateUtil.UPN_SEPARATOR);
                if ( separatorIndex!= -1 )
                {
                    user = principal.substring(0, separatorIndex);
                    domain = principal.substring(separatorIndex+1);
                    formatString = String.format("%s%c%s", "%1$s", ValidateUtil.UPN_SEPARATOR, "%2$s" );
                }
            }

            if ( separatorIndex != -1 )
            {
                if ( matchFromDomain.equalsIgnoreCase(domain) )
                {
                    if ( userMap.containsKey(user.toLowerCase()) )
                    {
                        user = userMap.get(user.toLowerCase());
                    }
                    domain = matchToDomian;
                }
                mapped = String.format(formatString, user, domain);
            }
            else
            {
                mapped = principal;
            }
        }

        return mapped;
    }

    private SearchResult mapToAlias(SearchResult searchResult)
    {
        // by the new design this is a no-op
        // keeping possible hook in place for now, so if we need to switch later
        // it will be easier

        SearchResult result = searchResult;
        return result;
    }

/*  private PersonUser buildAliased( PrincipalId aliasedId, PersonUser personUser )
    {
        return new PersonUser(
            aliasedId, personUser.getId(), personUser.getObjectId(),
            personUser.getDetail(), personUser.isDisabled(), personUser.isLocked());
    }

    private SolutionUser buildAliased( PrincipalId aliasedId, SolutionUser solutionUser )
    {
        return new SolutionUser(
            aliasedId, solutionUser.getId(), solutionUser.getObjectId(), solutionUser.getDetail(),
            solutionUser.isDisabled(), solutionUser.isExternal());
    }

    private Group buildAliased( PrincipalId aliasedId, Group group )
    {
        return new Group(aliasedId, group.getId(), group.getObjectId(), group.getDetail() );
    }
*/
    private <T extends Principal> Set<T> mapToAlias(Set<T> originalSet)
    {
        // by the new design this is a no-op
        // keeping possible hook in place for now, so if we need to switch later
        // it will be easier
        Set<T> finalSet = originalSet;
/*        if ( originalSet != null )
        {
            finalSet = new HashSet<T>(originalSet.size());
            for( T obj : originalSet )
            {
                finalSet.add(this.<T>mapToAlias(obj));
            }
        }
*/        return finalSet;
    }


    private <T extends Principal> T mapToAlias( T principal )
    {
        // by the new design this is a no-op
        // keeping possible hook in place for now, so if we need to switch later
        // it will be easier
        T result = principal;
/*        if ( principal != null )
        {
            PrincipalId aliasedId = this.mapToAlias(principal.getId());
            if ( aliasedId != principal.getId() )
            {
                if ( principal instanceof PersonUser )
                {
                    result = (T)this.buildAliased(aliasedId, (PersonUser)principal);
                }
                else if ( principal instanceof SolutionUser )
                {
                    result = (T)this.buildAliased(aliasedId, (SolutionUser)principal);
                }
                else if (principal instanceof Group)
                {
                    result = (T)this.buildAliased(aliasedId, (Group)principal);
                }
                else
                {
                    throw new RuntimeException( "Unexpected principal type:" + principal.getClass().getName() );
                }
            }
        }
*/
        return result;
    }
}
