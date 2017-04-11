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
package com.vmware.identity.idm.server.provider.ldap;

import java.net.URI;
import java.net.URISyntaxException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.Stack;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

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
import com.vmware.identity.idm.KnownSamlAttributes;
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
import com.vmware.identity.idm.server.provider.PooledLdapConnection;
import com.vmware.identity.idm.server.provider.PrincipalGroupLookupInfo;
import com.vmware.identity.idm.server.provider.UserSet;
import com.vmware.identity.idm.server.provider.activedirectory.ADSchemaMapping;
import com.vmware.identity.idm.server.provider.activedirectory.SecurityIdentifier;
import com.vmware.identity.interop.domainmanager.DomainAdapterFactory;
import com.vmware.identity.interop.domainmanager.DomainControllerInfo;
import com.vmware.identity.interop.domainmanager.DomainManagerException;
import com.vmware.identity.interop.domainmanager.DomainManagerNativeException;
import com.vmware.identity.interop.domainmanager.IDomainAdapter;
import com.vmware.identity.interop.ldap.DirectoryStoreProtocol;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.ILdapEntry;
import com.vmware.identity.interop.ldap.ILdapMessage;
import com.vmware.identity.interop.ldap.ILdapPagedSearchResult;
import com.vmware.identity.interop.ldap.LdapConstants;
import com.vmware.identity.interop.ldap.LdapFilterString;
import com.vmware.identity.interop.ldap.LdapScope;
import com.vmware.identity.interop.ldap.LdapValue;
import com.vmware.identity.interop.ldap.NoSuchObjectLdapException;
import com.vmware.identity.performanceSupport.IIdmAuthStat.ActivityKind;
import com.vmware.identity.performanceSupport.IIdmAuthStat.EventLevel;
import com.vmware.identity.performanceSupport.LdapQueryStat;

public class LdapWithAdMappingsProvider extends BaseLdapProvider

{

   /**
    *http://support.microsoft.com/kb/305144
    */
   private static enum AccountControlFlag
   {
      // 0x00000002 ADS_UF_ACCOUNTDISABLE
      //     The user account is disabled.
      FLAG_DISABLED_ACCOUNT   (0x00000002),
      // 0x00000010 ADS_UF_LOCKOUT
      //     The account is currently locked out.
      FLAG_LOCKED_ACCOUNT     (0x00000010),
      //ADS_UF_DONT_EXPIRE_PASSWD
      //     When set, the password will not expire on this account.
      FLAG_DONT_EXPIRE_PASSWD (0x00010000),
      // 0x00800000 ADS_UF_PASSWORD_EXPIRED
      //     The user password has expired. This flag is created by the system using data from the
      //     Pwd-Last-Set attribute and the domain policy.
      FLAG_PASSWORD_EXPIRED   (0x00800000);

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

   private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
           .getLogger(LdapWithAdMappingsProvider.class);

   private static final int DEFAULT_PAGE_SIZE = 1000;
   private static final int DEFAULT_RANGE_SIZE = 1000;

   private final ILdapSchemaMapping _ldapSchemaMapping;

   // Other constants
   private static final long INTVLS_100NS_IN_SEC = 10*1000*1000;
   private static final long INTVLS_100NS_1600TO1970 = 11644473600L*INTVLS_100NS_IN_SEC;


   private static Map<IdpKey, UpnSuffixCache> _alternativeUpnSuffixCache = new ConcurrentHashMap<IdpKey, UpnSuffixCache>();

   private static final long upnSuffixRefreshThreshold = 60000*60*6; // 6 hours
   // Writes and reads of "volatile" long and double values are always atomic

   private PwdLifeTimeValue _defaultDomainPolicyPwdLifeTimeCache = null;

   private static final ConnectionInfoCache _connInfoCache = new ConnectionInfoCache();

   // keep in sync with sso-config tool
   // use matching_rule_in_chain for nested groups searches
   public static final int FLAG_AD_MATCHING_RULE_IN_CHAIN = 0x1;
   // utilize group search base dn for nested groups resolution in token
   public static final int FLAG_DO_NOT_USE_BASE_DN_FOR_NESTED_GROUPS = 0x2;
   // resolve only direct parent groups
   public static final int FLAG_DIRECT_GROUPS_ONLY = 0x4;

   // Enable site affinity
   public static final int FLAG_ENABLE_SITE_AFFINITY = 0x8;

   //mapped user attribute names
   private final String ATTR_NAME_USER_ACCT_CTRL;
   private final String ATTR_NAME_SAM_ACCOUNT;
   private final String ATTR_USER_PRINCIPAL_NAME;

   private final String tenantName;

   private class IdpKey {

      private final String tenant;
      private final String idpName;

      public IdpKey(String tenant, String idpName) {
         this.tenant = tenant;
         this.idpName = idpName;
      }

      public String getTenant() {
         return tenant;
      }

      public String getIdpName() {
         return idpName;
      }

      @Override
      public int hashCode() {
         return Objects.hash(tenant, idpName);
      }

      @Override
      public boolean equals(Object o) {
         if (!(o instanceof IdpKey)) {
            return false;
         }

         IdpKey otherKey = (IdpKey) o;

         return tenant.equals(otherKey.getTenant()) && idpName.equals(otherKey.getIdpName());
      }
   }

   private class UpnSuffixCache {
      private final Set<String> suffixes;
      private final long retrievedAt;

      public UpnSuffixCache(Set<String> suffixes, long retrievedAt) {
         this.suffixes = suffixes;
         this.retrievedAt = retrievedAt;
      }

      public Set<String> getSuffixes() {
         return suffixes;
      }

      public long retrievedAt() {
         return retrievedAt;
      }
   }

   public LdapWithAdMappingsProvider(String tenantName, IIdentityStoreData store)
   {
       this(tenantName, store, null);
   }

   public LdapWithAdMappingsProvider(String tenantName, IIdentityStoreData store, Collection<X509Certificate> tenantTrustedCertificates)
   {
      super(tenantName, store, tenantTrustedCertificates);
      Validate.isTrue(
            this.getStoreDataEx().getProviderType() == IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING,
            "IIdentityStoreData must represent a store of 'IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING' type.");

      Validate.notEmpty(tenantName, "Tenant name must not be empty or null");
      this.tenantName = tenantName;

      if ( ( ( this.getStoreDataEx().getFlags() )
             &
             (~(FLAG_AD_MATCHING_RULE_IN_CHAIN | FLAG_DO_NOT_USE_BASE_DN_FOR_NESTED_GROUPS
                     | FLAG_DIRECT_GROUPS_ONLY | FLAG_ENABLE_SITE_AFFINITY))
           ) != 0 )
      {
          logger.warn(String.format("Unrecognized flags: [%d].", this.getStoreDataEx().getFlags()));
      }
      else
      {
          logger.debug(String.format("The IdentitySource flag is set to %d", this.getStoreDataEx().getFlags()));
      }

      _ldapSchemaMapping = new ADSchemaMapping( this.getStoreDataEx().getIdentityStoreSchemaMapping() );
      ATTR_NAME_SAM_ACCOUNT = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
      ATTR_USER_PRINCIPAL_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);
      ATTR_NAME_USER_ACCT_CTRL = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);

   }

   @Override
   public void probeConnectionSettings() throws Exception
   {
       if (!enableSiteAffinity())
           super.probeConnectionSettings();

       if (enableSiteAffinity()) {
	   try (ILdapConnectionEx connection = getConnection()) {
               checkDn(connection);
           }
       }
   }

   /* The logic of retrieving upnSuffixes for AD domain follows
    * http://support.microsoft.com/kb/269441
    */
   private Set<String> getAlterUpnSuffixes() throws Exception
   {
       Set<String> alterUpnSuffixes = new HashSet<String>();

       ILdapConnectionEx connection = null;
       ILdapMessage message = null;
       String ATTR_CONFIG_NAMING_CONTEXT = "configurationNamingContext";
       String ATTR_ROOT_DOMAIN_NAMING_CONTEXT = "rootDomainNamingContext";
       String ATTR_UPN_SUFFIXES = "uPNSuffixes";
       String configNamingContext = null;
       String[] attrNames = new String[2];

       try
       {
           // Search in RootDSE should not in GC (upnSuffix attribute is not represent in GC)
           Collection<String> connStrs = this.getStoreDataEx().getConnectionStrings();
           ArrayList<String> newConnStrs = new ArrayList<String>();
           for (String conn : connStrs)
           {
               URI uri = null;
               try
               {
                   uri = new URI(conn);
                   if (uri.getPort() == LdapConstants.LDAP_GC_PORT || uri.getPort() == LdapConstants.LDAP_SSL_GC_PORT)
                   {
                       int index = conn.lastIndexOf(':');
                       newConnStrs.add(conn.substring(0, index));
                   }
                   else
                   {
                       newConnStrs.add(conn);
                   }
               }
               catch (URISyntaxException e)
               {
                   logger.warn(String.format("bad uri: [%s]", conn));
               }
           }

           connection = getConnection(newConnStrs, false);

           try
           {   // Retrieve RootDSE object
               attrNames[0] = ATTR_CONFIG_NAMING_CONTEXT;
               attrNames[1] = ATTR_ROOT_DOMAIN_NAMING_CONTEXT;

               message = connection.search(
                   "", LdapScope.SCOPE_BASE, "(objectClass=*)", attrNames, false);

               ILdapEntry[] entries = message.getEntries();
               if (entries != null)
               {
                   if (entries.length > 1)
                   {
                       String msg = String.format("Multiple RootDSE objects are found for domain: %s", this.getDomain());
                       throw new IllegalStateException(msg);
                   }

                   configNamingContext = getStringValue(
                           entries[0].getAttributeValues(
                                   ATTR_CONFIG_NAMING_CONTEXT));

               }
           }
           catch(Exception e)
           {
               // Do not bail in case we cannot retrieve information from RootDSE
               // It just mean log in names using upnSuffix will not work
               logger.warn(String.format("Failed to retrieve information from RootDSE in AD Over Ldap provider %s", this.getDomain()), e);
           }
           finally
           {
               if (message != null)
               {
                   message.close();
               }
           }

           if (!ServerUtils.isNullOrEmpty(configNamingContext))
           {
               try
               {
                   // Retrieve CN=partitions, configNamingContext
                   attrNames[0] = ATTR_UPN_SUFFIXES;
                   message = connection.search(
                                            "CN=Partitions," + configNamingContext,
                                            LdapScope.SCOPE_BASE,
                                            "(objectClass=*)",
                                            attrNames, false);

                   ILdapEntry[] entries = message.getEntries();
                   if (entries != null)
                   {
                       if (entries.length > 1)
                       {
                           String msg = String.format("Multiple partition configuration objects are found for domain: %s", this.getDomain());
                           throw new IllegalStateException(msg);
                       }

                       Collection<String> suffixes = this.getOptionalStringValues(entries[0].getAttributeValues(
                                                                                  ATTR_UPN_SUFFIXES));
                       if (suffixes != null && suffixes.size() > 0)
                       {
                           for (String suffix : suffixes)
                           {
                               alterUpnSuffixes.add(ValidateUtil.getCanonicalUpnSuffix(suffix));
                           }
                       }
                   }
               }
               catch(Exception e)
               {
                   // Do not bail in case we cannot retrieve attribute upnSuffix
                   // It just mean log in names using upnSuffix will not work
                   logger.error(String.format("Failed to retrieve attribute upnSuffix in AD Over Ldap provider %s", this.getDomain()), e);
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
       finally
       {
           if (connection != null)
           {
               connection.close();
           }
       }

       return alterUpnSuffixes;
   }

   @Override
   public Set<String> getRegisteredUpnSuffixes()
   {
      UpnSuffixCache upnCache = _alternativeUpnSuffixCache.get(getIdpKey());
      long currentTime = System.currentTimeMillis();

      if (upnCache == null || upnCache.getSuffixes().isEmpty() || currentTime - upnCache.retrievedAt() >= upnSuffixRefreshThreshold)
      {
         try
         {
            Set<String> upnSuffixes = getAlterUpnSuffixes();
            upnCache = new UpnSuffixCache(upnSuffixes, currentTime);
            _alternativeUpnSuffixCache.put(getIdpKey(), upnCache);
         }
         catch (Exception e) {
             // Do not bail in case we cannot retrieve upnSuffixes, it just means login names using upnSuffix will not work
            logger.error(String.format("Failed to retrieve upnSuffixes in AD over LDAP provider '%s'", this.getDomain()), e);
         }
      }

      return upnCache == null ? Collections.<String>emptySet() : upnCache.getSuffixes();
   }

   private IdpKey getIdpKey() {
      return new IdpKey(tenantName, getName());
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
         Collection<Attribute> attributes) throws Exception
    {
        List<AttributeValuePair> result = new ArrayList<AttributeValuePair>();

        assert (attributes != null);

        IIdmAuthStatRecorder idmAuthStatRecorder = this.createIdmAuthStatRecorderInstance(
                DiagnosticsContextFactory.getCurrentDiagnosticsContext().getTenantName(),
                ActivityKind.GETATTRIBUTES, EventLevel.INFO, principalId);
        idmAuthStatRecorder.start();

        List<String> attrNames = new ArrayList<String>();
        final String ATTR_NAME_SAM_ACCOUNT = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
        final String ATTR_PRIMARY_GROUP_ID = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrimaryGroupId);
        final String ATTR_OBJECT_SID = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);

        Map<String, String> attrMap = this.getStoreDataEx().getAttributeMap();
        for (Attribute attr : attributes)
        {
            if ( KnownSamlAttributes.ATTRIBUTE_USER_GROUPS.equalsIgnoreCase(attr.getName()) )
            {
                attrNames.add(ATTR_PRIMARY_GROUP_ID);
            }
            else if ( KnownSamlAttributes.ATTRIBUTE_USER_PRINCIPAL_NAME.equalsIgnoreCase(attr.getName()) )
            {
                // we need to retrieve ATTR_NAME_CN
                // to make sure we use user name exactly as it is stored
                // in identity provider when constructing UPN
                attrNames.add(ATTR_NAME_SAM_ACCOUNT);
            }
            else if ( KnownSamlAttributes.ATTRIBUTE_USER_SUBJECT_TYPE.equalsIgnoreCase(attr.getName()) == false)
            {
                if (attrMap == null)
                {
                    throw new IllegalArgumentException(String.format(
                            "Attribute mapping is null.", attr.getName()));
                }

                String mappedAttr = attrMap.get(attr.getName());
                if (mappedAttr == null)
                {

                    /*
                     * Apparently this getAttributes is assuming used in the
                     * context of retrieving token attribute. To make it more
                     * general in order to query attribute that beyond those to
                     * be included in the token. Such altSecurityIdentities used
                     * in cert_based authentication.
                     *
                     * In this case, we include the attribute without mapping.
                     *
                     * Mary to review this.
                     */

                    mappedAttr = attr.getName();

                }
                attrNames.add(mappedAttr);
            }
        }

        // we always need object sid
        attrNames.add(ATTR_OBJECT_SID);

        AccountLdapEntryInfo ldapEntryInfo = null;
        ILdapEntry userLdapEntry = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();

            String baseDN = this.getStoreDataEx().getUserBaseDn();

            // find user using upn first (unique in forest);
            final String filter_by_upn = this.buildUserQueryByUpn(principalId);
            final String filter_by_acct = this.buildUserQueryByAccountName(principalId);

            ldapEntryInfo = findAccountLdapEntry(connection,
                    filter_by_upn,
                    filter_by_acct,
                    baseDN,
                    attrNames.toArray(new String[attrNames.size()]),
                    false,
                    principalId,
                    idmAuthStatRecorder);

            userLdapEntry = ldapEntryInfo.accountLdapEntry;

            byte[] resultObjectSID =
                ServerUtils.getBinaryValue(userLdapEntry.getAttributeValues(ATTR_OBJECT_SID));

            SecurityIdentifier sid = SecurityIdentifier.build(resultObjectSID);
            String userObjectId = sid.toString();

            AttributeValuePair pairGroupSids = new AttributeValuePair();
            pairGroupSids.setAttrDefinition(new Attribute(IdentityManager.INTERNAL_ATTR_GROUP_OBJECTIDS));
            if ( ServerUtils.isNullOrEmpty(userObjectId) == false )
            {
                pairGroupSids.getValues().add(userObjectId);
            }

            for (Attribute attr : attributes)
            {
                AttributeValuePair pair = new AttributeValuePair();
                pair.setAttrDefinition(attr);

                if ( KnownSamlAttributes.ATTRIBUTE_USER_GROUPS.equalsIgnoreCase(attr.getName()) )
                {
                    Set<Group> groups = new HashSet<Group>();

                    int primaryGroupRID =
                            ServerUtils.getIntValue(
                                    userLdapEntry.getAttributeValues(ATTR_PRIMARY_GROUP_ID));

                    byte[] userObjectSID =
                        ServerUtils.getBinaryValue(
                                userLdapEntry.getAttributeValues(ATTR_OBJECT_SID));

                    String primaryGroupDN =
                        this.getPrimaryGroupDNAndGroupObject(
                            connection,
                            userObjectSID, primaryGroupRID, groups,
                            idmAuthStatRecorder
                        );

                    populateNestedGroups(connection, userLdapEntry.getDN(), true, groups, idmAuthStatRecorder);
                    if ( (!this.useDirectGroupsOnly()) && (!ServerUtils.isNullOrEmpty(primaryGroupDN)))
                    {
                        populateNestedGroups(connection, primaryGroupDN, true, groups, idmAuthStatRecorder);
                    }

                    for (Group group : groups)
                    {
                        pair.getValues().add(group.getNetbios());
                        pairGroupSids.getValues().add(group.getObjectId());
                    }
                }
                else if ( KnownSamlAttributes.ATTRIBUTE_USER_SUBJECT_TYPE.equalsIgnoreCase(attr.getName()) )
                {
                    // no solution users in OpenLdap.
                    pair.getValues().add("false");
                }
                else if ( KnownSamlAttributes.ATTRIBUTE_USER_PRINCIPAL_NAME.equalsIgnoreCase(attr.getName()) )
                {
                    String userName = getStringValue(userLdapEntry.getAttributeValues(ATTR_NAME_SAM_ACCOUNT));

                    // Always return iUpn
                    String upn = String.format("%s@%s",
                                userName,
                                ServerUtils.getDomainFromDN(userLdapEntry.getDN()));
                    pair.getValues().add(upn);
                }
                else
                {
                    // attrMap is not null, and has proper mapping which is verified above when we build
                    // ldap attributes list
                    //TODO schai - verify with reviewer. assumption not true anymore.
                    String attrName = attrMap.get(attr.getName());
                    if (attrName == null) {
                        attrName = attr.getName();
                    }
                    LdapValue[] values = userLdapEntry.getAttributeValues(attrName);

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
                }
                result.add(pair);
            }

            result.add(pairGroupSids);
        }
        finally
        {
            if (ldapEntryInfo != null)
            {
                ldapEntryInfo.close_messages();
            }
        }

        idmAuthStatRecorder.end();
        return result;
    }

   protected
   String
   getPrimaryGroupDNAndGroupObject(
         ILdapConnectionEx connection,
         byte[]          userObjectSID,
         int             groupRID,
         Set<Group>      groups,
         IIdmAuthStatRecorder authStatRecorder
         ) throws NoSuchGroupException
   {
      String groupDn = null;
      String searchBaseDn =
          ( this.useGroupBaseDnForNestedGroups() )
              ? this.getStoreDataEx().getGroupBaseDn()
              : ServerUtils.getDomainDN(this.getDomain());

      SecurityIdentifier sid = SecurityIdentifier.build(userObjectSID);
      sid.setRID(groupRID);
      String groupSidString = sid.toString();

      final String ATTR_NAME_GROUP_CN = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);

      String[] attrNames = new String[] { ATTR_NAME_GROUP_CN };

      String filter = String.format(
              _ldapSchemaMapping.getGroupQueryByObjectUniqueId(),
              LdapFilterString.encode(groupSidString));

      long startTime = System.nanoTime();

      ILdapMessage message = connection.search(
            searchBaseDn,
            LdapScope.SCOPE_SUBTREE,
            filter,
            attrNames,
            false);

      if (authStatRecorder != null) {
          long elapsed = TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startTime);
          authStatRecorder.add(new LdapQueryStat(filter, searchBaseDn,
                  getConnectionString(connection), elapsed, 1));
      }

      try
      {
         ILdapEntry[] entries = message.getEntries();

         if ( ( entries != null && entries.length > 0 ) )
         {
             if (entries.length != 1)
             {
                 throw new IllegalStateException("Entry length > 1");
             }

             ILdapEntry entry = entries[0];

             String groupName = ServerUtils.getStringValue(
                 entry.getAttributeValues(ATTR_NAME_GROUP_CN)
             );

             String groupDomainName = ServerUtils.getDomainFromDN(entry.getDN());

             PrincipalId groupId = new PrincipalId( groupName, groupDomainName );

             Group g = new Group( groupId, null/*alias*/, groupSidString, null/*detail*/ );
             groups.add( g );

             groupDn = entry.getDN();
         }
         else // we did not find the entry
         {
             if ( !(this.useGroupBaseDnForNestedGroups() ) )
             {
                 logger.error(
                     String.format("Unable to find primary group with SID=[%s]", groupSidString )
                 );
             }
         }
      }
      finally
      {
         message.close();
      }

      return groupDn;
   }

   @Override
   public PersonUser findUser(
       PrincipalId id) throws Exception
   {
       if (!this.belongsToThisIdentityProvider(id.getDomain()))
       {
          String msg = String.format(
               "Domain of PrincipleId %s matches neither of the domain: [%s],"
                     +" alias: [%s] nor the registered upnSuffixes of the data store",
                     id, getStoreData().getName(), getStoreDataEx().getAlias());
          throw new InvalidPrincipalException(msg, id.getUPN());
        }

      PersonUser user = null;
      AccountLdapEntryInfo ldapEntryInfo = null;

      try (PooledLdapConnection pooledConnection = borrowConnection())
      {
	  ILdapConnectionEx connection = pooledConnection.getConnection();

          final String ATTR_NAME_SAM_ACCOUNT = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
          final String ATTR_FIRST_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
          final String ATTR_LAST_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
          final String ATTR_EMAIL_ADDRESS = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail);
          final String ATTR_DESCRIPTION = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription);
          final String ATTR_OBJECT_SID = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
          final String ATTR_USER_PRINCIPAL_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);

          final String ATTR_NAME_USER_ACCT_CTRL = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);
          final String ATTR_RESULTANT_PSO = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject);
          final String ATTR_PWD_LAST_SET = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet);
          final String ATTR_NAME_LOCKOUT_TIME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime);

          String[] attrNames = {
               ATTR_NAME_SAM_ACCOUNT,
               ATTR_USER_PRINCIPAL_NAME,
               ATTR_FIRST_NAME,
               ATTR_LAST_NAME,
               ATTR_EMAIL_ADDRESS,
               ATTR_DESCRIPTION,
               ATTR_NAME_USER_ACCT_CTRL,
               ATTR_RESULTANT_PSO,
               ATTR_PWD_LAST_SET,
               ATTR_NAME_LOCKOUT_TIME,
               ATTR_OBJECT_SID };

          final String filter_by_upn = this.buildUserQueryByUpn(id);
          final String filter_by_acct = this.buildUserQueryByAccountName(id);

         // Search from Users by default
         String searchBaseDn = this.getStoreDataEx().getUserBaseDn();

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
      }

      return user;
   }

   private long getPwdLifeTime(ILdapConnectionEx connection, String psoDn)
   {

      if (StringUtils.isNotEmpty(psoDn))
      {
         try {
            return getPwdLifeTimeFromPSO(connection, psoDn);
         }
         catch (Exception e){
            return getPwdLifeTimeFromDefaultDomainPolicy(connection);
         }
      }
      else
      {
         return getPwdLifeTimeFromDefaultDomainPolicy(connection);
      }
   }

   private long getPwdLifeTimeFromPSO(ILdapConnectionEx connection, String psoDn)
   {
       // we might be able to cache this, will be done later since
       // findUser is not a critical function for perf.

      // PSO time attributes are 64-bit, in -100 ns intervals
      // http://support.microsoft.com/kb/954414

       assert(StringUtils.isNotEmpty(psoDn));
       long maximumPasswordAge = PersonDetail.UNSPECIFIED_PASSWORD_LIFE_TIME_VALUE;
       final String ATTR_MSDS_MAXIMUM_PASSWORD_AGE = _ldapSchemaMapping.getPwdObjectAttribute(IdentityStoreAttributeMapping.AttributeIds.PasswordSettingsAttributeMaximumPwdAge);
       String[] attributes = { ATTR_MSDS_MAXIMUM_PASSWORD_AGE };

       ILdapMessage message = null;
       try {
           message = connection.search(
                                   psoDn,
                                   LdapScope.SCOPE_BASE,
                                   _ldapSchemaMapping.getPasswordSettingsQuery(),
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
           ILdapConnectionEx connection)
   {
      if (_defaultDomainPolicyPwdLifeTimeCache == null)
      {
         synchronized (this)
         {
            if (_defaultDomainPolicyPwdLifeTimeCache == null)
            {// instantiation of immutable object
               _defaultDomainPolicyPwdLifeTimeCache =
                       new PwdLifeTimeValue(
                                retrievePwdLifeTimeFromDefaultDomainPolicy(connection));
            }
         }
      }
      return _defaultDomainPolicyPwdLifeTimeCache.value;
  }

   private long retrievePwdLifeTimeFromDefaultDomainPolicy(ILdapConnectionEx connection)
   {
       final String ATTR_MAX_PWD_AGE = _ldapSchemaMapping.getDomainObjectAttribute(IdentityStoreAttributeMapping.AttributeIds.DomainAttributeMaxPwdAge);
       String domainDN = ServerUtils.getDomainDN(this.getStoreData().getName());
       long maxPwdAge = PersonDetail.UNSPECIFIED_PASSWORD_LIFE_TIME_VALUE;
       String[] attributes = { ATTR_MAX_PWD_AGE };
       ILdapMessage message = null;
       try {
          message =
                connection.search(
                      domainDN,
                      LdapScope.SCOPE_BASE,
                      _ldapSchemaMapping.getDomainObjectQuery(),
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
             final String ATTR_OBJECT_SID = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
             final String ATTR_USER_PRINCIPAL_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);

             final String ATTR_NAME_LOCKOUT_TIME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime);
             final String ATTR_RESULTANT_PSO = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject);
             final String ATTR_PWD_LAST_SET = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet);

             String[] attrNames = {
                  ATTR_NAME_CN,
                  ATTR_USER_PRINCIPAL_NAME,
                  ATTR_FIRST_NAME,
                  ATTR_LAST_NAME,
                  ATTR_EMAIL_ADDRESS,
                  ATTR_DESCRIPTION,
                  ATTR_ACCOUNT_FLAGS,
                  ATTR_NAME_LOCKOUT_TIME,
                  ATTR_OBJECT_SID,
                  ATTR_RESULTANT_PSO,
                  ATTR_PWD_LAST_SET
                  };

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
                   throw new InvalidPrincipalException(
                         String.format(
                               "user with entruUUID %s doesn't exist or multiple users with same name",
                               userEntryUuid), userEntryUuid);
               }

               int currentFlag = getOptionalIntegerValue(
                     entries[0].getAttributeValues(
                           ATTR_ACCOUNT_FLAGS),
                           0);

               user = this.buildPersonUser(entries[0], currentFlag, true, connection);
            }
            catch (NoSuchObjectLdapException e)
            {
               throw new InvalidPrincipalException(
                     String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage()), searchBaseDn);
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
        String filter = createSearchFilter(_ldapSchemaMapping.getAllUsersQuery(),
                _ldapSchemaMapping.getUserQueryByCriteriaForName(), searchString);
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

        if (limit == 0)
        {
            return users;
        }

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();

            final String ATTR_NAME_CN = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
            final String ATTR_USER_PRINCIPAL_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);
            final String ATTR_DESCRIPTION = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription);
            final String ATTR_FIRST_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
            final String ATTR_LAST_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
            final String ATTR_EMAIL_ADDRESS = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail);
            final String ATTR_ACCOUNT_FLAGS = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);
            final String ATTR_NAME_LOCKOUT_TIME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime);
            final String ATTR_OBJECT_SID = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);

            String[] attrNames = {
                    ATTR_NAME_CN,
                    ATTR_USER_PRINCIPAL_NAME,
                    ATTR_DESCRIPTION,
                    ATTR_FIRST_NAME,
                    ATTR_LAST_NAME,
                    ATTR_EMAIL_ADDRESS,
                    ATTR_ACCOUNT_FLAGS,
                    ATTR_NAME_LOCKOUT_TIME,
                    ATTR_OBJECT_SID
            };

            String searchBaseDn = getStoreDataEx().getUserBaseDn();

            try
            {
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
                                    int flag = getOptionalIntegerValue(entry.getAttributeValues(
                                                                        ATTR_ACCOUNT_FLAGS), 0);

                                    users.add(buildPersonUser(entry, flag, false, null));
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
            {//need to return detail message
                throw new InvalidPrincipalException(
                        String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage()), searchBaseDn);
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
           String filter,
           int limit) throws Exception
    {
        ValidateUtil.validateNotNull(groupId, "groupId");

        Set<PersonUser> users = new HashSet<PersonUser>();

        if (limit == 0) {
            // Short circuit since they're asking for a list of nothing anyway
            return users;
        }

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();

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

            do
            {
                membersResult = findMemberDnsInGroupInRange(connection,
                                                            _ldapSchemaMapping,
                                                            groupId,
                                                            ServerUtils.getDomainDN(this.getDomain()),
                                                            currRange,
                                                            rangeSize);

                if (membersResult != null && !membersResult.memberDns.isEmpty())
                {
                    try
                    {
                        usersInOneRange = this.findUsersByDNsWithFilter(connection, membersResult.memberDns, filter);
                    }
                    catch (Exception ex)
                    {
                        logger.warn(
                             String.format(
                                 "Find users in group %s By distinguishedName failed for range %d - %d ",
                                 groupId.getUPN(), (currRange-1)*rangeSize, currRange*rangeSize-1
                                 ),
                                 ex
                                );
                    }
                    if (!usersInOneRange.isEmpty())
                    {
                        if (usersInOneRange.size() < currLimit || currLimit < 0)
                        {
                            users.addAll(usersInOneRange);
                            if (currLimit > 0) {
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
            }while(membersResult.memberDns.size()==rangeSize && bContinueSearch && !membersResult.bNoMoreEntries);
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
      PrincipalInfo info = null;

      try (PooledLdapConnection pooledConnection = borrowConnection())
      {
	  ILdapConnectionEx connection = pooledConnection.getConnection();

          final String ATTR_NAME_CN = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
          final String ATTR_DESCRIPTION = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
          final String ATTR_OBJECT_SID = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

         String[] attrNames = { ATTR_NAME_CN, ATTR_DESCRIPTION, ATTR_OBJECT_SID };
         info = getPrincipalDN(connection, principalId);
         String filter = String.format(
                 _ldapSchemaMapping.getDirectParentGroupsQuery(),
                 LdapFilterString.encode(info.getDn()));

         try
         {
             Collection<ILdapMessage> messages = connection.paged_search(
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
            throw new InvalidPrincipalException(
                  String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage()), getStoreDataEx().getGroupBaseDn());
         }
      }

      return new PrincipalGroupLookupInfo(
          groups,
          (info != null) ?
              info.getObjectId() :
              null);
   }

   @Override
   public PrincipalGroupLookupInfo findNestedParentGroups(
       PrincipalId userId) throws Exception
   {
      Set<Group> groups = new HashSet<Group>();

      String userObjectId = null;

      AccountLdapEntryInfo ldapEntryInfo = null;
      try (PooledLdapConnection pooledConnection = borrowConnection())
      {
	 ILdapConnectionEx connection = pooledConnection.getConnection();
         String baseDN = getStoreDataEx().getUserBaseDn();

         // look up user by upn first (upn is unique in forest)
         final String filter_by_upn = this.buildUserQueryByUpn(userId);
         final String filter_by_acct = this.buildUserQueryByAccountName(userId);
         final String ATTR_USER_OBJECTID = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
         String attributes[] = {ATTR_USER_OBJECTID };

         ldapEntryInfo = findAccountLdapEntry(connection,
                 filter_by_upn,
                 filter_by_acct,
                 baseDN,
                 attributes,
                 false,
                 userId);
         ILdapEntry userLdapEntry = ldapEntryInfo.accountLdapEntry;

         // NestedParentGroups includes the direct parents, as well as the grandparents.
         populateNestedGroups(connection, userLdapEntry.getDN(), false, groups, null);
         userObjectId = getObjectSid(userLdapEntry, ATTR_USER_OBJECTID);
      }
      finally
      {
          if (ldapEntryInfo != null)
          {
              ldapEntryInfo.close_messages();
          }
      }

      return new PrincipalGroupLookupInfo(groups, userObjectId);
   }

   @Override
   public Group findGroup(
         PrincipalId groupId)
               throws Exception
   {
       final String ATTR_NAME_GROUP_CN = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
       final String ATTR_DESCRIPTION = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
       final String ATTR_OBJECT_SID = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

       String[] attrNames = { ATTR_NAME_GROUP_CN, ATTR_DESCRIPTION, ATTR_OBJECT_SID };
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
               throw new IllegalStateException("duplicated groups with name "+ groupId.getName());
            }

            return buildGroup(entries[0], null /* no default group dn */);
         }
         catch (NoSuchObjectLdapException e)
         {
            throw new InvalidPrincipalException(
                  String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage()), getStoreDataEx().getGroupBaseDn());
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
       final String ATTR_OBJECT_SID = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

       String[] attrNames = { ATTR_NAME_GROUP_CN, ATTR_DESCRIPTION, ATTR_OBJECT_SID };
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
                throw new InvalidPrincipalException(
                      String.format(
                            "Group with entryUuid %s doesn't exist or multiple groups with same name",
                            groupEntryUuid), groupEntryUuid);
             }

             return buildGroup(entries[0], null /* no default group dn */);
          }
          catch (NoSuchObjectLdapException e)
          {
             throw new InvalidPrincipalException(
                   String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage()), getStoreDataEx().getGroupBaseDn());
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
        String filter = createSearchFilter(_ldapSchemaMapping.getAllGroupsQuery(), _ldapSchemaMapping.getGroupQueryByCriteria(), searchString);
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

    private Set<Group> findGroupsInternal(
        String filter, String domainName, int limit)
            throws Exception
    {
        if (ServerUtils.isNullOrEmpty(domainName))
        {
            throw new InvalidArgumentException("findGroupsInternal failed - domainName should not be null or empty");
        }

        Set<Group> groups = new HashSet<Group>();

        // Short circuit since they're asking for an empty limit anyway
        if (limit == 0)
        {
            return groups;
        }

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();

            try
            {
                final String ATTR_NAME_CN = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
                final String ATTR_DESCRIPTION = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
                final String ATTR_OBJECT_SID = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

                String[] attrNames = { ATTR_NAME_CN, ATTR_DESCRIPTION, ATTR_OBJECT_SID };

                Collection<ILdapMessage> messages = connection.paged_search(
                        getStoreDataEx().getGroupBaseDn(),
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
                throw new InvalidPrincipalException(
                      String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage()), getStoreDataEx().getGroupBaseDn());
            }
        }

        return groups;
    }

    @Override
    public Set<Group> findGroupsInGroup(
          PrincipalId groupId, String searchString, int limit)
                throws Exception
    {
        String filter = createSearchFilter(_ldapSchemaMapping.getAllGroupsQuery(), _ldapSchemaMapping.getGroupQueryByCriteria(), searchString);
        return findGroupsInGroupInternal(groupId, filter, limit);
    }

    @Override
    public Set<Group> findGroupsByNameInGroup(
          PrincipalId groupId, String searchString, int limit)
                throws Exception
    {
        String filter = createSearchFilter(_ldapSchemaMapping.getAllGroupsQuery(), _ldapSchemaMapping.getGroupQueryByCriteriaForName(), searchString);
        return findGroupsInGroupInternal(groupId, filter, limit);
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
            ILdapConnectionEx connection = pooledConnection.getConnection();
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

            do
            {
                membersResult = findMemberDnsInGroupInRange(connection,
                                                            _ldapSchemaMapping,
                                                            groupId,
                                                            ServerUtils.getDomainDN(this.getDomain()),
                                                            currRange,
                                                            rangeSize);
                if (membersResult != null && !membersResult.memberDns.isEmpty())
                {
                    try
                    {
                        groupsInOneRange = this.findGroupsByDNsWithFilter(connection, membersResult.memberDns, filter);
                    }
                    catch (Exception ex)
                    {
                        logger.warn(
                             String.format(
                                 "Find groups in group %s By distinguishedName failed for range %d - %d ",
                                 groupId.getUPN(), (currRange-1)*rangeSize, currRange*rangeSize-1
                                 ),
                                ex
                               );
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
            }while(membersResult.memberDns.size()==rangeSize && bContinueSearch && !membersResult.bNoMoreEntries);
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
      AccountLdapEntryInfo ldapEntryInfo = null;
      ILdapEntry userLdapEntry = null;

      PooledLdapConnection pooledConnection;
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
         String searchBaseDn = this.getStoreDataEx().getUserBaseDn();

         final String filter_by_upn = this.buildUserQueryByUpn(id);
         final String filter_by_acct = this.buildUserQueryByAccountName(id);

         final String ATTR_ACCOUNT_FLAGS = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);
         final String ATTR_NAME_LOCKOUT_TIME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime);

         String attributes[] = { ATTR_ACCOUNT_FLAGS, ATTR_NAME_LOCKOUT_TIME };

         ldapEntryInfo = findAccountLdapEntry(pooledConnection.getConnection(),
                 filter_by_upn,
                 filter_by_acct,
                 searchBaseDn,
                 attributes,
                 false,
                 id);

         userLdapEntry = ldapEntryInfo.accountLdapEntry;

        accountFlags = getOptionalIntegerValue(
                userLdapEntry.getAttributeValues(
                       attributes[0]),
                       0);
        long lockoutTime = getOptionalLongValue(
                userLdapEntry.getAttributeValues(ATTR_NAME_LOCKOUT_TIME),
                PersonDetail.UNSPECIFIED_LOCKOUT_TIME_VALUE);

        if ( lockoutTime >= 1 )
        {
            accountFlags = (accountFlags | AccountControlFlag.FLAG_LOCKED_ACCOUNT.value);
        }
      }
      finally
      {
          if (ldapEntryInfo != null)
          {
              ldapEntryInfo.close_messages();
          }
          if (pooledConnection != null) {
              pooledConnection.close();
          }
      }

      return accountFlags;
   }

   private
   String getUserDN(PrincipalId id) throws Exception
   {
      AccountLdapEntryInfo ldapEntryInfo = null;
      try (PooledLdapConnection pooledConnection = borrowConnection())
      {
	  ILdapConnectionEx connection = pooledConnection.getConnection();

         String[] attrNames = {  null };

         // find user using upn first (unique in forest)
         final String filter_by_upn = this.buildUserQueryByUpn(id);
         final String filter_by_acct = this.buildUserQueryByAccountName(id);

         // Search from Users by default
         String searchBaseDn = this.getStoreDataEx().getUserBaseDn();

         ldapEntryInfo = findAccountLdapEntry(connection,
                 filter_by_upn,
                 filter_by_acct,
                 searchBaseDn,
                 attrNames,
                 true,
                 id);

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

   Group buildGroup(
         ILdapEntry entry, String groupDN)
   {
      final String ATTR_NAME_GROUP_CN = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
      final String ATTR_DESCRIPTION = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
      final String ATTR_OBJECT_SID = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

      LdapValue[] values = entry.getAttributeValues(ATTR_NAME_GROUP_CN);

      if (values == null || values.length != 1)
      {
         throw new IllegalStateException(
               "No cn attribute or multiple cn attributes for the group entry: "+ entry);
      }

      if (values[0].isEmpty())
      {
         throw new IllegalStateException("Error : Group has no name: " + entry);
      }

      String groupName = values[0].toString();

      if (groupDN == null)
      {
         groupDN = entry.getDN();
      }

      byte[] resultObjectSID =
            ServerUtils.getBinaryValue(entry
                  .getAttributeValues(ATTR_OBJECT_SID));

      SecurityIdentifier sid = SecurityIdentifier.build(resultObjectSID);
      String groupEntryObjectSid = sid.toString();

      String description = getOptionalLastStringValue(
            entry.getAttributeValues(ATTR_DESCRIPTION));

      PrincipalId gid = ServerUtils.getPrincipalId(null, groupName, ServerUtils.getDomainFromDN(groupDN));

      PrincipalId alias = ServerUtils.getPrincipalAliasId(groupName,
                                                            this.getStoreDataEx().getAlias());

      return new Group(gid, alias, groupEntryObjectSid, new GroupDetail(description));
   }

   Set<Group> populateNestedGroups(
       ILdapConnectionEx connection,
       String    dn,
       boolean   groupNameOnly,
       Set<Group> groups,
       IIdmAuthStatRecorder authStatRecorder)
             throws NoSuchGroupException, InvalidPrincipalException
   {
       Validate.notNull(groups, "groups");

       final String ATTR_NAME_GROUP_CN = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
       final String ATTR_DESCRIPTION = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
       final String ATTR_GROUP_SID = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

       ArrayList<String> attributeNames = new ArrayList<String>();
       attributeNames.add(ATTR_NAME_GROUP_CN);
       attributeNames.add(ATTR_GROUP_SID);
       if ( !groupNameOnly )
       {
           attributeNames.add(ATTR_DESCRIPTION);
       }

       final boolean bRecurse = (!this.useDirectGroupsOnly() ) && (!this.useMatchingRuleInChain());
       final String groupSearchBaseDn =
           ( this.useGroupBaseDnForNestedGroups()
               ? this.getStoreDataEx().getGroupBaseDn()
               : ServerUtils.getDomainDN(this.getDomain())
           );
       final String searchQueryTemplate =
           this.useMatchingRuleInChain()
           ? _ldapSchemaMapping.getNestedParentGroupsQuery()
           : _ldapSchemaMapping.getDirectParentGroupsQuery();

       if (logger.isDebugEnabled())
       {
           logger.debug(String.format(
              "LdapWithAdMappingsProvider.populateNestedGroups -- GroupSearchBaseDn: %s; query template: %s",
              groupSearchBaseDn, searchQueryTemplate));
       }
       int numberOfLdapSearches = 0;

       HashSet<String> groupsProcessed = new HashSet<String>();
       Stack<String> groupsToProcess = new Stack<String>();

       if(ServerUtils.isNullOrEmpty(dn) == false)
       {
          groupsToProcess.push( dn );
       }

       long startTimeForAllGrups = System.nanoTime();

       while( groupsToProcess.isEmpty() == false )
       {
           String currentDn = groupsToProcess.pop();
           groupsProcessed.add( currentDn );

           String filter = String.format(
               searchQueryTemplate,
               (this.useMatchingRuleInChain()
                    ? LdapFilterString.encodeMatchingRuleInChainDnFilter(currentDn)
                    : LdapFilterString.encode(currentDn))
           );

           String groupName = null;
           String groupDescription = null;
           String groupEntryObjectSid = null;

           ILdapPagedSearchResult prev_pagedResult = null;
           ILdapPagedSearchResult pagedResult = null;
           boolean isSearchFinished = false;

           try
           {
               int numOfQueriesPerGroup = 0;
               long startTimePerGroup = System.nanoTime();

               while ( !isSearchFinished )
               {
                   if ( logger.isTraceEnabled() )
                   {
                       logger.trace(
                          String.format(
                              "LdapWithAdMappingsProvider.populateNestedGroups -- running connection.search_one_page( %s )",
                              filter
                          )
                       );
                   }
                   pagedResult = connection.search_one_page(
                       groupSearchBaseDn,
                       LdapScope.SCOPE_SUBTREE,
                       filter,
                       attributeNames,
                       DEFAULT_PAGE_SIZE,
                       prev_pagedResult
                   );

                   numOfQueriesPerGroup += 1;
                   numberOfLdapSearches += 1;

                   if (pagedResult != null)
                   {
                       ILdapEntry[] entries = pagedResult.getEntries();
                       if( (entries != null) && ( entries.length > 0 ))
                       {
                           for(ILdapEntry entry: entries)
                           {
                               groupName = ServerUtils.getStringValue(
                                   entry.getAttributeValues(ATTR_NAME_GROUP_CN)
                               );

                               if( groupNameOnly == false )
                               {
                                   groupDescription = ServerUtils.getStringValue(
                                      entry.getAttributeValues(ATTR_DESCRIPTION));
                               }

                               byte[] resultObjectSID =
                                   ServerUtils.getBinaryValue(entry
                                      .getAttributeValues(ATTR_GROUP_SID));

                               SecurityIdentifier sid = SecurityIdentifier.build(resultObjectSID);
                               groupEntryObjectSid = sid.toString();

                               String groupDomainName = ServerUtils.getDomainFromDN(entry.getDN());

                               PrincipalId groupId = new PrincipalId( groupName, groupDomainName );
                               PrincipalId groupAlias = null;
                               GroupDetail groupDetail = null;

                               if(groupNameOnly == false)
                               {
                                   // If group lives in the registered Ad over Ldap IDP, we know the alias
                                   // Otherwise, we do not know the alias for the domain where group lives.
                                   if (groupDomainName.equalsIgnoreCase(this.getStoreData().getName()))
                                   {
                                       groupAlias = ServerUtils.getPrincipalAliasId(groupName, this.getAlias());
                                   }

                                   groupDetail = new GroupDetail(
                                      (groupDescription == null) ? "" : groupDescription );
                                }

                                Group g = new Group( groupId, groupAlias, groupEntryObjectSid, groupDetail );
                                groups.add( g );

                                if ( (bRecurse == true) && (groupsProcessed.contains(entry.getDN()) == false) )
                                {
                                    groupsToProcess.add(entry.getDN());
                                }
                            }
                        }
                    }
                    isSearchFinished = (pagedResult == null) || (pagedResult.isSearchFinished());
                    if (prev_pagedResult != null)
                    {
                        prev_pagedResult.close();
                        prev_pagedResult = null;
                    }
                    prev_pagedResult = pagedResult;
                    pagedResult = null;
                } // while !isSearchFinished

                // If summarizeLdapQueries is set false, log each ldap query
                if (authStatRecorder != null &&
                    !authStatRecorder.summarizeLdapQueries()) {
                    authStatRecorder.add(
                            new LdapQueryStat(
                                    filter,
                                    groupSearchBaseDn,
                                    getConnectionString(connection),
                                    TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startTimePerGroup),
                                    numOfQueriesPerGroup));
                }
            }
            finally
            {
                if (prev_pagedResult != null)
                {
                    prev_pagedResult.close();
                    prev_pagedResult = null;
                }
                if (pagedResult != null)
                {
                    pagedResult.close();
                    pagedResult = null;
                }
            }

        } // groupstoprocess not empty

        if (logger.isDebugEnabled())
        {
            logger.debug(String.format(
               "LdapWithAdMappingsProvider.populateNestedGroups -- ran [%d] ldap searches.",
               numberOfLdapSearches));
        }

        // If summarizeLdapQueries is set true, log once only with summary
        if (authStatRecorder != null &&
            authStatRecorder.summarizeLdapQueries()) {
            authStatRecorder.add(
                new LdapQueryStat(
                    searchQueryTemplate,
                    groupSearchBaseDn,
                    getConnectionString(connection),
                    TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startTimeForAllGrups),
                    numberOfLdapSearches));
        }

        return groups;
    }

    protected
    Collection<PersonUser>
    findUsersByDNsWithFilter(
          ILdapConnectionEx connection,
          Collection<String> memberDNs,
          String          filter
          ) throws Exception
    {
        ArrayList<PersonUser> users = new ArrayList<PersonUser>();

        final String ATTR_NAME_CN = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName );
        final String ATTR_DESCRIPTION = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription );
        final String ATTR_FIRST_NAME = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName );
        final String ATTR_LAST_NAME = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName );
        final String ATTR_EMAIL_ADDRESS = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail );
        final String ATTR_USER_PRINCIPAL_NAME = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName );
        final String ATTR_ACCOUNT_FLAGS = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl );
        final String ATTR_NAME_LOCKOUT_TIME = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime );
        final String ATTR_OBJECT_SID = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId );

        String[] attrNames = {
                ATTR_NAME_CN,
                ATTR_DESCRIPTION,
                ATTR_FIRST_NAME,
                ATTR_LAST_NAME,
                ATTR_EMAIL_ADDRESS,
                ATTR_USER_PRINCIPAL_NAME,
                ATTR_ACCOUNT_FLAGS,
                ATTR_NAME_LOCKOUT_TIME,
                ATTR_OBJECT_SID
        };

        if (memberDNs != null && !memberDNs.isEmpty())
        {
            filter = _ldapSchemaMapping.getDNFilter(filter, memberDNs);

            ILdapMessage message = null;
            try
            {
                message = connection.search(
                        ServerUtils.getDomainDN(this.getDomain()),
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
                                            ATTR_ACCOUNT_FLAGS),
                                            0);

                            PersonUser user = this.buildPersonUser(
                                    userEntry,
                                    flag, false, null);
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
         int        accountFlags,
         boolean    provideExtendedAccountInfo,
         ILdapConnectionEx connection
         )
   {
      if (provideExtendedAccountInfo)
      {
         ValidateUtil.validateNotNull(connection, "connection");
      }else
      {
         ValidateUtil.validateNull(connection, "connection");
      }
      final String ATTR_FIRST_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
      final String ATTR_LAST_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
      final String ATTR_EMAIL_ADDRESS = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail);
      final String ATTR_USER_PRINCIPAL_NAME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);
      final String ATTR_DESCRIPTION = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription);
      final String ATTR_NAME_SAM_ACCOUNT = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);

      final String ATTR_NAME_LOCKOUT_TIME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime);
      final String ATTR_OBJECT_SID = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
      final String ATTR_RESULTANT_PSO = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject);
      final String ATTR_PWD_LAST_SET = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet);

      String accountName = getStringValue(
              entry.getAttributeValues(
                    ATTR_NAME_SAM_ACCOUNT));

       String description =
              getOptionalLastStringValue(
                    entry.getAttributeValues(
                          ATTR_DESCRIPTION));

       String firstName =
            getOptionalStringValue(
                  entry.getAttributeValues(
                        ATTR_FIRST_NAME));

      String lastName =
            getOptionalStringValue(
                  entry.getAttributeValues(
                        ATTR_LAST_NAME));

      String email = getOptionalStringValue(
            entry.getAttributeValues(
                  ATTR_EMAIL_ADDRESS));

      String upn = getOptionalStringValue(
            entry.getAttributeValues(
                  ATTR_USER_PRINCIPAL_NAME));

      PrincipalId id = ServerUtils.getPrincipalId(upn, accountName, ServerUtils.getDomainFromDN(entry.getDN()));

      PrincipalId alias = ServerUtils.getPrincipalAliasId(accountName,
              getStoreDataEx().getAlias());

      String resultObjectSid = null;
      long pwdLastSet = PersonDetail.UNSPECIFIED_TS_VALUE;
      long lockoutTime = PersonDetail.UNSPECIFIED_LOCKOUT_TIME_VALUE;
      long pwdLifeTime = PersonDetail.UNSPECIFIED_PASSWORD_LIFE_TIME_VALUE;

      if ( provideExtendedAccountInfo )
      {
          byte[] resultObjectSID =
                  ServerUtils.getBinaryValue(entry
                        .getAttributeValues(ATTR_OBJECT_SID));

            SecurityIdentifier sid = SecurityIdentifier.build(resultObjectSID);
            resultObjectSid = sid.toString();

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

            // if pwdNeverExpired is set in user account control, treat it as non-defined pwd life time.
            if (!AccountControlFlag.FLAG_DONT_EXPIRE_PASSWD.isSet(accountFlags))
            {
                pwdLifeTime = getPwdLifeTime(connection, resultantPSODn);
            }
      }

      PersonDetail detail = new PersonDetail.Builder()
                                  .firstName(firstName)
                                  .lastName(lastName)
                                  .userPrincipalName(upn)
                                  .emailAddress(email)
                                  .description(description)
                                  .pwdLastSet(pwdLastSet, IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING)
                                  .pwdLifeTime(pwdLifeTime, IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING)
                               .build();

      return new PersonUser(
            id,
            alias,
            resultObjectSid,
            detail,
            AccountControlFlag.FLAG_DISABLED_ACCOUNT.isSet(accountFlags),
            AccountControlFlag.FLAG_LOCKED_ACCOUNT.isSet(accountFlags) || (lockoutTime >= 1));
   }

   private class PrincipalInfo
   {
       private final String _dn;
       private final String _objectId;
       public PrincipalInfo(String dn, String objectId)
       {
           this._dn = dn;
           this._objectId = objectId;
       }
       public String getDn()
       {
           return this._dn;
       }

       public String getObjectId()
       {
           return this._objectId;
       }
   }

   private
   PrincipalInfo getPrincipalDN(
       ILdapConnectionEx connection,
       PrincipalId principalId
       ) throws Exception
   {
       final String ATTR_USER_OBJECT_ID = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
       final String ATTR_GROUP_OBJECT_ID = _ldapSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);

       String[] attrNames = { ATTR_USER_OBJECT_ID, ATTR_GROUP_OBJECT_ID, null };
       AccountLdapEntryInfo ldapEntryInfo = null;

       // search user/groups with samAccount first
       final String filter_by_acct = buildUserOrGroupFilterByAccountName(principalId);
       final String filter_by_upn = buildUserQueryByUpn(principalId);

       try
       {
           ldapEntryInfo = findAccountLdapEntry(connection,
                   filter_by_upn,
                   filter_by_acct,
                   ServerUtils.getDomainDN(getStoreData().getName()),
                   attrNames,
                   false,
                   principalId);
            String objectId = getObjectSid(ldapEntryInfo.accountLdapEntry, ATTR_USER_OBJECT_ID);
            if(ServerUtils.isNullOrEmpty(objectId))
            {
                objectId = getObjectSid(ldapEntryInfo.accountLdapEntry, ATTR_GROUP_OBJECT_ID);
            }
            return new PrincipalInfo(ldapEntryInfo.accountLdapEntry.getDN(), objectId);
       }
       finally
       {
           if (ldapEntryInfo != null)
           {
               ldapEntryInfo.close_messages();
           }
       }
   }

   private static String getObjectSid(ILdapEntry entry, String objectSidAttributeName)
   {
       String sidStr = null;
       if ( ( entry != null ) && (ServerUtils.isNullOrEmpty(objectSidAttributeName) == false) )
       {
           byte[] resultObjectSID =
                ServerUtils.getBinaryValue(entry
                     .getAttributeValues(objectSidAttributeName));

           if ((resultObjectSID != null) && (resultObjectSID.length != 0))
           {
               SecurityIdentifier sid = SecurityIdentifier.build(resultObjectSID);
               sidStr = sid.toString();
           }
       }
       return sidStr;
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
          final String ATTR_USER_PRINCIPAL_NAME = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName );
          final String ATTR_ACCOUNT_FLAGS = _ldapSchemaMapping.getUserAttribute( IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl );
          final String ATTR_NAME_LOCKOUT_TIME = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime);
          final String ATTR_OBJECT_SID = _ldapSchemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);

         String[] attrNames = {
               ATTR_NAME_CN,
               ATTR_USER_PRINCIPAL_NAME,
               ATTR_DESCRIPTION,
               ATTR_FIRST_NAME,
               ATTR_LAST_NAME,
               ATTR_EMAIL_ADDRESS,
               ATTR_ACCOUNT_FLAGS,
               ATTR_NAME_LOCKOUT_TIME,
               ATTR_OBJECT_SID
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
                                 int flags = getOptionalIntegerValue(
                                               entry.getAttributeValues(
                                               ATTR_ACCOUNT_FLAGS),
                                               0);

                                 long lockoutTime = getOptionalLongValue(
                                         entry.getAttributeValues(ATTR_NAME_LOCKOUT_TIME),
                                         PersonDetail.UNSPECIFIED_LOCKOUT_TIME_VALUE);

                                 boolean isBitSet = ( controlBit.isSet(flags) || ( (AccountControlFlag.FLAG_LOCKED_ACCOUNT.isSet(controlBit.value)) && (lockoutTime >= 1) ));

                                 if (!isBitSet)
                                 {
                                     continue;
                                 }
                                 users.add(buildPersonUser(entry, flags, false, null));
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
            throw new InvalidPrincipalException(
                  String.format("errorCode; %d; %s", e.getErrorCode(), e.getMessage()), searchBaseDn);
         }
      }

      return users;
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

        final String ATTR_NAME_CN = _ldapSchemaMapping.getGroupAttribute( IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName );
        final String ATTR_DESCRIPTION = _ldapSchemaMapping.getGroupAttribute( IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription );
        final String ATTR_OBJECT_SID = _ldapSchemaMapping.getGroupAttribute( IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId );

        String[] attrNames = { ATTR_NAME_CN, ATTR_DESCRIPTION, ATTR_OBJECT_SID };

        if (memberDNs != null && !memberDNs.isEmpty())
        {
            filter = _ldapSchemaMapping.getDNFilter(filter, memberDNs);

            ILdapMessage message = null;
            try
            {
                message = connection.search(
                        ServerUtils.getDomainDN(this.getDomain()),
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

   private String buildUserOrGroupFilterByAccountName(PrincipalId principal)
   {
       ValidateUtil.validateNotNull(principal, "principal");

       String escapedsAMAccountName = LdapFilterString.encode(principal.getName());
           return String.format(
               this._ldapSchemaMapping.getUserOrGroupQueryByAccountName(),
               escapedsAMAccountName);
   }

   private String buildUserQueryByUpn(PrincipalId principal)
   {
       ValidateUtil.validateNotNull(principal, "principal");

       String escapedPrincipalName = LdapFilterString.encode(GetUPN(principal));
       return String.format(
               this._ldapSchemaMapping.getUserQueryByUpn(),
               escapedPrincipalName);
   }

   private String buildUserQueryByAccountName(PrincipalId principal)
   {
       ValidateUtil.validateNotNull(principal, "principal");

       String escapedsAMAccountName = LdapFilterString.encode(principal.getName());
       return String.format(
               this._ldapSchemaMapping.getUserQueryByAccountName(),
               escapedsAMAccountName);
   }

   private boolean useDirectGroupsOnly()
   {
       return (this.getStoreDataEx().getFlags() & LdapWithAdMappingsProvider.FLAG_DIRECT_GROUPS_ONLY ) != 0;
   }

   private boolean useMatchingRuleInChain()
   {
       return  ( ( (this.getStoreDataEx().getFlags() & LdapWithAdMappingsProvider.FLAG_AD_MATCHING_RULE_IN_CHAIN ) != 0 )
                  &&
                  ( this.useDirectGroupsOnly() == false ) );
   }

   private boolean useGroupBaseDnForNestedGroups()
   {
       return (this.getStoreDataEx().getFlags() & LdapWithAdMappingsProvider.FLAG_DO_NOT_USE_BASE_DN_FOR_NESTED_GROUPS ) == 0;
   }

   private boolean enableSiteAffinity()
   {
       return (this.getStoreDataEx().getFlags() & LdapWithAdMappingsProvider.FLAG_ENABLE_SITE_AFFINITY ) != 0;
   }

   private
   PooledLdapConnection
   borrowConnectionWithdomainStr(String domainNameStr, boolean bSsl) throws Exception
   {
       PooledLdapConnection connection = null;
       ArrayList<String> connStrs = new ArrayList<String>();

       try
       {
           // Get GC connection first
           String connStr = String.format("%s://%s:%d", bSsl ? "ldaps" : "ldap",
                                          domainNameStr,
                                          bSsl ? LdapConstants.LDAP_SSL_GC_PORT : LdapConstants.LDAP_GC_PORT);
           connStrs.add(connStr);
           logger.trace(String.format("The affinitized connection is %s", connStr));
           connection = borrowConnection(connStrs, false);
           _connInfoCache.addConnInfo(this.getDomain(), connStrs);

           return connection;
       }
       catch(Exception gc_ex)
       {
           String errMsg = String.format("Failed to get GC connection to domain %s due to %s",
                   domainNameStr, gc_ex.getMessage());
           logger.error(errMsg, gc_ex);
       }

       // Continue attempt to get non-GC connection
       try
       {
           String connStr = String.format("%s://%s:%d", bSsl ? "ldaps" : "ldap",
                   domainNameStr,
                   bSsl ? LdapConstants.LDAP_SSL_PORT : LdapConstants.LDAP_PORT);
           connStrs.add(connStr);
           logger.trace(String.format("The affinitized connection is %s", connStr));
           connection = borrowConnection(connStrs, false);
           _connInfoCache.addConnInfo(this.getDomain(), connStrs);

           return connection;
       }
       catch(Exception ex)
       {
           String errMsg = String.format("Failed to get a connection to domain %s due to %s",
                   domainNameStr, ex.getMessage());
           logger.error(errMsg, ex);

           throw ex;
       }
   }

    private
    PooledLdapConnection
    borrowConnectionWithSiteAffinity() throws Exception
    {
        boolean bForceRediscover = false;
        String domainName = this.getDomain();
        List<String> connStrs = _connInfoCache.findConnInfo(domainName);
        if (connStrs != null && !connStrs.isEmpty())
        {
            try
            {
                return borrowConnection(connStrs, false);
            }
            catch(Exception ex)
            {
                logger.error(String.format("Failed to get ldap connection with cached connection [%s] "
                        + "due to %s", connStrs.get(0), ex.getMessage()), ex);
                _connInfoCache.deleteConnInfo(domainName);
                bForceRediscover = true;
            }
        }

        // In case connInfo is null or
        // not able to establish connection using the connection information
        // Do the complete site affinity logic
        DomainControllerInfo siteInfo = null;
        IDomainAdapter domainAdapter =
                DomainAdapterFactory.getInstance().getDomainAdapter();

        try
        {
            if (bForceRediscover)
            {
                siteInfo = domainAdapter.getDcInfoWithRediscover(domainName);
            }
            else
            {
                siteInfo = domainAdapter.getDcInfo(domainName);
            }
        }
        catch(DomainManagerNativeException|DomainManagerException exDcInfo)
        {
            logger.error(String.format("Get domain controller information (site info) for domain [%s] failed due to %s",
                    domainName, exDcInfo.getMessage()), exDcInfo);
        }

        if (siteInfo == null)
        {
            throw new IDMException(String.format("Failed to do site affinity for domain %s", domainName));
        }

        // Use affinitized site information
        String domainNameStr = domainName;
        if (!ServerUtils.isNullOrEmpty(siteInfo.domainFQDN))
        {
            domainNameStr = siteInfo.domainFQDN;
        }
        else if (!ServerUtils.isNullOrEmpty(siteInfo.domainIpAddress))
        {
            domainNameStr = siteInfo.domainIpAddress;
        }

        // Determine whether we do ldaps or ldap based on customer provided ldap connection strings
        Collection<URI> customerURIs = ServerUtils.toURIObjects(this.getStoreDataEx().getConnectionStrings());
        boolean isLdaps = false;

        if (customerURIs == null || customerURIs.isEmpty())
        {
            isLdaps = true;
        }
        else
        {
            for (URI uri : customerURIs)
            {
                isLdaps = (uri == null) ? true : DirectoryStoreProtocol.LDAPS.getName().equalsIgnoreCase(uri.getScheme());
                if (uri != null && isLdaps) break;
            }
        }

        try
        {
            return borrowConnectionWithdomainStr(domainNameStr, isLdaps);
        }
        catch(Exception ex)
        {
            String errMsg = String.format("Cannot get a %s connection to domain %s with site affinity due to %s",
                    isLdaps ? "ldaps" : "ldap", domainNameStr, ex.getMessage());
            logger.error(errMsg, ex);
            throw ex;
        }
    }

   @Override
    public Collection<SecurityDomain> getDomains() {
        Collection<SecurityDomain> domains = new HashSet<SecurityDomain>();
        domains.add(new SecurityDomain(super.getDomain(), super.getAlias()));
        return domains;
    }

@Override
public PrincipalId findActiveUser(String attributeName, String attributeValue) throws Exception {
        Validate.notEmpty(attributeName, "attributeName");
        Validate.notEmpty(attributeValue, "attributeValue");

        PooledLdapConnection pooledConnection;
        AccountLdapEntriesInfo entriesInfo = null;

        IIdmAuthStatRecorder idmAuthStatRecorder = this.createIdmAuthStatRecorderInstance(
                DiagnosticsContextFactory.getCurrentDiagnosticsContext().getTenantName(),
                ActivityKind.AUTHENTICATE, EventLevel.INFO, attributeValue);
        idmAuthStatRecorder.start();

        try {
            try
            {
                pooledConnection = borrowConnection();
            }
            catch (Exception ex)
            {
                throw new IDMException("Failed to establish server connection", ex);
            }

            try
            {
                 String[] attrNames =
                        { ATTR_NAME_SAM_ACCOUNT, ATTR_USER_PRINCIPAL_NAME , ATTR_NAME_USER_ACCT_CTRL};

                 String searchBaseDn = this.getStoreDataEx().getUserBaseDn();

                 final String filter_by_attr = this.buildUserQueryByAttribute(attributeName, attributeValue);

                entriesInfo = findAccountLdapEntry(pooledConnection.getConnection(),
                         filter_by_attr,
                         searchBaseDn,
                         attrNames,
                        false, attributeValue, idmAuthStatRecorder);


                ILdapEntry entry = entriesInfo.accountLdapEntries.iterator().next();
                 String accountName =
                        getStringValue(entry.getAttributeValues(ATTR_NAME_SAM_ACCOUNT));

                 String userPrincipalName =
                        getOptionalStringValue(entry.getAttributeValues(ATTR_USER_PRINCIPAL_NAME));

                 int currentFlag =
                     getOptionalIntegerValue(
                                entry.getAttributeValues(ATTR_NAME_USER_ACCT_CTRL), 0);

                 if ( !AccountControlFlag.FLAG_DISABLED_ACCOUNT.isSet(currentFlag))
                 {
                     return ServerUtils.getPrincipalId(userPrincipalName, accountName, this.getDomain());
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

        } finally {
            idmAuthStatRecorder.end();
        }

    }

    @Override
    public UserSet findActiveUsersInDomain(String attributeName, String attributeValue
            , String userDomain, String additionalAttribute)
            throws Exception {
        Validate.notEmpty(attributeName, "attributeName");
        Validate.notEmpty(attributeValue, "attributeValue");
        Validate.notEmpty(userDomain, "userDomain");

        UserSet result = new UserSet();
        PooledLdapConnection pooledConnection;

        AccountLdapEntriesInfo entries = null;

        IIdmAuthStatRecorder idmAuthStatRecorder = this.createIdmAuthStatRecorderInstance(DiagnosticsContextFactory.getCurrentDiagnosticsContext()
                .getTenantName(), ActivityKind.AUTHENTICATE, EventLevel.INFO, attributeValue);
        idmAuthStatRecorder.start();
        try {
            try {
                pooledConnection = borrowConnection();
            } catch (Exception ex) {
                throw new IDMException("Failed to establish server connection", ex);
            }

            try {
                ArrayList<String> attrNames = new ArrayList<String>();
                attrNames.add(ATTR_NAME_SAM_ACCOUNT);
                attrNames.add(ATTR_USER_PRINCIPAL_NAME);
                attrNames.add(ATTR_NAME_USER_ACCT_CTRL);

                if (additionalAttribute != null) {
                    attrNames.add(additionalAttribute);
                }

                String searchBaseDn = this.getStoreDataEx().getUserBaseDn();
                String filter_by_attr = this.buildUserQueryByAttribute(attributeName, attributeValue);
                entries = findAccountLdapEntries(pooledConnection.getConnection(), filter_by_attr, searchBaseDn,
                        attrNames.toArray(new String[attrNames.size()]), false,
                        attributeValue, true, idmAuthStatRecorder);

                String accountName = null;
                for (ILdapEntry entry : entries.accountLdapEntries) {

                    accountName = getStringValue(entry.getAttributeValues(ATTR_NAME_SAM_ACCOUNT));

                    String userPrincipalName = getOptionalStringValue(entry.getAttributeValues(ATTR_USER_PRINCIPAL_NAME));

                    int currentFlag = getOptionalIntegerValue(entry.getAttributeValues(ATTR_NAME_USER_ACCT_CTRL), 0);

                    Collection<String> additionalAttrValues = null;

                    if (additionalAttribute != null) {
                        additionalAttrValues = getOptionalStringValues(entry.getAttributeValues(additionalAttribute));
                    }

                    if (!AccountControlFlag.FLAG_DISABLED_ACCOUNT.isSet(currentFlag) && !AccountControlFlag.FLAG_LOCKED_ACCOUNT.isSet(currentFlag)) {
                        result.put(ServerUtils.getPrincipalId(userPrincipalName, accountName, userDomain), additionalAttrValues);
                    }
                }
                if (result.isEmpty()) {
                    throw new InvalidPrincipalException(String.format("User account '%s@%s' is not active. ", accountName, userDomain),
                            String.format("%s@%s", accountName, userDomain));
                }

            } finally {
                if (entries != null) {
                    entries.close();
                }
                if (pooledConnection != null) {
                    pooledConnection.close();
                }
            }

        } finally {
            idmAuthStatRecorder.end();
        }

        return result;
    }

    private String buildUserQueryByAttribute(String attrName, String attrValue)
    {
        ValidateUtil.validateNotNull(attrName, "attrName");
        ValidateUtil.validateNotNull(attrValue, "attrValue");

        String escapedsAttrName = LdapFilterString.encode(attrName);
        String escapedsAttrVal = LdapFilterString.encode(attrValue);

        return String.format(
                this._ldapSchemaMapping.getUserQueryByAttribute(),
                escapedsAttrName, escapedsAttrVal);
    }

    private PooledLdapConnection borrowConnection() throws Exception {
	if (enableSiteAffinity()) {
	    return borrowConnectionWithSiteAffinity();
	} else {
	    return borrowConnection(this.getStoreDataEx().getConnectionStrings(), false);
	}
    }

    @Override
    public String getStoreUPNAttributeName() {
        return ATTR_USER_PRINCIPAL_NAME;
    }
}
