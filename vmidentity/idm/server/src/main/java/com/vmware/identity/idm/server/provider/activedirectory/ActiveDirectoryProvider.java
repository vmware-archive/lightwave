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
 * Active Directory Provider
 *
 * @author:  Sriram Nambakam <snambakam@vmware.com>
 *
 * @version: 1.0
 * @since:   2011-12-7
 *
 */
package com.vmware.identity.idm.server.provider.activedirectory;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Stack;

import javax.security.auth.login.LoginException;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.ActiveDirectoryJoinInfo;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SearchResult;
import com.vmware.identity.idm.SecurityDomain;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.DcInfoCache;
import com.vmware.identity.idm.server.IdentityManager;
import com.vmware.identity.idm.server.IdmDomainState;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.config.ServerIdentityStoreData;
import com.vmware.identity.idm.server.performance.IIdmAuthStatRecorder;
import com.vmware.identity.idm.server.performance.IdmAuthStatRecorder;
import com.vmware.identity.idm.server.performance.NoopIdmAuthStatRecorder;
import com.vmware.identity.idm.server.provider.BaseLdapProvider;
import com.vmware.identity.idm.server.provider.IIdentityProvider;
import com.vmware.identity.idm.server.provider.ILdapSchemaMapping;
import com.vmware.identity.idm.server.provider.NoSuchGroupException;
import com.vmware.identity.interop.accountmanager.AccountAdapterFactory;
import com.vmware.identity.interop.accountmanager.AccountInfo;
import com.vmware.identity.interop.accountmanager.AccountManagerException;
import com.vmware.identity.interop.accountmanager.AccountManagerNativeException;
import com.vmware.identity.interop.accountmanager.IAccountAdapter;
import com.vmware.identity.interop.accountmanager.IAccountAdapter.ACCOUNT_TYPE;
import com.vmware.identity.interop.domainmanager.DomainAdapterFactory;
import com.vmware.identity.interop.domainmanager.DomainControllerInfo;
import com.vmware.identity.interop.domainmanager.DomainManagerException;
import com.vmware.identity.interop.domainmanager.DomainManagerNativeException;
import com.vmware.identity.interop.domainmanager.DomainTrustInfo;
import com.vmware.identity.interop.domainmanager.IDomainAdapter;
import com.vmware.identity.interop.idm.IIdmClientLibrary;
import com.vmware.identity.interop.idm.IdmClientLibraryFactory;
import com.vmware.identity.interop.idm.IdmNativeException;
import com.vmware.identity.interop.idm.UserInfo;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.ILdapEntry;
import com.vmware.identity.interop.ldap.ILdapMessage;
import com.vmware.identity.interop.ldap.LdapFilterString;
import com.vmware.identity.interop.ldap.LdapScope;
import com.vmware.identity.interop.ldap.LdapValue;
import com.vmware.identity.interop.ldap.SaslBindFailLdapException;
import com.vmware.identity.interop.ldap.ServerDownLdapException;
import com.vmware.identity.interop.ldap.SizeLimitExceededLdapException;
import com.vmware.identity.performanceSupport.IIdmAuthStat.ActivityKind;
import com.vmware.identity.performanceSupport.ILdapQueryStat;
import com.vmware.identity.performanceSupport.LdapQueryStat;

public class ActiveDirectoryProvider extends BaseLdapProvider implements IIdentityProvider
{
   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(ActiveDirectoryProvider.class);

   private static final String ATTR_SUBJECT_TYPE = "subjectType";  //constructed by IDM

   // Other constants
   private static final int USER_ACCT_DISABLED_FLAG =             0x0002;
   private static final int USER_ACCT_LOCKED_FLAG   =             0x0010;
   private static final int USER_ACCT_DONT_EXPIRE_PASSWD =    0x00010000;
   private static final long INTVLS_100NS_IN_SEC = 10*1000*1000;
   private static final long INTVLS_100NS_1600TO1970 = 11644473600L*INTVLS_100NS_IN_SEC;
   private static final int DEFAULT_PAGE_SIZE = 1000;
   private static final int DEFAULT_RANGE_SIZE = 1000;

   private final Set<String> _specialAttributes;
   private PwdLifeTimeValue _defaultDomainPolicyPwdLifeTimeCache = null;
   private final ILdapSchemaMapping _adSchemaMapping;
   private final String ATTR_NAME_SAM_ACCOUNT;
   private final String ATTR_MEMBER_OF;
   private final String ATTR_PRIMARY_GROUP_ID;
   private final String ATTR_USER_PRINCIPAL_NAME;
   private final String ATTR_OBJECT_SID;

   private static final AccountPacInfoCache _accountCache = new AccountPacInfoCache();

   public ActiveDirectoryProvider(IIdentityStoreData store)
   {
      super(store);

      Validate.isTrue(
            getStoreDataEx().getProviderType() ==
            IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY,
            "IIdentityStoreData must represent a store of 'IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY' type.");

      if (!(this.getStoreDataEx() instanceof ServerIdentityStoreData))
      {
          throw new IllegalStateException("IdentityStoreData must be an instance of ServerIdentityStoreData");
      }

      ActiveDirectoryJoinInfo machineJoinInfo = IdmDomainState.getInstance().getDomainJoinInfo();
      if (machineJoinInfo == null)
      {
          log.warn("There may be a domain join status change since native AD is configured. " +
                    "ActiveDirectoryProvider can function properly only when machine is properly joined");
      }

      _specialAttributes = new HashSet<String>();
      _specialAttributes.add(ATTR_SUBJECT_TYPE);
      _adSchemaMapping = new ADSchemaMapping( this.getStoreDataEx().getIdentityStoreSchemaMapping() );

      ATTR_NAME_SAM_ACCOUNT = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
      ATTR_MEMBER_OF = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeMemberOf);
      ATTR_PRIMARY_GROUP_ID = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrimaryGroupId);
      ATTR_USER_PRINCIPAL_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);
      ATTR_OBJECT_SID = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
   }

   @Override
   public String getDomain()
   {
       String domainName = null;
      ActiveDirectoryJoinInfo machineJoinInfo = IdmDomainState.getInstance().getDomainJoinInfo();
      if (machineJoinInfo == null)
      {
          log.warn("There may be a domain join status change since native AD is configured. " +
                  "ActiveDirectoryProvider can function properly only when machine is properly joined");
          domainName = this.getStoreData().getName();
      }
      else
      {
          domainName = machineJoinInfo.getName();
      }

      return domainName;
   }

   @Override
   public String getAlias()
   {
       String domainAlias = null;
      ActiveDirectoryJoinInfo machineJoinInfo = IdmDomainState.getInstance().getDomainJoinInfo();
      if (machineJoinInfo == null)
      {
          log.warn("There may be a domain join status change since native AD is configured. ActiveDirectoryProvider can function properly only when machine is properly joined");
          domainAlias = this.getStoreDataEx().getAlias();
      }
      else
      {
          domainAlias = machineJoinInfo.getAlias();
      }

      return domainAlias;
   }

   // principal is non-null, checked by caller
   // if principalId is userId, need normalize to default upn
   // if principalId is groupId, no need to normalize (group does not have upn)
   // A non-null principalInfo is always returned
   private
   PrincipalId
   normalizeAliasInPrincipalWithTrusts(PrincipalId principal, boolean bFoundAsUser)
   {
       String domainName = principal.getDomain();
       boolean bFoundInTrust = false;
       PrincipalId normalizedPrincipal = principal;
       DomainTrustInfo[] trustsInfo = IdmDomainState.getInstance().getDomainTrustInfo();

       if (trustsInfo != null && trustsInfo.length > 0)
       {
           for (DomainTrustInfo trust : trustsInfo)
           {
               if (trust == null || trust.dcInfo == null) continue;

               // make the user principal to always contain domain name, not alias
               if( domainName.equalsIgnoreCase( trust.dcInfo.domainNetBiosName ))
               {
                   normalizedPrincipal = new PrincipalId( principal.getName(), trust.dcInfo.domainName );
                   bFoundInTrust = true;
                   break;
               }
           }
       }

       if (!bFoundInTrust)
       {
           normalizedPrincipal = this.normalizeAliasInPrincipal(principal);
       }

       try
       {
           if (bFoundAsUser)
           {
               normalizedPrincipal = findUserPrincipalIdByAcctAdapter(normalizedPrincipal);
           }
       }
       catch(Exception e)
       {
           log.info(String.format("Failed to retrieve default UPN for principal %s ",
                   principal.getUPN()), e);
       }

       return normalizedPrincipal;
   }

   @Override
   public
   PrincipalId
   authenticate(PrincipalId principal, String password) throws LoginException
   {
       ValidateUtil.validateNotNull(principal, "principal");

       IIdmAuthStatRecorder idmAuthStatRecorder = this.createIdmAuthStatRecorderInstance(
               DiagnosticsContextFactory.getCurrentDiagnosticsContext().getTenantName(),
               ActivityKind.AUTHENTICATE, principal);
       idmAuthStatRecorder.start();

       principal = this.normalizeAliasInPrincipalWithTrusts(principal, true);

       IIdmClientLibrary idmAdapter =
               IdmClientLibraryFactory.getInstance().getLibrary();

       UserInfo user = idmAdapter.AuthenticateByPassword(principal.getName(),
                                                         principal.getDomain(),
                                                         password);

       saveUserInfo(user);

       idmAuthStatRecorder.end();

       return ServerUtils.getPrincipalId(
                            user.getUPN(),
                            user.getName(),
                            user.getDomain());
   }

   @Override
   public boolean IsActive(PrincipalId id) throws Exception
   {
      ValidateUtil.validateNotNull(id, "id");
      id = normalizeAliasInPrincipalWithTrusts(id, true);
      boolean isActive = false;

      try
      {
          int accountFlags = this.retrieveUserAccountFlagsByLdap(id);
          isActive = ((accountFlags & USER_ACCT_DISABLED_FLAG) == 0);
      }
      catch(Exception e)
      {
          log.error(String.format("Failed to retrieve user account flag for [%s] via ldap ", id.getUPN()),  e);

          PersonUser user = this.findUserByAcctAdapter(id);
          isActive = !user.isDisabled();
      }

      return isActive;
   }

   @Override
   public void checkUserAccountFlags(PrincipalId principalId) throws IDMException
   {
       //No ops -- For AD, account status is parsed thru Kerberos returned code.
   }

   private int retrieveUserAccountFlagsByLdap(PrincipalId id) throws IDMException
   {
      ValidateUtil.validateNotNull(id, "principalId");

      int accountFlags = 0;
      ILdapConnectionEx connection = null;

      try
      {
         connection = this.getNonGcConnToDomain(id.getDomain());
      }
      catch (Exception ex)
      {
         throw new IDMException("Failed to establish server connection", ex);
      }

      AccountLdapEntryInfo ldapEntryInfo = null;
      try
      {
          String baseDN = ServerUtils.getDomainDN(id.getDomain());

          final String ATTR_NAME_USER_ACCT_CTRL = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);
          String attributes[] = { ATTR_NAME_USER_ACCT_CTRL };

          final String filter_by_upn = this.buildUserQueryWithUpnByPrincipalId(id);
          final String filter_by_acct = this.buildUserQueryWithAccountNameByPrincipalId(id);

          ldapEntryInfo = findAccountLdapEntry(connection,
                  filter_by_upn,
                  filter_by_acct,
                  baseDN,
                  attributes,
                  false,
                  id);

          accountFlags = this.getOptionalIntegerValue(
                  ldapEntryInfo.accountLdapEntry.getAttributeValues(
                           attributes[0]),
                           0);
      }
      finally
      {
          if (ldapEntryInfo != null)
          {
              ldapEntryInfo.close_messages();
          }
          if (connection != null)
          {
              connection.close();
          }
      }

      return accountFlags;
   }

   @Override
   public PersonUser findUser(PrincipalId id) throws Exception
   {
      ValidateUtil.validateNotNull(id, "id");
      id = normalizeAliasInPrincipalWithTrusts(id, true);
      PersonUser user = null;

      try
      {
          user = findUserByLdap(id);
      }
      catch(Exception e)
      {
          log.info(String.format("Failed to find user %s via ldap search ", id.getUPN() + e.getMessage()));

          user = findUserByAcctAdapter(id);
      }

      return user;
   }

   @Override
   public PersonUser findUserByObjectId(String userObjectSid) throws Exception
   {
       PersonUser user = null;
       try
       {
           user = findUserByObjectIdInDomain(userObjectSid);
           if (user == null)
           {
               throw new InvalidPrincipalException(String.format("Cannnot find user with sid %s in domain %s",
                                                   userObjectSid, this.getDomain()), userObjectSid);
           }
       }
       catch(Exception e)
       {
           // try GC
           user = findUserByObjectIdInGC(userObjectSid);
       }

       return user;
   }

    @Override
    public
    Collection<AttributeValuePair>
    getAttributes(
        PrincipalId           principalId,
        Collection<Attribute> attributes
    ) throws Exception
    {
        ValidateUtil.validateNotNull(principalId, "principalId");

        IIdmAuthStatRecorder idmAuthStatRecorder = this.createIdmAuthStatRecorderInstance(
                DiagnosticsContextFactory.getCurrentDiagnosticsContext().getTenantName(),
                ActivityKind.GETATTRIBUTES, principalId);
        idmAuthStatRecorder.start();

        principalId = normalizeAliasInPrincipalWithTrusts(principalId, true);

        UserInfoEx acctInfo = _accountCache.findAccount(principalId.getUPN().toLowerCase());
        Collection<AttributeValuePair> result = null;
        if (acctInfo == null)
        {
           result = getAttributesByLdap(principalId, attributes, idmAuthStatRecorder);
        }
        else
        {
           result = getAttributesByPac(acctInfo, attributes);
        }

        idmAuthStatRecorder.end();

        return result;
   }

   @Override
   public Group findGroup(PrincipalId groupId) throws Exception
   {
       ValidateUtil.validateNotNull(groupId, "groupId");
       groupId = normalizeAliasInPrincipalWithTrusts(groupId, false);
       Group group = null;

       try
       {
           group = findGroupByLdap(groupId);
       }
       catch(Exception e)
       {
           log.info(String.format("Failed to find group %s via ldap search ", groupId.getUPN() + e.getMessage()));
           group = findGroupByAcctAdapter(groupId);
       }

       return group;
   }

   @Override
   public Set<Group> findGroups(String searchString, String domainName, int limit) throws Exception
   {
       String filter = createSearchFilter(_adSchemaMapping.getAllGroupsQuery(), _adSchemaMapping.getGroupQueryByCriteria(), searchString);
       return findGroupsInternal(filter, domainName, limit);
   }

   @Override
   public Set<Group> findGroupsByName(String searchString, String domainName, int limit) throws Exception
   {
       String filter = createSearchFilter(_adSchemaMapping.getAllGroupsQuery(), _adSchemaMapping.getGroupQueryByCriteriaForName(), searchString);
       return findGroupsInternal(filter, domainName, limit);
   }

   @Override
   public Set<PersonUser> findUsers(String searchString, String domainName, int limit) throws Exception
   {
       String filter = createSearchFilter(_adSchemaMapping.getAllUsersQuery(), _adSchemaMapping.getUserQueryByCriteria(), searchString);
       return findUsersInternal(filter, domainName, limit);
   }

   @Override
   public Set<PersonUser> findUsersByName(String searchString, String domainName, int limit) throws Exception
   {
       String filter = createSearchFilter(_adSchemaMapping.getAllUsersQuery(), _adSchemaMapping.getUserQueryByCriteriaForName(), searchString);
       return findUsersInternal(filter, domainName, limit);
   }

   @Override
   public
   Set<PersonUser>
   findUsersInGroup(PrincipalId groupId, String searchString, int limit) throws Exception
   {
       String filter = createSearchFilter(_adSchemaMapping.getAllUsersQuery(), _adSchemaMapping.getUserQueryByCriteria(), searchString);
       return findUsersInGroupInternal(groupId, filter, limit);
   }

    @Override
    public Set<PersonUser> findUsersByNameInGroup(PrincipalId groupId, String searchString, int limit) throws Exception
    {
        String filter = createSearchFilter(_adSchemaMapping.getAllUsersQuery(), _adSchemaMapping.getUserQueryByCriteriaForName(), searchString);
        return findUsersInGroupInternal(groupId, filter, limit);
    }

    @Override
    public Group findGroupByObjectId(String groupObjectSid) throws Exception
    {
        Group group = null;
        try
        {
            // try current domain first( domain search always give more complete information )
            group = findGroupByObjectIdInDomain(groupObjectSid);
            if (group == null)
            {
                throw new InvalidPrincipalException(String.format("Cannnot find group with sid %s in domain %s",
                                                    groupObjectSid, this.getDomain()), groupObjectSid);
            }
        }
        catch(Exception e)
        {
            // try GC
            group = findGroupByObjectIdInGC(groupObjectSid);
        }

        return group;
    }

   @Override
   public
   Set<Group> findDirectParentGroups(PrincipalId principalId) throws Exception
   {
       Set<Group> groups = new HashSet<Group>();
       ValidateUtil.validateNotNull(principalId, "principalId");
       principalId = normalizeAliasInPrincipalWithTrusts(principalId, true);

       ILdapConnectionEx connection = this.getNonGcConnToDomain(principalId.getDomain());

       try
       {
          final String ATTR_NAME_SAM_ACCOUNT = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
          final String ATTR_DESCRIPTION = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
          final String ATTR_OBJECT_SID = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);
           String[] attrNames = { ATTR_NAME_SAM_ACCOUNT, ATTR_DESCRIPTION, ATTR_OBJECT_SID };

           String filter = String.format(
               _adSchemaMapping.getDirectParentGroupsQuery(),
               LdapFilterString.encode(this.getPrincipalDN(connection, principalId, ServerUtils.getDomainDN(principalId.getDomain()))));

           Collection<ILdapMessage> messages = connection.paged_search(
                      ServerUtils.getDomainDN(principalId.getDomain()),
                      LdapScope.SCOPE_SUBTREE,
                      filter,
                      Arrays.asList(attrNames),
                      DEFAULT_PAGE_SIZE,
                      -1);

           if (messages != null && messages.size() > 0)
           {
               for (ILdapMessage message : messages)
               {
                   try
                   {
                       ILdapEntry[] entries = message.getEntries();

                       if (entries != null && entries.length > 0)
                       {
                           for (ILdapEntry entry : entries)
                           {
                               groups.add(this.buildGroup(entry, null));
                           }
                       }
                   }
                   finally
                   {
                       message.close();
                   }
               }
            }
        }
        finally
        {
            if (connection != null)
            {
                connection.close();
            }
        }

       return groups;
   }

   @Override
   public
   Set<Group>
   findGroupsInGroup(PrincipalId groupId, String searchString, int limit) throws Exception
   {
       String filter = createSearchFilter(_adSchemaMapping.getAllGroupsQuery(), _adSchemaMapping.getGroupQueryByCriteria(), searchString);
       return findGroupsInGroupInternal(groupId, filter, limit);
   }

   @Override
   public Set<Group> findGroupsByNameInGroup(PrincipalId groupId, String searchString, int limit) throws Exception
   {
       String filter = createSearchFilter(_adSchemaMapping.getAllGroupsQuery(), _adSchemaMapping.getGroupQueryByCriteriaForName(), searchString);
       return findGroupsInGroupInternal(groupId, filter, limit);
   }

   @Override
   public
   Set<Group> findNestedParentGroups(PrincipalId userId) throws Exception
   {
       Set<Group> groups = Collections.emptySet();
       ValidateUtil.validateNotNull(userId, "principalId");
       userId = normalizeAliasInPrincipalWithTrusts(userId, true);

       UserInfoEx acctInfo = _accountCache.findAccount(userId.getUPN().toLowerCase());
       if (acctInfo == null)
       {
           groups = findNestedParentGroupsByLdap(userId);
       }
       else
       {
           groups = findNestedParentGroupsByPac(acctInfo);
       }

       return groups;
   }

   @Override
   public
   Set<PersonUser> findDisabledUsers(String searchString, int limit) throws Exception
   {
       Set<PersonUser> users = Collections.emptySet();
       try
       {
           // try GC first, if failed fall back in domain
           users = findDisabledUsersInGc(searchString, limit);
       }
       catch(Exception e)
       {
           if (e instanceof SizeLimitExceededLdapException)
           {
               throw e;
           }
           // try current domain
           users = findDisabledUsersInDomain(searchString, limit);
       }

       return users;
   }

   @Override
   public Set<PersonUser> findLockedUsers(String searchString, int limit) throws Exception
   {
       Set<PersonUser> users = Collections.emptySet();
       try
       {
           // try GC first, if failed fall back in domain
           users = findLockedUsersInGC(searchString, limit);
       }
       catch(Exception e)
       {
           if (e instanceof SizeLimitExceededLdapException)
           {
               throw e;
           }
           // try current domain
           users = findLockedUsersInDomain(searchString, limit);
       }

       return users;
   }

   @Override
   public SearchResult find(String searchString, String domainName, int limit) throws Exception
   {
      Set<PersonUser> users = this.findUsers(searchString, domainName, limit<0? -1:(limit/2 + limit%2));
      int limitGroup = limit<0? -1:(limit- ((users != null)? users.size() : 0));
      Set<Group> groups = null;
      if (limitGroup != 0)
      {
         groups = this.findGroups(searchString, domainName, limitGroup);
      }
      return new SearchResult( users, null, groups );
   }

   @Override
   public SearchResult findByName(String searchString, String domainName, int limit) throws Exception
   {
      return new SearchResult(
            this.findUsersByName(searchString, domainName, limit < 0 ? -1 : limit/2),
            null, /* service principals */
            this.findGroupsByName(searchString, domainName, limit < 0 ? -1 : limit/2+limit%2));
   }

   ////////////////////////////////////////////
   // protected/private
   ////////////////////////////////////////////

   protected
   String
   getPrimaryGroupDN(
       ILdapConnectionEx connection,
       String          searchBaseDn,
       byte[]          userObjectSID,
       int             groupRID,
       IIdmAuthStatRecorder authStatRecorder) throws NoSuchGroupException
   {
      SecurityIdentifier sid = SecurityIdentifier.build(userObjectSID);

      sid.setRID(groupRID);

      String groupSidString = sid.toString();

      String[] attrNames = { };

      String filter = String.format(
              _adSchemaMapping.getGroupQueryByObjectUniqueId(),
              LdapFilterString.encode(groupSidString));

      long startTime = System.currentTimeMillis();

      ILdapMessage message = connection.search(
            searchBaseDn,
            LdapScope.SCOPE_SUBTREE,
            filter,
            attrNames,
            true);

      if (authStatRecorder != null) {
          authStatRecorder.add(new LdapQueryStat(filter, searchBaseDn,
                  getConnectionString(connection), System.currentTimeMillis() - startTime, 1));
      }

      try
      {
         ILdapEntry[] entries = message.getEntries();

         if (entries == null || entries.length == 0)
         {
             throw new NoSuchGroupException(
                     String.format( "Group was not found. GroupSID= '%s'.", groupSidString)
                     );
         }
         else if (entries.length != 1)
         {
            throw new IllegalStateException("Entry length >1");
         }

         return entries[0].getDN();
      }
      finally
      {
          message.close();
      }
   }

   protected
   Collection<Group>
   findGroupsByDNsWithFilter(
           ILdapConnectionEx connection,
           Collection<String> memberDNs,
           String          filter
           ) throws Exception
   {
       ArrayList<Group> groups = new ArrayList<Group>();

       final String ATTR_NAME_SAM_ACCOUNT = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
       final String ATTR_DESCRIPTION = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
       final String ATTR_OBJECT_SID = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

       String[] attrNames = { ATTR_NAME_SAM_ACCOUNT, ATTR_DESCRIPTION, ATTR_OBJECT_SID };

       if (memberDNs != null && !memberDNs.isEmpty())
       {
           filter = _adSchemaMapping.getDNFilter(filter, memberDNs);
           ILdapMessage message = null;

           try
           {
               message = connection.search(
                       "",
                       LdapScope.SCOPE_SUBTREE,
                       filter,
                       attrNames,
                       false);
               if( message.getEntries() != null && message.getEntries().length > 0 )
               {
                   ILdapEntry[] groupEntries = message.getEntries();

                   if (groupEntries != null)
                   {
                       for (ILdapEntry groupEntry : groupEntries)
                       {
                           Group group =  this.buildGroup(groupEntry, null);
                           if (group != null)
                           {
                               groups.add(group);
                           }
                       }
                   }
               }

           }
           finally
           {
               if (message != null)
               {
                   message.close();
               }
           }
       }

       return groups;
   }

   protected
   Collection<PersonUser>
   findUsersByDNsWithFilter(
         ILdapConnectionEx gc_connection,
         Collection<String> memberDNs,
         String          filter
         ) throws Exception
   {
      ArrayList<PersonUser> users = new ArrayList<PersonUser>();

      final String ATTR_FIRST_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
      final String ATTR_LAST_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
      final String ATTR_EMAIL_ADDRESS = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail);
      final String ATTR_DESCRIPTION = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription);
      final String ATTR_NAME_SAM_ACCOUNT = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
      final String ATTR_NAME_USER_ACCT_CTRL = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);
      final String ATTR_USER_PRINCIPAL_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);

      String[] attrNames = {
            ATTR_NAME_SAM_ACCOUNT,
            ATTR_USER_PRINCIPAL_NAME,
            ATTR_DESCRIPTION,
            ATTR_FIRST_NAME,
            ATTR_LAST_NAME,
            ATTR_EMAIL_ADDRESS,
            ATTR_NAME_USER_ACCT_CTRL
      };

      if (memberDNs != null && !memberDNs.isEmpty())
      {
          filter = _adSchemaMapping.getDNFilter(filter, memberDNs);

          ILdapMessage message = null;
          try
          {
              message = gc_connection.search(
                      "",
                      LdapScope.SCOPE_SUBTREE,
                      filter,
                      attrNames,
                      false);
              if( message.getEntries() != null && message.getEntries().length > 0 )
              {
                  ILdapEntry[] userEntries = message.getEntries();

                  if (userEntries != null)
                  {
                      for (ILdapEntry userEntry : userEntries)
                      {
                          int flag = this.getOptionalIntegerValue(
                                          userEntry.getAttributeValues(
                                          ATTR_NAME_USER_ACCT_CTRL),
                                          0);

                          PersonUser user = this.buildPersonUser(
                                  userEntry,
                                  flag);
                          if (user != null)
                          {
                              users.add(user);
                          }
                      }
                  }
              }
          }
          finally
          {
              if (message != null)
              {
                 message.close();
              }
          }
      }

      return users;
   }

   private
   PersonUser
   buildPersonUser(
         ILdapEntry entry,
         int        accountFlags
         ) throws Exception
   {
       return this.buildPersonUser( entry, accountFlags, false, null );
   }

   private
   PersonUser
   buildPersonUser(
         ILdapEntry entry,
         int        accountFlags,
         boolean    provideExtendedAccountInfo,
         ILdapConnectionEx connection
         ) throws Exception
   {
      final String ATTR_FIRST_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
      final String ATTR_LAST_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
      final String ATTR_EMAIL_ADDRESS = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail);
      final String ATTR_USER_PRINCIPAL_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);
      final String ATTR_DESCRIPTION = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription);
      final String ATTR_NAME_SAM_ACCOUNT = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);

      final String ATTR_NAME_LOCKOUT_TIME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime);
      final String ATTR_OBJECT_SID = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
      final String ATTR_RESULTANT_PSO = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject);
      final String ATTR_PWD_LAST_SET = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet);

      String accountName = this.getStringValue(
              entry.getAttributeValues(
                    ATTR_NAME_SAM_ACCOUNT));

      String description =
              this.getOptionalLastStringValue(
                    entry.getAttributeValues(
                          ATTR_DESCRIPTION));

      String firstName =
            this.getOptionalStringValue(
                  entry.getAttributeValues(
                        ATTR_FIRST_NAME));

      String lastName =
            this.getOptionalStringValue(
                  entry.getAttributeValues(
                        ATTR_LAST_NAME));

      String email = this.getOptionalStringValue(
            entry.getAttributeValues(
                  ATTR_EMAIL_ADDRESS));

      String upn = this.getOptionalStringValue(
            entry.getAttributeValues(
                  ATTR_USER_PRINCIPAL_NAME));

      String userDomainName = ServerUtils.getDomainFromDN(entry.getDN());
      PrincipalId id = ServerUtils.getPrincipalId(upn, accountName, userDomainName);

      DomainControllerInfo dcInfo = obtainDcInfo(userDomainName);
      PrincipalId alias = null;
      if (dcInfo != null)
      {
          alias = ServerUtils.getPrincipalAliasId(accountName, dcInfo.domainNetBiosName);
      }

      String resultObjectSid = null;
      long pwdLastSet = PersonDetail.UNSPECIFIED_TS_VALUE;
      long lockoutTime = PersonDetail.UNSPECIFIED_LOCKOUT_TIME_VALUE;
      long pwdLifeTime = PersonDetail.UNSPECIFIED_PASSWORD_LIFE_TIME_VALUE;

      if ( provideExtendedAccountInfo )
      {
            resultObjectSid = getUserSid(entry, ATTR_OBJECT_SID);

            String resultantPSODn = getOptionalStringValue(
                    entry.getAttributeValues(ATTR_RESULTANT_PSO));

            pwdLastSet = getOptionalLongValue(
                    entry.getAttributeValues(ATTR_PWD_LAST_SET),
                    PersonDetail.UNSPECIFIED_TS_VALUE);

            lockoutTime = getOptionalLongValue(
                    entry.getAttributeValues(ATTR_NAME_LOCKOUT_TIME),
                    PersonDetail.UNSPECIFIED_LOCKOUT_TIME_VALUE);

            if (pwdLastSet != PersonDetail.UNSPECIFIED_TS_VALUE)
            {
                // timestamp from AD is in 100 nanosecond intervals since January 1, 1601 (UTC)
                // Need to be converted to in secs since Unix epoch (January 1, 1970 (UTC)
                pwdLastSet = (pwdLastSet - INTVLS_100NS_1600TO1970)/INTVLS_100NS_IN_SEC;
            }

            boolean pwdNeverExpired = ((accountFlags & USER_ACCT_DONT_EXPIRE_PASSWD) != 0);

            // if pwdNeverExpired is set in user account control, treat it as non-defined pwd life time.
            if (!pwdNeverExpired)
            {
                pwdLifeTime = getPwdLifeTime(connection, resultantPSODn, ServerUtils.getDomainDN(userDomainName));
            }
      }

      PersonDetail detail = new PersonDetail.Builder()
                                  .firstName(firstName)
                                  .lastName(lastName)
                                  .userPrincipalName(upn)
                                  .emailAddress(email)
                                  .description(description)
                                  .pwdLastSet(pwdLastSet, IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY)
                                  .pwdLifeTime(pwdLifeTime, IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY)
                               .build();

      boolean disabled =  ((accountFlags & USER_ACCT_DISABLED_FLAG) != 0);
      boolean locked   = (((accountFlags & USER_ACCT_LOCKED_FLAG) != 0) || (lockoutTime >= 1));

      return new PersonUser(
            id,
            alias,
            resultObjectSid,
            detail,
            disabled,
            locked);
   }

   private static String getUserSid(ILdapEntry entry, String objectSidAttributeName)
   {
       String sidStr = null;
       if ( ( entry != null ) && (ServerUtils.isNullOrEmpty(objectSidAttributeName) == false) )
       {
           byte[] resultObjectSID =
                ServerUtils.getBinaryValue(entry
                     .getAttributeValues(objectSidAttributeName));

           SecurityIdentifier sid = SecurityIdentifier.build(resultObjectSID);
           sidStr = sid.toString();
       }
       return sidStr;
   }

   private
   Group
   buildGroup(ILdapEntry[] entries, String groupDN) throws NoSuchGroupException
   {
      if (entries == null || entries.length == 0)
      {
          // in case no group found, return null
          return null;
      }
      else if (entries.length != 1)
      {
         throw new IllegalStateException("Entries length > 1");
      }

      return this.buildGroup(entries[0], groupDN);
   }

   private
   Group
   buildGroup(ILdapEntry entry, String groupDN) throws NoSuchGroupException
   {
       final String ATTR_NAME_SAM_ACCOUNT = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
       final String ATTR_DESCRIPTION = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
       final String ATTR_OBJECT_SID = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

       LdapValue[] values = entry.getAttributeValues(ATTR_NAME_SAM_ACCOUNT);

       if (values == null || values.length == 0)
       {
           throw new NoSuchGroupException("Null Ldap entry");
       }

       if (values.length != 1)
       {
          throw new IllegalStateException("Ldap entry length > 1");
       }

       if (values[0].isEmpty())
       {
           throw new IllegalStateException("Error : Group has no name");
       }

       String groupName = values[0].toString();

       if (groupDN == null)
       {
           groupDN = entry.getDN();
       }

       byte[] resultObjectSID =
                ServerUtils.getBinaryValue(entry.getAttributeValues(ATTR_OBJECT_SID));
       String resultObjectSid = null;

       if (resultObjectSID != null && resultObjectSID.length != 0)
       {
           SecurityIdentifier sid = SecurityIdentifier.build(resultObjectSID);
           resultObjectSid = sid.toString();
       }

       String description = this.getOptionalLastStringValue(
            entry.getAttributeValues(ATTR_DESCRIPTION));

       String domainName = ServerUtils.getDomainFromDN(groupDN);
       PrincipalId gid = new PrincipalId(groupName, domainName);
       PrincipalId alias = null;

       // If group does not belong to the registered AD IDP, need look up its domain NetBios for alias
       if (!this.isSameDomainUpn(domainName))
       {
           DomainControllerInfo dcInfo = obtainDcInfo(domainName);
           if (dcInfo != null)
           {
               alias = ServerUtils.getPrincipalAliasId(groupName, dcInfo.domainNetBiosName);
           }
       }
       else
       {
           alias = ServerUtils.getPrincipalAliasId(groupName, getStoreDataEx().getAlias());
       }

       return new Group(gid, alias, resultObjectSid, new GroupDetail(description));
   }

   private
   String
   getPrincipalDN(
         ILdapConnectionEx connection,
         PrincipalId principalId,
         String searchBaseDn
         ) throws Exception
   {
       String[] attrNames = { };
       AccountLdapEntryInfo ldapEntryInfo = null;

       // search user/groups with samAccount first
       final String filter_by_acct = buildUserOrGroupFilterWithAccountNameByPrincipalId(principalId);
       final String filter_by_upn = buildUserQueryWithUpnByPrincipalId(principalId);

       try
       {
           ldapEntryInfo = findAccountLdapEntry(connection,
                   filter_by_upn,
                   filter_by_acct,
                   searchBaseDn,
                   attrNames,
                   true,
                   principalId);

            return ldapEntryInfo.accountLdapEntry.getDN();
       }
       finally
       {
           if (ldapEntryInfo != null)
           {
               ldapEntryInfo.close_messages();
           }
       }
    }

   private class LdapGroupInfo
   {
      private final String _groupName;
      private final String _groupObjectSid;
      private final String _groupDescription;
      private final String[] _parentGroups;

      LdapGroupInfo(String groupName, String groupObjectSid, String groupDescription, String[] parentGroups)
      {
         this._groupName = groupName;
         this._groupObjectSid = groupObjectSid;
         this._groupDescription = groupDescription;
         this._parentGroups = parentGroups;
      }

      public String getGroupName()
      {
         return this._groupName;
      }
      public String getGroupSid()
      {
         return this._groupObjectSid;
      }
      public String getGroupDescription()
      {
         return this._groupDescription;
      }
      public String[] getParentGroups()
      {
         return this._parentGroups;
      }
   }

   private LdapGroupInfo getGroupInfo(String groupDn, boolean getDescription, List<ILdapQueryStat> ldapQueries) throws Exception
   {
       ILdapConnectionEx connection = null;
       LdapGroupInfo groupInfo = null;

       try
       {
           connection = this.getNonGcConnToDomain(ServerUtils.getDomainFromDN(groupDn));

           String[] ldapGroupAttributes = null;
           final String ATTR_NAME_SAM_ACCOUNT = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
           final String ATTR_DESCRIPTION = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
           final String ATTR_OBJECT_SID = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);
           final String ATTR_MEMBER_OF = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMemberOf);

           if( getDescription == true )
           {
               ldapGroupAttributes = new String[] { ATTR_NAME_SAM_ACCOUNT, ATTR_DESCRIPTION, ATTR_MEMBER_OF, ATTR_OBJECT_SID };
           }
           else
           {
               ldapGroupAttributes = new String[] { ATTR_NAME_SAM_ACCOUNT, ATTR_MEMBER_OF, ATTR_OBJECT_SID };
           }

           long startTime = System.currentTimeMillis();
           String filter = _adSchemaMapping.getAllGroupsQuery();
           ILdapMessage message = connection.search(
                groupDn,
                LdapScope.SCOPE_BASE,
                filter,
                ldapGroupAttributes,
                false);

           if (ldapQueries != null) {
               ldapQueries.add(new LdapQueryStat(filter, groupDn,
                       getConnectionString(connection),
                       System.currentTimeMillis() - startTime, 1));
           }

           try
           {
               ILdapEntry[] entries = message.getEntries();
               if((entries != null) && ( entries.length > 0 ) )
               {
                   String groupName = null;
                   String groupDescription = null;
                   String[] parentGroups = null;
                   String groupObjectSid = null;

                   groupName = ServerUtils.getStringValue(
                      entries[0].getAttributeValues(ATTR_NAME_SAM_ACCOUNT)
                      );

                   byte[] groupObjectSID =
                           ServerUtils.getBinaryValue(
                               entries[0].getAttributeValues(ATTR_OBJECT_SID));

                   if (groupObjectSID != null && groupObjectSID.length != 0)
                   {
                       SecurityIdentifier sid = SecurityIdentifier.build(groupObjectSID);
                       groupObjectSid = sid.toString();
                   }

                   if( getDescription == true )
                   {
                       groupDescription = getOptionalLastStringValue(
                             entries[0].getAttributeValues(ATTR_DESCRIPTION)
                             );
                }

                parentGroups = ServerUtils.getMultiStringValue(
                      entries[0].getAttributeValues(ATTR_MEMBER_OF)
                      );

                groupInfo = new LdapGroupInfo( groupName, groupObjectSid, groupDescription, parentGroups );
             }
          }
          finally
          {
              message.close();
          }
      }
       finally
       {
           if (connection != null)
           {
               connection.close();
           }
       }

      return groupInfo;
   }

   private
   Set<Group>
   getNestedGroups(
         String[]        userGroups,
         boolean         groupNameOnly,
         IIdmAuthStatRecorder authStatRecorder)
   {
      Set<Group> groups = new HashSet<Group>();

      if( ( userGroups != null ) && (userGroups.length > 0) )
      {
         HashSet<String> groupsProcessed = new HashSet<String>();
         Stack<String> groupsToProcess = new Stack<String>();

         for(String groupDn : userGroups )
         {
            if(ServerUtils.isNullOrEmpty(groupDn) == false)
            {
               groupsToProcess.push( groupDn );
            }
         }

         long startTimeForAllGrups = System.currentTimeMillis();
         int numberOfLdapSearches = 0;
         List<ILdapQueryStat> ldapQueries = null;
         if(authStatRecorder != null &&
            (authStatRecorder instanceof IdmAuthStatRecorder)){ // Set ldapQueries only if profiling is required.
             ldapQueries = new LinkedList<ILdapQueryStat>();
         }

         while( groupsToProcess.isEmpty() == false )
         {
            String groupDn = groupsToProcess.pop();
            if ( groupsProcessed.contains(groupDn) == false )
            {
                LdapGroupInfo gi = null;
                try
                {
                    List<ILdapQueryStat> ldapQueriesToBeFilled = ldapQueries;
                    if(authStatRecorder != null &&
                       authStatRecorder.summarizeLdapQueries() &&
                       numberOfLdapSearches > 1){ // Collect only one query if summarizeLdapQueries is set
                        ldapQueriesToBeFilled = null;
                    }

                    gi = this.getGroupInfo( groupDn, !groupNameOnly, ldapQueriesToBeFilled);
                    numberOfLdapSearches += 1;
                }
                catch (Exception e)
                {
                    log.warn(String.format("getGroupInfo failed with %s", e.getMessage()));
                }

                if( gi != null )
                {
                    String groupDomainName = ServerUtils.getDomainFromDN(groupDn);
                    String groupDomainAlias = null;

                    PrincipalId groupId = new PrincipalId( gi.getGroupName(), groupDomainName );
                    PrincipalId groupAlias = null;
                    GroupDetail groupDetail = null;

                    if (groupNameOnly == false)
                    {
                        if (!this.isSameDomainUpn(groupDomainName))
                        {
                            DomainControllerInfo dcInfo = obtainDcInfo(groupDomainName);
                            if (dcInfo != null)
                            {
                                groupDomainAlias = dcInfo.domainNetBiosName;
                            }
                        }
                        else groupDomainAlias = this.getStoreDataEx().getAlias();

                        groupAlias = ServerUtils.getPrincipalAliasId(gi.getGroupName(), groupDomainAlias);

                        groupDetail = new GroupDetail(
                               (gi.getGroupDescription() == null) ? "" : gi.getGroupDescription()
                               );
                     }

                     Group g = new Group( groupId, groupAlias, gi.getGroupSid(), groupDetail );
                     groups.add( g );

                     if ( gi.getParentGroups() != null )
                     {
                         for(String gDn : gi.getParentGroups() )
                         {
                             if(ServerUtils.isNullOrEmpty(gDn) == false)
                             {
                                groupsToProcess.push( gDn );
                             }
                         }
                     }

                     groupsProcessed.add( groupDn );
                 }
             }
         }

         if (authStatRecorder != null) {
             if(!authStatRecorder.summarizeLdapQueries()) {
                 authStatRecorder.add(ldapQueries);
             } else if (ldapQueries.size() > 0){
                 authStatRecorder.add(
                         new LdapQueryStat(
                             ldapQueries.get(0).getQueryString(),
                             ldapQueries.get(0).getBaseDN(),
                             ldapQueries.get(0).getConnectionString(),
                             System.currentTimeMillis() - startTimeForAllGrups,
                             numberOfLdapSearches));
             }
         }
     }

     return groups;
     }

   private long getPwdLifeTime(ILdapConnectionEx connection, String psoDn, String domainName) throws Exception
   {

      if (StringUtils.isNotEmpty(psoDn))
      {
         return getPwdLifeTimeFromPSO(connection, psoDn);
      }
      else
      {
         return getPwdLifeTimeFromDefaultDomainPolicy(connection, domainName);
      }
   }

   private long getPwdLifeTimeFromPSO(ILdapConnectionEx connection, String psoDn) throws Exception
   {
       // we might be able to cache this, will be done later since
       // findUser is not a critical function for perf.

      // PSO time attributes are 64-bit, in -100 ns intervals
      // http://support.microsoft.com/kb/954414

       assert(StringUtils.isNotEmpty(psoDn));
       long maximumPasswordAge = PersonDetail.UNSPECIFIED_PASSWORD_LIFE_TIME_VALUE;
       final String ATTR_MSDS_MAXIMUM_PASSWORD_AGE = _adSchemaMapping.getPwdObjectAttribute(IdentityStoreAttributeMapping.AttributeIds.PasswordSettingsAttributeMaximumPwdAge);
       String[] attributes = { ATTR_MSDS_MAXIMUM_PASSWORD_AGE };

       ILdapMessage message = null;
       try {
           message = connection.search(
                                   psoDn,
                                   LdapScope.SCOPE_BASE,
                                   _adSchemaMapping.getPasswordSettingsQuery(),
                                   attributes,
                                   false);
           ILdapEntry[] entries = message.getEntries();
           if (entries == null || entries.length == 0)
           {
               throw new IllegalStateException(String.format("password policy not found: [%s]", psoDn));
           } else if (entries.length > 1)
           {
               throw new IllegalStateException(
                   String.format("[%d] duplicated password policy found: [%s]", entries.length, psoDn));
           }
           else {
               assert(entries.length == 1);
               maximumPasswordAge = getOptionalLongValue(
                       entries[0].getAttributeValues(ATTR_MSDS_MAXIMUM_PASSWORD_AGE),
                       PersonDetail.UNSPECIFIED_PASSWORD_LIFE_TIME_VALUE);
           }
       }
       finally
       {
           if (message != null) {
               message.close();
           }
       }

       return (-maximumPasswordAge)/INTVLS_100NS_IN_SEC;
   }

   private long getPwdLifeTimeFromDefaultDomainPolicy(
           ILdapConnectionEx connection, String domainDN) throws Exception
   {
      if (_defaultDomainPolicyPwdLifeTimeCache == null)
      {
         synchronized (this)
         {
            if (_defaultDomainPolicyPwdLifeTimeCache == null)
            {// instantiation of immutable object
               _defaultDomainPolicyPwdLifeTimeCache =
                       new PwdLifeTimeValue(
                                retrievePwdLifeTimeFromDefaultDomainPolicy(connection, domainDN));
            }
         }
      }
      return _defaultDomainPolicyPwdLifeTimeCache.value;
  }

   private long retrievePwdLifeTimeFromDefaultDomainPolicy(ILdapConnectionEx connection, String domainDN) throws Exception
   {
       final String ATTR_MAX_PWD_AGE = _adSchemaMapping.getDomainObjectAttribute(IdentityStoreAttributeMapping.AttributeIds.DomainAttributeMaxPwdAge);
       long maxPwdAge = PersonDetail.UNSPECIFIED_PASSWORD_LIFE_TIME_VALUE;
       String[] attributes = { ATTR_MAX_PWD_AGE };
       ILdapMessage message = null;
       try {
          message =
                connection.search(
                      domainDN,
                      LdapScope.SCOPE_BASE,
                      _adSchemaMapping.getDomainObjectQuery(),
                      attributes,
                      false);
          ILdapEntry[] entries = message.getEntries();
          if (entries == null || entries.length == 0) {
             throw new IllegalStateException("Default Domain Policy not found");
          } else if (entries.length > 1) {
             throw new IllegalStateException(
                   String.format("[%d] duplicated default domain policy found",
                         entries.length));
          } else {
             assert (entries.length == 1);
             maxPwdAge =
                   getOptionalLongValue(
                         entries[0].getAttributeValues(ATTR_MAX_PWD_AGE),
                         PersonDetail.UNSPECIFIED_PASSWORD_LIFE_TIME_VALUE);
          }
       } finally {
          if (message != null) {
             message.close();
          }
       }
       return (-maxPwdAge) / INTVLS_100NS_IN_SEC;
   }

   /**
    * Save PAC user info to cache
    *
    * @param user
    */
   public void saveUserInfo(UserInfo user)
   {
       UserInfoEx userResolved = resolveDomainFQDNInAcctInfo(user);
       PrincipalId principal = new PrincipalId(user.getName(), user.getDomain());
       // cache user information
       updateAcctCache(principal, userResolved);
   }

   private void updateAcctCache(PrincipalId principal, UserInfoEx acctInfo)
   {
       String upnName = acctInfo.getUserUpnName();
       _accountCache.addAccount(acctInfo, upnName);
       // principal.getUPN() could be explicit UPN
       if (!principal.getUPN().equalsIgnoreCase(upnName))
       {
           _accountCache.addAccount(acctInfo, principal.getUPN());
       }
   }

   /**
    *  Create an immutable, internal private class to ensure the usage of
    *  immutable object for double-checked locking idiom (JMM guarantees this)
    */

   private final class PwdLifeTimeValue
   {
       private final long value;

       public PwdLifeTimeValue(long aValue)
       {
           value = aValue;
       }
   }

   private class AttrsSet{
       protected List<String> attrNames;
       protected List<Attribute> regularAttrs;
       protected HashMap<String, Attribute> specialAttrs;

       AttrsSet(List<String> attrNames, List<Attribute> regularAttrs, HashMap<String, Attribute> specialAttrs)
       {
           this.attrNames = attrNames;
           this.regularAttrs = regularAttrs;
           this.specialAttrs = specialAttrs;
       }
   }

   private
   AttrsSet
   getMappedAttrNames(
       Collection<Attribute> attributes
       )
   {
       assert (attributes != null);

       List<String> attrNames = new ArrayList<String>();
       List<Attribute> regularAttrs = new ArrayList<Attribute>();
       HashMap<String, Attribute> specialAttrs = new HashMap<String, Attribute>();

       Map<String, String> attrMap = this.getStoreDataEx().getAttributeMap();
       if (attrMap != null)
       {
           for (Attribute attr : attributes)
           {
               String mappedAttr = attrMap.get(attr.getName());
               if (mappedAttr == null)
               {
                   throw new IllegalArgumentException(String.format(
                           "No attribute mapping found for [%s]",
                           attr.getName()));
               }
               if (this._specialAttributes.contains(mappedAttr))
               {
                   specialAttrs.put(mappedAttr, attr);
               } else
               {
                   regularAttrs.add(attr);
                   attrNames.add(mappedAttr);
               }
           }
       }

       if (attrNames.contains(ATTR_MEMBER_OF))
       {
           attrNames.add(ATTR_PRIMARY_GROUP_ID);
       }

       // we need to include object sid all the time
       attrNames.add(ATTR_OBJECT_SID);

       // we need to retrieve ATTR_NAME_SAM_ACCOUNT
       // to make sure we use user name exactly as it is stored
       // in identity provider when constructing UPN
       attrNames.add(ATTR_NAME_SAM_ACCOUNT);

       return new AttrsSet(attrNames, regularAttrs, specialAttrs);

   }

   private
   Collection<AttributeValuePair>
   getAttributesByPac(
       UserInfoEx acctInfo,
       Collection<Attribute> attributes
       ) throws Exception
   {
       List<AttributeValuePair> result = new ArrayList<AttributeValuePair>();

       AttrsSet attrs = getMappedAttrNames(attributes);
       int iAttr = 0;
       AttributeValuePair pairGroupSids = new AttributeValuePair();
       pairGroupSids.setAttrDefinition(new Attribute(IdentityManager.INTERNAL_ATTR_GROUP_OBJECTIDS));
       String userSid = acctInfo.getUserSid();
       if ( ServerUtils.isNullOrEmpty(userSid) == false )
       {
           pairGroupSids.getValues().add(userSid);
       }
       for (Attribute attr : attrs.regularAttrs)
       {
           AttributeValuePair pair = new AttributeValuePair();
           pair.setAttrDefinition(attr);
           String attrName = attrs.attrNames.get(iAttr);

          if (attrName.equalsIgnoreCase(ATTR_MEMBER_OF))
          {
              for (GroupName groupName : acctInfo.getGroupNamesInfo())
              {
                  pair.getValues().add(String.format("%s\\%s",
                                                     groupName.getGroupDomainFQDN(),
                                                     groupName.getGroupAccountName()));
                  pairGroupSids.getValues().add(groupName.getGroupSid());
              }
          }
          else if (attrName.equalsIgnoreCase(ATTR_NAME_SAM_ACCOUNT))
          {
              pair.getValues().add(acctInfo.getUserSamAccount());
          }
          else if (attrName.equalsIgnoreCase(ATTR_USER_PRINCIPAL_NAME))
          {
              // this UPN is already default UPN (constructed using samAccountName@domainName)
              // using pac info
              pair.getValues().add(acctInfo.getUserUpnName());
          }

          result.add(pair);

          iAttr++;
      }
      result.add(pairGroupSids);

      Iterator<String> iter = attrs.specialAttrs.keySet().iterator();
      while (iter.hasNext())
      {
          String key = iter.next();

          if (key.equals(ATTR_SUBJECT_TYPE))
          {
               AttributeValuePair avPair = new AttributeValuePair();
               avPair.setAttrDefinition(attrs.specialAttrs.get(key));
               avPair.getValues().add("false");
               result.add(avPair);
          }
      }

      return result;
   }

   private
   Collection<AttributeValuePair>
   getAttributesByLdap(
        PrincipalId           principalId,
        Collection<Attribute> attributes,
        IIdmAuthStatRecorder authStatRecorder
        ) throws Exception
   {
       List<AttributeValuePair> result = new ArrayList<AttributeValuePair>();
       AttrsSet attrs = getMappedAttrNames(attributes);

       String userName = null;
       ILdapConnectionEx connection = null;
       AccountLdapEntryInfo ldapEntryInfo = null;
       ILdapEntry userLdapEntry = null;

       try
       {
          connection = this.getNonGcConnToDomain(principalId.getDomain());
       }
       catch (Exception ex)
       {
          throw new IDMException("Failed to establish server connection", ex);
       }

       try
       {
           String baseDN = ServerUtils.getDomainDN(principalId.getDomain());

           // find user using upn first (unique in forest)
           final String filter_by_upn = this.buildUserQueryWithUpnByPrincipalId(principalId);
           final String filter_by_acct = this.buildUserQueryWithAccountNameByPrincipalId(principalId);

           ldapEntryInfo = findAccountLdapEntry(connection,
                   filter_by_upn,
                   filter_by_acct,
                   baseDN,
                   attrs.attrNames.toArray(new String[attrs.attrNames.size()]),
                   false,
                   principalId,
                   authStatRecorder);

           userLdapEntry = ldapEntryInfo.accountLdapEntry;
           userName = this.getStringValue(userLdapEntry
                   .getAttributeValues(ATTR_NAME_SAM_ACCOUNT));
           String userSid = getUserSid(userLdapEntry, ATTR_OBJECT_SID);

           int iAttr = 0;

           AttributeValuePair pairGroupSids = new AttributeValuePair();
           pairGroupSids.setAttrDefinition(new Attribute(IdentityManager.INTERNAL_ATTR_GROUP_OBJECTIDS));

           if ( ServerUtils.isNullOrEmpty(userSid) == false )
           {
               pairGroupSids.getValues().add(userSid);
           }

           for (Attribute attr : attrs.regularAttrs)
           {
               AttributeValuePair pair = new AttributeValuePair();

               pair.setAttrDefinition(attr);

               String attrName = attrs.attrNames.get(iAttr);
               LdapValue[] values = null;

               if (attrName.equals(ATTR_USER_PRINCIPAL_NAME))
               {
                   // use iUpn to represent a user
                   List<String> upnVals = pair.getValues();
                   String userDomainName = ServerUtils.getDomainFromDN(userLdapEntry.getDN());
                   upnVals.add(String.format(
                           "%s@%s", userName, userDomainName));
               }
               else
               {
                   values = userLdapEntry.getAttributeValues(attrName);
               }

               if (attrName.equals(ATTR_MEMBER_OF))
               {
                   int primaryGroupRID =
                       ServerUtils.getIntValue(
                               userLdapEntry.getAttributeValues(ATTR_PRIMARY_GROUP_ID));

                   byte[] userObjectSID =
                       ServerUtils.getBinaryValue(
                               userLdapEntry.getAttributeValues(ATTR_OBJECT_SID));

                   String groupSearchBaseDn = ServerUtils.getDomainDN(principalId.getDomain());
                   String primaryGroupDN =
                       this.getPrimaryGroupDN(
                           connection, groupSearchBaseDn, userObjectSID, primaryGroupRID, authStatRecorder);

                   if (ServerUtils.isNullOrEmpty(primaryGroupDN))
                   {
                       throw new IllegalStateException("Error: Found empty Primary Group DN");
                   }

                   List<String> curGroups = new ArrayList<String>();

                   String[] userGroups = ServerUtils.getMultiStringValue(values);

                   if (userGroups != null && userGroups.length > 0)
                   {
                       for (String groupName : userGroups)
                       {
                           curGroups.add(groupName);
                       }
                   }

                   curGroups.add(primaryGroupDN);

                   Set<Group> groups = this.getNestedGroups(
                                            curGroups.toArray(new String[curGroups.size()]), true, authStatRecorder);

                   for (Group group : groups)
                   {
                       pair.getValues().add(group.getNetbios());
                       pairGroupSids.getValues().add(group.getObjectId());
                   }
               } else if (values != null && values.length > 0)
               {
                   for (LdapValue value : values)
                   {
                       if (!value.isEmpty())
                       {
                           String val = value.toString();
                           pair.getValues().add(val);
                       }
                   }
               }

               result.add(pair);

               iAttr++;
           }
           result.add(pairGroupSids);
       }
       finally
       {
           if (ldapEntryInfo != null)
           {
               ldapEntryInfo.close_messages();
           }
           if (connection != null)
           {
               connection.close();
           }
       }

       Iterator<String> iter = attrs.specialAttrs.keySet().iterator();
       while (iter.hasNext())
       {
           String key = iter.next();

           if (key.equals(ATTR_SUBJECT_TYPE))
           {
               AttributeValuePair avPair = new AttributeValuePair();

               avPair.setAttrDefinition(attrs.specialAttrs.get(key));
               avPair.getValues().add("false");
               result.add(avPair);
           }
       }

       return result;
   }

   private
   Set<Group> findNestedParentGroupsByPac(UserInfoEx acctInfo) throws Exception
   {
       Set<Group> groups = new HashSet<Group>();

       if (acctInfo != null && acctInfo.getGroupNamesInfo() != null)
       {
           for (GroupName groupNameInfo : acctInfo.getGroupNamesInfo())
           {
               if (groupNameInfo == null) continue;

               groups.add(new Group(new PrincipalId(groupNameInfo.getGroupAccountName(), groupNameInfo.getGroupDomainFQDN()),
                                    new PrincipalId(groupNameInfo.getGroupAccountName(), groupNameInfo.getGroupDomainNetbios()),
                                    groupNameInfo.getGroupSid(),
                                    new GroupDetail("")));
           }
       }

       return groups;
   }

   private
   Set<Group> findNestedParentGroupsByLdap(PrincipalId userId) throws Exception
   {
      Set<Group> groups = Collections.emptySet();

      ILdapConnectionEx connection = this.getNonGcConnToDomain(userId.getDomain());

      AccountLdapEntryInfo ldapEntryInfo = null;
      try
      {
         String baseDN = ServerUtils.getDomainDN(userId.getDomain());

         final String ATTR_MEMBER_OF = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeMemberOf);
         String attributes[] = { ATTR_MEMBER_OF };
         String[] userGroups = null;

         // look up user by upn first (upn is unique in forest)
         final String filter_by_upn = this.buildUserQueryWithUpnByPrincipalId(userId);
         final String filter_by_acct = this.buildUserQueryWithAccountNameByPrincipalId(userId);

         ldapEntryInfo = findAccountLdapEntry(connection,
                                              filter_by_upn,
                                              filter_by_acct,
                                              baseDN,
                                              attributes,
                                              false,
                                              userId);

         userGroups = ServerUtils.getMultiStringValue(ldapEntryInfo.accountLdapEntry.getAttributeValues( ATTR_MEMBER_OF ));

         groups = this.getNestedGroups( userGroups, false, null );
      }
      finally
      {
          if (ldapEntryInfo != null)
          {
              ldapEntryInfo.close_messages();
          }
          if (connection != null)
          {
              connection.close();
          }
      }

      return groups;
   }

   // return the principal ID constructed using samAccountName and domainName
   private PrincipalId findUserPrincipalIdByAcctAdapter(PrincipalId id) throws InvalidPrincipalException
   {
       try
       {
          IAccountAdapter accountAdapter = AccountAdapterFactory.getInstance().getAccountAdapter();
           AccountInfo acctInfo = accountAdapter.lookupByName(id.getUPN());
            if (acctInfo.acctType != ACCOUNT_TYPE.USER)
            {
              throw new InvalidPrincipalException(
                       String.format(
                             "Principal id %s does not exist",
                             id.getUPN()), id.getUPN());
            }
           // returned acctName is NT4 style
           String acctName = acctInfo.accountName;
           int idx = acctName.indexOf('\\');
           if (idx <= 0)
           {
              throw new IllegalStateException(
                    String.format(
                          "Invalid account name format for [%s]",
                          acctName));
           }
           String samAccountName = acctName.substring(idx + 1);
           String domainNetBiosName = acctName.substring(0, idx);

           DomainControllerInfo dcInfo = obtainDcInfo(domainNetBiosName);

           return dcInfo != null ? new PrincipalId(samAccountName, dcInfo.domainName) : id;
       }
       catch(AccountManagerNativeException|AccountManagerException e)
       {
           throw new InvalidPrincipalException(
                 String.format(
                       "Principal id %s does not exist",
                       id.getUPN()), id.getUPN());
       }
   }

   // The given principalId is already normalized samAccountName@domainName
   private PersonUser findUserByAcctAdapter(PrincipalId id) throws InvalidPrincipalException
   {
       try
       {
           String samAccountName = id.getName();
           String domainName = id.getDomain();
           IAccountAdapter accountAdapter = AccountAdapterFactory.getInstance().getAccountAdapter();

           com.vmware.identity.interop.accountmanager.UserInfo userInfo =
                   accountAdapter.findUserByName(samAccountName, domainName);

           DomainControllerInfo dcInfo = obtainDcInfo(id.getDomain());

           return new PersonUser(
                     id,
                     dcInfo != null ? new PrincipalId(samAccountName, dcInfo.domainNetBiosName) : id,
                     userInfo.accountSid,
                     new com.vmware.identity.idm.PersonDetail.Builder()
                     .firstName("")
                     .lastName("")
                     .emailAddress("")
                     .description("")
                     .userPrincipalName(id.getUPN()).build(),
                     userInfo.bIsDisabled,
                     userInfo.bIsLocked);
       }
       catch(AccountManagerNativeException|AccountManagerException e)
       {
          throw new InvalidPrincipalException(
                 String.format(
                       "Principal id %s does not exist",
                       id.getUPN()), id.getUPN());
       }
   }

   private PersonUser findUserByLdap(PrincipalId id) throws Exception
   {
       PersonUser user = null;
      ILdapConnectionEx connection = null;
      AccountLdapEntryInfo ldapEntryInfo = null;

      try
      {
         connection = this.getNonGcConnToDomain(id.getDomain());
      }
      catch (Exception ex)
      {
         throw new IDMException("Failed to establish server connection", ex);
      }

      try
      {
          String searchBaseDn = ServerUtils.getDomainDN(id.getDomain());

          final String ATTR_FIRST_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
          final String ATTR_USER_PRINCIPAL_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);
          final String ATTR_LAST_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
          final String ATTR_EMAIL_ADDRESS = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail);
          final String ATTR_DESCRIPTION = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription);
          final String ATTR_NAME_SAM_ACCOUNT = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
          final String ATTR_NAME_USER_ACCT_CTRL = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);
          final String ATTR_NAME_LOCKOUT_TIME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime);
          final String ATTR_OBJECT_SID = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
          final String ATTR_RESULTANT_PSO = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject);
          final String ATTR_PWD_LAST_SET = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet);

          String[] attrNames = {
               ATTR_FIRST_NAME,
               ATTR_LAST_NAME,
               ATTR_EMAIL_ADDRESS,
               ATTR_DESCRIPTION,
               ATTR_NAME_SAM_ACCOUNT,
               ATTR_USER_PRINCIPAL_NAME,
               ATTR_NAME_USER_ACCT_CTRL,
               ATTR_NAME_LOCKOUT_TIME,
               ATTR_OBJECT_SID,
               ATTR_RESULTANT_PSO,
               ATTR_PWD_LAST_SET,
         };

         final String filter_by_upn = this.buildUserQueryWithUpnByPrincipalId(id);
         final String filter_by_acct = this.buildUserQueryWithAccountNameByPrincipalId(id);

         ldapEntryInfo = findAccountLdapEntry(connection,
                                              filter_by_upn,
                                              filter_by_acct,
                                              searchBaseDn,
                                              attrNames,
                                              false,
                                              id);
         int accountFlags =
                 this.getOptionalIntegerValue(
                         ldapEntryInfo.accountLdapEntry.getAttributeValues(
                             ATTR_NAME_USER_ACCT_CTRL),
                             0);

         user = this.buildPersonUser(ldapEntryInfo.accountLdapEntry, accountFlags, true, connection);
      }
      finally
      {
         if (ldapEntryInfo != null)
         {
             ldapEntryInfo.close_messages();
         }
         if (connection != null)
         {
             connection.close();
         }
      }

      return user;
   }

   private Group findGroupByAcctAdapter(PrincipalId id) throws InvalidPrincipalException
   {
       try
       {
           IAccountAdapter accountAdapter = AccountAdapterFactory.getInstance().getAccountAdapter();
           // On linux, lookup group needs NT4 style name, so try to get a NT4 name
           String domainName = id.getDomain();
           DomainControllerInfo dcInfo = obtainDcInfo(domainName);
           String groupName = dcInfo != null ? String.format("%s\\%s", dcInfo.domainNetBiosName, id.getName())
                                              : id.getUPN();
           AccountInfo acctInfo = accountAdapter.lookupByName(groupName);

           if (acctInfo.acctType != ACCOUNT_TYPE.GROUP)
           {
             throw new InvalidPrincipalException(
                      String.format(
                            "Principal id %s does not exist",
                            id.getUPN()), id.getUPN());
           }
           // returned acctName is NT4 style
           String acctName = acctInfo.accountName;
           int idx = acctName.indexOf('\\');
           if (idx <= 0)
           {
              throw new IllegalStateException(
                    String.format(
                          "Invalid account name format for [%s]",
                          acctName));
           }
           String samAccountName = acctName.substring(idx + 1);
           String domainNetBiosName = acctName.substring(0, idx);

           return new Group(
                     dcInfo != null ? new PrincipalId(samAccountName, dcInfo.domainName) : id,
                     new PrincipalId(samAccountName, domainNetBiosName),
                     acctInfo.accountSid,
                     new GroupDetail(""));
       }
       catch(AccountManagerNativeException|AccountManagerException e)
       {
           throw new InvalidPrincipalException(
                 String.format(
                       "Principal id %s does not exist",
                       id.getUPN()), id.getUPN());
       }
   }

   private Group findGroupByLdap(PrincipalId groupId) throws Exception
   {
       final String ATTR_NAME_SAM_ACCOUNT = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
       final String ATTR_DESCRIPTION = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
       final String ATTR_OBJECT_SID = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

       String[] attrNames = { ATTR_NAME_SAM_ACCOUNT, ATTR_DESCRIPTION, ATTR_OBJECT_SID};
       String filter = String.format(_adSchemaMapping.getGroupQueryByAccountName(), LdapFilterString.encode(groupId.getName()));

       ILdapConnectionEx connection = null;

       try
       {
          connection = this.getNonGcConnToDomain(groupId.getDomain());
       }
       catch (Exception ex)
       {
          throw new IDMException("Failed to establish server connection", ex);
       }

       try
       {
          ILdapMessage message = connection.search(
                ServerUtils.getDomainDN(groupId.getDomain()),
                LdapScope.SCOPE_SUBTREE,
                filter,
                attrNames,
                false);

          try
          {
             return this.buildGroup(message.getEntries(), null /* group dn */);
          }
          finally
          {
              message.close();
          }
       }
       finally
       {
           if (connection != null)
           {
               connection.close();
           }
       }
   }

   private String buildUserOrGroupFilterWithAccountNameByPrincipalId(PrincipalId principal)
   {
       ValidateUtil.validateNotNull(principal, "principal");

       String escapedsAMAccountName = LdapFilterString.encode(principal.getName());
           return String.format(
               this._adSchemaMapping.getUserOrGroupQueryByAccountName(), escapedsAMAccountName );
   }

   private String buildUserQueryWithAccountNameByPrincipalId(PrincipalId principal)
   {
       ValidateUtil.validateNotNull(principal, "principal");

       String escapedsAMAccountName = LdapFilterString.encode(principal.getName());
           return String.format(
               this._adSchemaMapping.getUserQueryByAccountName(), escapedsAMAccountName );
   }

   private String buildUserQueryWithUpnByPrincipalId(PrincipalId principal)
   {
       ValidateUtil.validateNotNull(principal, "principal");

       String escapedPrincipalName = LdapFilterString.encode(GetUPN(principal));
           return String.format(
               this._adSchemaMapping.getUserQueryByUpn(), escapedPrincipalName );
   }

   private ILdapConnectionEx getLdapConnection(String domainName, boolean bUseGC, boolean bForceRediscover) throws Exception
   {
       ILdapConnectionEx conn = null;
       DomainControllerInfo dcInfo = bForceRediscover
                                     ? obtainDcInfoWithRediscover(domainName)
                                     : obtainDcInfo(domainName);
       ArrayList<String> connStrs = new ArrayList<String>();
       if (dcInfo != null)
       {
           if (!ServerUtils.isNullOrEmpty(dcInfo.domainFQDN))
           {
               connStrs.add(String.format("ldap://%s", dcInfo.domainFQDN));
           }
           else
           {
               // use GC domain name directly
               connStrs.add(String.format("ldap://%s", domainName));
           }
           conn = this.getConnection(connStrs, bUseGC);
       }

       return conn;
   }

   // Get a GC ldap connection to the registered AD provider domain
   private
   ILdapConnectionEx
   getGcConnForDomain(String domainName) throws Exception
   {
       return getAdConnection(domainName, true);
   }

   private
   ILdapConnectionEx
   getNonGcConnToDomain(String domainName) throws Exception
   {
       return getAdConnection(domainName, false);
   }

   private
   ILdapConnectionEx
   getAdConnection(String domainName, boolean bUseGC) throws Exception
   {
       ILdapConnectionEx conn = null;
       try
       {
           conn = getLdapConnection(domainName, bUseGC, false);
       }
       // Ideally only catch IdmNativeException where error code == LW_LDAP_SERVER_DOWN
       // ServerDownLdap is caught and 'translated' to 'SaslBindFailLdapException' on windows
       catch(ServerDownLdapException|IdmNativeException|SaslBindFailLdapException e)
       {
           // in case connection cannot be established remove the dcInfo from DcInfoCache
           // i.e. when the primary DC goes down, we want to remove this from dcCache and re-affinitize
           removeDcInfo(domainName);
           log.error(String.format("Failed to get %s connection to domain %s - domain controller might be offline",
                     bUseGC ? "GC" : "non-GC", domainName), e);

           try
           {
               conn = getLdapConnection(domainName, bUseGC, true);
           }
           catch(Exception e_inner)
           {
               String errMsg = String.format("Failed to get %s connection to domain %s in retry",
                                      bUseGC ? "GC" : "non-GC", domainName);
               log.error(errMsg, e);

               throw new IDMException(errMsg);
           }
       }

       return conn;
   }

   private PersonUser findUserByObjectIdInDomain(String userObjectSid) throws Exception
   {
       ILdapConnectionEx connection = null;

       try
       {
           connection = this.getNonGcConnToDomain(this.getDomain());
       }
       catch (Exception ex)
       {
           throw new IDMException("Failed to establish server connection", ex);
       }

       try
       {
           return findUserByObjectIdInternal(userObjectSid,
                                             connection,
                                             this.getStoreDataEx().getUserBaseDn());
       }
       finally
       {
           if (connection != null)
           {
               connection.close();
           }
       }
   }

   private PersonUser findUserByObjectIdInGC(String userObjectSid) throws Exception
   {
       ILdapConnectionEx connection = null;

       try
       {
           connection = this.getGcConnForDomain(this.getDomain());
       }
       catch (Exception ex)
       {
           throw new IDMException("Failed to establish server connection", ex);
       }

       try
       {
           return findUserByObjectIdInternal(userObjectSid,
                                             connection,
                                             "");
       }
       finally
       {
           if (connection != null)
           {
               connection.close();
           }
       }
   }

   private PersonUser findUserByObjectIdInternal(String userObjectSid, ILdapConnectionEx connection, String searchBaseDn) throws Exception
   {
      ValidateUtil.validateNotEmpty(userObjectSid, "User ObjectSid");

      PersonUser user = null;
      ILdapMessage message = null;

      try
      {
         final String ATTR_FIRST_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
         final String ATTR_LAST_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
         final String ATTR_EMAIL_ADDRESS = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail);
         final String ATTR_DESCRIPTION = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription);
         final String ATTR_NAME_SAM_ACCOUNT = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
         final String ATTR_USER_PRINCIPAL_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);
         final String ATTR_NAME_USER_ACCT_CTRL = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);
         final String ATTR_NAME_LOCKOUT_TIME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime);
         final String ATTR_OBJECT_SID = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
         final String ATTR_RESULTANT_PSO = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject);
         final String ATTR_PWD_LAST_SET = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet);

         String[] attrNames = {
               ATTR_FIRST_NAME,
               ATTR_LAST_NAME,
               ATTR_EMAIL_ADDRESS,
               ATTR_DESCRIPTION,
               ATTR_NAME_SAM_ACCOUNT,
               ATTR_USER_PRINCIPAL_NAME,
               ATTR_NAME_USER_ACCT_CTRL,
               ATTR_NAME_LOCKOUT_TIME,
               ATTR_OBJECT_SID,
               ATTR_RESULTANT_PSO,
               ATTR_PWD_LAST_SET
         };

         String filter = String.format(
               _adSchemaMapping.getUserQueryByObjectUniqueId(),
               LdapFilterString.encode(userObjectSid));

         message = connection.search(searchBaseDn, LdapScope.SCOPE_SUBTREE,
               filter, attrNames, false);

         ILdapEntry[] entries = message.getEntries();

         if (entries == null || entries.length != 1)
         {
            // Use doesn't exist or multiple user same name
            throw new InvalidPrincipalException(
                  String.format(
                        "user with objectSid %s doesn't exist or multiple users with same name",
                        userObjectSid), userObjectSid);
         }

         int accountFlags =
                 this.getOptionalIntegerValue(
                             entries[0].getAttributeValues(
                             ATTR_NAME_USER_ACCT_CTRL),
                             0);

         user = this.buildPersonUser(entries[0], accountFlags, true, connection);
      }
      finally
      {
          if (null != message)
          {
              message.close();
          }
      }

      return user;
   }

   private Set<Group> findGroupsInternal(String filter, String domainName, int limit) throws Exception
   {
       if (ServerUtils.isNullOrEmpty(domainName))
       {
           throw new InvalidArgumentException("findGroupsInternal failed - domainName should not be null or empty");
       }

       Set<Group> groups = new HashSet<Group>();

       // Short circuit since we're being asked to send back an empty list anyway
       if (limit == 0)
       {
           return groups;
       }

       ILdapConnectionEx connection = null;
       String searchBaseDn = ServerUtils.getDomainDN(domainName);

       try
       {
           try
           {
               if (this.isSameDomainUpn(domainName))
               {
                   connection = this.getNonGcConnToDomain(this.getDomain());
                   searchBaseDn = ServerUtils.getDomainDN(this.getDomain());
               }
               else
               {
                   connection = this.getNonGcConnToDomain(domainName);
               }
           }
           catch (Exception ex)
           {
               throw new IDMException("Failed to establish server connection", ex);
           }


           final String ATTR_NAME_SAM_ACCOUNT = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
           final String ATTR_DESCRIPTION = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
           final String ATTR_OBJECT_SID = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

           String[] attrNames = { ATTR_NAME_SAM_ACCOUNT, ATTR_DESCRIPTION, ATTR_OBJECT_SID };

           Collection<ILdapMessage> messages = connection.paged_search(
                   searchBaseDn,
                   LdapScope.SCOPE_SUBTREE,
                   filter,
                   Arrays.asList(attrNames),
                   DEFAULT_PAGE_SIZE,
                   limit);

           if (messages != null && messages.size() > 0)
           {
               for (ILdapMessage message : messages)
               {
                   try
                   {
                       ILdapEntry[] entries = message.getEntries();

                       if (entries != null && entries.length > 0)
                       {
                           for (ILdapEntry entry : entries)
                           {
                               groups.add(this.buildGroup(entry, null));
                           }
                       }
                   }
                   finally
                   {
                       if (message != null)
                       {
                           message.close();
                       }
                   }
               }
           }
       }
       finally
       {
           if (connection != null)
           {
               connection.close();
           }
       }

       return groups;
   }

   private Set<Group> findGroupsInGroupInternal(PrincipalId groupId, String filter, int limit) throws Exception
   {
       ValidateUtil.validateNotNull(groupId, "groupId");

       Set<Group> groups = new HashSet<Group>();

       // Short circuit since they're asking for a list of nothing anyway
       if (limit == 0)
       {
          return groups;
       }

       groupId = normalizeAliasInPrincipalWithTrusts(groupId, false);
       ILdapConnectionEx connection = null;

       try
       {
           connection = this.getNonGcConnToDomain(groupId.getDomain());

           int currRange = 1;
           boolean bContinueSearch = true;
           Collection<Group> groupsInOneRange = Collections.emptyList();
           MemberDnsResult membersResult = new MemberDnsResult(new ArrayList<String>(), false);

           int currLimit = limit;
           int rangeSize = DEFAULT_RANGE_SIZE;
           if (limit > 0 && limit < DEFAULT_RANGE_SIZE)
           {
               rangeSize = limit;
           }

           ILdapConnectionEx gc_connection = null;
           try
           {
               gc_connection = this.getGcConnForDomain(this.getDomain());

               do
               {
                   membersResult = findMemberDnsInGroupInRange(connection,
                                                               _adSchemaMapping,
                                                               groupId,
                                                               ServerUtils.getDomainDN(groupId.getDomain()),
                                                               currRange,
                                                               rangeSize);

                   if (membersResult != null && !membersResult.memberDns.isEmpty())
                   {
                       try
                       {
                           groupsInOneRange = this.findGroupsByDNsWithFilter(gc_connection, membersResult.memberDns, filter);
                       }
                       catch (Exception ex)
                       {
                           log.warn(
                                String.format(
                                    "Find groups in group %s By distinguishedName failed for range %d - %d ",
                                    groupId.getUPN(), (currRange-1)*rangeSize, currRange*rangeSize-1), ex);
                       }

                       if (!groupsInOneRange.isEmpty())
                       {
                           if (groupsInOneRange.size() < currLimit || currLimit < 0)
                           {
                               groups.addAll(groupsInOneRange);

                               if (currLimit > 0)
                               {
                                   currLimit = currLimit - groupsInOneRange.size();
                               }

                           }
                           else
                           {
                               groups.addAll(new ArrayList<Group>(groupsInOneRange).subList(0, currLimit));
                               bContinueSearch = false;
                           }
                       }
                   }
                   currRange++;
               }
               while(membersResult.memberDns.size()==rangeSize &&
                       bContinueSearch && !membersResult.bNoMoreEntries);
           }
           finally
           {
               if (gc_connection != null)
               {
                   gc_connection.close();
               }
           }
       }
       finally
       {
           if (connection != null)
           {
               connection.close();
           }
       }

       return groups;
   }

   private Set<PersonUser> findUsersInternal(String filter, String domainName, int limit) throws Exception
   {
       if (ServerUtils.isNullOrEmpty(domainName))
       {
           throw new InvalidArgumentException("findUsersInternal failed - domainName should not be null or empty");
       }

       Set<PersonUser> users = new HashSet<PersonUser>();
       // Short circuit since we're being asked for an empty result anyway
       if (limit == 0)
       {
           return users;
       }

       ILdapConnectionEx connection = null;
       String searchBaseDn = ServerUtils.getDomainDN(domainName);

       try
       {
           try
           {
               if (this.isSameDomainUpn(domainName))
               {
                   connection = this.getNonGcConnToDomain(this.getDomain());
                   searchBaseDn = ServerUtils.getDomainDN(this.getDomain());
               }
               else
               {
                   connection = this.getNonGcConnToDomain(domainName);
               }
           }
           catch (Exception ex)
           {
               throw new IDMException("Failed to establish server connection", ex);
           }

           final String ATTR_FIRST_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
           final String ATTR_LAST_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
           final String ATTR_EMAIL_ADDRESS = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail);
           final String ATTR_DESCRIPTION = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription);
           final String ATTR_NAME_SAM_ACCOUNT = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
           final String ATTR_NAME_USER_ACCT_CTRL = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);
           final String ATTR_USER_PRINCIPAL_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);

           String[] attrNames = {
                   ATTR_NAME_SAM_ACCOUNT,
                   ATTR_USER_PRINCIPAL_NAME,
                   ATTR_DESCRIPTION,
                   ATTR_FIRST_NAME,
                   ATTR_LAST_NAME,
                   ATTR_EMAIL_ADDRESS,
                   ATTR_NAME_USER_ACCT_CTRL
           };

           Collection<ILdapMessage> messages = connection.paged_search(
                   searchBaseDn,
                   LdapScope.SCOPE_SUBTREE,
                   filter,
                   Arrays.asList(attrNames),
                   DEFAULT_PAGE_SIZE,
                   limit);

           if (messages != null && messages.size() > 0)
           {
               for (ILdapMessage message : messages)
               {
                   try
                   {

                       ILdapEntry[] entries = message.getEntries();

                       if (entries != null && entries.length > 0)
                       {
                           for (ILdapEntry entry : entries)
                           {
                               int flag = this.getOptionalIntegerValue(
                                      entry.getAttributeValues(
                                      ATTR_NAME_USER_ACCT_CTRL),
                                      0);

                               users.add(this.buildPersonUser(entry, flag));
                           }
                       }
                   }
                   finally
                   {
                       if (message != null)
                       {
                           message.close();
                       }
                   }

               }
           }

       }
       finally
       {
           if (connection != null)
           {
               connection.close();
           }
       }

       return users;
   }

   private Set<PersonUser> findUsersInGroupInternal(PrincipalId groupId, String filter, int limit) throws Exception
   {
       ValidateUtil.validateNotNull(groupId, "groupId");

       Set<PersonUser> users = new HashSet<PersonUser>();

       // Short circuit since they're asking for a list of nothing anyway
       if (limit == 0)
       {
           return users;
       }

       ILdapConnectionEx connection = null;

       try
       {
           connection = this.getNonGcConnToDomain(groupId.getDomain());

           int currRange = 1;
           boolean bContinueSearch = true;
           Collection<PersonUser> usersInOneRange = Collections.emptyList();
           MemberDnsResult membersResult = new MemberDnsResult(new ArrayList<String>(), false);

           int currLimit = limit;
           int rangeSize = DEFAULT_RANGE_SIZE;
           if (limit > 0 && limit < DEFAULT_RANGE_SIZE)
           {
               rangeSize = limit;
           }

           ILdapConnectionEx gc_connection = null;
           try
           {
               gc_connection = this.getGcConnForDomain(this.getDomain());

               do
               {
                   membersResult = findMemberDnsInGroupInRange(
                           connection,
                           _adSchemaMapping,
                           groupId,
                           ServerUtils.getDomainDN(groupId.getDomain()),
                           currRange,
                           rangeSize);

                   if (membersResult != null && !membersResult.memberDns.isEmpty())
                   {
                       try
                       {
                           usersInOneRange = this.findUsersByDNsWithFilter(gc_connection, membersResult.memberDns, filter);
                       }
                       catch (Exception ex)
                       {
                           log.warn(String.format(
                                   "Find users in group %s By distinguishedName failed for range %d - %d ",
                                   groupId.getUPN(), (currRange - 1) * rangeSize, currRange * rangeSize - 1), ex);
                       }

                       if (!usersInOneRange.isEmpty())
                       {
                           if (usersInOneRange.size() < currLimit || currLimit < 0)
                           {
                               users.addAll(usersInOneRange);

                               if (currLimit > 0)
                               {
                                   currLimit = currLimit - usersInOneRange.size();
                               }

                           }
                           else
                           {
                               users.addAll(new ArrayList<PersonUser>(usersInOneRange).subList(0, currLimit));
                               bContinueSearch = false;
                           }
                       }
                   }
                   currRange++;
               }
               while (membersResult.memberDns.size() == rangeSize && bContinueSearch
                       && !membersResult.bNoMoreEntries);
           }
           finally
           {
               if (gc_connection != null)
               {
                   gc_connection.close();
               }
           }
       }
       finally
       {
           if (connection != null)
           {
               connection.close();
           }
       }

       return users;
   }

   private Group findGroupByObjectIdInGC(String groupObjectSid) throws Exception
   {
       ILdapConnectionEx connection = null;

       try
       {
           connection = this.getGcConnForDomain(this.getDomain());
       }
       catch (Exception ex)
       {
           throw new IDMException("Failed to establish server connection", ex);
       }

       try
       {
            return findGroupByObjectIdInternal(groupObjectSid, connection, "");
       }

       finally
       {
           if (connection != null)
           {
               connection.close();
           }
       }
   }

   private Group findGroupByObjectIdInDomain(String groupObjectSid) throws Exception
   {
       ILdapConnectionEx connection = null;

       try
       {
           connection = this.getNonGcConnToDomain(this.getDomain());
       }
       catch (Exception ex)
       {
           throw new IDMException("Failed to establish server connection", ex);
       }

       try
       {
           return findGroupByObjectIdInternal(groupObjectSid, connection, this.getStoreDataEx().getGroupBaseDn());
       }
       finally
       {
           if (connection != null)
           {
               connection.close();
           }
       }
   }

   private Group findGroupByObjectIdInternal(String groupObjectSid, ILdapConnectionEx connection, String searchBaseDn) throws Exception
   {
       final String ATTR_NAME_SAM_ACCOUNT = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
       final String ATTR_DESCRIPTION = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
       final String ATTR_OBJECT_SID = _adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

       String[] attrNames = { ATTR_NAME_SAM_ACCOUNT, ATTR_DESCRIPTION, ATTR_OBJECT_SID };
       String filter = String.format(_adSchemaMapping.getGroupQueryByObjectUniqueId(), LdapFilterString.encode(groupObjectSid));

       ILdapMessage message = connection.search(
                                       searchBaseDn,
                                       LdapScope.SCOPE_SUBTREE,
                                       filter,
                                       attrNames,
                                       false);

       try
       {
           return this.buildGroup(message.getEntries(), null /* group dn */);
       }
       finally
       {
           if (message != null)
           {
               message.close();
           }
       }
   }

   private Set<PersonUser> findDisabledUsersInGc(String searchString, int limit) throws Exception
   {
       ILdapConnectionEx connection = null;

       try
       {
           connection = this.getGcConnForDomain(this.getDomain());
       }
       catch (Exception ex)
       {
           throw new IDMException("Failed to establish server connection", ex);
       }

       try
       {
           return findDisabledUsersInternal(searchString, connection, "", limit);
       }
       finally
       {
           if (connection != null)
           {
               connection.close();
           }
       }
   }

   private Set<PersonUser> findDisabledUsersInDomain(String searchString, int limit) throws Exception
   {
       ILdapConnectionEx connection = null;

       try
       {
           connection = this.getNonGcConnToDomain(this.getDomain());
       }
       catch (Exception ex)
       {
           throw new IDMException("Failed to establish server connection", ex);
       }

       try
       {
            return findDisabledUsersInternal(searchString,
                                             connection,
                                             this.getStoreDataEx().getUserBaseDn(),
                                             limit);
       }
       finally
       {
           if (connection != null)
           {
               connection.close();
           }
       }
   }

   private
   Set<PersonUser> findDisabledUsersInternal(String searchString, ILdapConnectionEx connection, String searchBaseDn, int limit) throws Exception
   {
       Set<PersonUser> users = new HashSet<PersonUser>();

       final String ATTR_FIRST_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
       final String ATTR_LAST_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
       final String ATTR_EMAIL_ADDRESS = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail);
       final String ATTR_DESCRIPTION = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription);
       final String ATTR_NAME_SAM_ACCOUNT = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
       final String ATTR_NAME_USER_ACCT_CTRL = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);
       final String ATTR_USER_PRINCIPAL_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);

       String[] attrNames = {
           ATTR_NAME_SAM_ACCOUNT,
           ATTR_USER_PRINCIPAL_NAME,
           ATTR_DESCRIPTION,
           ATTR_FIRST_NAME,
           ATTR_LAST_NAME,
           ATTR_EMAIL_ADDRESS,
           ATTR_NAME_USER_ACCT_CTRL
       };

       String filter = _adSchemaMapping.getAllDisabledUsersQuery();

       if (!searchString.isEmpty())
       {
          filter = String.format(
             _adSchemaMapping.getUserQueryByCriteria(),
             LdapFilterString.encode(searchString));
       }

       Collection<ILdapMessage> messages = connection.paged_search(
           searchBaseDn,
           LdapScope.SCOPE_SUBTREE,
           filter,
           Arrays.asList(attrNames),
           DEFAULT_PAGE_SIZE,
           limit);

       if (messages != null && messages.size() > 0)
       {
           for (ILdapMessage message : messages)
           {
               try
               {
                   ILdapEntry[] entries = message.getEntries();

                   if (entries != null && entries.length > 0)
                   {
                       for (ILdapEntry entry : entries)
                       {
                           int flag = this.getOptionalIntegerValue(
                                entry.getAttributeValues(
                                ATTR_NAME_USER_ACCT_CTRL),
                                0);

                            boolean disabled = ((flag & USER_ACCT_DISABLED_FLAG) != 0);

                            if (!disabled)
                            {
                                continue;
                            }

                            users.add(this.buildPersonUser(entry, flag));
                       }
                   }
               }/* innter try */
               finally
               {
                   if (message != null)
                   {
                       message.close();
                   }
               }
           } /* for */
       }/* if */

       return users;
   }

   private Set<PersonUser> findLockedUsersInGC(String searchString, int limit) throws Exception
   {
       ILdapConnectionEx connection = null;

       try
       {
           connection = this.getGcConnForDomain(this.getDomain());
       }
       catch (Exception ex)
       {
           throw new IDMException("Failed to establish server connection", ex);
       }

       try
       {
           return  findLockedUsersInternal(searchString,
                                           connection,
                                           "",
                                           limit);
       }
       finally
       {
           if (connection != null)
           {
                  connection.close();
           }
       }
   }

   private Set<PersonUser> findLockedUsersInDomain(String searchString, int limit) throws Exception
   {
       ILdapConnectionEx connection = null;

       try
       {
            connection = this.getNonGcConnToDomain(this.getDomain());
       }
       catch (Exception ex)
       {
           throw new IDMException("Failed to establish server connection", ex);
       }

       try
       {
            return findLockedUsersInternal(searchString,
                                           connection,
                                           this.getStoreDataEx().getUserBaseDn(),
                                           limit);
       }
       finally
       {
           if (connection != null)
           {
               connection.close();
           }
       }
   }

   private Set<PersonUser> findLockedUsersInternal(String searchString, ILdapConnectionEx connection, String searchBaseDn, int limit) throws Exception
   {
       Set<PersonUser> users = new HashSet<PersonUser>();

       final String ATTR_FIRST_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
       final String ATTR_LAST_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
       final String ATTR_EMAIL_ADDRESS = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail);
       final String ATTR_DESCRIPTION = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription);
       final String ATTR_NAME_SAM_ACCOUNT = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
       final String ATTR_NAME_USER_ACCT_CTRL = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);
       final String ATTR_USER_PRINCIPAL_NAME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);
       final String ATTR_NAME_LOCKOUT_TIME = _adSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime);

       String[] attrNames = {
           ATTR_NAME_SAM_ACCOUNT,
           ATTR_USER_PRINCIPAL_NAME,
           ATTR_DESCRIPTION,
           ATTR_FIRST_NAME,
           ATTR_LAST_NAME,
           ATTR_EMAIL_ADDRESS,
           ATTR_NAME_USER_ACCT_CTRL,
           ATTR_NAME_LOCKOUT_TIME
        };

       String filter = _adSchemaMapping.getAllDisabledUsersQuery();

       if (!searchString.isEmpty())
       {
          filter = String.format(
             _adSchemaMapping.getUserQueryByCriteria(),
             LdapFilterString.encode(searchString));
       }
       Collection<ILdapMessage> messages = connection.paged_search(
           searchBaseDn,
           LdapScope.SCOPE_SUBTREE,
           filter,
           Arrays.asList(attrNames),
           DEFAULT_PAGE_SIZE,
           limit);

       if (messages != null && messages.size() > 0)
       {
           for (ILdapMessage message : messages)
           {
               try
               {
                   ILdapEntry[] entries = message.getEntries();

                   if (entries != null && entries.length > 0)
                   {
                       for (ILdapEntry entry : entries)
                       {
                           int flag = this.getOptionalIntegerValue(
                                  entry.getAttributeValues(
                                  ATTR_NAME_USER_ACCT_CTRL),
                                  0);

                            long lockoutTime = this.getOptionalLongValue(
                                  entry.getAttributeValues(
                                  ATTR_NAME_LOCKOUT_TIME),
                                  PersonDetail.UNSPECIFIED_LOCKOUT_TIME_VALUE);

                            boolean locked = (((flag & USER_ACCT_LOCKED_FLAG) != 0) || (lockoutTime >= 1));

                            if (!locked)
                            {
                                continue;
                            }

                            users.add(this.buildPersonUser(entry, flag));
                        }
                   } /* if */
              }/* inner try */
              finally
              {
                  if (message != null)
                  {
                      message.close();
                  }
              }
          }
       }

       return users;
   }

   private UserInfoEx resolveDomainFQDNInAcctInfo(UserInfo acctInfo)
   {
       ArrayList<GroupName> resolvedGroups = new ArrayList<GroupName>();

       if (acctInfo.getGroups() != null)
       {
           Iterator<String> iGroupNames = acctInfo.getGroups().iterator();
           Iterator<String> iGroupSids = acctInfo.getGroupSids().iterator();
           while(iGroupNames.hasNext())
           {
               String groupName = iGroupNames.next();
               String groupSid = iGroupSids.next();
               if (ServerUtils.isNullOrEmpty(groupName) || ServerUtils.isNullOrEmpty(groupSid)) continue;

               try
               {
                   GroupName groupNameInfo = resolveGroupName(groupName, groupSid);
                   if (groupNameInfo == null)
                   {
                       log.warn(String.format("Failed to resolve group with name [%s], groupSid [%s]", groupName, groupSid));
                   }
                   else
                   {
                       resolvedGroups.add(groupNameInfo);
                   }
               }
               catch(Exception e)
               {
                   log.warn(String.format("Failed to resolve group with name [%s], groupSid [%s]", groupName, groupSid));
               }
           }
       }

       return new UserInfoEx(acctInfo.getName(), acctInfo.getUPN(), resolvedGroups, acctInfo.getUserSid());
   }

   private GroupName resolveGroupName(String groupName, String groupSid)
   {
       ValidateUtil.validateNetBios(groupName, "Group Name");
       GroupName groupNameInfo = null;
       // Convert group domain NetBios to domainFQDN
       // try to find in domain controller cache
       String[] nameParts = groupName.split("\\\\");

       DomainControllerInfo dcInfo = obtainDcInfo(nameParts[0]);
       // if cannot resolve to long FQDN, simply use the name returned by kerberos pac as both domainName and domainAlias
       // This allows localOsGroup be returned in case domain user is a member of a localOsGroup
       groupNameInfo = new GroupName(nameParts[1],
                                     dcInfo != null ? dcInfo.domainName : nameParts[0],
                                     dcInfo != null ? dcInfo.domainNetBiosName : nameParts[0],
                                     groupSid);


       return groupNameInfo;
   }

   private DomainControllerInfo obtainDcInfo(String domainName)
   {
       return obtainDcInfoInternal(domainName, false);
   }

   private DomainControllerInfo obtainDcInfoWithRediscover(String domainName)
   {
       return obtainDcInfoInternal(domainName, true);
   }

   private DomainControllerInfo obtainDcInfoInternal(String domainName, boolean bForceRediscover)
   {
       DcInfoCache dcInfoCache = IdmDomainState.getInstance().getDcInfoCache();

       DomainControllerInfo dcInfo = dcInfoCache.findDcInfo(domainName);
       if (dcInfo == null)
       {
           IDomainAdapter domainAdapter =
                   DomainAdapterFactory.getInstance().getDomainAdapter();

           try
           {
               if (bForceRediscover)
               {
                   dcInfo = domainAdapter.getDcInfoWithRediscover(domainName);
               }
               else
               {
                   dcInfo = domainAdapter.getDcInfo(domainName);
               }
           }
           catch(DomainManagerNativeException|DomainManagerException exDcInfo)
           {
               log.warn(String.format("obtainDcInfo for domain [%s] failed ", domainName) + exDcInfo.getMessage());
           }

           if (dcInfo != null)
           {
               dcInfoCache.addDcInfo(dcInfo);
           }
       }

       return dcInfo;
   }

   private void removeDcInfo(String domainName)
   {
       DcInfoCache dcInfoCache = IdmDomainState.getInstance().getDcInfoCache();

       DomainControllerInfo dcInfo = dcInfoCache.findDcInfo(domainName);
       if (dcInfo != null)
       {
           log.info(String.format("removeDcInfo - domain [%s], domainFQDN [%s], domainIpAddress [%s]",
                                                  domainName, dcInfo.domainFQDN, dcInfo.domainIpAddress));
           dcInfoCache.deleteDcInfo(domainName);
       }
   }

    @Override
    public Collection<SecurityDomain> getDomains() {

        Set<SecurityDomain> domains = new HashSet<SecurityDomain>();

        domains.add(new SecurityDomain(this.getDomain(), this.getAlias()));
        // get domain trusts Info
        DomainTrustInfo[] trustsInfo = IdmDomainState.getInstance().getDomainTrustInfo();
        if (trustsInfo != null && trustsInfo.length > 0)
        {
            for (DomainTrustInfo trust : trustsInfo)
            {
                // 2-way trusts
                if (ServerKrbUtils.IsTwoWayTrusted(trust) || ServerKrbUtils.IsOneWayTrusted(trust))
                {
                    domains.add(new SecurityDomain(
                                            trust.dcInfo.domainName,
                                            trust.dcInfo.domainNetBiosName));
                }
            }
        }

        return Collections.unmodifiableSet(domains);
    }

    // userName used to configure AD with an explicit SPN has to be in default UPN format
    // meaning userName is samAccount@domainName (where domainName cannot be an upn suffix)
    public static void probeAdConnectivity(
       IIdentityStoreData idsData
       ) throws IDMException
    {
       String userUpn = idsData.getExtendedIdentityStoreData().getUserName();
       String userPassword = idsData.getExtendedIdentityStoreData().getPassword();

       int idx = userUpn.indexOf(ValidateUtil.UPN_SEPARATOR);
       String domainName = userUpn.substring(idx + 1);
       String userName = userUpn.substring(0, idx);

       IIdmClientLibrary idmAdapter =
               IdmClientLibraryFactory.getInstance().getLibrary();

       try {
          idmAdapter.AuthenticateByPassword(userName, domainName, userPassword);
      } catch (Exception e) {
          String msg = String.format("probeAdConnectivity failed for %s", userUpn);
          log.error(msg);
          throw new InvalidPrincipalException(msg, userUpn);
      }
   }

   // We may need to Filter out computer accounts in all user query
   /*private String getAllUsersQuery()
   {
       String filter = _adSchemaMapping.getAllUsersQuery();
       //return String.format("(&((!(objectClass=computer))(!(samAccountType=805306370))%s))", filter);
       return String.format("(&(!(objectClass=computer))%s)", filter);
   }

   private String getUsersQueryByCritera(String searchString)
   {
       String filter = String.format(
                  _adSchemaMapping.getUserQueryByCriteria(),
                  searchString);
       return String.format("(&(!(objectClass=computer))%s)", filter);
   }*/
}
