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

package com.vmware.identity.idm;

import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.ObjectUtils;

/**
 * Created by IntelliJ IDEA.
 * User: mpotapova
 * Date: 1/19/12
 * Time: 7:57 PM
 * To change this template use File | Settings | File Templates.
 */
public class IdentityStoreData implements IIdentityStoreData
{
    private static final long serialVersionUID = 3252243223114983629L;

    public static IdentityStoreData CreateSystemIdentityStoreData( String Name )
    {
        return IdentityStoreData.CreateSystemIdentityStoreData(Name, 0);
    }

    public static IdentityStoreData CreateSystemIdentityStoreData( String Name, int flags )
    {
        // only domainType, and name are used ...
        return new IdentityStoreData(
                DomainType.SYSTEM_DOMAIN,
                Name,
                null,
                IdentityStoreType.IDENTITY_STORE_TYPE_VMWARE_DIRECTORY,
                AuthenticationType.SRP,
                null,
                0,
                false, /* use machine account */
                null,
                null,
                null,
                null,
                null,
                null,
                null,
                null,
                flags,
                null,
                null,
                null, //hintAttributeName
                true // linkingWithUPN
                );
    }

    public
    static
    IdentityStoreData
    createActiveDirectoryIdentityStoreData(
        String name,
        String userName,
        boolean useMachineAccount,
        String servicePrincipalName,
        String password,
        Map<String, String> attributesMap,
        IdentityStoreSchemaMapping schemaMapping,
        int[] authnTypes
    )
    {
        return IdentityStoreData.createActiveDirectoryIdentityStoreData(
            name, userName, useMachineAccount, servicePrincipalName, password,
            attributesMap, schemaMapping, 0, authnTypes
        );
    }

    public
    static
    IdentityStoreData
    createActiveDirectoryIdentityStoreData(
        String name,
        String userName,
        boolean useMachineAccount,
        String servicePrincipalName,
        String password,
        Map<String, String> attributesMap,
        IdentityStoreSchemaMapping schemaMapping,
        int flags,
        int[] authnTypes
    )
    {
        return IdentityStoreData.CreateExternalIdentityStoreData(
                name,
                null, // alias - provider alias name
                IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY,
                AuthenticationType.USE_KERBEROS,
                null, // friendlyName
                300,  // searchTimeoutSeconds is set to 300
                userName,
                useMachineAccount,
                servicePrincipalName,
                password,
                null,
                null,
                Collections.<String> emptyList(),
                attributesMap,
                schemaMapping,
                null,
                flags,
                authnTypes
        );
    }

    public
    static
    IdentityStoreData
    createActiveDirectoryIdentityStoreDataWithPIVControls(
        String name,
        String userName,
        boolean useMachineAccount,
        String servicePrincipalName,
        String password,
        Map<String, String> attributesMap,
        IdentityStoreSchemaMapping schemaMapping,
        int flags,
        int[] authnTypes,
        String hintAttrName,
        boolean linkAccountWithUPN
    )
    {
        return IdentityStoreData.CreateExternalIdentityStoreData(
                name,
                null, // alias - provider alias name
                IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY,
                AuthenticationType.USE_KERBEROS,
                null, // friendlyName
                300,  // searchTimeoutSeconds is set to 300
                userName,
                useMachineAccount,
                servicePrincipalName,
                password,
                null,
                null,
                Collections.<String> emptyList(),
                attributesMap,
                schemaMapping,
                null,
                flags,
                null, //certs
                authnTypes,
                hintAttrName,
                linkAccountWithUPN
        );
    }



    public static IdentityStoreData CreateSystemIdentityStoreDataWithUpnSuffixes( String Name, Set<String> upnSuffixes)
    {
        return IdentityStoreData.CreateSystemIdentityStoreDataWithUpnSuffixes(Name, null, upnSuffixes);
    }

    public static IdentityStoreData CreateSystemIdentityStoreDataWithUpnSuffixes( String Name, String alias, Set<String> upnSuffixes)
    {
        return IdentityStoreData.CreateSystemIdentityStoreDataWithUpnSuffixes(Name, alias, upnSuffixes, 0);
    }

    public static IdentityStoreData CreateSystemIdentityStoreDataWithUpnSuffixes( String Name, String alias, Set<String> upnSuffixes, int flags)
    {
        // only domainType, and name are used ...
        return new IdentityStoreData(
                DomainType.SYSTEM_DOMAIN,
                Name,
                alias,
                IdentityStoreType.IDENTITY_STORE_TYPE_VMWARE_DIRECTORY,
                AuthenticationType.PASSWORD,
                null,
                0,
                false,
                null,
                null,
                null,
                null,
                null,
                null,
                null,
                upnSuffixes,
                flags,
                null,
                null,
                null, //hintAttributeName
                true // linkingWithUPN
                );
    }

    public static
    IdentityStoreData CreateLocalOSIdentityStoreData( String Name )
    {
        // only domainType, and name are used ...
        return IdentityStoreData.CreateLocalOSIdentityStoreData(Name, null);
    }

    public static
    IdentityStoreData CreateLocalOSIdentityStoreData( String Name, String alias )
    {
        return IdentityStoreData.CreateLocalOSIdentityStoreData(Name, alias, 0);
    }

    public static
    IdentityStoreData CreateLocalOSIdentityStoreData( String Name, String alias, int flags )
    {
        // only domainType, and name are used ...
        return new IdentityStoreData(
                DomainType.LOCAL_OS_DOMAIN,
                Name,
                alias,
                IdentityStoreType.IDENTITY_STORE_TYPE_LOCAL_OS,
                AuthenticationType.PASSWORD,
                null,
                0,
                false,
                null,
                null,
                null,
                null,
                null,
                null,
                null,
                null,
                flags,
                null,
                null,
                null,
                true);
    }

    public
    static
    IdentityStoreData
    CreateExternalIdentityStoreData(
        String Name,
        String alias,
        IdentityStoreType  type,
        AuthenticationType authenticationType,
        String friendlyName,
        int searchTimeoutSeconds,
        String userName,
        String password,
        String userBaseDN,
        String groupBaseDN,
        Collection<String>  connectionStrings,
        Map<String, String> attributesMap
    )
    {
        return IdentityStoreData.CreateExternalIdentityStoreData(
            Name,
            alias,
            type,
            authenticationType,
            friendlyName,
            searchTimeoutSeconds,
            userName,
            password,
            userBaseDN,
            groupBaseDN,
            connectionStrings,
            attributesMap,
            null,
            null
        );
    }

    public
    static
    IdentityStoreData
    CreateExternalIdentityStoreData(
        String Name,
        String alias,
        IdentityStoreType  type,
        AuthenticationType authenticationType,
        String friendlyName,
        int searchTimeoutSeconds,
        String userName,
        String password,
        String userBaseDN,
        String groupBaseDN,
        Collection<String>  connectionStrings,
        Map<String, String> attributesMap,
        IdentityStoreSchemaMapping schemaMapping
    )
    {
        return IdentityStoreData.CreateExternalIdentityStoreData(
            Name,
            alias,
            type,
            authenticationType,
            friendlyName,
            searchTimeoutSeconds,
            userName,
            password,
            userBaseDN,
            groupBaseDN,
            connectionStrings,
            attributesMap,
            schemaMapping,
            null
        );
    }

    public
    static
    IdentityStoreData
    CreateExternalIdentityStoreData(
        String Name,
        String alias,
        IdentityStoreType  type,
        AuthenticationType authenticationType,
        String friendlyName,
        int searchTimeoutSeconds,
        String userName,
        String password,
        String userBaseDN,
        String groupBaseDN,
        Collection<String>  connectionStrings,
        Map<String, String> attributesMap,
        IdentityStoreSchemaMapping schemaMapping,
        Set<String> upnSuffixes
    )
    {
        return IdentityStoreData.CreateExternalIdentityStoreData(
            Name,
            alias,
            type,
            authenticationType,
            friendlyName,
            searchTimeoutSeconds,
            userName,
            password,
            userBaseDN,
            groupBaseDN,
            connectionStrings,
            attributesMap,
            schemaMapping,
            upnSuffixes,
            0);
    }

    public
    static
    IdentityStoreData
    CreateExternalIdentityStoreData(
        String Name,
        String alias,
        IdentityStoreType  type,
        AuthenticationType authenticationType,
        String friendlyName,
        int searchTimeoutSeconds,
        String userName,
        String password,
        String userBaseDN,
        String groupBaseDN,
        Collection<String>  connectionStrings,
        Map<String, String> attributesMap,
        IdentityStoreSchemaMapping schemaMapping,
        Set<String> upnSuffixes,
        int flags
    )
    {
        return new IdentityStoreData(
                    DomainType.EXTERNAL_DOMAIN,
                    Name,
                    alias,
                    type,
                    authenticationType,
                    friendlyName,
                    searchTimeoutSeconds,
                    false,
                    userName,
                    password,
                    userBaseDN,
                    groupBaseDN,
                    connectionStrings,
                    attributesMap,
                    schemaMapping,
                    upnSuffixes,
                    flags,
                    null,
                    null,
                    null, //hintAttributeName
                    true // linkingWithUPN
                    );
    }

    public
    static
    IdentityStoreData
    CreateExternalIdentityStoreData(
        String Name,
        String alias,
        IdentityStoreType  type,
        AuthenticationType authenticationType,
        String friendlyName,
        int searchTimeoutSeconds,
        String userName,
        boolean useMachineAccount,
        String servicePrincipalName,
        String password,
        String userBaseDN,
        String groupBaseDN,
        Collection<String>  connectionStrings,
        Map<String, String> attributesMap,
        int[] authnTypes
    )
    {
        return IdentityStoreData.CreateExternalIdentityStoreData(
            Name,
            alias,
            type,
            authenticationType,
            friendlyName,
            searchTimeoutSeconds,
            userName,
            useMachineAccount,
            servicePrincipalName,
            password,
            userBaseDN,
            groupBaseDN,
            connectionStrings,
            attributesMap,
            null,
            authnTypes
        );
    }

    public
    static
    IdentityStoreData
    CreateExternalIdentityStoreData(
        String Name,
        String alias,
        IdentityStoreType  type,
        AuthenticationType authenticationType,
        String friendlyName,
        int searchTimeoutSeconds,
        String userName,
        boolean useMachineAccount,
        String servicePrincipalName,
        String password,
        String userBaseDN,
        String groupBaseDN,
        Collection<String>  connectionStrings,
        Map<String, String> attributesMap,
        IdentityStoreSchemaMapping schemaMapping,
        int[] authnTypes
    )
    {
        return IdentityStoreData.CreateExternalIdentityStoreData(
            Name,
            alias,
            type,
            authenticationType,
            friendlyName,
            searchTimeoutSeconds,
            userName,
            useMachineAccount,
            servicePrincipalName,
            password,
            userBaseDN,
            groupBaseDN,
            connectionStrings,
            attributesMap,
            schemaMapping,
            0,
            authnTypes);
    }

    public
    static
    IdentityStoreData
    CreateExternalIdentityStoreData(
        String Name,
        String alias,
        IdentityStoreType  type,
        AuthenticationType authenticationType,
        String friendlyName,
        int searchTimeoutSeconds,
        String userName,
        boolean useMachineAccount,
        String servicePrincipalName,
        String password,
        String userBaseDN,
        String groupBaseDN,
        Collection<String>  connectionStrings,
        Map<String, String> attributesMap,
        IdentityStoreSchemaMapping schemaMapping,
        int flags,
        int[] authnTypes
    )
    {
        return new IdentityStoreData(
                    DomainType.EXTERNAL_DOMAIN,
                    Name,
                    alias,
                    type,
                    authenticationType,
                    friendlyName,
                    searchTimeoutSeconds,
                    useMachineAccount,
                    userName,
                    servicePrincipalName,
                    password,
                    userBaseDN,
                    groupBaseDN,
                    connectionStrings,
                    attributesMap,
                    schemaMapping,
                    null,
                    flags,
                    null,
      authnTypes, null, true);
    }

    public
    static
    IdentityStoreData
    CreateExternalIdentityStoreData(
        String Name,
        String alias,
        IdentityStoreType  type,
        AuthenticationType authenticationType,
        String friendlyName,
        int searchTimeoutSeconds,
        String userName,
        boolean useMachineAccount,
        String servicePrincipalName,
        String password,
        String userBaseDN,
        String groupBaseDN,
        Collection<String>  connectionStrings,
        Map<String, String> attributesMap,
        IdentityStoreSchemaMapping schemaMapping,
        Set<String> upnSuffixes,
        int[] authnTypes
    )
    {
        return IdentityStoreData.CreateExternalIdentityStoreData(
            Name,
            alias,
            type,
            authenticationType,
            friendlyName,
            searchTimeoutSeconds,
            userName,
            useMachineAccount,
            servicePrincipalName,
            password,
            userBaseDN,
            groupBaseDN,
            connectionStrings,
            attributesMap,
            schemaMapping,
            upnSuffixes,
            0,
            authnTypes);
    }
    public
    static
    IdentityStoreData
    CreateExternalIdentityStoreData(
        String Name,
        String alias,
        IdentityStoreType  type,
        AuthenticationType authenticationType,
        String friendlyName,
        int searchTimeoutSeconds,
        String userName,
        boolean useMachineAccount,
        String servicePrincipalName,
        String password,
        String userBaseDN,
        String groupBaseDN,
        Collection<String>  connectionStrings,
        Map<String, String> attributesMap,
        IdentityStoreSchemaMapping schemaMapping,
        Set<String> upnSuffixes,
        Collection<X509Certificate> certificates,
        int[] authnTypes
    )
    {
	return new IdentityStoreData(
                 DomainType.EXTERNAL_DOMAIN,
                 Name,
                 alias,
                 type,
                 authenticationType,
                 friendlyName,
                 searchTimeoutSeconds,
                 useMachineAccount,
                 userName,
                 servicePrincipalName,
                 password,
                 userBaseDN,
                 groupBaseDN,
                 connectionStrings,
                 attributesMap,
                 schemaMapping,
                 upnSuffixes,
                 0,
                 certificates,
      authnTypes, null, true);
    }
    public
    static
    IdentityStoreData
    CreateExternalIdentityStoreData(
        String Name,
        String alias,
        IdentityStoreType  type,
        AuthenticationType authenticationType,
        String friendlyName,
        int searchTimeoutSeconds,
        String userName,
        boolean useMachineAccount,
        String servicePrincipalName,
        String password,
        String userBaseDN,
        String groupBaseDN,
        Collection<String>  connectionStrings,
        Map<String, String> attributesMap,
        IdentityStoreSchemaMapping schemaMapping,
        Set<String> upnSuffixes,
        int flags,
        int[] authnTypes
    )
    {
        return new IdentityStoreData(
                    DomainType.EXTERNAL_DOMAIN,
                    Name,
                    alias,
                    type,
                    authenticationType,
                    friendlyName,
                    searchTimeoutSeconds,
                    useMachineAccount,
                    userName,
                    servicePrincipalName,
                    password,
                    userBaseDN,
                    groupBaseDN,
                    connectionStrings,
                    attributesMap,
                    schemaMapping,
                    upnSuffixes,
                    flags,
                    null,
      authnTypes, null, true);
    }

    public
    static
    IdentityStoreData
    CreateExternalIdentityStoreData(
        String Name,
        String alias,
        IdentityStoreType  type,
        AuthenticationType authenticationType,
        String friendlyName,
        int searchTimeoutSeconds,
        String userName,
        boolean useMachineAccount,
        String servicePrincipalName,
        String password,
        String userBaseDN,
        String groupBaseDN,
        Collection<String>  connectionStrings,
        Map<String, String> attributesMap,
        IdentityStoreSchemaMapping schemaMapping,
        Set<String> upnSuffixes,
        int flags,
        Collection<X509Certificate> certificates,
        int[] authnTypes
    )
    {
        return new IdentityStoreData(
                    DomainType.EXTERNAL_DOMAIN,
                    Name,
                    alias,
                    type,
                    authenticationType,
                    friendlyName,
                    searchTimeoutSeconds,
                    useMachineAccount,
                    userName,
                    servicePrincipalName,
                    password,
                    userBaseDN,
                    groupBaseDN,
                    connectionStrings,
                    attributesMap,
                    schemaMapping,
                    upnSuffixes,
                    flags,
                    certificates,
        authnTypes,
        null, true);
    }

    public
    static
    IdentityStoreData
    CreateExternalIdentityStoreData(
        String Name,
        String alias,
        IdentityStoreType  type,
        AuthenticationType authenticationType,
        String friendlyName,
        int searchTimeoutSeconds,
        String userName,
        boolean useMachineAccount,
        String servicePrincipalName,
        String password,
        String userBaseDN,
        String groupBaseDN,
        Collection<String>  connectionStrings,
        Map<String, String> attributesMap,
        IdentityStoreSchemaMapping schemaMapping,
        Set<String> upnSuffixes,
        int flags,
        Collection<X509Certificate> certificates,
        int[] authnTypes,
        String hintAttributeName,
        boolean accountLinkUseUPN

    )
    {
        return new IdentityStoreData(
                    DomainType.EXTERNAL_DOMAIN,
                    Name,
                    alias,
                    type,
                    authenticationType,
                    friendlyName,
                    searchTimeoutSeconds,
                    useMachineAccount,
                    userName,
                    servicePrincipalName,
                    password,
                    userBaseDN,
                    groupBaseDN,
                    connectionStrings,
                    attributesMap,
                    schemaMapping,
                    upnSuffixes,
                    flags,
                    certificates,
                    authnTypes,
                    hintAttributeName,
                    accountLinkUseUPN);
    }

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
        return this._extendedData;
    }

    @Override
    public int hashCode()
    {
        final int prime = 31;
        int result = 1;
        result = prime * result + _name.hashCode();
        result = prime * result + ((_domainType == null) ? 0 : _domainType.hashCode());
        result = prime * result + ((_extendedData == null) ? 0 : _extendedData.hashCode());
        return result;
    }

    @Override
    public boolean equals(Object obj)
    {
        if (this == obj)
        {
            return true;
        }
        if (obj == null || this.getClass() != obj.getClass())
        {
           return false;
        }

       IdentityStoreData other = (IdentityStoreData) obj;
       return _domainType == other._domainType
           && _name.equals(other._name)
           && ObjectUtils.equals(_extendedData, other._extendedData);
    }

    private
    IdentityStoreData(
        DomainType domainType,
        String     name,
        String     alias,
        IdentityStoreType  type,
        AuthenticationType authenticationType,
        String friendlyName,
        int    searchTimeoutSeconds,
        boolean useMachineAccount,
        String userPrincipal,
        String password,
        String userBaseDN,
        String groupBaseDN,
        Collection<String>  connectionStrings,
        Map<String, String> attributesMap,
        IdentityStoreSchemaMapping schemaMapping,
        Set<String> upnSuffixes,
        int flags,
        Collection<X509Certificate> certificates,
        int[] authnTypes,
        String hintAttributeName,
        boolean linkingWithUPN
    )
    {
        this(domainType,
             name,
             alias,
             type,
             authenticationType,
             friendlyName,
             searchTimeoutSeconds,
             useMachineAccount,
             userPrincipal,
             null,
             password,
             userBaseDN,
             groupBaseDN,
             connectionStrings,
             attributesMap,
             schemaMapping,
             upnSuffixes,
             flags,
             certificates,
             authnTypes,
             hintAttributeName,
             linkingWithUPN
             );
    }

    private
    IdentityStoreData(
        DomainType domainType,
        String     name,
        String     alias,
        IdentityStoreType  type,
        AuthenticationType authenticationType,
        String friendlyName,
        int    searchTimeoutSeconds,
        boolean useMachineAccount,
        String userPrincipal,
        String servicePrincipalName,
        String password,
        String userBaseDN,
        String groupBaseDN,
        Collection<String>  connectionStrings,
        Map<String, String> attributesMap,
        IdentityStoreSchemaMapping schemaMapping,
        Set<String> upnSuffixes,
        int flags,
        Collection<X509Certificate> certificates, int[] authnTypes,
        String certMappingHintAttributeName,
        boolean certMappingUseUPN
    )
    {
        ValidateUtil.validateIdsDomainName(name, domainType);

        if ( (alias != null) && (alias.isEmpty() == true ) )
        {
            alias = null;
        }

        this._domainType = domainType;
        this._name = name;

        if(this._domainType == DomainType.EXTERNAL_DOMAIN)
        {
            // If not registering native AD, validate userBaseDN, groupBaseDN, connection string
            if (!(type == IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY &&
                authenticationType == AuthenticationType.USE_KERBEROS))
            {
                ValidateUtil.validateDNFormat(userBaseDN);
                ValidateUtil.validateDNFormat(groupBaseDN);
                ValidateUtil.validateNotNull(connectionStrings, "connectionStrings");
                if (connectionStrings.size() < 1)
                {
                    throw new IllegalArgumentException(
                     "At least 1 connection string info must be provided.");
                }
            }

            if ( ( type == IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING )
                &&
                ( authenticationType != AuthenticationType.PASSWORD )
              )
            {
                throw new IllegalArgumentException(
                   String.format(
                       "Authentication type [%s] is not supported for Identity store type [%s].",
                       authenticationType.name(), type.name()
                   )
               );
            }

            String normUserPrincipal = null;
            // if not using machine account, normalize userPrincipalName and validate password
            if (!useMachineAccount)
            {
                normUserPrincipal = ValidateUtil.validateIdsUserName(userPrincipal, type, authenticationType);
                ValidateUtil.validateNotNull(password, "password");
            }

            this._extendedData = new IdentityStoreDataEx(
                alias, type, authenticationType, friendlyName, searchTimeoutSeconds, normUserPrincipal, useMachineAccount, servicePrincipalName,
                    password, userBaseDN, groupBaseDN, connectionStrings, attributesMap, schemaMapping, upnSuffixes, flags, certificates, authnTypes,
                    certMappingHintAttributeName, certMappingUseUPN);
        }
        else if (this._domainType == DomainType.SYSTEM_DOMAIN)
        {//only return the upnSuffixes for system domain
           this._extendedData =
               new IdentityStoreDataEx(
                       alias, /* alias */
                       null, /* type  */
                       null, /* authentication type */
                       null, /* friendly name */
                       -1,   /* search timeout */
                       null, /* user name */
                       useMachineAccount, /* use machine account */
                       null, /* service principal name */
                       null, /* password */
                       null, /* user base dn */
                       null, /* group base dn */
                       Collections.<String> emptyList(), /* connection strings */
                       null, /* attributes map */
                       null, /* schema map */
                       upnSuffixes,
                       flags,
                       certificates,
                            authnTypes,
                            certMappingHintAttributeName, certMappingUseUPN);
        }
        else // local os
        {
            if ( ( alias != null ) && (alias.length() > 0 ) )
            {
                this._extendedData =
                    new IdentityStoreDataEx(
                        alias, /* alias */
                        null, /* type  */
                        null, /* authentication type */
                        null, /* friendly name */
                        -1,   /* search timeout */
                        null, /* user name */
                        false, /* use machine account */
                        null, /* service principal name */
                        null, /* password */
                        null, /* user base dn */
                        null, /* group base dn */
                        Collections.<String> emptyList(), /* connection strings */
                        null, /* attributes map */
                        null, /* schema map */
                        null,
                        flags,
                        null,
                        authnTypes,
                        certMappingHintAttributeName,
                        certMappingUseUPN);
            }
            else
            {
                this._extendedData = null;
            }
        }
    }

    private final DomainType _domainType;
    private final String _name;
    private IIdentityStoreDataEx _extendedData;

    private class IdentityStoreDataEx implements IIdentityStoreDataEx
    {
        private static final long serialVersionUID = 3195296862504173541L;
        private final String _alias;
        private final IdentityStoreType _type;
        private final AuthenticationType _authenticationType;
        private final String _friendlyName;
        private final int _searchTimeoutSeconds;
        private final String _userName;
        private final boolean _useMachineAccount;
        private final String _servicePrincipalName;
        private String _password;
        private final String _userBaseDN;
        private final String _groupBaseDN;
        private final Collection<String> _connectionStrings;
        private Map<String, String> _attributesMap;
        private final IdentityStoreSchemaMapping _schemaMapping;
        private Set<String> _upnSuffixes;
        private final int _flags;
        private final Collection<X509Certificate> _certificates;
        private final int[] _authnTypes;
        private final String _hintAttributeName;
        private final boolean _accountLinkingUseUPN;

        public
        IdentityStoreDataEx(
            String alias,
            IdentityStoreType type,
            AuthenticationType authenticationType,
            String friendlyName,
            int searchTimeoutSeconds,
            String userName,
            boolean useMachineAccount, /* optional */
            String servicePrincipalName, /*optional*/
            String password,
            String userBaseDN,
            String groupBaseDN,
            Collection<String> connectionStrings,
            Map<String, String> attributesMap,
            IdentityStoreSchemaMapping schemaMapping,
            Set<String> upnSuffixes,
            int flags,
            Collection<X509Certificate> certificates,
            int[] authnTypes,
                String hintAttributeName,
                boolean linkingUseUPN
            )
        {

            this._alias = alias;
            this._type = type;
            this._authenticationType = authenticationType;
            this._friendlyName = friendlyName;
            this._searchTimeoutSeconds = searchTimeoutSeconds;
            this._userName = userName;
            this._useMachineAccount = useMachineAccount;
            this._servicePrincipalName = servicePrincipalName;
            this._password = password;
            this._userBaseDN = userBaseDN;
            this._groupBaseDN = groupBaseDN;
            this._connectionStrings =
            Collections.unmodifiableCollection(
                            new ArrayList<String>(connectionStrings));
            this._attributesMap = null;
            if ((attributesMap != null) && (attributesMap.size() > 0) )
            {
                this._attributesMap =
                Collections.unmodifiableMap(
                        new HashMap<String, String>(attributesMap));
            }

            this._schemaMapping = schemaMapping;
            this._upnSuffixes = null;
            if ( ( upnSuffixes != null ) && (upnSuffixes.isEmpty() == false) )
            {
                this._upnSuffixes = new HashSet<String>(upnSuffixes.size());
                for(String suffix : upnSuffixes)
                {
                    this._upnSuffixes.add(ValidateUtil.getCanonicalUpnSuffix(suffix.toUpperCase()));
                }
                this._upnSuffixes = Collections.<String>unmodifiableSet(this._upnSuffixes);
            }
            this._flags = flags;
            this._certificates = certificates;
            this._authnTypes = authnTypes;
            this._hintAttributeName = hintAttributeName;
            this._accountLinkingUseUPN = linkingUseUPN;
        }

        @Override
        public int hashCode()
        {
            final int prime = 31;
            int result = 1;
            result = prime * result + ObjectUtils.hashCode(_alias);
            result = prime * result + ObjectUtils.hashCode(_attributesMap);
            result = prime * result + ObjectUtils.hashCode(_authenticationType);
            result = prime * result + ObjectUtils.hashCode(new HashSet<String>(_connectionStrings));
            result = prime * result + ObjectUtils.hashCode(_friendlyName);
            result = prime * result + ObjectUtils.hashCode(_groupBaseDN);
            result = prime * result + ObjectUtils.hashCode(_password);
            result = prime * result + _searchTimeoutSeconds;
            result = prime * result + ObjectUtils.hashCode(_type);
            result = prime * result + ObjectUtils.hashCode(_userBaseDN);
            result = prime * result + ObjectUtils.hashCode(_userName);
            result = prime * result + ObjectUtils.hashCode(_schemaMapping);
            result = prime * result + ObjectUtils.hashCode(_upnSuffixes == null ?
                  null : _upnSuffixes);
            result = prime * result + _flags;
            result = prime * result + ObjectUtils.hashCode(_certificates);
            return result;
        }

        @Override
        public boolean equals(Object obj)
        {
            if (this == obj) {
                return true;
            }
            if (obj == null || this.getClass() != obj.getClass()) {
                return false;
            }

            IdentityStoreDataEx other = (IdentityStoreDataEx) obj;
            return ObjectUtils.equals(_alias, other._alias)
                && ObjectUtils.equals(_attributesMap, other._attributesMap)
                && _authenticationType == other._authenticationType
                && _connectionStrings.size() == other._connectionStrings.size()
                && _connectionStrings.containsAll(other._connectionStrings)
                && ObjectUtils.equals(_friendlyName, other._friendlyName)
                && ObjectUtils.equals(_groupBaseDN, other._groupBaseDN)
                && ObjectUtils.equals(_password, other._password)
                && _searchTimeoutSeconds == other._searchTimeoutSeconds
                && _type == other._type
                && ObjectUtils.equals(_userBaseDN, other._userBaseDN)
                && ObjectUtils.equals(_userName, other._userName)
                && ObjectUtils.equals(_schemaMapping, other._schemaMapping)
                && ObjectUtils.equals(_upnSuffixes, other._upnSuffixes)
                && _flags == other._flags
                && ((_certificates == null && other._certificates == null)
                        || (_certificates.size() == other._certificates.size() && _certificates.containsAll(other._certificates)));
        }

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
        public boolean useMachineAccount()
        {
            return this._useMachineAccount;
        }

        @Override
        public String getServicePrincipalName()
        {
            return this._servicePrincipalName;
        }

        @Override
        public String getPassword()
        {
            return this._password;
        }

        // setPassword is used to set
        // (1) Encrypted password before writing to directory
        // (2) Decrypted password after reading it from directory
        @Override
        public void setPassword(String password)
        {
            this._password = password;
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
        public Collection<String> getConnectionStrings()
        {
            return this._connectionStrings;
        }

        @Override
        public int getSearchTimeoutSeconds()
        {
            return this._searchTimeoutSeconds;
        }

        @Override
        public Map<String, String> getAttributeMap()
        {
            return this._attributesMap;
        }

        @Override
        public IdentityStoreSchemaMapping getIdentityStoreSchemaMapping() {
            return this._schemaMapping;
        }

        @Override
        public Set<String> getUpnSuffixes() {
           return this._upnSuffixes;
        }

        @Override
        public int getFlags() {
            return this._flags;
        }

        @Override
        public Collection<X509Certificate> getCertificates()
        {
            return this._certificates;
        }

        @Override
        public int[] getAuthnTypes()
        {
            return this._authnTypes;
        }

        @Override
        public boolean getCertLinkingUseUPN() {
          return this._accountLinkingUseUPN;
        }

        @Override
        public String getCertUserHintAttributeName() {
          return this._hintAttributeName;
        }
    }
}
