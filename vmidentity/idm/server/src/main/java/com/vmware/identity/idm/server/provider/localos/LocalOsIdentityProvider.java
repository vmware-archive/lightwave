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

package com.vmware.identity.idm.server.provider.localos;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.security.auth.login.LoginException;

import org.apache.commons.lang.Validate;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.PasswordExpiredException;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SearchResult;
import com.vmware.identity.idm.SecurityDomain;
import com.vmware.identity.idm.UserAccountLockedException;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.IdentityManager;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.provider.IIdentityProvider;
import com.vmware.identity.idm.server.provider.NoSuchGroupException;
import com.vmware.identity.idm.server.provider.NoSuchUserException;
import com.vmware.identity.idm.server.provider.PrincipalGroupLookupInfo;
import com.vmware.identity.idm.server.provider.UserSet;
import com.vmware.identity.interop.ossam.GroupInfo;
import com.vmware.identity.interop.ossam.IOsSamAdapter;
import com.vmware.identity.interop.ossam.OsSamAdapterFactory;
import com.vmware.identity.interop.ossam.OsSamConstants;
import com.vmware.identity.interop.ossam.OsSamGroupNotFoundException;
import com.vmware.identity.interop.ossam.OsSamUserNotFoundException;
import com.vmware.identity.interop.ossam.UserInfo;

public class LocalOsIdentityProvider implements IIdentityProvider
{
    public static final String FIRST_NAME_ATTRIBUTE = "FirstName";
    public static final String LAST_NAME_ATTRIBUTE = "LastName";
    public static final String GROUPS_ATTRIBUTE = "GroupsName";
    public static final String SUBJECT_TYPE_ATTRIBUTE = "SubjectType";
    public static final String USER_PRINCIPAL_NAME_ATTRIBUTE = "UserPrincipalName";
    public static final String EMAIL_ATTRIBUTE = "Email";
    public static final char UPN_SEPARATOR = '@';

    private final IIdentityStoreData _store;

    private static final Log logger = LogFactory
          .getLog(LocalOsIdentityProvider.class);

    public LocalOsIdentityProvider(IIdentityStoreData store)
    {
        Validate.notNull( store, "store Must not be null." );
        Validate.notNull(
            store.getExtendedIdentityStoreData(), "store.getExtendedIdentityStoreData() Must not be null." );

        Validate.notEmpty( store.getName(), "IIdentityStoreData MUST have a name." );
        Validate.isTrue(
                store.getDomainType() == DomainType.LOCAL_OS_DOMAIN,
                "IIdentityStoreData must represent a 'LOCAL_OS_DOMAIN'.");

        Validate.isTrue(
                store.getExtendedIdentityStoreData().getProviderType() ==
                        IdentityStoreType.IDENTITY_STORE_TYPE_LOCAL_OS,
                "IIdentityStoreData must represent a store of " +
                        "'IDENTITY_STORE_TYPE_LOCAL_OS' type.");

        Validate.notNull(
                store.getExtendedIdentityStoreData().getAttributeMap(),
                "store.getExtendedIdentityStoreData().getAttributeMap() Must not be null.");

        this._store = store;
    }

    @Override
    public String getName()
    {
        return this._store.getName();
    }

    @Override
    public String getDomain()
    {
        return this._store.getName();
    }

    @Override
    public String getAlias()
    {
        return this._store.getExtendedIdentityStoreData().getAlias();
    }

    @Override
    public Set<String> getRegisteredUpnSuffixes()
    {
        return Collections.<String>emptySet();
    }

    @Override
    public PrincipalId authenticate(PrincipalId principal, String password) throws LoginException
    {
        this.validatePrincipal( principal );
        ValidateUtil.validateNotNull( password, "password" );

        principal = ServerUtils.normalizeAliasInPrincipal(principal, this.getDomain(), this.getAlias());

        IOsSamAdapter samAdapter = getSamAdapter();

        try
        {
            samAdapter.LogonUser( principal.getName(), password );

            return ServerUtils.getPrincipalId(null, principal.getName(), this.getDomainName());
        }
        catch(Exception ex)
        {
            throw new LoginException( ex.getMessage() );
        }
    }

    @Override
    public
    Collection<AttributeValuePair>
    getAttributes(
        PrincipalId           principalId,
        Collection<Attribute> attributes
        ) throws Exception
    {
        this.validatePrincipal( principalId );

        try
        {
            Collection<AttributeValuePair> attributeValues =
                                                    Collections.emptyList();

            if ( (attributes != null) && (attributes.size() > 0) )
            {
                Map<String, String> attributeMap =
                        _store.getExtendedIdentityStoreData().getAttributeMap();

                IOsSamAdapter samAdapter = getSamAdapter();
                UserInfo userInfo = samAdapter.getLocalUserInfo(principalId.getName());
                if(userInfo == null)
                {
                    throw new NoSuchUserException(
                        String.format( "User '%s' was not found.", principalId.getName() )
                    );
                }
                List<String> groupsList = null;

                attributeValues = new ArrayList<AttributeValuePair>(
                                                        attributes.size()+1);

                AttributeValuePair pairGroupSids = new AttributeValuePair();
                pairGroupSids.setAttrDefinition(new Attribute(IdentityManager.INTERNAL_ATTR_GROUP_OBJECTIDS));
                pairGroupSids.getValues().add(LocalOsIdentityProvider.getObjectId(buildUserPrincipalId(this.getDomainName(), userInfo)));

                for( Attribute attribute : attributes )
                {
                    if(attribute != null)
                    {
                        String matchedAttribute = attributeMap.get( attribute.getName());
                        if(matchedAttribute == null)
                        {
                            throw new IllegalArgumentException(
                                String.format("Unknown attribute '%s'.", attribute.getName())
                            );
                        }

                        AttributeValuePair avPair = new AttributeValuePair();

                        avPair.setAttrDefinition( attribute );

                        attributeValues.add(avPair);

                        if( matchedAttribute.equals( FIRST_NAME_ATTRIBUTE ) )
                        {
                            avPair.getValues().add(
                                                GetFirstName(
                                                    userInfo.getFullName(),
                                                    principalId.getName()) );
                        }
                        else if (matchedAttribute.equals( LAST_NAME_ATTRIBUTE ))
                        {
                            avPair.getValues().add(
                                                GetLastName(
                                                    userInfo.getFullName()) );
                        }
                        else if (matchedAttribute.equals( GROUPS_ATTRIBUTE ))
                        {
                            if(groupsList == null)
                            {
                                groupsList =
                                        samAdapter.GetLocalUserGroups(
                                                        principalId.getName(),
                                                        true );
                                if(groupsList == null)
                                {
                                    groupsList = new ArrayList<String>(0);
                                }
                            }

                            boolean hasAlias = !ServerUtils.isNullOrEmpty(this.getAlias());
                            for(String group : groupsList)
                            {
                                // the expected format for the group names
                                // is <domain name>\<group name>
                                avPair.getValues().add(this.getDomainName() + "\\" + group);
                                // if the group has an alias include the aliased group in the list
                                if ( hasAlias )
                                {
                                    avPair.getValues().add(this.getAlias() + "\\" + group);
                                }
                                pairGroupSids.getValues().add(
                                        LocalOsIdentityProvider.getObjectId(buildGroupPrincipalId(this.getDomainName(), group))
                                );
                            }
                        }
                        else if (matchedAttribute.equals(SUBJECT_TYPE_ATTRIBUTE))
                        {
                            avPair.getValues().add("false");
                        }
                        else if (matchedAttribute.equals(USER_PRINCIPAL_NAME_ATTRIBUTE))
                        {
                            // make sure we use user name exactly as it is stored
                            // in identity provider when constructing UPN
                            avPair.getValues().add(userInfo.getName() + "@" + this.getDomainName());
                        }
                        else if (matchedAttribute.equals(EMAIL_ATTRIBUTE))
                        {
                            avPair.getValues().add("");
                        }
                        else
                        {
                            throw new IllegalArgumentException(
                                    String.format("Unknown attribute mapping: attribute='%s', mapping to ='%s'.",
                                        attribute.getName(),
                                        matchedAttribute
                                    )
                            );
                        }
                    }
                }
                attributeValues.add(pairGroupSids);
            }

            return attributeValues;
        }
        catch(OsSamUserNotFoundException ex)
        {
            throw new NoSuchUserException(
                    String.format( "User '%s' was not found.", principalId.getName() )
            );
        }
    }

    @Override
    public PersonUser findUser(PrincipalId id) throws Exception
    {
        this.validatePrincipal( id );

        PersonUser user = null;

        try
        {
            IOsSamAdapter samAdapter = getSamAdapter();
            UserInfo userInfo = samAdapter.getLocalUserInfo(id.getName());

            if(userInfo != null)
            {
                user = LocalOsIdentityProvider.buildPersonUser( this.getDomainName(), userInfo, this.getAlias() );
            }
        }
        catch(OsSamUserNotFoundException ex)
        {
           logger.info("User " + id.getName() + " not found, returning null.");
           user = null;
        }

        return user;
    }

    @Override
    public PersonUser findUserByObjectId(String userObjectId) throws Exception
    {
        // userObjectId should be in the format of userName@localOs
        PrincipalId id = null;
        try
        {
            id = LocalOsIdentityProvider.getPrincipalId(userObjectId);
        }
        catch(Exception e)
        {
            return null;
        }

        return findUser(id);
    }

    @Override
    public Set<PersonUser> findUsers(String searchString, String domainName, int limit) throws Exception
    {
        return findUsersInternal(searchString, domainName, limit, false);
    }

    @Override
    public Set<PersonUser> findUsersByName(String searchString, String domainName, int limit) throws Exception
    {
        return findUsersInternal(searchString, domainName, limit, true);
    }

    private Set<PersonUser> findUsersInternal(String searchString, String domainName, int limit, boolean bIsByNameOnly) throws Exception
    {
        Set<PersonUser> users = new HashSet<PersonUser>();

        if(searchString != null)
        {
            IOsSamAdapter samAdapter = getSamAdapter();
            List<UserInfo> usersList = samAdapter.getLocalUsers();

            if ( ( usersList != null ) && (usersList.size() > 0) )
            {
                int i = 0;
                for( UserInfo userInfo : usersList )
                {
                    if(userInfo != null)
                    {
                        if(LocalOsIdentityProvider.isMatch( userInfo, searchString, bIsByNameOnly))
                        {
                            if (i++ < limit || limit < 0)
                            {
                                users.add(
                                    LocalOsIdentityProvider.buildPersonUser( this.getDomainName(), userInfo, this.getAlias() ));
                            }
                            else break;
                        }
                    }
                }
            }
        }

        return users;
    }

    @Override
    public Set<PersonUser> findUsersInGroup(PrincipalId groupId, String searchString, int limit) throws Exception
    {
        return findUsersInGroupInternal(groupId, searchString, limit, false);
    }

    @Override
    public Set<PersonUser> findUsersByNameInGroup(PrincipalId groupId, String searchString, int limit) throws Exception
    {
        return findUsersInGroupInternal(groupId, searchString, limit, true);
    }

    private Set<PersonUser> findUsersInGroupInternal(PrincipalId groupId, String searchString, int limit, boolean byNameOnly) throws Exception
    {
        this.validatePrincipal( groupId );

        try
        {
            Set<PersonUser> users = new HashSet<PersonUser>();

            if(searchString != null && limit != 0)
            {
                IOsSamAdapter samAdapter = getSamAdapter();
                List<String> usersList = samAdapter.GetUsersInGroup( groupId.getName() );

                if ((usersList != null) && (usersList.size() > 0) )
                {
                    for( String userName : usersList)
                    {
                        int numUsersToRet = 0;
                        UserInfo userInfo = null;
                        try
                        {
                            userInfo = samAdapter.getLocalUserInfo( userName );
                        }
                        catch(OsSamUserNotFoundException ex)
                        {
                            userInfo = null;
                        }

                        if(userInfo != null)
                        {
                            if( LocalOsIdentityProvider.isMatch( userInfo, searchString, byNameOnly ) )
                            {
                                if (limit <= 0 || numUsersToRet < limit)
                                {
                                    users.add(
                                        LocalOsIdentityProvider.buildPersonUser( this.getDomainName(), userInfo, this.getAlias() )
                                    );
                                    numUsersToRet++;
                                }
                            }
                        }
                    }
                }
            }

            return users;
        }
        catch(OsSamGroupNotFoundException ex)
        {
            throw new NoSuchGroupException(
                String.format( "Group '%s' does not exist.", groupId.getName() )
            );
        }
    }

    @Override
    public Set<PersonUser> findDisabledUsers(String searchString, int limit) throws Exception
    {
        Set<PersonUser> users = new HashSet<PersonUser>();

        if( searchString != null )
        {
            users = this.findUsers( searchString, this.getDomainName(), limit );

            Set<PersonUser> disabledUsers = new HashSet<PersonUser>();
            for(PersonUser user : users)
            {
                if(user.isDisabled())
                {
                    disabledUsers.add( user );
                }
            }

            users = disabledUsers;
        }

        return users;
    }

    @Override
    public Set<PersonUser> findLockedUsers(String searchString, int limit) throws Exception
    {
        Set<PersonUser> users = new HashSet<PersonUser>();

        if( searchString != null )
        {
            users = this.findUsers( searchString, this.getDomainName(), limit );

            Set<PersonUser> lockedUsers = new HashSet<PersonUser>();
            for(PersonUser user : users)
            {
                if(user.isLocked())
                {
                    lockedUsers.add( user );
                }
            }

            users = lockedUsers;
        }

        return users;
    }

    @Override
    public PrincipalGroupLookupInfo findDirectParentGroups(PrincipalId principalId) throws Exception
    {
        // per admin interface doc principal could be user or group ...
        try
        {
            return findGroupsForUser( principalId, false );
        }
        catch(NoSuchUserException ex)
        {
            // check if this is group....
            Group group = this.findGroup( principalId );
            if( group == null )
            {
                throw ex;
            }
            else
            {
                // local groups do not have nesting ...
                return new PrincipalGroupLookupInfo(
                    Collections.<Group>emptySet(),
                    group.getObjectId() );
            }
        }
    }

    @Override
    public Group findGroup(PrincipalId groupId) throws Exception
    {
        this.validatePrincipal( groupId );

        Group group = null;

        try
        {
            IOsSamAdapter samAdapter = getSamAdapter();
            GroupInfo groupInfo = samAdapter.getLocalGroupInfo( groupId.getName() );

            if(groupInfo != null)
            {
                group = LocalOsIdentityProvider.buildGroup( this.getDomainName(), groupInfo, this.getAlias() );
            }
        }
        catch(OsSamGroupNotFoundException ex)
        {
            group = null;
        }

        return group;
    }

    @Override
    public Group findGroupByObjectId(String groupObjectId) throws Exception
    {
        // groupObjectId should be in the format of groupName@localOs
        PrincipalId id = null;
        try
        {
            id = LocalOsIdentityProvider.getPrincipalId(groupObjectId);
        }
        catch(Exception e)
        {
            return null;
        }

        return findGroup(id);
    }

    @Override
    public Set<Group> findGroups(String searchString, String domainName, int limit) throws Exception
    {
        return findGroupsInternal(searchString, domainName, limit, false);
    }

    @Override
    public Set<Group> findGroupsByName(String searchString, String domainName, int limit) throws Exception
    {
        return findGroupsInternal(searchString, domainName, limit, true);
    }

   private Set<Group> findGroupsInternal(String searchString, String domainName, int limit, boolean bIsByNameOnly) throws Exception
    {
        Set<Group> groups = new HashSet<Group>();

        if(searchString != null)
        {
            IOsSamAdapter samAdapter = getSamAdapter();
            List<GroupInfo> localGroups = samAdapter.getLocalGroups();
            if ( ( localGroups != null ) && (localGroups.size() > 0) )
            {
                int i = 0;
                for( GroupInfo groupInfo : localGroups )
                {
                    if(groupInfo != null)
                    {
                        if ( LocalOsIdentityProvider.isMatch( groupInfo, searchString, bIsByNameOnly) )
                        {
                            if (i++ < limit || limit < 0)
                            {
                                groups.add(
                                    LocalOsIdentityProvider.buildGroup(this.getDomainName(), groupInfo, this.getAlias()));
                            }
                            else break;
                        }
                    }
                }
            }
        }

        return groups;
    }

    @Override
    public Set<Group> findGroupsInGroup(PrincipalId groupId, String searchString, int limit) throws Exception
    {
        return findGroupsInGroupInternal(groupId, searchString, limit, false);
    }

    @Override
    public Set<Group> findGroupsByNameInGroup(PrincipalId groupId, String searchString, int limit) throws Exception
    {
        return findGroupsInGroupInternal(groupId, searchString, limit, true);
    }

    private Set<Group> findGroupsInGroupInternal(PrincipalId groupId, String searchString, int limit, boolean byNameOnly) throws Exception
    {
        this.validatePrincipal( groupId );

        try
        {
            Set<Group> groups = new HashSet<Group>();
            if(searchString != null && limit != 0)
            {
                // check that group exists
                IOsSamAdapter samAdapter = getSamAdapter();
                GroupInfo group = samAdapter.getLocalGroupInfo( groupId.getName() );

                assert(group != null);

                // local group does not have local groups within it.
            }
            return groups;
        }
        catch(OsSamGroupNotFoundException ex)
        {
            throw new NoSuchGroupException(
                    String.format( "Group '%s' was not found.", groupId.getName() )
            );
        }
    }

    @Override
    public SearchResult find(String searchString, String domainName, int limit) throws Exception
    {
       Set<PersonUser> users = this.findUsers(searchString, domainName, limit<0? -1: (limit/2 + limit%2));
       int limitGroup = limit<0? -1: (limit- ((users != null)? users.size() : 0));
       Set<Group> groups = null;
       if (limitGroup != 0)
       {
          groups = this.findGroups(searchString, domainName, limitGroup);
       }
       return new SearchResult( users, null /* service principals */, groups );
    }

    @Override
    public SearchResult findByName(String searchString, String domainName, int limit) throws Exception
    {
        Set<PersonUser> users = this.findUsersByName(searchString, domainName, limit/2);
        Set<Group> groups = this.findGroupsByName(searchString, domainName, limit/2 + limit%2);

        return new SearchResult( users, null /* service principals */, groups );
    }

    @Override
    public boolean IsActive(PrincipalId id) throws Exception
    {
        this.validatePrincipal( id );
        UserInfo userInfo = retrieveUserInfo(id);
        return !LocalOsIdentityProvider.isDisabled(userInfo);
    }

    @Override
    public void checkUserAccountFlags(PrincipalId principalId)
          throws IDMException
    {
       this.validatePrincipal( principalId );

       UserInfo userInfo = null;
       try
       {
          userInfo = retrieveUserInfo(principalId);
       }
       catch (Exception e)
       {
          throw new IDMException(e.getMessage(), e);
       }

       if (LocalOsIdentityProvider.isLocked(userInfo))
       {
          throw new UserAccountLockedException(
                String.format("User account locked: %s", principalId));
       }
       else if (LocalOsIdentityProvider.isPasswordExpired(userInfo))
       {
          throw new PasswordExpiredException(
                String.format("User account expired: %s", principalId));
       }
    }

    @Override
    public PrincipalGroupLookupInfo findNestedParentGroups(PrincipalId userId) throws Exception
    {
        return this.findGroupsForUser( userId, true );
    }

    @Override
    public Collection<SecurityDomain> getDomains() {
        Collection<SecurityDomain> domains = new HashSet<SecurityDomain>();
        domains.add(new SecurityDomain(
                            this.getDomainName(),
                            this.getAlias()));
        return domains;
    }

    /////////////////////////////////////////////////
    // privates
    /////////////////////////////////////////////////

    private PrincipalGroupLookupInfo findGroupsForUser(PrincipalId userId, boolean nesting) throws Exception
    {
        this.validatePrincipal( userId );

        Set<Group> groups = new HashSet<Group>();
        UserInfo userInfo = null;

        try
        {
            IOsSamAdapter samAdapter = getSamAdapter();
            userInfo = samAdapter.getLocalUserInfo(userId.getName());
            if(userInfo == null)
            {
                throw new NoSuchUserException(
                    String.format( "User '%s' was not found.", userId.getName() )
                );
            }
            List<String> groupsList = samAdapter.GetLocalUserGroups( userId.getName(), nesting );
            if ( (groupsList != null) && (groupsList.size() > 0) )
            {
                GroupInfo groupInfo = null;
                String groupName = null;
                for(int i = 0; i < groupsList.size(); i++)
                {
                    groupName = groupsList.get(i);
                    if(!ServerUtils.isNullOrEmpty( groupName ))
                    {
                        try
                        {
                            groupInfo = samAdapter.getLocalGroupInfo( groupName );
                        }
                        catch(OsSamGroupNotFoundException ex)
                        {
                            groupInfo = null;
                        }

                        if(groupInfo != null)
                        {
                            groups.add(
                                    LocalOsIdentityProvider.buildGroup( this.getDomainName(), groupInfo, this.getAlias() ) );
                        }
                    }
                }
            }
        }
        catch(OsSamUserNotFoundException ex)
        {
            throw new NoSuchUserException(
                    String.format( "User '%s' was not found.", userId.getName() )
            );
        }
        return new PrincipalGroupLookupInfo(
            groups,
            (userInfo != null) ?
                LocalOsIdentityProvider.getObjectId(
                    buildUserPrincipalId( this.getDomain(), userInfo )) :
                null);
    }

    private UserInfo retrieveUserInfo(PrincipalId id) throws Exception
    {
        try
        {
            IOsSamAdapter samAdapter = getSamAdapter();
            UserInfo userInfo = samAdapter.getLocalUserInfo( id.getName() );

            if(userInfo == null)
            {
                throw new NoSuchUserException(
                    String.format( "User '%s' was not found.", id.getName() )
                );
            }
            return userInfo;
        }
        catch(OsSamUserNotFoundException ex)
        {
            throw new NoSuchUserException(
                String.format( "User '%s' was not found.", id.getName() )
            );
        }
    }

    private String getDomainName()
    {
        return this._store.getName();
    }

    private static IOsSamAdapter getSamAdapter()
    {
        return OsSamAdapterFactory.getInstance().getOsSamAdapter();
    }

    private void validatePrincipal( PrincipalId principalId )
    {
        ValidateUtil.validateNotNull( principalId, "id" );
        ValidateUtil.validateNotEmpty( principalId.getName(), "id.getName()" );
        ValidateUtil.validateNotEmpty( principalId.getDomain(), "id.getDomain()" );

        if( !this.getDomainName().equalsIgnoreCase( principalId.getDomain() ) &&
            ( (this.getAlias() == null) || !this.getAlias().equalsIgnoreCase( principalId.getDomain() ) ) )
        {
            throw new IllegalArgumentException(
                    String.format( "Unrecognized domain name '%s'.", principalId.getDomain() )
            );
        }
    }

    private static boolean isMatch(GroupInfo groupInfo, String matchTo, boolean bIsNameOnly)
    {
        return (groupInfo != null)
                &&
                (
                    bIsNameOnly ?   isMatch(groupInfo.getName(), matchTo)
                                :
                                  ( isMatch(groupInfo.getName(), matchTo)
                                   ||
                                    isMatch(groupInfo.getComment(), matchTo))
                );
    }

    private static boolean isMatch(UserInfo userInfo, String matchTo, boolean bIsNameOnly)
    {
        return (userInfo != null)
                &&
                (
                    bIsNameOnly ? ( isMatch(userInfo.getName(), matchTo)
                                   ||
                                    isMatch(userInfo.getFullName(), matchTo))
                                :
                                  ( isMatch(userInfo.getName(), matchTo)
                                   ||
                                    isMatch(userInfo.getFullName(), matchTo)
                                   ||
                                    isMatch(userInfo.getComment(), matchTo))
                );
    }

    private static boolean isMatch(String matchWhat, String matchTo)
    {
        boolean isMatch = false;
        if ( (matchWhat != null) && (matchWhat.length() > 0))
        {
            if (( matchTo == null ) || matchTo.isEmpty() )
            {
                isMatch = true;
            }
            else
            {
                isMatch = matchWhat.contains( matchTo );
            }
        }

        return isMatch;
    }

    private static String GetFirstName(String fullName, String fallback)
    {
        String firstName = null;
        if( fullName != null )
        {
            // we need to figure out how do we want to split in reality
            fullName = fullName.trim();
            int index = fullName.indexOf(' ');

            if(index > 0)
            {
                firstName = fullName.substring(0, index);
            }
            else
            {
                firstName = fullName;
            }
        }
        if((firstName == null) || (firstName.isEmpty()))
        {
            firstName = fallback;
        }
        return firstName;
    }

    private static String GetLastName(String fullName )
    {
        String lastName = null;
        if(fullName != null)
        {
            // we need to figure out how do we want to split in reality
            fullName = fullName.trim();
            int index = fullName.indexOf(' ');

            if(index > 0)
            {
                // because we trimmed the string, if we found the ' ' it will not be the last char
                // so index + 1 is OK.
                lastName = fullName.substring( index + 1 );
                lastName = lastName.trim();
            }
        }
        return lastName;
    }

    private static PrincipalId buildUserPrincipalId( String domain, UserInfo userInfo )
    {
        PrincipalId userId = new PrincipalId( userInfo.getName(), domain );
        return userId;
    }

    private static PersonUser buildPersonUser( String domain, UserInfo userInfo, String alias )
    {
        PrincipalId userId = buildUserPrincipalId( domain, userInfo );
        return new PersonUser(
            userId,
            ServerUtils.getPrincipalAliasId(userId.getName(), alias),
            LocalOsIdentityProvider.getObjectId(userId),
            new PersonDetail.Builder()
                .firstName(GetFirstName( userInfo.getFullName(), userInfo.getName() ))
                .lastName(GetLastName( userInfo.getFullName()))
                .emailAddress(null)
                .description(userInfo.getComment())
            .build(),
            isDisabled( userInfo ),
            isLocked( userInfo )
        );
    }

    private static boolean isDisabled( UserInfo userInfo )
    {
        return ((userInfo.getFlags() & OsSamConstants.UF_ACCOUNT_DISABLE) == OsSamConstants.UF_ACCOUNT_DISABLE);
    }

    private static boolean isLocked( UserInfo userInfo )
    {
        return ((userInfo.getFlags() & OsSamConstants.UF_LOCKOUT) == OsSamConstants.UF_LOCKOUT);
    }

    private static boolean isPasswordExpired( UserInfo userInfo )
    {
        return ((userInfo.getFlags() & OsSamConstants.UF_PASSWORD_EXPIRED) == OsSamConstants.UF_PASSWORD_EXPIRED);
    }

    private static PrincipalId buildGroupPrincipalId( String domain, GroupInfo groupInfo )
    {
        return buildGroupPrincipalId( domain, groupInfo.getName() );
    }

    private static PrincipalId buildGroupPrincipalId( String domain, String groupName )
    {
        PrincipalId groupId = new PrincipalId( groupName, domain );
        return groupId;
    }

    private static Group buildGroup( String domain, GroupInfo groupInfo, String alias )
    {
        PrincipalId groupId = buildGroupPrincipalId(domain, groupInfo );
        return new Group(
            groupId,
            ServerUtils.getPrincipalAliasId(groupInfo.getName(), alias),
            LocalOsIdentityProvider.getObjectId(groupId),
            new GroupDetail( groupInfo.getComment()));
    }

    private static String getObjectId(PrincipalId id)
    {
        ValidateUtil.validateNotNull(id, "LocalOs PrincipalId");
        return String.format("%s@%s", id.getName(), id.getDomain());
    }

    private static PrincipalId getPrincipalId(String objectId)
    {
        ValidateUtil.validateNotEmpty(objectId, "LocalOs objectId");
        int idx = objectId.indexOf(UPN_SEPARATOR);

        if (idx <= 0)
        {
           throw new IllegalStateException(
                 String.format(
                       "Invalid localOs objectId format for [%s]",
                       objectId));
        }
        else
        {
           return new PrincipalId(objectId.substring(0, idx),
                                  objectId.substring(idx + 1));
        }
    }

    @Override
    public PrincipalId findActiveUser(String attributeName, String attributeValue) throws Exception {
        throw new IDMException("findActiveUser() not supported in localos provider");
    }

    @Override
    public UserSet findActiveUsersInDomain(String attributeName, String attributeValue
            , String userDomain, String additionalAttribute)
            throws Exception {
        throw new IDMException("findActiveUsersInDomain() not supported in localos provider");
    }

    @Override
    public String getStoreUPNAttributeName() {
        return USER_PRINCIPAL_NAME_ATTRIBUTE;
    }

    @Override
    public String getStoreUserHintAttributeName() throws IDMException {
        throw new IDMException("getStoreUserHintAttributeName() not supported in localos provider");
    }
    @Override
    public boolean getCertificateMappingUseUPN() {
        return true;
    }
}
