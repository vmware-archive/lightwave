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

import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IIdentityStoreDataEx;
import com.vmware.identity.idm.IdentityStoreData;
import com.vmware.identity.idm.IdentityStoreSchemaMapping;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.ValidateUtil;

/*
 *   The class should be internal to com.vmware.identity.idm.server;
 *   It should never be exposed to outside of the server.
 */
public class ServerIdentityStoreData implements IIdentityStoreData, IIdentityStoreDataEx
{
    /**
     *
     */
    private static final long serialVersionUID = 7593904989372676913L;
    private final DomainType _domainType;
    private final String _name;
    private String _alias;
    private IdentityStoreType _type;
    private AuthenticationType _authenticationType;
    private String _friendlyName;
    private int _searchTimeoutSeconds;
    private String _userName;
    private boolean _useMachineAccount;
    private String _servicePrincipalName;
    private String _password;
    private String _userBaseDN;
    private String _groupBaseDN;
    private Collection<String> _connectionStrings;
    private Map<String, String> _attributesMap;
    private IdentityStoreSchemaMapping _schemaMapping;
    private Set<String> _upnSuffixes;
    private int _flags;
    private Collection<X509Certificate> _certificates;
    private int[] _authnTypes;
    private String _hintAttributeName;
    private boolean _accountLinkingUseUPN = true;

    public ServerIdentityStoreData( DomainType domainType, String name )
    {
        ValidateUtil.validateNotEmpty(name, "name");
        ValidateUtil.validateNotNull(domainType, "domainType");

        this._domainType = domainType;
        this._name = name;
        this._type = IdentityStoreType.IDENTITY_STORE_TYPE_VMWARE_DIRECTORY;
        this._alias = null;
        this._authenticationType = AuthenticationType.PASSWORD;
        this._friendlyName = null;
        this._searchTimeoutSeconds = 0;
        this._userName = null;
        this._password = null;
        this._userBaseDN = null;
        this._groupBaseDN = null;
        this._connectionStrings = null;
        this._attributesMap = null;
        this._schemaMapping = null;
        this._upnSuffixes = null;
        this._flags = 0;
        this._certificates = null;
        this._authnTypes = null;
        this._hintAttributeName = null;
        this._accountLinkingUseUPN = true;
    }

    /*
    * IIdentityStoreData
    */
    @Override
    public String getName()
    {
        return this._name;
    }

    @Override
    public DomainType getDomainType()
    {
        return this._domainType;
    }

    @Override
    public IIdentityStoreDataEx getExtendedIdentityStoreData()
    {
        return this;
    }

    /*
    * IIdentityStoreDataEx
    */
    @Override
    public IdentityStoreType getProviderType()
    {
        return this._type;
    }

    @Override
    public String getAlias()
    {
        return this._alias;
    }

    @Override
    public AuthenticationType getAuthenticationType()
    {
        return this._authenticationType;
    }

    @Override
    public String getUserName()
    {
        return this._userName;
    }

    @Override
    public int[] getAuthnTypes()
    {
        return this._authnTypes;
    }

    @Override
    public String getServicePrincipalName()
    {
        return this._servicePrincipalName;
    }

    @Override
    public boolean useMachineAccount()
    {
        return this._useMachineAccount;
    }

    @Override
    public String getPassword()
    {
        return this._password;
    }

    @Override
    public String getFriendlyName()
    {
        return this._friendlyName;
    }

    @Override
    public String getUserBaseDn()
    {
        return this._userBaseDN;
    }

    @Override
    public String getGroupBaseDn()
    {
        return this._groupBaseDN;
    }

    @Override
    public Collection<String> getConnectionStrings() {
        return this._connectionStrings;
    }

    @Override
    public Set<String> getUpnSuffixes() {
       return this._upnSuffixes;
    }
    @Override
    public int getSearchTimeoutSeconds()
    {
        return this._searchTimeoutSeconds;
    }

    @Override
    public Map<String, String> getAttributeMap() {
        return this._attributesMap;
    }

    @Override
    public IdentityStoreSchemaMapping getIdentityStoreSchemaMapping() {
        return this._schemaMapping;
    }

    @Override
    public int getFlags()
    {
        return this._flags;
    }

    @Override
    public Collection<X509Certificate> getCertificates()
    {
        return this._certificates;
    }

    @Override
    public boolean getCertLinkingUseUPN() {
      return _accountLinkingUseUPN;
    }

    @Override
    public String getCertUserHintAttributeName() {
      return _hintAttributeName;
    }

    /*
    * public methods
    */

    public void setAuthnTypes(int[] authnTypes){
        this._authnTypes = authnTypes;
    }

    public void setProviderType(IdentityStoreType type)
    {
        ValidateUtil.validateNotNull(type, "type");
        this._type = type;
    }

    public void setAlias(String alias)
    {
        this._alias = alias;
    }

    public void setAuthenticationType(AuthenticationType authenticationType)
    {
        this._authenticationType = authenticationType;
    }

    public void setUserName(String userName)
    {
        this._userName = userName;
    }

    public void setServicePrincipalName(String servicePrincipalName)
    {
        this._servicePrincipalName = servicePrincipalName;
    }

    public void setUseMachineAccount(boolean val)
    {
        this._useMachineAccount = val;
    }

    @Override
   public void setPassword(String password)
    {
        this._password = password;
    }

    public void setFriendlyName(String friendlyName)
    {
        this._friendlyName = friendlyName;
    }

    public void setUserBaseDn(String userBaseDn)
    {
        this._userBaseDN = userBaseDn;
    }

    public void setGroupBaseDn(String groupBaseDn)
    {
        this._groupBaseDN = groupBaseDn;
    }

    public void setSearchTimeoutSeconds(int searchTimeoutSeconds)
    {
        ValidateUtil.validateNonNegativeNumber(searchTimeoutSeconds, "searchTimeoutSeconds");
        this._searchTimeoutSeconds = searchTimeoutSeconds;
    }

   public void setConnectionStrings(Collection<String> connectionStrings) {
        if(this._domainType == DomainType.LOCAL_OS_DOMAIN)
        {
            ValidateUtil.validateNull( connectionStrings, "connectionStrings" );
            this._connectionStrings = null;
        }
        else
        {
            ValidateUtil.validateNotNull( connectionStrings, "connectionStrings" );
            if(connectionStrings.size() < 1)
            {
                throw new IllegalArgumentException("At least 1 connection strimng must be provided.");
            }

            this._connectionStrings = Collections.unmodifiableCollection(new ArrayList<String>(connectionStrings));
        }
    }

   public void setUpnSuffixes(Collection<String> upnSuffixes)
   {
      this._upnSuffixes = null;
      if ( ( upnSuffixes != null ) && (upnSuffixes.isEmpty() == false) )
      {
          this._upnSuffixes = new HashSet<String>(upnSuffixes.size());
          for( String suffix : upnSuffixes )
          {
              this._upnSuffixes.add(ValidateUtil.getCanonicalUpnSuffix(suffix));
          }
          this._upnSuffixes = Collections.<String>unmodifiableSet(this._upnSuffixes);
      }
   }

   public void setAttributeMap(Map<String, String> attrMap)
    {
        this._attributesMap = null;
        if (( attrMap != null) && (attrMap.size() > 0) )
        {
            this._attributesMap = Collections.unmodifiableMap(new HashMap<String, String>(attrMap));
        }
    }

    public void setSchemaMapping(IdentityStoreSchemaMapping schemaMapping)
    {
        this._schemaMapping = schemaMapping;
    }

    public void setFlags(int flags)
    {
        this._flags = flags;
    }

    public void setCertificates(Collection<X509Certificate> certificates)
    {
        this._certificates = certificates;
    }

    public IdentityStoreData getExternalIdentityStoreData()
    {
        IdentityStoreData storeData = null;

        if( this.getDomainType() == DomainType.LOCAL_OS_DOMAIN )
        {
            storeData = IdentityStoreData.CreateLocalOSIdentityStoreData( this.getName(), this.getAlias() );
        }
        else if (this.getDomainType() == DomainType.SYSTEM_DOMAIN )
        {
            storeData = IdentityStoreData.CreateSystemIdentityStoreDataWithUpnSuffixes(
                  this.getName(),
                  this.getAlias(),
                  (this.getUpnSuffixes() != null? Collections.unmodifiableSet((this.getUpnSuffixes())):null));
        }
        else if (this.getDomainType() == DomainType.EXTERNAL_DOMAIN)
        {
            storeData = IdentityStoreData.CreateExternalIdentityStoreData(
                    this.getName(), this.getAlias(), this.getProviderType(),
                    this.getAuthenticationType(), this.getFriendlyName(),
                    this.getSearchTimeoutSeconds(), this.getUserName(),
                    this.useMachineAccount(), this.getServicePrincipalName(),
                    this.getPassword()!= null? this.getPassword() : "",
                    this.getUserBaseDn(), this.getGroupBaseDn(), this.getConnectionStrings(),
                    this.getAttributeMap(), this.getIdentityStoreSchemaMapping(),
                    (this.getUpnSuffixes() != null? Collections.unmodifiableSet((this.getUpnSuffixes())):null),
                    this.getFlags(), this.getCertificates(), this.getAuthnTypes(),
                    this.getCertUserHintAttributeName(),this.getCertLinkingUseUPN()
            );
        }
        else
        {
            throw new RuntimeException(
                    String.format(
                            "unsupported domain type: '%s'",
                            this.getDomainType().toString()
                    )
            );
        }

        return storeData;
    }

    public void setHintAttributeName(String _hintAttributeName) {
      this._hintAttributeName = _hintAttributeName;
    }

    public void setAccountLinkingUseUPN(boolean _accountLinkingUseUPN) {
      this._accountLinkingUseUPN = _accountLinkingUseUPN;
    }

}