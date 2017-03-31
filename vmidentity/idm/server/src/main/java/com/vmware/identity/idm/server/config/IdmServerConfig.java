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

package com.vmware.identity.idm.server.config;

import java.net.URI;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.TimeUnit;

import org.apache.commons.lang.SystemUtils;

import com.vmware.af.PasswordCredential;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.vmaf.VmafClientUtil;
import com.vmware.identity.interop.PlatformUtils;
import com.vmware.identity.interop.ldap.DirectoryStoreProtocol;
import com.vmware.identity.interop.ldap.LdapConstants;
import com.vmware.identity.interop.registry.IRegistryAdapter;
import com.vmware.identity.interop.registry.IRegistryKey;
import com.vmware.identity.interop.registry.RegKeyAccess;
import com.vmware.identity.interop.registry.RegistryAdapterFactory;
import com.vmware.identity.interop.registry.RegistryNoSuchKeyOrValueException;

/**
 * Created by IntelliJ IDEA.
 * User: mpotapova
 * Date: 1/30/12
 * Time: 5:21 PM
 * To change this template use File | Settings | File Templates.
 */
public class IdmServerConfig
{
   private static final String CONFIG_ROOT_KEY =
         "Software\\VMware\\Identity\\Configuration";
   private static String CONFIG_DIRECTORY_ROOT_KEY;
   private static String CONFIG_DIRECTORY_PARAMETERS_KEY;
   private static final String CONFIG_DIRECTORY_DCACCOUNT_DN_VALUE =
         "dcAccountDN";

   private static int POLLING_INTERVAL_MILLISECS = 1000;
   // config stores related properties
   private final String CONFIG_STORE_TYPE_KEY = "ConfigStoreType";

   // settings for system domain identity store
   private final String CONFIG_SYSTEM_DOMAIN_ATTRIBUTES_MAP = "SystemDomainAttributesMap";
   private final String CONFIG_SYSTEM_DOMAIN_SEARCH_TIMEOUT = "SystemDomainSearchTimeout";

   // multi-tenancy configuration
   private final String CONFIG_SYSTEM_MULTI_TENANT = "MultiTenant";

   // integer 1 - on; if this is on, then SystemDomain Alias and UserAliases will take effect
   private final String CONFIG_SP_SYSTEM_DOMAIN_BACKCOMPAT_MODE = "SPSystemDomainBackCompat";
   // settings for back compat service provider system-domain aliasing
   // string value such as 'System-Domain'
   // non-null, non-empty alias signals back-compat mode
   private final String CONFIG_SP_SYSTEM_DOMAIN_ALIAS = "SPSystemDomainAlias";
   // multi-string value mapping from-to such as 'admin@administrator'
   // pick a upn separator, because it cannot be part of a valid accountName -> safe to use as a separator
   // we will only ever use this if system-domain alias is specified
   private final String CONFIG_SP_SYSTEM_DOMAIN_USER_ALIASES = "SPSystemDomainUserAliases";
   // LDAP IdPs certificates validation enabled property
   private final String CONFIG_LDAPS_CERT_VALIDATION_KEY = "LdapsCertValidation";
   private final String CONFIG_LDAP_CONNECTION_POOL_MAX = "LdapConnectionPoolMax";
   private final String CONFIG_LDAP_CONNECTION_POOL_EVICTION_INTERVAL = "LdapConnectionPoolEvictionInterval";
   private final String CONFIG_LDAP_CONNECTION_POOL_IDLE_TIME = "LdapConnectionPoolIdleTime";
   private final String CONFIG_LDAP_CONNECTION_POOL_MAX_WAIT = "LdapConnectionPoolMaxWait";

   private static final String UPN_SEP_STR = new String(new char[] {ValidateUtil.UPN_SEPARATOR});

   // config stores related properties

   //This allows us to switch between ldap and ldaps
   //Hard coded for now, it can be changed to runtime configurable by reading from a new registry value
   private final boolean _useSSL = false;

   private ConfigStoreType _configStoreType;
   private String _directoryConfigStoreHost;
   private int _directoryConfigStorePort;
   private String _directoryConfigStoreUserName;
   private AuthenticationType _directoryConfigStoreAuthType;

   // settings for system domain identity store
   private IdentityStoreType _systemDomainIdentityStoreType;
   private Collection<URI> _systemDomainConnectionInfo;
   private String _systemDomainUserBaseDn;
   private String _systemDomainGroupBaseDn;
   private Map<String, String> _systemDomainAttributesMap;
   private int _systemDomainSearchTimeout;

   private boolean _isSingleTenantConfig;
   private boolean _isServiceProviderSystemDomainInBackCompatMode;
   private String _serviceProviderSystemDomainAlias;
   private Map<String, String> _serviceProviderSystemDomainUserAliases;

   private final List<Attribute> _defaultAttributes;

   // IdmAuthStats registry configuration
   private final String CONFIG_SYSTEM_IDM_AUTH_STATS_CACHE_DEPTH = "AuthStatsCacheDepth";
   private final String CONFIG_SYSTEM_IDM_AUTH_STATS_SUMMARIZE_LDAP_QUERIES = "AuthStatsSummarizedLdapQueries";
   private boolean _idmAuthStatsSummarizeLdapQueries = false;
   private int _idmAuthStatsCacheDepth = 10;

   public static final String ATTRIBUTE_GROUPS = "http://rsa.com/schemas/attr-names/2009/01/GroupIdentity";
   public static final String ATTRIBUTE_FIRST_NAME = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/givenname";
   public static final String ATTRIBUTE_LAST_NAME = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/surname";
   public static final String ATTRIBUTE_SUBJECT_TYPE = "http://vmware.com/schemas/attr-names/2011/07/isSolution";
   public static final String ATTRIBUTE_USER_PRINCIPAL_NAME = "http://schemas.xmlsoap.org/claims/UPN";
   public static final String ATTRIBUTE_EMAIL = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/emailaddress";

   private static final String ATTRIBUTE_GROUPS_FRIENDLY_NAME = "Groups";
   private static final String ATTRIBUTE_FIRST_NAME_FRIENDLY_NAME = "givenName";
   private static final String ATTRIBUTE_LAST_NAME_FRIENDLY_NAME = "surname";
   private static final String ATTRIBUTE_SUBJECT_TYPE_FRIENDLY_NAME = "Subject Type";
   private static final String ATTRIBUTE_USER_PRINCIPAL_NAME_FRIENDLY_NAME = "userPrincipalName";
   private static final String ATTRIBUTE_EMAIL_FRIENDLY_NAME = "email";

   private static final String ATTRNAME_FORMAT_URI = "urn:oasis:names:tc:SAML:2.0:attrname-format:uri";
   private static final int DEFAULT_LDAP_CONNECTIONS = 2000;
   private static final long DEFAULT_LDAP_CONNECTION_POOL_EVICTION_INTERVAL = TimeUnit.MINUTES.toMillis(1);
   private static final long DEFAULT_LDAP_CONNECTION_POOL_IDLE_TIME = TimeUnit.MINUTES.toMillis(1);
   private static final long DEFAULT_LDAP_CONNECTION_MAX_WAIT_MILIS = TimeUnit.SECONDS.toMillis(30);

   private static IdmServerConfig ourInstance = new IdmServerConfig();

   public static IdmServerConfig getInstance() {
      return ourInstance;
   }

   public ConfigStoreType getTypeOfConfigurationStore()
   {
      return this._configStoreType;
   }

   public String getDirectoryConfigStoreHost()
   {
      return this._directoryConfigStoreHost;
   }

   public int getDirectoryConfigStorePort()
   {
      return this._directoryConfigStorePort;
   }

   public String getDirectoryConfigStoreUserName()
   {
      return this._directoryConfigStoreUserName;
   }

   public AuthenticationType getDirectoryConfigStoreAuthType()
   {
      return this._directoryConfigStoreAuthType;
   }

   public String getDirectoryConfigStorePassword()
   {
      PasswordCredential creds = VmafClientUtil.getMachineAccountCredentials();
      String password = null;

      if (creds != null) {
         password = creds.getPassword();
      }

      return password;
   }

   public String getDirectoryConfigStoreDomain()
   {
      return VmafClientUtil.getDomainName();
   }

   public IdentityStoreType getSystemDomainIdentityStoreType()
   {
      return this._systemDomainIdentityStoreType;
   }

   public Collection<URI> getSystemDomainConnectionInfo()
   {
      return this._systemDomainConnectionInfo;
   }

   public AuthenticationType getSystemDomainAuthenticationType()
   {
      return this.getDirectoryConfigStoreAuthType();
   }

   public String getSystemDomainUserName()
   {
      return getDirectoryConfigStoreUserName();
   }

   public String getSystemDomainPassword()
   {
      return getDirectoryConfigStorePassword();
   }

   public String getSystemDomainUserBaseDn()
   {
      return this._systemDomainUserBaseDn;
   }

   public String getSystemDomainGroupBaseDn()
   {
      return this._systemDomainGroupBaseDn;
   }

   public Map<String, String> getSystemDomainAttributesMap()
   {
      return this._systemDomainAttributesMap;
   }

   public int getSystemDomainSearchTimeout()
   {
      return this._systemDomainSearchTimeout;
   }

   public boolean getIsSingletenantConfig()
   {
      return this._isSingleTenantConfig;
   }

   public boolean isServiceProviderSystemDomainInBackCompatMode()
   {
       return (this._isServiceProviderSystemDomainInBackCompatMode == true)
              &&
              (ServerUtils.isNullOrEmpty(this.getServiceProviderSystemDomianAlias()) == false);
   }

   public String getServiceProviderSystemDomianAlias()
   {
       return this._serviceProviderSystemDomainAlias;
   }

   public Map<String, String> getServiceProviderSystemDomianUserAliases()
   {
       return this._serviceProviderSystemDomainUserAliases;
   }

   public String getTenantsSystemDomainName(String tenantName)
   {
      ValidateUtil.validateNotEmpty(tenantName, "tenantName");
      return String.format("%s", tenantName);
   }

   public String getTenantAdminUserName(String tenantName, String adminAccountName)
   {
      ValidateUtil.validateNotEmpty( tenantName, "tenantName" );
      ValidateUtil.validateNotEmpty( adminAccountName, "adminAccountName" );
      return String.format(
            "%s@%s",
            adminAccountName,
            getTenantsSystemDomainName(tenantName));
   }

   public List<Attribute> getDefaultAttributesList()
   {
      return this._defaultAttributes;
   }

   public int getIdmAuthStatsCacheDepth(){
       return this._idmAuthStatsCacheDepth;
   }

   public boolean getIdmAuthStatsSummarizeLdapQueries(){
       return this._idmAuthStatsSummarizeLdapQueries;
   }

   public boolean isLdapsCertValidationEnabled()
   {
      IRegistryAdapter regAdapter = RegistryAdapterFactory.getInstance().getRegistryAdapter();
      try(IRegistryKey rootKey = regAdapter.openRootKey((int) RegKeyAccess.KEY_READ))
      {
         String isCertValidationEnabled = regAdapter.getStringValue(rootKey, CONFIG_ROOT_KEY, CONFIG_LDAPS_CERT_VALIDATION_KEY, true);

         if(isCertValidationEnabled == null)
         {
            return false;
         }
         else
         {
            return isCertValidationEnabled.equalsIgnoreCase(Boolean.TRUE.toString());
         }
       }
   }

    public int getLdapConnPoolMaxConnections() {
	IRegistryAdapter regAdapter = RegistryAdapterFactory.getInstance().getRegistryAdapter();
	try (IRegistryKey rootKey = regAdapter.openRootKey((int) RegKeyAccess.KEY_READ)) {
	    Integer maxLdapConnections = regAdapter.getIntValue(rootKey, CONFIG_ROOT_KEY,
		    CONFIG_LDAP_CONNECTION_POOL_MAX, true);

	    if (maxLdapConnections == null) {
		return DEFAULT_LDAP_CONNECTIONS;
	    } else {
		return maxLdapConnections;
	    }
	}
    }

    public long getLdapConnPoolMaxWait() {
	IRegistryAdapter regAdapter = RegistryAdapterFactory.getInstance().getRegistryAdapter();
	long ldapConnectionMaxWaitMilis = DEFAULT_LDAP_CONNECTION_MAX_WAIT_MILIS;

	try (IRegistryKey rootKey = regAdapter.openRootKey((int) RegKeyAccess.KEY_READ)) {
	    String ldapConnectionMaxWaitMilisValue = regAdapter.getStringValue(rootKey, CONFIG_ROOT_KEY,
		    CONFIG_LDAP_CONNECTION_POOL_MAX_WAIT, true);

	    if (ldapConnectionMaxWaitMilisValue != null) {
		try {
		    ldapConnectionMaxWaitMilis = Long.parseLong(ldapConnectionMaxWaitMilisValue);
		    if (ldapConnectionMaxWaitMilis <= 0) {
			ldapConnectionMaxWaitMilis = DEFAULT_LDAP_CONNECTION_MAX_WAIT_MILIS;
		    }
		} catch (NumberFormatException e) {
		    ldapConnectionMaxWaitMilis = DEFAULT_LDAP_CONNECTION_MAX_WAIT_MILIS;
		}
	    }
	}

	return ldapConnectionMaxWaitMilis;
    }

    public long getLdapConnPoolEvictionInterval() {
	IRegistryAdapter regAdapter = RegistryAdapterFactory.getInstance().getRegistryAdapter();
	long ldapConnectionPoolEvictionInterval = DEFAULT_LDAP_CONNECTION_POOL_EVICTION_INTERVAL;

	try (IRegistryKey rootKey = regAdapter.openRootKey((int) RegKeyAccess.KEY_READ)) {
	    String ldapConnectionPoolEvictionIntervalValue = regAdapter.getStringValue(rootKey, CONFIG_ROOT_KEY,
		    CONFIG_LDAP_CONNECTION_POOL_EVICTION_INTERVAL, true);

	    if (ldapConnectionPoolEvictionIntervalValue != null) {
		try {
		    ldapConnectionPoolEvictionInterval = Long.parseLong(ldapConnectionPoolEvictionIntervalValue);
		    if (ldapConnectionPoolEvictionInterval <= 0)
			ldapConnectionPoolEvictionInterval = DEFAULT_LDAP_CONNECTION_POOL_EVICTION_INTERVAL;
		} catch (NumberFormatException e) {
		    ldapConnectionPoolEvictionInterval = DEFAULT_LDAP_CONNECTION_POOL_EVICTION_INTERVAL;
		}
	    }
	}

	return ldapConnectionPoolEvictionInterval;
    }

    public long getLdapConnPoolIdleTime() {
	IRegistryAdapter regAdapter = RegistryAdapterFactory.getInstance().getRegistryAdapter();
	long ldapConnectionPoolIdleTime = DEFAULT_LDAP_CONNECTION_POOL_IDLE_TIME;

	try (IRegistryKey rootKey = regAdapter.openRootKey((int) RegKeyAccess.KEY_READ)) {
	    String ldapConnectionPoolIdleTimeValue = regAdapter.getStringValue(rootKey, CONFIG_ROOT_KEY,
		    CONFIG_LDAP_CONNECTION_POOL_IDLE_TIME, true);

	    if (ldapConnectionPoolIdleTimeValue != null) {
		try {
		    ldapConnectionPoolIdleTime = Long.parseLong(ldapConnectionPoolIdleTimeValue);
		    if (ldapConnectionPoolIdleTime <= 0)
			ldapConnectionPoolIdleTime = DEFAULT_LDAP_CONNECTION_POOL_IDLE_TIME;
		} catch (NumberFormatException e) {
		    ldapConnectionPoolIdleTime = DEFAULT_LDAP_CONNECTION_POOL_IDLE_TIME;
		}
	    }
	}

	return ldapConnectionPoolIdleTime;
    }

   private IdmServerConfig()
   {
      if (SystemUtils.IS_OS_LINUX)
      {
         CONFIG_DIRECTORY_ROOT_KEY = "Services\\vmdir";
      }
      else
      {
         CONFIG_DIRECTORY_ROOT_KEY =
               "System\\CurrentControlset\\Services\\VMwareDirectoryService";
      }
      CONFIG_DIRECTORY_PARAMETERS_KEY = CONFIG_DIRECTORY_ROOT_KEY + "\\Parameters";

      IRegistryAdapter regAdapter =
            RegistryAdapterFactory.getInstance().getRegistryAdapter();

      IRegistryKey rootKey = regAdapter.openRootKey(
            (int) RegKeyAccess.KEY_READ);

      try
      {
         _configStoreType = getConfigStoreType(regAdapter, rootKey);

         getServiceProviderUsername();
         getServiceProviderConnParams(regAdapter, rootKey);
         loadConfigStoreProperties();
         loadSystemDomainProperties( regAdapter, rootKey );
         loadTenancyConfig(regAdapter, rootKey);
         loadIdmAuthStatsConfig(regAdapter, rootKey);

         ArrayList<Attribute> attributesList = new ArrayList<Attribute>();
         Attribute attr = new Attribute(ATTRIBUTE_GROUPS);
         attr.setFriendlyName(ATTRIBUTE_GROUPS_FRIENDLY_NAME);
         attr.setNameFormat(ATTRNAME_FORMAT_URI);

         attributesList.add(attr);

         attr = new Attribute(ATTRIBUTE_FIRST_NAME);
         attr.setFriendlyName(ATTRIBUTE_FIRST_NAME_FRIENDLY_NAME);
         attr.setNameFormat(ATTRNAME_FORMAT_URI);

         attributesList.add(attr);

         attr = new Attribute(ATTRIBUTE_LAST_NAME);
         attr.setFriendlyName(ATTRIBUTE_LAST_NAME_FRIENDLY_NAME);
         attr.setNameFormat(ATTRNAME_FORMAT_URI);

         attributesList.add(attr);

         attr = new Attribute(ATTRIBUTE_SUBJECT_TYPE);
         attr.setFriendlyName(ATTRIBUTE_SUBJECT_TYPE_FRIENDLY_NAME);
         attr.setNameFormat(ATTRNAME_FORMAT_URI);

         attributesList.add(attr);

         attr = new Attribute(ATTRIBUTE_USER_PRINCIPAL_NAME);
         attr.setFriendlyName(ATTRIBUTE_USER_PRINCIPAL_NAME_FRIENDLY_NAME);
         attr.setNameFormat(ATTRNAME_FORMAT_URI);

         attributesList.add(attr);

         attr = new Attribute(ATTRIBUTE_EMAIL);
         attr.setFriendlyName(ATTRIBUTE_EMAIL_FRIENDLY_NAME);
         attr.setNameFormat(ATTRNAME_FORMAT_URI);

         attributesList.add(attr);

         this._defaultAttributes = Collections.unmodifiableList(attributesList);
      }
      finally
      {
         rootKey.close();
      }
   }

   private
   void
   loadConfigStoreProperties()
   {
      if ( this._configStoreType == ConfigStoreType.VMWARE_DIRECTORY )
      {
         _directoryConfigStoreHost = "localhost";
      }
      else
      {
         _directoryConfigStorePort = 0;
         _directoryConfigStoreHost = null;
         _directoryConfigStoreUserName = null;
      }
   }

   private
   void
   loadSystemDomainProperties(
         IRegistryAdapter regAdapter,
         IRegistryKey     rootKey
         )
   {
      _systemDomainIdentityStoreType =
            IdentityStoreType.IDENTITY_STORE_TYPE_VMWARE_DIRECTORY;

      ArrayList<URI> list = new ArrayList<URI>();

      URI uri = _useSSL?
            DirectoryStoreProtocol.LDAPS.getUri(_directoryConfigStoreHost, _directoryConfigStorePort):
            DirectoryStoreProtocol.LDAP.getUri( _directoryConfigStoreHost, _directoryConfigStorePort);

      if (null != uri)
      {
         list.add(uri);
      }
      else
      {
         throw new IllegalStateException(
               String.format("cannot get valid uri for host:port [%s, %d]",
                     _directoryConfigStoreHost, _directoryConfigStorePort));
      }

      _systemDomainConnectionInfo = Collections.unmodifiableCollection(list);

      _systemDomainSearchTimeout =
            regAdapter.getIntValue(
                  rootKey,
                  CONFIG_ROOT_KEY,
                  CONFIG_SYSTEM_DOMAIN_SEARCH_TIMEOUT,
                  true);

      Collection<String> attributeList =
            regAdapter.getMultiStringValue(
                  rootKey,
                  CONFIG_ROOT_KEY,
                  CONFIG_SYSTEM_DOMAIN_ATTRIBUTES_MAP,
                  false);

      Map<String, String> attrMap = new HashMap<String, String>();
      Map.Entry<String, String> pair = null;

      for(String attr : attributeList)
      {
         if(!ServerUtils.isNullOrEmpty( attr ))
         {
            pair = ServerUtils.getAttributeKeyValueFromString(attr);
            attrMap.put(pair.getKey(), pair.getValue());
         }
      }

      if (!attrMap.isEmpty())
      {
         _systemDomainAttributesMap = Collections.unmodifiableMap(attrMap);
      }

      this._isServiceProviderSystemDomainInBackCompatMode = false;
      this._serviceProviderSystemDomainAlias = null;
      this._serviceProviderSystemDomainUserAliases = new HashMap<String,String>(1);
      Integer val = regAdapter.getIntValue(rootKey, CONFIG_ROOT_KEY, CONFIG_SP_SYSTEM_DOMAIN_BACKCOMPAT_MODE, true);
      if ( val != null )
      {
          this._isServiceProviderSystemDomainInBackCompatMode = (val.intValue() != 0);
      }

      if ( this._isServiceProviderSystemDomainInBackCompatMode )
      {
          this._serviceProviderSystemDomainAlias = regAdapter.getStringValue(
               rootKey, CONFIG_ROOT_KEY, CONFIG_SP_SYSTEM_DOMAIN_ALIAS, true);
          if ( ServerUtils.isNullOrEmpty(this._serviceProviderSystemDomainAlias) == false )
          {
              Collection<String> usersMapList = regAdapter.getMultiStringValue(
                      rootKey,
                      CONFIG_ROOT_KEY,
                      CONFIG_SP_SYSTEM_DOMAIN_USER_ALIASES,
                      true);
              if ( usersMapList != null && usersMapList.size() > 0 )
              {
                  String[] userFromTo = null;

                  for(String mapping : usersMapList)
                  {
                     if(!ServerUtils.isNullOrEmpty( mapping ))
                     {
                         userFromTo = mapping.split(UPN_SEP_STR);
                         if ( ( userFromTo.length == 2 ) &&
                              (ServerUtils.isNullOrEmpty(userFromTo[0]) == false) &&
                              (ServerUtils.isNullOrEmpty(userFromTo[1]) == false) )
                         {
                             this._serviceProviderSystemDomainUserAliases.put( userFromTo[0], userFromTo[1] );
                         }
                     }
                  }
              }
          }
      }
      this._serviceProviderSystemDomainUserAliases = Collections.unmodifiableMap(this._serviceProviderSystemDomainUserAliases);
   }

   private
   void loadTenancyConfig(IRegistryAdapter regAdapter, IRegistryKey regKey)
   {
      int value = regAdapter.getIntValue(
            regKey,
            CONFIG_ROOT_KEY,
            CONFIG_SYSTEM_MULTI_TENANT,
            false);

      _isSingleTenantConfig = (value == 0);
   }

   private
   ConfigStoreType
   getConfigStoreType(IRegistryAdapter regAdapter, IRegistryKey rootKey)
   {
      String storeType = regAdapter.getStringValue(
            rootKey,
            CONFIG_ROOT_KEY,
            CONFIG_STORE_TYPE_KEY,
            false);

      return ConfigStoreType.valueOf(storeType.toUpperCase(Locale.ENGLISH));
   }

   private void getServiceProviderUsername() {
      PasswordCredential creds = VmafClientUtil.getMachineAccountCredentials();
      String domain = getDirectoryConfigStoreDomain();

      if (creds != null) {
          this._directoryConfigStoreUserName = ServerUtils.getUpn(creds.getUserName(), domain);
      }
      this._directoryConfigStoreAuthType = AuthenticationType.SRP;
   }

   private void getServiceProviderConnParams(
         IRegistryAdapter regAdapter, IRegistryKey rootKey)
   {
      String portValueName = _useSSL?
            PlatformUtils.CONFIG_DIRECTORY_LDAPS_PORT_KEY :
            PlatformUtils.CONFIG_DIRECTORY_LDAP_PORT_KEY;

      Integer port = null;
      if (SystemUtils.IS_OS_LINUX)
      {
         port = pollIntValue(
               regAdapter,
               rootKey,
               CONFIG_DIRECTORY_PARAMETERS_KEY,
               portValueName);
      }
      else
      {
         port = regAdapter.getIntValue(
               rootKey,
               CONFIG_DIRECTORY_PARAMETERS_KEY,
               portValueName,
               true);
      }
      // fall back to lotus default port
      if (port == null || port == 0)
      {
          port = LdapConstants.LDAP_PORT_LOTUS;
      }

      _directoryConfigStorePort = _useSSL ? LdapConstants.LDAP_SSL_PORT : port;

   }

   private String pollStringValue(
         IRegistryAdapter regAdapter,
         IRegistryKey rootKey,
         String key,
         String value)
   {
      boolean keepPolling = false;
      do
      {
         keepPolling = false;
         try
         {
            return PlatformUtils.getStringValue(regAdapter, rootKey, key, value);
         }
         catch (RegistryNoSuchKeyOrValueException ex)
         {
            sleep(POLLING_INTERVAL_MILLISECS);
            keepPolling = true;
         }
         catch (IllegalStateException ex)
         {
            sleep(POLLING_INTERVAL_MILLISECS);
            keepPolling = true;
         }
      } while (keepPolling);
      throw new IllegalStateException(String.format(
            "Failed to retrieve registry value [key : %s] [value : %s]", key,
            value));
   }

   private Integer pollIntValue(
         IRegistryAdapter regAdapter,
         IRegistryKey rootKey,
         String key,
         String value)
   {
      boolean keepPolling = false;
      do
      {
         keepPolling = false;
         try
         {
            return PlatformUtils.getIntValue(regAdapter, rootKey, key, value);
         }
         catch (RegistryNoSuchKeyOrValueException ex)
         {
            sleep(POLLING_INTERVAL_MILLISECS);
            keepPolling = true;
         }
         catch (IllegalStateException ex)
         {
            sleep(POLLING_INTERVAL_MILLISECS);
            keepPolling = true;
         }
      } while (keepPolling);
      throw new IllegalStateException(String.format(
            "Failed to retrieve registry value [key : %s] [value : %s]", key,
            value));
   }

   private void sleep(int millisec)
   {
      try
      {
         Thread.sleep(millisec);
      }
      catch (InterruptedException e)
      {//empty
      }
   }

    private void loadIdmAuthStatsConfig(IRegistryAdapter regAdapter,
            IRegistryKey regKey) {
        Integer v = regAdapter.getIntValue(regKey, CONFIG_ROOT_KEY,
                CONFIG_SYSTEM_IDM_AUTH_STATS_CACHE_DEPTH, true);
        this._idmAuthStatsCacheDepth = 10; // default cache depth is 10
        if (v != null && v >= 0 && v < 100) {
            this._idmAuthStatsCacheDepth = v;
        }
        v = regAdapter.getIntValue(regKey, CONFIG_ROOT_KEY,
                CONFIG_SYSTEM_IDM_AUTH_STATS_SUMMARIZE_LDAP_QUERIES, true);
        this._idmAuthStatsSummarizeLdapQueries = true; // default behavior is summarizing ldapQueries
        if (v != null && v == 0) {
            this._idmAuthStatsSummarizeLdapQueries = false;
        }
    }
}
