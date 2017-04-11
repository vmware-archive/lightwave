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
 * LDAP Provider
 *
 * @author:  Sriram Nambakam <snambakam@vmware.com>
 *
 * @version: 1.0
 * @since:   2011-12-7
 *
 */

package com.vmware.identity.idm.server.provider.ldap;

import java.security.InvalidParameterException;
import java.security.cert.X509Certificate;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Stack;
import java.util.TimeZone;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

import javax.security.auth.login.LoginException;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
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
import com.vmware.identity.idm.server.performance.IIdmAuthStatRecorder;
import com.vmware.identity.idm.server.provider.BaseLdapProvider;
import com.vmware.identity.idm.server.provider.ILdapSchemaMapping;
import com.vmware.identity.idm.server.provider.NoSuchGroupException;
import com.vmware.identity.idm.server.provider.NoSuchUserException;
import com.vmware.identity.idm.server.provider.PooledLdapConnection;
import com.vmware.identity.idm.server.provider.PrincipalGroupLookupInfo;
import com.vmware.identity.idm.server.provider.UserSet;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.ILdapEntry;
import com.vmware.identity.interop.ldap.ILdapMessage;
import com.vmware.identity.interop.ldap.ILdapMessageEx;
import com.vmware.identity.interop.ldap.LdapFilterString;
import com.vmware.identity.interop.ldap.LdapScope;
import com.vmware.identity.interop.ldap.LdapValue;
import com.vmware.identity.interop.ldap.NoSuchObjectLdapException;
import com.vmware.identity.interop.ldap.SizeLimitExceededLdapException;
import com.vmware.identity.interop.ldap.UnavailableCritExtensionLdapException;
import com.vmware.identity.performanceSupport.IIdmAuthStat.ActivityKind;
import com.vmware.identity.performanceSupport.IIdmAuthStat.EventLevel;

public class LdapProvider extends BaseLdapProvider
{
    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(LdapProvider.class);
    private static enum AccountControlFlag
   {
      FLAG_DISABLED_ACCOUNT(    0x0002),
      FLAG_LOCKED_ACCOUNT  (    0x0010),
      FLAG_PASSWORD_EXPIRED(0x00080000);

      final int value;
      AccountControlFlag(int v)
      {
         value = v;
      }

      public int getValue()
      {
         return value;
      }

      boolean isSet(int accountFlags)
      {
         return  0 != (this.value & accountFlags);
      }
   }

   private static final String SPECIAL_ATTR_SUBJECT_TYPE = "subjectType";
   private static final String SPECIAL_ATTR_USER_PRINCIPAL_NAME = "userPrincipalName";
   private static final String SPECIAL_ATTR_MEMBER_OF = "memberOf";

   private static final int DEFAULT_PAGE_SIZE = 1000;
   // 10000 seems reasonable limit for unlimited non-paged results...
   private static int NON_PAGED_SEARCH_MAX_RESULT_RETURN = DEFAULT_PAGE_SIZE*10;

   private final Set<String> _specialAttributes;
   private final ILdapSchemaMapping _ldapSchemaMapping;
   private static final int INVALID_VALUE_PWD_LOCKOUT_DURATION = -1;
   private static final int UNINITIALIZED_VALUE_PWD_LOCKOUT_DURATION = -2;
   private int _pwdPolicyPwdLockoutDuration = UNINITIALIZED_VALUE_PWD_LOCKOUT_DURATION;

   private final AtomicInteger _pagedResultSupportedFlag;
   private static final int PAGED_RESULT_SUPPORTED_UNKNOWN = 0;
   private static final int PAGED_RESULT_SUPPORTED_YES = 1;
   private static final int PAGED_RESULT_SUPPORTED_NO = 2;
   private static final int LDAP_SIZELIMIT_EXCEEDED = 0x04;

   private final boolean _userGroupMembersListLinkExists;
   private final boolean _userGroupMembersListLinkIsDn;
   private final boolean _groupGroupMembersListLinkExists;
   private final boolean _groupGroupMembersListLinkIsDn;

   private final String USER_GROUP_MEMBERS_LIST_LINK_ATTRIBUTE;
   private final String GROUP_GROUP_MEMBERS_LIST_LINK_ATTRIBUTE;

   public LdapProvider(String tenantName, IIdentityStoreData store)
   {
       this(tenantName, store, null);
   }

   public LdapProvider(String tenantName, IIdentityStoreData store, Collection<X509Certificate> tenantTrustedCertificates)
   {
      super(tenantName, store, tenantTrustedCertificates);
      Validate.isTrue(
            this.getStoreDataEx().getProviderType() == IdentityStoreType.IDENTITY_STORE_TYPE_LDAP,
            "IIdentityStoreData must represent a store of 'IDENTITY_STORE_TYPE_LDAP' type.");

      _specialAttributes = new HashSet<String>();
      _specialAttributes.add(SPECIAL_ATTR_SUBJECT_TYPE.toLowerCase());
      _specialAttributes.add(SPECIAL_ATTR_USER_PRINCIPAL_NAME.toLowerCase());
      _specialAttributes.add(SPECIAL_ATTR_MEMBER_OF.toLowerCase());
      _ldapSchemaMapping = new OpenLdapSchemaMapping( this.getStoreDataEx().getIdentityStoreSchemaMapping() );
      _pagedResultSupportedFlag = new AtomicInteger(PAGED_RESULT_SUPPORTED_UNKNOWN);

      USER_GROUP_MEMBERS_LIST_LINK_ATTRIBUTE = _ldapSchemaMapping.getUserAttribute(
          IdentityStoreAttributeMapping.AttributeIds.UserAttributeGroupMembersListLink
      );
      GROUP_GROUP_MEMBERS_LIST_LINK_ATTRIBUTE = _ldapSchemaMapping.getGroupAttribute(
              IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupMembersListLink
      );
      _userGroupMembersListLinkExists = _ldapSchemaMapping.doesLinkExist(USER_GROUP_MEMBERS_LIST_LINK_ATTRIBUTE);
      _userGroupMembersListLinkIsDn = _ldapSchemaMapping.isDnAttribute(USER_GROUP_MEMBERS_LIST_LINK_ATTRIBUTE);
      _groupGroupMembersListLinkExists = _ldapSchemaMapping.doesLinkExist(GROUP_GROUP_MEMBERS_LIST_LINK_ATTRIBUTE);
      _groupGroupMembersListLinkIsDn = _ldapSchemaMapping.isDnAttribute(GROUP_GROUP_MEMBERS_LIST_LINK_ATTRIBUTE);
   }

   @Override
   public Set<String> getRegisteredUpnSuffixes()
   {
       // open ldap does not support userPrincipalName and such cannot have upn suffixes
       return null;
   }

   @Override
   public PrincipalId authenticate(
         PrincipalId principal, String password)
               throws LoginException
   {
      ValidateUtil.validateNotNull( principal, "principal" );

      IIdmAuthStatRecorder idmAuthStatRecorder = this.createIdmAuthStatRecorderInstance(
              DiagnosticsContextFactory.getCurrentDiagnosticsContext().getTenantName(),
              ActivityKind.AUTHENTICATE, EventLevel.INFO, principal);
      idmAuthStatRecorder.start();

      principal = this.normalizeAliasInPrincipal(principal);
      ILdapConnectionEx connection = null;

      try
      {
	  connection = super.getConnection( getUserDN(principal), password, AuthenticationType.PASSWORD, false);
      }
      catch(Exception ex)
      {
         log.error("Failed authentication.", ex);
         throw ((LoginException)new LoginException("Login failed").initCause(ex));
      }
      finally
      {
         if (connection != null)
         {
            connection.close();
         }
      }

      idmAuthStatRecorder.end();

      return principal;
   }

   @Override
   public
   Collection<AttributeValuePair> getAttributes(
         PrincipalId           principalId,
         Collection<Attribute> attributes)
               throws Exception
    {
        ValidateUtil.validateNotNull(principalId,  "principalId");

        IIdmAuthStatRecorder idmAuthStatRecorder = this.createIdmAuthStatRecorderInstance(
                DiagnosticsContextFactory.getCurrentDiagnosticsContext().getTenantName(),
                ActivityKind.GETATTRIBUTES, EventLevel.INFO, principalId);
        idmAuthStatRecorder.start();

        List<AttributeValuePair> result = new ArrayList<AttributeValuePair>();

        assert (attributes != null);

        List<String> attrNames = new ArrayList<String>();
        List<Attribute> regularAttrs = new ArrayList<Attribute>();
        Map<String, Attribute> specialAttrs = new HashMap<String, Attribute>();
        String userName = null;
        String ATTR_NAME_CN = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
        String ATTR_ENTRY_UUID = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);

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
                if (_specialAttributes.contains(mappedAttr.toLowerCase()))
                {
                    specialAttrs.put(mappedAttr, attr);
                } else
                {
                    regularAttrs.add(attr);
                    attrNames.add(mappedAttr);
                }
            }
        }

        // we need to retrieve ATTR_NAME_CN
        // to make sure we use user name exactly as it is stored
        // in identity provider when constructing UPN
        attrNames.add(ATTR_NAME_CN);
        attrNames.add(ATTR_ENTRY_UUID);
        if ( (this._userGroupMembersListLinkIsDn == false) && (this._userGroupMembersListLinkExists) )
        {
            attrNames.add(USER_GROUP_MEMBERS_LIST_LINK_ATTRIBUTE);
        }

        ILdapMessage message = null;
        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            try
            {
                String baseDN = this.getStoreDataEx().getUserBaseDn();

                final String filter = this.buildUserQueryByPrincipalId(principalId);

                message = connection.search(
                    baseDN, LdapScope.SCOPE_SUBTREE, filter,
                    attrNames.toArray(new String[attrNames.size()]), false);

                ILdapEntry[] entries = message.getEntries();

                if (entries != null)
                {
                    if (entries.length > 1)
                    {
                        String msg = String.format("multiple entries are found: %s", principalId.getUPN());
                        log.error(msg);
                        throw new InvalidPrincipalException(msg, principalId.getUPN());
                    }

                    int iAttr = 0;
                    ILdapEntry theEntry = entries[0];

                    userName = getUserAccountName(entries[0], ATTR_NAME_CN);

                    String userObjectId = getOptionalFirstStringValue(entries[0].getAttributeValues(ATTR_ENTRY_UUID));

                    AttributeValuePair pairGroupSids = new AttributeValuePair();
                    pairGroupSids.setAttrDefinition(new Attribute(IdentityManager.INTERNAL_ATTR_GROUP_OBJECTIDS));
                    if ( ServerUtils.isNullOrEmpty(userObjectId) == false )
                    {
                        pairGroupSids.getValues().add(userObjectId);
                    }

                    for (Attribute attr : regularAttrs)
                    {
                        AttributeValuePair pair = new AttributeValuePair();

                        pair.setAttrDefinition(attr);

                        String attrName = attrNames.get(iAttr);

                        LdapValue[] values = null;

                        values = theEntry.getAttributeValues(attrName);

                        if (values != null && values.length > 0)
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

                    Iterator<String> iter = specialAttrs.keySet().iterator();
                    while (iter.hasNext())
                    {
                        String key = iter.next();

                        if (key.equalsIgnoreCase(SPECIAL_ATTR_SUBJECT_TYPE))
                        {
                            AttributeValuePair avPair = new AttributeValuePair();

                            avPair.setAttrDefinition(specialAttrs.get(key));
                            // no solution users in OpenLdap.
                            String subjectTypeValue = "false";
                            avPair.getValues().add(subjectTypeValue);
                            result.add(avPair);
                        }
                        else if( key.equalsIgnoreCase(SPECIAL_ATTR_USER_PRINCIPAL_NAME) )
                        {
                            // open ldap does not have upn
                            String upn = String.format("%s@%s",
                                    userName,
                                    this.getDomain());
                            AttributeValuePair avPair = new AttributeValuePair();

                            avPair.setAttrDefinition(specialAttrs.get(key));
                            avPair.getValues().add(upn);
                            result.add(avPair);
                        }
                        else if (key.equalsIgnoreCase(SPECIAL_ATTR_MEMBER_OF))
                        {
                            //The following implementation does not use objectSID
                            String userMembershipId = null;
                            if (this._userGroupMembersListLinkIsDn)
                            {
                                userMembershipId = theEntry.getDN();
                            }
                            else if (this._userGroupMembersListLinkExists )
                            {
                                userMembershipId = getOptionalFirstStringValue(
                                    theEntry.getAttributeValues(USER_GROUP_MEMBERS_LIST_LINK_ATTRIBUTE));
                            }
                            Set<Group> groups = Collections.emptySet();
                            if ( ServerUtils.isNullOrEmpty(userMembershipId) == false )
                            {
                                groups =
                                   getNestedGroups(connection, userMembershipId, true);
                            }
                            AttributeValuePair avPair = new AttributeValuePair();

                            avPair.setAttrDefinition(specialAttrs.get(key));

                            for (Group group : groups)
                            {
                                avPair.getValues().add(group.getNetbios());
                                pairGroupSids.getValues().add(group.getObjectId());
                            }
                            result.add(avPair);
                        }
                    }// while
                    result.add(pairGroupSids);
                }
                else
                {
                    String msg = String.format(
                            "object not found -- baseDN: [%s], scope: [%s], filter: [%s]",
                            baseDN, LdapScope.SCOPE_SUBTREE, filter);
                    log.error(msg);
                    throw new InvalidPrincipalException(msg, principalId.getUPN());
                }
            } catch (NoSuchObjectLdapException e)
            {
                String msg = String.format( "errorCode; %d; %s", e.getErrorCode(), e.getMessage());
                log.error(msg, e);
                throw new InvalidPrincipalException(msg,  principalId.getUPN());
            } finally
            {
                if (message != null)
                {
                    message.close();
                }
            }
        }

        idmAuthStatRecorder.end();

        return result;
    }

   @Override
   public PersonUser findUser(
         PrincipalId id)
               throws Exception
   {
       if (!this.belongsToThisIdentityProvider(id.getDomain()))
       {
          String msg = String.format(
               "Domain of PrincipleId %s matches neither of the domain: [%s],"
                     +" alias: [%s] nor the registered upnSuffixes of the data store",
                     id, getStoreData().getName(), getStoreDataEx().getAlias());
          log.error(msg);
          throw new InvalidPrincipalException(msg, id.getUPN());
       }

      PersonUser user = null;

      try (PooledLdapConnection pooledConnection = borrowConnection())
      {
	  ILdapConnectionEx connection = pooledConnection.getConnection();
          final String ATTR_NAME_CN = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
          final String ATTR_FIRST_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
          final String ATTR_LAST_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
          final String ATTR_EMAIL_ADDRESS = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail);
          final String ATTR_DESCRIPTION = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription);
          final String ATTR_ACCOUNT_FLAGS = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);
          final String ATTR_ENTRY_UUID = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
          final String ATTR_PWD_ACCOUNT_LOCKED_TIME = getMappedAttrPwdAccountLockedTime();

          String[] attrNames = {
               ATTR_NAME_CN,
               ATTR_FIRST_NAME,
               ATTR_LAST_NAME,
               ATTR_EMAIL_ADDRESS,
               ATTR_DESCRIPTION,
               ATTR_ACCOUNT_FLAGS,
               ATTR_PWD_ACCOUNT_LOCKED_TIME,
               ATTR_ENTRY_UUID };

          final String filter = this.buildUserQueryByPrincipalId(id);

         // Search from Users by default
         String searchBaseDn = this.getStoreDataEx().getUserBaseDn();

         ILdapMessage message = null;
         try
         {
            message = connection.search(
               searchBaseDn,
               LdapScope.SCOPE_SUBTREE,
               filter,
               attrNames,
               false);

            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length != 1)
            {
               // Use doesn't exist or multiple user same name
               String msg = String.format("user %s doesn't exist or multiple users with same name",
                       id.getName());
               log.error(msg);
               throw new InvalidPrincipalException(msg, id.getUPN());
            }

            int currentFlag = getAccountFlags(entries[0]);

            user = this.buildPersonUser(entries[0], currentFlag, true);
         }
         catch (NoSuchObjectLdapException e)
         {
             String msg = String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage());
             log.error(msg, e);
             throw new InvalidPrincipalException(msg, id.getUPN());
         }
         finally
         {
            if (message != null) {
               message.close();
            }
         }
      }

      return user;
   }

   private int getAccountFlags(ILdapEntry entry) {
      final String ATTR_ACCOUNT_FLAGS =
            _ldapSchemaMapping
                  .getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);
      final String ATTR_PWD_ACCOUNT_LOCKED_TIME =
            getMappedAttrPwdAccountLockedTime();
      int currentFlag = 0;
      if (entry.getAttributeValues(ATTR_ACCOUNT_FLAGS) != null)
      {
         currentFlag = getOptionalIntegerValue(
                     entry.getAttributeValues(ATTR_ACCOUNT_FLAGS), 0);
      }
      else if (entry.getAttributeValues(ATTR_PWD_ACCOUNT_LOCKED_TIME) != null)
      {
         String generalizedTime =
               getOptionalStringValue(entry
                     .getAttributeValues(ATTR_PWD_ACCOUNT_LOCKED_TIME));
         if (StringUtils.isNotEmpty(generalizedTime)) {
            // handle special case when account is permanently locked
            // http://tools.ietf.org/html/draft-behera-ldap-password-policy-10#section-5.3
            if (generalizedTime.equals("000001010000Z")) {
		log.info("User account is permanently locked!");
		return AccountControlFlag.FLAG_LOCKED_ACCOUNT.getValue();
	    }
            SimpleDateFormat formatter = new SimpleDateFormat("yyyyMMddhhmmss");
            formatter.setTimeZone(TimeZone.getTimeZone("GMT"));
            boolean accountLocked = false;
            try {
               Date lockedTs = formatter.parse(generalizedTime);
               long lockoutDur = getLockoutDurationSettings();
               if (lockoutDur == INVALID_VALUE_PWD_LOCKOUT_DURATION)
               {//invalid setting -- cannot load due to multiple entries or no entries
                  return currentFlag;
               }
               else
               {
                  accountLocked =
                     !(lockedTs.getTime() < (System.currentTimeMillis() - TimeUnit.SECONDS.toMillis(lockoutDur)));
               }
            } catch (ParseException e) {
               throw new IllegalArgumentException(e.getMessage());
            }
            if (accountLocked)
            {
               currentFlag = AccountControlFlag.FLAG_LOCKED_ACCOUNT.getValue();
            }
         }
      }
      return currentFlag;
   }

   private int getLockoutDurationSettings()
   {
      if (this._pwdPolicyPwdLockoutDuration == UNINITIALIZED_VALUE_PWD_LOCKOUT_DURATION)
      {
         synchronized(this)
         {
            if (this._pwdPolicyPwdLockoutDuration == UNINITIALIZED_VALUE_PWD_LOCKOUT_DURATION)
            {
               this._pwdPolicyPwdLockoutDuration = loadPwdPolicyPwdLockoutDuration();
            }
         }
      }
      return this._pwdPolicyPwdLockoutDuration;
   }

   private String getMappedAttrPwdAccountLockedTime()
   {
      return _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime);
   }

   private int loadPwdPolicyPwdLockoutDuration()
   {
      int  value;

      try (PooledLdapConnection pooledConnection = borrowConnection())
      {
	  ILdapConnectionEx connection = pooledConnection.getConnection();

         final String ATTR_NAME_CN = _ldapSchemaMapping.getUserAttribute(
                IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
         final String ATTR_PWD_POLICY_ACCOUNT_LOCKOUT_DURATION =_ldapSchemaMapping
            .getPwdObjectAttribute(IdentityStoreAttributeMapping.AttributeIds.PwdPolicyAttributePwdLockoutDuration);

          String[] attrNames = {
               ATTR_NAME_CN,
               ATTR_PWD_POLICY_ACCOUNT_LOCKOUT_DURATION};

         String filter = _ldapSchemaMapping.getPasswordSettingsQuery();

         // Search from Users by default
         String searchBaseDn = this.getStoreDataEx().getUserBaseDn();

         ILdapMessage message = null;
         try
         {
            message = connection.search(
               searchBaseDn,
               LdapScope.SCOPE_SUBTREE,
               filter,
               attrNames,
               false);

            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length != 1)
            { // Password Policy not configured or there are duplicated ones
               return INVALID_VALUE_PWD_LOCKOUT_DURATION;
            }

            value = getOptionalIntegerValue(
                  entries[0].getAttributeValues(ATTR_PWD_POLICY_ACCOUNT_LOCKOUT_DURATION),
                  INVALID_VALUE_PWD_LOCKOUT_DURATION);
         }
         finally
         {
            if (message != null) {
               message.close();
            }
         }
      }
      catch (Exception e)
      {// return invalid value when exception during loading
         value = INVALID_VALUE_PWD_LOCKOUT_DURATION;
      }

      return value;
   }

   @Override
   public PersonUser findUserByObjectId(String userEntryUuid) throws Exception
   {
         PersonUser user = null;

         try (PooledLdapConnection pooledConnection = borrowConnection())
         {
             ILdapConnectionEx connection = pooledConnection.getConnection();

             final String ATTR_NAME_CN = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
             final String ATTR_FIRST_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
             final String ATTR_LAST_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
             final String ATTR_EMAIL_ADDRESS = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail);
             final String ATTR_DESCRIPTION = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription);
             final String ATTR_ACCOUNT_FLAGS = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);
             final String ATTR_ENTRY_UUID = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
             final String ATTR_PWD_ACCOUNT_LOCKED_TIME = getMappedAttrPwdAccountLockedTime();

             String[] attrNames = {
                  ATTR_NAME_CN,
                  ATTR_FIRST_NAME,
                  ATTR_LAST_NAME,
                  ATTR_EMAIL_ADDRESS,
                  ATTR_DESCRIPTION,
                  ATTR_ACCOUNT_FLAGS,
                  ATTR_PWD_ACCOUNT_LOCKED_TIME,
                  ATTR_ENTRY_UUID };

            String filter =
                String.format(
                        _ldapSchemaMapping.getUserQueryByObjectUniqueId(),
                        LdapFilterString.encode(userEntryUuid));

            // Search from Users by default
            String searchBaseDn = this.getStoreDataEx().getUserBaseDn();

            ILdapMessage message = null;
            try
            {
               message = connection.search(
                  searchBaseDn,
                  LdapScope.SCOPE_SUBTREE,
                  filter,
                  attrNames,
                  false);

               ILdapEntry[] entries = message.getEntries();

               if (entries == null || entries.length != 1)
               {
                   // Use doesn't exist or multiple user same name
                   String msg = String.format(
                           "user with entruUUID %s doesn't exist or multiple users with same name",
                           userEntryUuid);
                   log.error(msg);
                   throw new InvalidPrincipalException(msg, userEntryUuid);
               }

               int currentFlag = getAccountFlags(entries[0]);

               user = this.buildPersonUser(entries[0], currentFlag, true);
            }
            catch (NoSuchObjectLdapException e)
            {
                String msg = String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage());
                log.error(msg, e);
                throw new InvalidPrincipalException(msg, userEntryUuid);
            }
            finally
            {
               if (message != null) {
                  message.close();
               }
            }
         }

         return user;
   }

    @Override
    public Set<PersonUser> findUsers(
          String searchString, String domainName, int limit)
                throws Exception
    {
        String filter = createSearchFilter(_ldapSchemaMapping.getAllUsersQuery(), _ldapSchemaMapping.getUserQueryByCriteria(), searchString);
        return findUsersInternal(filter, domainName, limit);
    }

    @Override
    public Set<PersonUser> findUsersByName(
          String searchString, String domainName, int limit)
                throws Exception
    {
        String filter = createSearchFilter(_ldapSchemaMapping.getAllUsersQuery(), _ldapSchemaMapping.getUserQueryByCriteriaForName(), searchString);
        return findUsersInternal(filter, domainName, limit);
    }

    private Set<PersonUser> findUsersInternal(
          String filter, String domainName, int limit)
                throws Exception
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

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();

            final String ATTR_NAME_CN = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
            final String ATTR_DESCRIPTION = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription);
            final String ATTR_FIRST_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
            final String ATTR_LAST_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
            final String ATTR_EMAIL_ADDRESS = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail);
            final String ATTR_ACCOUNT_FLAGS = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);
            final String ATTR_ENTRY_UUID = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
            final String ATTR_PWD_ACCOUNT_LOCKED_TIME = getMappedAttrPwdAccountLockedTime();

            String[] attrNames = {
                    ATTR_NAME_CN,
                    ATTR_DESCRIPTION,
                    ATTR_FIRST_NAME,
                    ATTR_LAST_NAME,
                    ATTR_EMAIL_ADDRESS,
                    ATTR_ACCOUNT_FLAGS,
                    ATTR_PWD_ACCOUNT_LOCKED_TIME,
                    ATTR_ENTRY_UUID
            };

            String searchBaseDn = getStoreDataEx().getUserBaseDn();

            try
            {
                Collection<ILdapMessage> messages = ldap_search(
                        connection,
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

                                    int flag = getAccountFlags(entry);

                                    users.add(buildPersonUser(entry, flag));
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
            catch (NoSuchObjectLdapException e)
            {
                String msg =String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage());
                log.error(msg, e);
                throw new InvalidPrincipalException(msg, filter);
            }
        }

        return users;
    }

    @Override
    public Set<PersonUser> findUsersInGroup(
          PrincipalId groupId,
          String searchString,
          int limit) throws Exception
    {
        String filter = createSearchFilter(_ldapSchemaMapping.getAllUsersQuery(), _ldapSchemaMapping.getUserQueryByCriteria(), searchString);
        return findUsersInGroupInternal(groupId, filter, limit);
    }

    @Override
    public Set<PersonUser> findUsersByNameInGroup(
          PrincipalId groupId,
          String searchString,
          int limit) throws Exception
    {
        String filter = createSearchFilter(_ldapSchemaMapping.getAllUsersQuery(), _ldapSchemaMapping.getUserQueryByCriteriaForName(), searchString);
        return findUsersInGroupInternal(groupId, filter, limit);
    }

    private Set<PersonUser> findUsersInGroupInternal(
           PrincipalId groupId,
           String userFilter,
           int limit) throws Exception
    {
        ValidateUtil.validateNotNull(groupId, "groupId");;

        Set<PersonUser> users = new HashSet<PersonUser>();

        if (limit == 0) {
            // Short circuit since they're asking for a list of nothing anyway
            return users;
        }

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            final String ATTR_MEMBER = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList);

            String[] attrNames = {    ATTR_MEMBER };

            String groupFilter =  String.format(
                    _ldapSchemaMapping.getGroupQueryByAccountName(),
                    LdapFilterString.encode(groupId.getName()));

            ILdapMessage message = null;
            try
            {
                message = connection.search(
                        getStoreDataEx().getGroupBaseDn(),
                        LdapScope.SCOPE_SUBTREE,
                        groupFilter,
                        attrNames,
                        false);

                ILdapEntry[] entries = message.getEntries();

                if (entries == null || entries.length == 0)
                {
                    String msg = String.format("group %s doesn't exist", groupId.getName());
                    log.error(msg);
                    throw new InvalidPrincipalException(msg, groupId.getUPN());
                }
                else if (entries.length > 1)
                {
                    String msg = String.format("multiple groups same name %s", groupId.getUPN());
                    log.error(msg);
                    throw new InvalidPrincipalException(msg, groupId.getUPN());
                }

                Collection<String> dNs = ServerUtils.getMultiStringValueAsCollection(
                        entries[0].getAttributeValues(ATTR_MEMBER));

                if (dNs != null)
                {
                    int numUsersToRet = 0;
                    for (String dN : dNs)
                    {
                        //non-user dn will be filtered out when the user oc is applied in
                        // the following search. Therefore, no users in nested group is returned
                        PersonUser user = findUserByDNByFilter(
                              connection,
                              dN,
                              userFilter);
                        if ( user != null && (limit <= 0 || numUsersToRet < limit))
                        {
                            users.add(user);
                            numUsersToRet++;
                        }
                    }
                }
            }
            catch (NoSuchObjectLdapException e)
            {
                String msg = String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage());
                log.error(msg, e);
                throw new InvalidPrincipalException(msg, groupFilter);
            }

            finally
            {
                if (message != null) {
                    message.close();
                }
            }
        }

        return users;
    }

   @Override
   public Set<PersonUser> findDisabledUsers(
         String searchString, int limit)
               throws Exception
   {
      return getUsersWithAccountControlFlag(searchString, AccountControlFlag.FLAG_DISABLED_ACCOUNT, limit);
   }

   @Override
   public Set<PersonUser> findLockedUsers(
         String searchString, int limit)
               throws Exception
   {
      return getUsersWithAccountControlFlag(searchString, AccountControlFlag.FLAG_LOCKED_ACCOUNT, limit);
   }

   @Override
   public
   PrincipalGroupLookupInfo findDirectParentGroups(
         PrincipalId principalId)
               throws Exception
   {
      Set<Group> groups = new HashSet<Group>();
      PrincipalInfo principalInfo = null;

      try (PooledLdapConnection pooledConnection = borrowConnection())
      {
	  ILdapConnectionEx connection = pooledConnection.getConnection();

          final String ATTR_NAME_CN = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
          final String ATTR_DESCRIPTION = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
          final String ATTR_ENTRY_UUID = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

         String[] attrNames = { ATTR_NAME_CN, ATTR_DESCRIPTION, ATTR_ENTRY_UUID };
         principalInfo = getPrincipalGroupMembershipId(connection, principalId);
         if ((principalInfo != null) && (ServerUtils.isNullOrEmpty(principalInfo.getGroupMembershipId()) == false))
         {
             String filter = String.format(
                     _ldapSchemaMapping.getDirectParentGroupsQuery(),
                     LdapFilterString.encode(principalInfo.getGroupMembershipId()));

             try
             {
                 Collection<ILdapMessage> messages = ldap_search(
                      connection,
                      getStoreDataEx().getGroupBaseDn(),
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
                                     groups.add(buildGroup(entry, null));
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
             catch (NoSuchObjectLdapException e)
             {
                 String msg = String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage());
                 log.error(msg, e);
                 throw new InvalidPrincipalException(msg, principalId.getUPN());
             }
         }
      }

      return new PrincipalGroupLookupInfo(
          groups,
          (principalInfo!= null) ?
              principalInfo.getObjectId() :
              null
      );
   }

   @Override
   public PrincipalGroupLookupInfo findNestedParentGroups(
         PrincipalId userId)
               throws Exception
   {
      Set<Group> groups = new HashSet<Group>();

      PrincipalInfo principalInfo = null;

      try (PooledLdapConnection pooledConnection = borrowConnection())
      {
	  ILdapConnectionEx connection = pooledConnection.getConnection();
          principalInfo = getPrincipalGroupMembershipId(connection, userId);

          if ((principalInfo != null) && ( ServerUtils.isNullOrEmpty(principalInfo.getGroupMembershipId()) == false) )
          {
             // NestedParentGroups includes the direct parents, as well as the grandparents.
             groups = getNestedGroups(connection, principalInfo.getGroupMembershipId(), false);
          }
      }

      return new PrincipalGroupLookupInfo(
          groups,
          (principalInfo!= null) ?
              principalInfo.getObjectId() :
              null
      );
   }

   @Override
   public Group findGroup(
         PrincipalId groupId)
               throws Exception
   {
       final String ATTR_NAME_GROUP_CN = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
       final String ATTR_DESCRIPTION = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
       final String ATTR_ENTRY_UUID = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

       String[] attrNames = { ATTR_NAME_GROUP_CN, ATTR_DESCRIPTION, ATTR_ENTRY_UUID };
       String filter = String.format(
           _ldapSchemaMapping.getGroupQueryByAccountName(),
           LdapFilterString.encode(groupId.getName()));

      try (PooledLdapConnection pooledConnection = borrowConnection())
      {
	 ILdapConnectionEx connection = pooledConnection.getConnection();
         ILdapMessage message = null;

         try
         {
            message = connection.search(
                  getStoreDataEx().getGroupBaseDn(),
                  LdapScope.SCOPE_SUBTREE,
                  filter,
                  attrNames,
                  false);
            ILdapEntry[] entries = message.getEntries();
            if (entries == null || entries.length == 0)
            {
               return null; // if no group found
            }
            else if (entries.length != 1)
            {
                String msg = "duplicated groups with name "+ groupId.getUPN();
                log.error(msg);
                throw new InvalidPrincipalException(msg, groupId.getUPN());
            }

            return buildGroup(entries[0], null /* no default group dn */);
         }
         catch (NoSuchObjectLdapException e)
         {
             String msg = String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage());
             log.error(msg, e);
             throw new InvalidPrincipalException(msg, groupId.getUPN());
         }
         finally
         {
            if (message != null) {
               message.close();
            }
         }
      }
   }

   @Override
   public Group findGroupByObjectId(
       String groupEntryUuid
       ) throws Exception
   {
       final String ATTR_NAME_GROUP_CN = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
       final String ATTR_DESCRIPTION = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
       final String ATTR_ENTRY_UUID = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

       String[] attrNames = { ATTR_NAME_GROUP_CN, ATTR_DESCRIPTION, ATTR_ENTRY_UUID };
       String filter = String.format(
               _ldapSchemaMapping.getGroupQueryByObjectUniqueId(),
               LdapFilterString.encode(groupEntryUuid));

       try (PooledLdapConnection pooledConnection = borrowConnection())
       {
	  ILdapConnectionEx connection = pooledConnection.getConnection();
          ILdapMessage message = null;

          try
          {
             message = connection.search(
                   getStoreDataEx().getGroupBaseDn(),
                   LdapScope.SCOPE_SUBTREE,
                   filter,
                   attrNames,
                   false);
             ILdapEntry[] entries = message.getEntries();
             if (entries == null || entries.length != 1)
             {
                // Use doesn't exist or multiple user same name
                 String msg = String.format(
                         "Group with entryUuid %s doesn't exist or multiple groups with same name",
                         groupEntryUuid);
                 log.error(msg);
                 throw new InvalidPrincipalException(msg, groupEntryUuid);
             }

             return buildGroup(entries[0], null /* no default group dn */);
          }
          catch (NoSuchObjectLdapException e)
          {
              String msg = String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage());
              log.error(msg, e);
              throw new InvalidPrincipalException(msg, groupEntryUuid);
          }
          finally
          {
             if (message != null) {
                message.close();
             }
          }
       }
   }

    @Override
    public Set<Group> findGroups(
        String searchString, String domainName, int limit)
            throws Exception
    {
        String filter = createSearchFilter(_ldapSchemaMapping.getAllGroupsQuery(),
                _ldapSchemaMapping.getGroupQueryByCriteria(), searchString);
        return findGroupsInternal(filter, domainName, limit);
    }

    @Override
    public Set<Group> findGroupsByName(
        String searchString, String domainName, int limit)
            throws Exception
    {
        String filter = createSearchFilter(_ldapSchemaMapping.getAllGroupsQuery(), _ldapSchemaMapping.getGroupQueryByCriteriaForName(), searchString);
        return findGroupsInternal(filter, domainName, limit);
    }

    @Override
    public Set<Group> findGroupsInGroup(
          PrincipalId groupId,
          String searchString,
          int limit) throws Exception
    {
        String filter = createSearchFilter(_ldapSchemaMapping.getAllGroupsQuery(), _ldapSchemaMapping.getGroupQueryByCriteria(), searchString);
        return findGroupsInGroupInternal(groupId, filter, limit);
    }

    @Override
    public Set<Group> findGroupsByNameInGroup(
          PrincipalId groupId,
          String searchString,
          int limit) throws Exception
    {
        String filter = createSearchFilter(_ldapSchemaMapping.getAllGroupsQuery(), _ldapSchemaMapping.getGroupQueryByCriteriaForName(), searchString);
        return findGroupsInGroupInternal(groupId, filter, limit);
    }

    private Set<Group> findGroupsInternal(
        String filter, String domainName, int limit)
            throws Exception
    {
        Set<Group> groups = new HashSet<Group>();

        final String ATTR_NAME_CN = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
        final String ATTR_DESCRIPTION = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
        final String ATTR_ENTRY_UUID = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

        String[] attrNames = { ATTR_NAME_CN, ATTR_DESCRIPTION, ATTR_ENTRY_UUID };

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();

            try
            {
                Collection<ILdapMessage> messages = ldap_search(
                        connection,
                        this.getStoreDataEx().getGroupBaseDn(),
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
                                    groups.add(buildGroup(entry, null));
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
            catch (NoSuchObjectLdapException e)
            {
                String msg = String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage());
                log.error(msg, e);
                throw new InvalidPrincipalException(msg, filter);
            }
        }

        return groups;
    }

    private Set<Group> findGroupsInGroupInternal(
        PrincipalId groupId, String filter, int limit)
              throws Exception
    {
        ValidateUtil.validateNotNull(groupId, "groupId");

        Set<Group> groups = new HashSet<Group>();

        if (limit == 0) {
            // Short circuit since they're asking for a list of nothing anyway
            return groups;
        }

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            final String ATTR_MEMBER = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList);

            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames = { ATTR_MEMBER };

            String groupFilter = String.format(
                    _ldapSchemaMapping.getGroupQueryByAccountName(),
                    LdapFilterString.encode(groupId.getName()));

            ILdapMessage message = null;
            try {
                message = connection.search(
                        getStoreDataEx().getGroupBaseDn(),
                        LdapScope.SCOPE_SUBTREE,
                        groupFilter,
                        attrNames,
                        false);

                ILdapEntry[] entries = message.getEntries();

                if (entries == null || entries.length == 0)
                {
                    String msg = String.format("invalid principal: %s", groupId.getName());
                    log.error(msg);
                    throw new InvalidPrincipalException(msg, groupId.getUPN());
                }
                else if ( entries.length > 1 )
                {
                    String msg = String.format("found duplicated group entries with id: %s",groupId.getUPN());
                    log.error(msg);
                    throw new InvalidPrincipalException(msg, groupId.getUPN());
                }
                else
                {
                    String[] values = ServerUtils.getMultiStringValue(
                          entries[0].getAttributeValues(ATTR_MEMBER));
                    if (values != null)
                    {
                        int numGroupsToRet = 0;
                        for (String memberDN : values)
                        {
                            Group group = findGroupByDNWithFilter(connection, memberDN, filter);

                            if (null != group && (limit <=0 || numGroupsToRet < limit))
                            {
                                groups.add(group);
                                numGroupsToRet++;
                            }
                        }
                    }

                }
            }
            catch (NoSuchObjectLdapException e)
            {
                String msg = String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage());
                log.error(msg, e);
                throw new InvalidPrincipalException(msg, groupId.getUPN());
            }
            finally {
                if ( message != null) {
                    message.close();
                }
            }
        }

        return groups;
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
      return new SearchResult( users, null, groups );
   }

   @Override
   public SearchResult findByName(String searchString, String domainName, int limit) throws Exception
   {
      return new SearchResult(
            findUsersByName(searchString, domainName, limit < 0 ? -1 : limit/2),
            null, /* service principals */
            findGroupsByName(searchString, domainName, limit < 0 ? -1 : limit/2 + limit%2));
   }

   @Override
   public boolean IsActive(PrincipalId id) throws Exception
   {
      return !AccountControlFlag.FLAG_DISABLED_ACCOUNT.isSet(retrieveUserAccountFlags(id));
   }

   @Override
   public void checkUserAccountFlags(PrincipalId principalId)
         throws IDMException
   {
      int accountFlags = retrieveUserAccountFlags(principalId);
      if (AccountControlFlag.FLAG_LOCKED_ACCOUNT.isSet(accountFlags))
      {
         throw new UserAccountLockedException(
               String.format("User account locked: %s", principalId));
      }
      else if (AccountControlFlag.FLAG_PASSWORD_EXPIRED.isSet(accountFlags))
      {
         throw new PasswordExpiredException(
               String.format("User account expired: %s", principalId));
      }
   }

   private int retrieveUserAccountFlags(PrincipalId id) throws IDMException
   {
      int accountFlags = 0;
      PooledLdapConnection pooledConnection = null;

      try
      {
         pooledConnection = borrowConnection();
      }
      catch (Exception e)
      {
         throw new IDMException("Cannot established a connection to server", e);
      }

      try
      {
         final String filter = this.buildUserQueryByPrincipalId(id);

         final String ATTR_ACCOUNT_FLAGS = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);
         final String ATTR_PWD_ACCOUNT_LOCKED_TIME = getMappedAttrPwdAccountLockedTime();


         String attributes[] = { ATTR_ACCOUNT_FLAGS, ATTR_PWD_ACCOUNT_LOCKED_TIME };

         ILdapMessage message = null;
         try
         {
            message = pooledConnection.getConnection().search(
                  this.getStoreDataEx().getUserBaseDn(),
                  LdapScope.SCOPE_SUBTREE,
                  filter,
                  attributes,
                  false);

            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length == 0)
            {
                String msg = String.format("No such principal %s was found in %s",
                        id.getName(), id.getDomain());
                log.error(msg);
                throw new InvalidPrincipalException(msg, id.getUPN());
            }

            if (entries.length > 1)
            {
                String msg = String.format("Duplicate entries were found", id.getUPN());
                log.error(msg);
                throw new InvalidPrincipalException(msg, id.getUPN());
            }

            accountFlags = getAccountFlags(entries[0]);
         }
         catch (NoSuchObjectLdapException e)
         {
             String msg = String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage());
             log.error(msg, e);
             throw new InvalidPrincipalException(msg, id.getUPN());
         }
         finally
         {
            if (message != null) {
               message.close();
            }
         }
      }
      finally
      {
	  if (pooledConnection != null)
	      pooledConnection.close();
      }
      return accountFlags;
   }

   String getUserDN(PrincipalId id) throws Exception
   {
      try (PooledLdapConnection pooledConnection = borrowConnection())
      {
	  ILdapConnectionEx connection = pooledConnection.getConnection();

         String[] attrNames = {  null };

         final String filter = this.buildUserQueryByPrincipalId(id);

         // Search from Users by default
         String searchBaseDn = this.getStoreDataEx().getUserBaseDn();

         ILdapMessage message = null;
         try
         {
            message = connection.search(
                  searchBaseDn,
                  LdapScope.SCOPE_SUBTREE,
                  filter,
                  attrNames,
                  true);

            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length == 0)
            {
                String msg = String.format( "User '%s' was not found.", id.getName());
                log.error(msg);
                throw new NoSuchUserException(msg);
            }
            else if (entries.length != 1)
            {
                String msg = String.format("Duplicate entries were found: %s", id.getUPN());
                log.error(msg);
                throw new InvalidPrincipalException(msg, id.getUPN());
            }

            return entries[0].getDN();
         }
         catch (NoSuchObjectLdapException e)
         {
             String msg = String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage());
             log.error(msg, e);
             throw new InvalidPrincipalException( msg, id.getUPN());
         }
         finally
         {
            if (message != null) {
               message.close();
            }
         }
      }
   }

   Group buildGroup(
         ILdapEntry entry, String groupDN) throws InvalidPrincipalException
   {
      final String ATTR_NAME_GROUP_CN = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
      final String ATTR_DESCRIPTION = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
      final String ATTR_ENTRY_UUID = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

      String groupName = null;
      try
      {
          groupName = getFirstStringValue(entry.getAttributeValues(ATTR_NAME_GROUP_CN));
      }
      catch(InvalidParameterException ex)
      {
          String msg = String.format("empty value for [%s] found and is not allowed: %s", ATTR_NAME_GROUP_CN, groupDN);
          log.error(msg);
          throw new InvalidPrincipalException(msg, groupDN);
      }

      if (groupDN == null)
      {
         groupDN = entry.getDN();
      }

      String groupEntryUuid = null;
      String description = null;
      try {
          groupEntryUuid = getOptionalFirstStringValue(
            entry.getAttributeValues(ATTR_ENTRY_UUID));

          description = getOptionalLastStringValue(
            entry.getAttributeValues(ATTR_DESCRIPTION));
      } catch (IllegalStateException e) {
          String msg = String.format("multiple values for uuid and descritption found and is not allowed: %s", groupDN);
          log.error(msg, e);
          throw new InvalidPrincipalException(msg, groupDN);
      }

      PrincipalId gid = ServerUtils.getPrincipalId(null, groupName, this.getDomain());

      PrincipalId alias = ServerUtils.getPrincipalAliasId(groupName,
                                                            this.getStoreDataEx().getAlias());

      return new Group(gid, alias, groupEntryUuid, new GroupDetail(description));
   }

   Set<Group> getNestedGroups(
         ILdapConnectionEx connection,
         String    membershipId,
         boolean   groupNameOnly)
               throws NoSuchGroupException, InvalidPrincipalException
   {
      Set<Group> groups = new HashSet<Group>();
      if(ServerUtils.isNullOrEmpty(membershipId) == false)
      {
          final String ATTR_NAME_GROUP_CN = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
          final String ATTR_DESCRIPTION = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
          final String ATTR_ENTRY_UUID = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);
          ArrayList<String> attributeNames = getAttributesList(ATTR_NAME_GROUP_CN, ATTR_ENTRY_UUID, ATTR_DESCRIPTION, !groupNameOnly );

          HashSet<String> groupsProcessed = new HashSet<String>();
          Stack<String> groupsToProcess = new Stack<String>();
          groupsToProcess.push( membershipId );

          while( groupsToProcess.isEmpty() == false )
          {
              String currentMembershipId = groupsToProcess.pop();
              if ( groupsProcessed.contains(currentMembershipId) == false )
              {
                  String filter =
                          String.format(
                                  _ldapSchemaMapping.getDirectParentGroupsQuery(),
                                  LdapFilterString.encode(currentMembershipId));

                  Collection<ILdapMessage> messages = null;
                  try
                  {
                      messages = ldap_search(
                                 connection,
                                 getStoreDataEx().getGroupBaseDn(),
                                 LdapScope.SCOPE_SUBTREE,
                                 filter,
                                 attributeNames,
                                 DEFAULT_PAGE_SIZE,
                                 -1);

                      String groupMembershipId = null;

                      if (messages != null && messages.size() > 0)
                      {
                          for (ILdapMessage message : messages)
                          {
                              ILdapEntry[] entries = message.getEntries();
                              if((entries != null) && ( entries.length > 0 ) )
                              {
                                  for(ILdapEntry entry : entries )
                                  {
                                      Group g = buildGroupObject(entry, ATTR_NAME_GROUP_CN, ATTR_ENTRY_UUID, ATTR_DESCRIPTION, !groupNameOnly);

                                      if (this._groupGroupMembersListLinkIsDn)
                                      {
                                          groupMembershipId = entry.getDN();
                                      }
                                      else if (this._groupGroupMembersListLinkExists)
                                      {
                                          groupMembershipId = getOptionalFirstStringValue(
                                              entry.getAttributeValues(GROUP_GROUP_MEMBERS_LIST_LINK_ATTRIBUTE) );
                                      }

                                      groups.add( g );

                                      if(ServerUtils.isNullOrEmpty(groupMembershipId) == false)
                                      {
                                          groupsToProcess.push( groupMembershipId );
                                      }
                                  }
                              }
                          }
                      }
                  }
                  catch (NoSuchObjectLdapException e) {
                     log.error(
                         String.format("Failed to search for grup membership for [%s]", currentMembershipId),
                         e
                     );
                     throw e;
                  }
                  finally
                  {
                      ServerUtils.disposeLdapMessages(messages);
                  } // try

                  groupsProcessed.add(currentMembershipId);
              }
          }
      }
      return groups;
   }

   private ArrayList<String> getAttributesList( String ATTR_NAME_GROUP_CN, String ATTR_ENTRY_UUID, String ATTR_DESCRIPTION, boolean getDescription )
   {
       ArrayList<String> attributeNames = new ArrayList<String>(4);
       attributeNames.add(ATTR_NAME_GROUP_CN);
       attributeNames.add(ATTR_ENTRY_UUID);
       if ( getDescription == true )
       {
           attributeNames.add(ATTR_DESCRIPTION);
       }
       if ( (this._groupGroupMembersListLinkIsDn == false) && (this._groupGroupMembersListLinkExists == true) )
       {
           attributeNames.add(GROUP_GROUP_MEMBERS_LIST_LINK_ATTRIBUTE);
       }

       return attributeNames;
   }

   private Group buildGroupObject(ILdapEntry entry, String ATTR_NAME_GROUP_CN, String ATTR_ENTRY_UUID, String ATTR_DESCRIPTION, boolean getDescription)
   {
       String groupName = null;
       String groupDescription = null;
       String groupEntryUuid = null;
       groupName = getFirstStringValue(
           entry.getAttributeValues(ATTR_NAME_GROUP_CN)
       );

       if( getDescription == true )
       {
           groupDescription = getOptionalLastStringValue(
               entry.getAttributeValues(ATTR_DESCRIPTION));
       }

       groupEntryUuid = getOptionalFirstStringValue(
           entry.getAttributeValues(ATTR_ENTRY_UUID));

       PrincipalId groupId = new PrincipalId( groupName, this.getStoreData().getName() );
       PrincipalId groupAlias = null;
       GroupDetail groupDetail = null;

       if(getDescription == true)
       {
           groupAlias = ServerUtils.getPrincipalAliasId(groupName, this.getStoreDataEx().getAlias());

           groupDetail = new GroupDetail(
               (groupDescription == null) ? "" : groupDescription
           );
       }
       Group g = new Group( groupId, groupAlias, groupEntryUuid, groupDetail );
       return g;
   }

   PersonUser findUserByDNByFilter(
         ILdapConnectionEx connection,
         String          userDN,
         String          filter
         ) throws InvalidPrincipalException
   {
       final String ATTR_NAME_CN = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName );
       final String ATTR_DESCRIPTION = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription );
       final String ATTR_FIRST_NAME = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName );
       final String ATTR_LAST_NAME = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName );
       final String ATTR_EMAIL_ADDRESS = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail );
       final String ATTR_ACCOUNT_FLAGS = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl );
       final String ATTR_ENTRY_UUID = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
       final String ATTR_PWD_ACCOUNT_LOCKED_TIME = getMappedAttrPwdAccountLockedTime();

      PersonUser user = null;
      String[] attrNames = {
            ATTR_NAME_CN,
            ATTR_DESCRIPTION,
            ATTR_FIRST_NAME,
            ATTR_LAST_NAME,
            ATTR_EMAIL_ADDRESS,
            ATTR_ACCOUNT_FLAGS,
            ATTR_PWD_ACCOUNT_LOCKED_TIME,
            ATTR_ENTRY_UUID
      };

      ILdapMessage message = connection.search(
            userDN,
            LdapScope.SCOPE_BASE,
            filter,
            attrNames,
            false);

      try
      {
         ILdapEntry[] entries = message.getEntries();

         if (entries != null && entries.length == 1)
         {
            int flag = getAccountFlags(entries[0]);

            user = buildPersonUser(
                  entries[0],
                  flag);
         }
      }
      finally
      {
          if (message != null)
          {
              message.close();
          }
      }
      return user;
   }

   PersonUser buildPersonUser(
           ILdapEntry entry,
           int        accountFlags
           ) throws InvalidPrincipalException
   {
       return this.buildPersonUser(entry, accountFlags, false);
   }

    PersonUser buildPersonUser(ILdapEntry entry, int accountFlags,
            boolean provideExtendedAccountInfo)
            throws InvalidPrincipalException
    {
        final String ATTR_NAME_CN =
                _ldapSchemaMapping
                        .getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
        final String ATTR_DESCRIPTION =
                _ldapSchemaMapping
                        .getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription);
        final String ATTR_FIRST_NAME =
                _ldapSchemaMapping
                        .getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
        final String ATTR_LAST_NAME =
                _ldapSchemaMapping
                        .getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
        final String ATTR_EMAIL_ADDRESS =
                _ldapSchemaMapping
                        .getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail);
        final String ATTR_ENTRY_UUID =
                _ldapSchemaMapping
                        .getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);

        String accountName = null;
        String description = null;
        String firstName = null;
        String lastName = null;
        String email = null;
        try {
            accountName = getUserAccountName(entry, ATTR_NAME_CN);

            description = getOptionalLastStringValue(entry
                            .getAttributeValues(ATTR_DESCRIPTION));

            firstName = getOptionalLastStringValue(entry
                            .getAttributeValues(ATTR_FIRST_NAME));

            lastName = getOptionalLastStringValue(entry
                            .getAttributeValues(ATTR_LAST_NAME));

            email = getOptionalLastStringValue(entry
                            .getAttributeValues(ATTR_EMAIL_ADDRESS));

        } catch (IllegalStateException e) {
            String cn = getStringValues(entry.getAttributeValues(ATTR_NAME_CN))
                            .iterator().next();
            String principalId = String.format("%s@%s", cn, getDomain());
            String message = String.format(
                            "multiple values for attributes for %s are not allowed: %s ",
                            ATTR_NAME_CN, principalId);
            log.error(message);
            throw new InvalidPrincipalException(message, principalId);
        }

        String resultEntryUUID = null;

        PrincipalId id = ServerUtils.getPrincipalId(null, accountName, this.getDomain());

        PrincipalId alias = ServerUtils.getPrincipalAliasId(accountName, this
                        .getStoreDataEx().getAlias());

        if (provideExtendedAccountInfo) {
            resultEntryUUID =
                getOptionalFirstStringValue(entry.getAttributeValues(ATTR_ENTRY_UUID));
        }
        PersonDetail detail =
                new PersonDetail.Builder().firstName(firstName)
                        .lastName(lastName).userPrincipalName(null)
                        .emailAddress(email).description(description).build();

        boolean disabled =
                AccountControlFlag.FLAG_DISABLED_ACCOUNT.isSet(accountFlags);

        boolean locked =
                AccountControlFlag.FLAG_LOCKED_ACCOUNT.isSet(accountFlags);

        return new PersonUser(id, alias, resultEntryUUID, detail, disabled,
                locked);
    }

    private class PrincipalInfo
    {
        private final String _groupMembershipId;
        private final String _objectId;
        public PrincipalInfo(String groupMembershipId, String objectId)
        {
            this._groupMembershipId = groupMembershipId;
            this._objectId = objectId;
        }
        public String getGroupMembershipId()
        {
            return this._groupMembershipId;
        }

        public String getObjectId()
        {
            return this._objectId;
        }
    }

    PrincipalInfo getPrincipalGroupMembershipId(
         ILdapConnectionEx connection,
         PrincipalId principalId
         ) throws Exception
   {
      final String ATTR_USER_OBJECTID = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);

      String principalMembershipId = null;
      String principalObjectId = null;
      String[] attrNames = ( (this._userGroupMembersListLinkIsDn) || (this._userGroupMembersListLinkExists == false) ) ?
          new String[] { ATTR_USER_OBJECTID, null } :
          new String[] { USER_GROUP_MEMBERS_LIST_LINK_ATTRIBUTE, ATTR_USER_OBJECTID, null };
      final String userFilter = this.buildUserQueryByPrincipalId(principalId);

      try(ILdapMessage userMessage =
         connection.search(
             this.getStoreDataEx().getUserBaseDn(),
             LdapScope.SCOPE_SUBTREE,
             userFilter,
             attrNames,
             false))
      {
         ILdapEntry[] entries = userMessage.getEntries();

         if ( ( entries != null ) && (entries.length == 1) )
         {
             if ( this._userGroupMembersListLinkIsDn )
             {
                 principalMembershipId = entries[0].getDN();
             }
             else if (this._userGroupMembersListLinkExists)
             {
                 principalMembershipId = getOptionalFirstStringValue(
                     entries[0].getAttributeValues(USER_GROUP_MEMBERS_LIST_LINK_ATTRIBUTE));
             }
             principalObjectId = getOptionalFirstStringValue(entries[0].getAttributeValues(ATTR_USER_OBJECTID));
         }
         else if ( (entries != null) && (entries.length > 1) )
         {
             String msg = String.format("Multiple values found for %s", principalId.getUPN());
             log.error(msg);
             throw new InvalidPrincipalException(msg, principalId.getUPN());
         }
      }
      catch (NoSuchObjectLdapException e)
      {
          String msg = String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage());
          log.error(msg, e);
          throw new InvalidPrincipalException(msg, principalId.getUPN());
      }

      // try groups
      if ( ServerUtils.isNullOrEmpty(principalMembershipId) )
      {
          final String ATTR_GROUP_OBJECTID = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

          attrNames = ( (this._groupGroupMembersListLinkIsDn) || (this._groupGroupMembersListLinkExists == false) ) ?
              new String[] { ATTR_GROUP_OBJECTID, null } :
              new String[] { GROUP_GROUP_MEMBERS_LIST_LINK_ATTRIBUTE, ATTR_GROUP_OBJECTID, null };
          final String groupFilter =
              String.format(
                 this._ldapSchemaMapping.getGroupQueryByAccountName(),
                 LdapFilterString.encode(principalId.getName()));

          try(ILdapMessage groupMessage =
              connection.search(
                 this.getStoreDataEx().getGroupBaseDn(),
                 LdapScope.SCOPE_SUBTREE,
                 groupFilter,
                 attrNames,
                 false))
           {
              ILdapEntry[] entries = groupMessage.getEntries();

              if ( (entries == null) || (entries.length == 0) )
              {
                  String msg = String.format("PrincipalId not found: %s", principalId);
                  log.error(msg);
                  throw new InvalidPrincipalException(msg, principalId.getUPN());
              }
              else if ( entries.length > 1 )
              {
                  String msg = String.format("Multiple values found for %s", principalId.getUPN());
                  log.error(msg);
                  throw new InvalidPrincipalException(msg, principalId.getUPN());
              }
              else
              {
                  if ( this._groupGroupMembersListLinkIsDn )
                  {
                      principalMembershipId = entries[0].getDN();
                  }
                  else if (this._groupGroupMembersListLinkExists)
                  {
                      principalMembershipId = getOptionalFirstStringValue(
                          entries[0].getAttributeValues(GROUP_GROUP_MEMBERS_LIST_LINK_ATTRIBUTE));
                  }
                  principalObjectId = getOptionalFirstStringValue(entries[0].getAttributeValues(ATTR_GROUP_OBJECTID));
              }
           }
           catch (NoSuchObjectLdapException e)
           {
               String msg = String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage());
               log.error(msg, e);
               throw new InvalidPrincipalException(msg, principalId.getUPN());
           }
      }

      return new PrincipalInfo(principalMembershipId, principalObjectId);
   }

   Set<PersonUser> getUsersWithAccountControlFlag(
         String searchString, AccountControlFlag controlBit, int limit)
         throws Exception
   {
      Set<PersonUser> users = new HashSet<PersonUser>();

      try (PooledLdapConnection pooledConnection = borrowConnection())
      {
	  ILdapConnectionEx connection = pooledConnection.getConnection();

          final String ATTR_NAME_CN = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName );
          final String ATTR_DESCRIPTION = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription );
          final String ATTR_FIRST_NAME = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName );
          final String ATTR_LAST_NAME = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName );
          final String ATTR_EMAIL_ADDRESS = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail );
          final String ATTR_ACCOUNT_FLAGS = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl );
          final String ATTR_ENTRY_UUID = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
          final String ATTR_PWD_ACCOUNT_LOCKED_TIME = getMappedAttrPwdAccountLockedTime();

         String[] attrNames = {
               ATTR_NAME_CN,
               ATTR_DESCRIPTION,
               ATTR_FIRST_NAME,
               ATTR_LAST_NAME,
               ATTR_EMAIL_ADDRESS,
               ATTR_ACCOUNT_FLAGS,
               ATTR_PWD_ACCOUNT_LOCKED_TIME,
               ATTR_ENTRY_UUID
         };

         String searchBaseDn = getStoreDataEx().getUserBaseDn();

         //find all users -- perf hit? we might need index on userAccessControl.
         try
         {
             String filter = searchString.isEmpty()
                     ? _ldapSchemaMapping.getAllUsersQuery()
                     :
                     String.format(
                         _ldapSchemaMapping.getUserQueryByCriteria(),
                         LdapFilterString.encode(searchString));

             Collection<ILdapMessage> messages = ldap_search(
                  connection,
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
                                 int flags = getAccountFlags(entry);

                                 boolean isBitSet = controlBit.isSet(flags);

                                 if (!isBitSet)
                                 {
                                     continue;
                                 }
                                 users.add(buildPersonUser(entry, flags));
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
         catch (NoSuchObjectLdapException e)
         {
             String msg = String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage());
             log.error(msg, e);
             throw new InvalidPrincipalException(msg, searchBaseDn);
         }
      }

      return users;
   }


    Group findGroupByDNWithFilter(
          ILdapConnectionEx connection,
          String          entryDn,
          String          filter)
    throws InvalidPrincipalException
    {
        final String ATTR_NAME_CN = _ldapSchemaMapping.getGroupAttribute( IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName );
        final String ATTR_DESCRIPTION = _ldapSchemaMapping.getGroupAttribute( IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription );
        final String ATTR_ENTRY_UUID = _ldapSchemaMapping.getGroupAttribute( IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId );

        String[] attrNames = { ATTR_NAME_CN, ATTR_DESCRIPTION, ATTR_ENTRY_UUID };

        ILdapMessage message = null;
        try
        {
            message = connection.search(
                 entryDn,
                 LdapScope.SCOPE_BASE,
                 filter,
                 attrNames,
                 false);

            ILdapEntry[] entries = message.getEntries();
            if (entries == null || entries.length == 0)
            {
                return null;    // return null when the entry is not a group object
            }

            return buildGroup(entries[0], entryDn);
        }
        catch (NoSuchObjectLdapException e)
        {
            String msg = String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage());
            log.error(msg, e);
            throw new InvalidPrincipalException(msg, entryDn);
        }
        finally
        {
            if (message != null) {
                message.close();
            }
        }
    }

   private String buildUserQueryByPrincipalId(PrincipalId principal) throws InvalidPrincipalException
   {
       ValidateUtil.validateNotNull(principal, "principal");

       if ( this.isSameDomainUpn(principal) )
       {
           String escapedsAMAccountName = LdapFilterString.encode(principal.getName());
           return String.format(
               this._ldapSchemaMapping.getUserQueryByAccountName(),
               escapedsAMAccountName );
       }
       else
       {
           throw new InvalidPrincipalException("Unrecognized domain name in user Upn.", principal.getUPN() );
       }
   }

   private String getUserAccountName(ILdapEntry entry, String attributeName) throws InvalidPrincipalException
   {
       try
       {
           return getFirstStringValue(entry.getAttributeValues(attributeName));
       }
       catch(InvalidParameterException ex)
       {
           String msg =
               String.format(
                   "Required user identity attribute [%s] is missing for user dn=[%s]",
                   attributeName,
                   entry.getDN()
               );
           log.error(msg, ex);
           throw new InvalidPrincipalException(msg, entry.getDN());
       }
   }

    @Override
    public Collection<SecurityDomain> getDomains() {
        Collection<SecurityDomain> domains = new HashSet<SecurityDomain>();
        domains.add(new SecurityDomain(super.getDomain(), super.getAlias()));
        return domains;
    }

    private Collection<ILdapMessage> ldap_search(
        ILdapConnectionEx connection,
        String searchBaseDn,
        LdapScope scope,
        String filter,
        Collection<String> attributes,
        int pageSize,
        int limit)
    {
        Collection<ILdapMessage> result = null;
        int pagingSupported = _pagedResultSupportedFlag.get();
        if ( pagingSupported == LdapProvider.PAGED_RESULT_SUPPORTED_YES )
        {
            result = connection.paged_search(searchBaseDn, scope, filter, attributes, pageSize, limit);
        }
        else if( pagingSupported == LdapProvider.PAGED_RESULT_SUPPORTED_NO )
        {
            result = ldap_search_no_paging(connection, searchBaseDn, scope, filter, attributes, limit);
        }
        else if (pagingSupported == LdapProvider.PAGED_RESULT_SUPPORTED_UNKNOWN)
        {
            try
            {
                result = connection.paged_search(searchBaseDn, scope, filter, attributes, pageSize, limit);
                // if limit is set to 0, connection.paged_search returns without running any query,
                // so we don't really know if paging is supported or not.
                if( limit != 0 )
                {
                    _pagedResultSupportedFlag.compareAndSet(LdapProvider.PAGED_RESULT_SUPPORTED_UNKNOWN, LdapProvider.PAGED_RESULT_SUPPORTED_YES);
                }
            }
            catch(UnavailableCritExtensionLdapException ex)
            {
                log.warn(
                    String.format(
                        "Paged search is unavailable for identity source={name=[%s], url=[%s]} falling back to regular search.",
                        this.getName(), ((this.getStoreDataEx() != null) ? this.getStoreDataEx().getConnectionStrings() : "")
                    )
                );
                _pagedResultSupportedFlag.compareAndSet(LdapProvider.PAGED_RESULT_SUPPORTED_UNKNOWN, LdapProvider.PAGED_RESULT_SUPPORTED_NO);
                result = ldap_search_no_paging(connection, searchBaseDn, scope, filter, attributes, limit);
            }
        }

        return result;
    }

    private Collection<ILdapMessage> ldap_search_no_paging(
            ILdapConnectionEx connection,
            String searchBaseDn,
            LdapScope scope,
            String filter,
            Collection<String> attributes,
            int limit)
    {
        Collection<ILdapMessage> result = null;

        // refer to LdapConnection.paged_search which
        // short circuits for limit = 0;
        if ( limit != 0 )
        {
            // refer to LdapConnection.paged_search (for unlimited results)
            ILdapMessage m = connection.search_ext(
                searchBaseDn, scope, filter, attributes, false, null, null, null,
                ((limit != -1) ? limit :(NON_PAGED_SEARCH_MAX_RESULT_RETURN + 1))
            );
            if ( ( limit == -1 ) && ( m instanceof ILdapMessageEx ))
            {
                int numberEntries = ((ILdapMessageEx)m).getEntriesCount();
                if ( numberEntries > (NON_PAGED_SEARCH_MAX_RESULT_RETURN) )
                {
                    log.error(
                        String.format(
                            "too many search results for query: searchBaseDn=[%s], scope=[%s], filter=[%s]; number of entries=[%s]",
                            searchBaseDn, scope, filter, numberEntries
                        )
                    );
                    throw new SizeLimitExceededLdapException(LDAP_SIZELIMIT_EXCEEDED,
                        String.format("search has more than %d results to return",
                        NON_PAGED_SEARCH_MAX_RESULT_RETURN));
                }
            }
            result = new ArrayList<ILdapMessage>(1);
            result.add(m);
        }
        return result;
    }

    @Override
    public UserSet findActiveUsersInDomain(String ldapAttrName, String attributeValue, String userDomain, String additionalAttribute)
            throws Exception {
        Validate.notEmpty(ldapAttrName, "ldapAttrName");
        Validate.notEmpty(attributeValue, "attributeValue");
        Validate.notEmpty(userDomain, "userDomain");

        PooledLdapConnection pooledConnection;
        AccountLdapEntriesInfo entries = null;
        UserSet result = new UserSet();

        try {
            pooledConnection = this.borrowConnection();
        } catch (Exception ex) {
            throw new IDMException("Failed to establish server connection", ex);
        }

        try {
            final String ATTR_NAME_SAM_ACCOUNT = _ldapSchemaMapping
                    .getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
            final String ATTR_NAME_USER_ACCT_CTRL = _ldapSchemaMapping
                    .getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);

            ArrayList<String> attrNames = new ArrayList<String>();
            attrNames.add(ATTR_NAME_SAM_ACCOUNT);
            attrNames.add(ATTR_NAME_USER_ACCT_CTRL);

            if (additionalAttribute != null) {
                attrNames.add(additionalAttribute);
            }

            String searchBaseDn = this.getStoreDataEx().getUserBaseDn();
            String idsAttrNameForSearch;
            String attrValueForSearch;

            if (ldapAttrName.equalsIgnoreCase(_ldapSchemaMapping
                    .getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName))) {
                // if upn, use sam account to search
                idsAttrNameForSearch = ATTR_NAME_SAM_ACCOUNT;
                attrValueForSearch = attributeValue.split("@")[0];
            } else {
                idsAttrNameForSearch = ldapAttrName;
                attrValueForSearch = attributeValue;
            }
            ValidateUtil.validateNotNull(idsAttrNameForSearch, "idsAttrNameForSearch");

            final String filter_by_attr;

            if (ldapAttrName.equals(SPECIAL_ATTR_USER_PRINCIPAL_NAME)) {

                PrincipalId pid = ServerUtils.getPrincipalId(attributeValue);
                filter_by_attr = this.buildUserQueryByPrincipalId(pid);
            } else {

                filter_by_attr = this.buildUserQueryByAttribute(idsAttrNameForSearch, attrValueForSearch);
            }

            entries = findAccountLdapEntries(pooledConnection.getConnection(), filter_by_attr
                    , searchBaseDn, attrNames.toArray(new String[attrNames.size()]), false,
                    attributeValue, true, null);

            String accountName = null;

            for (ILdapEntry entry : entries.accountLdapEntries) {

                accountName = getStringValue(entry.getAttributeValues(ATTR_NAME_SAM_ACCOUNT));
                int currentFlag = getOptionalIntegerValue(entry.getAttributeValues(ATTR_NAME_USER_ACCT_CTRL), 0);

                Collection<String> additionalAttrValues = null;

                if (additionalAttribute != null) {
                    additionalAttrValues = getOptionalStringValues(entry.getAttributeValues(additionalAttribute));
                }

                if (!AccountControlFlag.FLAG_DISABLED_ACCOUNT.isSet(currentFlag) && !AccountControlFlag.FLAG_LOCKED_ACCOUNT.isSet(currentFlag)) {
                    result.put(new PrincipalId(accountName, userDomain), additionalAttrValues);
                }
            }
            if (result.isEmpty()) {
                throw new InvalidPrincipalException(String.format("User account '%s@%s' is not active. ", accountName, userDomain), String.format(
                        "%s@%s", accountName, userDomain));

            }

        } finally {
            if (entries != null) {
                entries.close();
            }
            if (pooledConnection != null) {
                pooledConnection.close();
            }
        }

        return result;
    }

    @Override
    public PrincipalId findActiveUser(String ldapAttrName, String attributeValue) throws Exception {
        Validate.notEmpty(ldapAttrName, "ldapAttrName");
        Validate.notEmpty(attributeValue, "attributeValue");

        PooledLdapConnection pooledConnection;
        AccountLdapEntriesInfo entriesInfo = null;

        try
        {
            pooledConnection = this.borrowConnection();
        }
        catch (Exception ex)
        {
            throw new IDMException("Failed to establish server connection", ex);
        }

        try
        {
            final String ATTR_NAME_SAM_ACCOUNT = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName );
            final String ATTR_NAME_USER_ACCT_CTRL = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl );

             String[] attrNames =
                    { ATTR_NAME_SAM_ACCOUNT, ATTR_NAME_USER_ACCT_CTRL};

             String searchBaseDn = this.getStoreDataEx().getUserBaseDn();
             String idsAttrNameForSearch;
             String attrValueForSearch;

             if (ldapAttrName.equalsIgnoreCase(_ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName))) {
                 //if upn, use sam account to search
                 idsAttrNameForSearch = ATTR_NAME_SAM_ACCOUNT;
                 attrValueForSearch = attributeValue.split("@")[0];
             } else {
                 idsAttrNameForSearch = ldapAttrName;
                 attrValueForSearch = attributeValue;
             }
             ValidateUtil.validateNotNull(idsAttrNameForSearch, "idsAttrNameForSearch");

             final String filter_by_attr = this.buildUserQueryByAttribute(idsAttrNameForSearch, attrValueForSearch);

            entriesInfo = findAccountLdapEntry(pooledConnection.getConnection(),
                     filter_by_attr,
                     searchBaseDn,
                     attrNames,
                     false,attributeValue,null);

            ILdapEntry ldapEntry = entriesInfo.accountLdapEntries.iterator().next();
             String accountName =
                    getStringValue(ldapEntry.getAttributeValues(ATTR_NAME_SAM_ACCOUNT));

             int currentFlag =
                 getOptionalIntegerValue(
                            ldapEntry.getAttributeValues(ATTR_NAME_USER_ACCT_CTRL), 0);

             if ( !AccountControlFlag.FLAG_DISABLED_ACCOUNT.isSet(currentFlag))
             {
                 return new PrincipalId(accountName, this.getDomain());
             }
             else
             {
                 throw new InvalidPrincipalException(String.format(
                         "User account '%s@%s' is not active. ",
                         accountName, this.getDomain()), String.format(
                         "%s@%s", accountName, getDomain()));
             }

        }
        finally
        {
            if (entriesInfo != null) {
                entriesInfo.close();
            }
            if (pooledConnection != null)
            {
                 pooledConnection.close();
            }
        }
    }

    private String buildUserQueryByAttribute(String vmdirAttrName, String attributeValue) {
        ValidateUtil.validateNotNull(vmdirAttrName, "vmdirAttrName");
        ValidateUtil.validateNotNull(attributeValue, "attributeValue");

        String escapedsAttrName = LdapFilterString.encode(vmdirAttrName);
        String escapedsAttrValue = LdapFilterString.encode(attributeValue);
        return String.format(
            this._ldapSchemaMapping.getUserQueryByAttribute(),
            escapedsAttrName, escapedsAttrValue );
    }

    private PooledLdapConnection borrowConnection() throws Exception {
	return borrowConnection(getStoreDataEx().getConnectionStrings(), false);
    }

    @Override
    public String getStoreUPNAttributeName() {
        return _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);
    }

}
