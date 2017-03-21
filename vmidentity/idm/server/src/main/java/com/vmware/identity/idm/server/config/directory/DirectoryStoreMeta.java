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

package com.vmware.identity.idm.server.config.directory;

import java.net.MalformedURLException;
import java.net.URL;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Stack;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.AlternativeOCSP;
import com.vmware.identity.idm.AlternativeOCSPList;
import com.vmware.identity.idm.AssertionConsumerService;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeConsumerService;
import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.CertificateType;
import com.vmware.identity.idm.ClientCertPolicy;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IIdentityStoreDataEx;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.PasswordExpiration;
import com.vmware.identity.idm.RSAAMInstanceInfo;
import com.vmware.identity.idm.RSAAgentConfig;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.ServiceEndpoint;
import com.vmware.identity.idm.SignatureAlgorithm;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.IdentityManager;
import com.vmware.identity.idm.server.IdmCertificate;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.config.IConfigStore;
import com.vmware.identity.idm.server.config.ServerIdentityStoreData;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.ILdapEntry;
import com.vmware.identity.interop.ldap.ILdapMessage;
import com.vmware.identity.interop.ldap.LdapFilterString;
import com.vmware.identity.interop.ldap.LdapMod;
import com.vmware.identity.interop.ldap.LdapScope;
import com.vmware.identity.interop.ldap.LdapValue;

interface ILdapObject<T>
{
    String getCn( T object );
    String lookupObject(ILdapConnectionEx connection, String baseDn,
            LdapScope scope, String objectCn, String additionalFilter);
    String getDnFromCn(String parentDn, String objectCn);
    String getDnFromObject(String parentDn, T object);

    String getInSetSearchFilter(String propertyName, String[] values );

    void createObject( ILdapConnectionEx connection, String objectDn, T object );
    void deleteObject( ILdapConnectionEx connection, String objectDn );

    List<T> searchObjects( ILdapConnectionEx connection, String baseDn, LdapScope scope );
    List<T> searchObjects(
        ILdapConnectionEx connection, String baseDn, LdapScope scope, String additionalFilter
    );

    // returns Dn
    String lookupObject(
        ILdapConnectionEx connection, String baseDn, LdapScope scope, String objectCn);
    Set<String> lookupObjects(
            ILdapConnectionEx connection, String baseDn, LdapScope scope, String objectCn, String objectClass);

    T retrieveObject(
        ILdapConnectionEx connection, String baseDn, LdapScope scope, String objectCn );
    Set<T> retrieveObjects(ILdapConnectionEx connection, String baseDn, LdapScope scope);

    void updateObject(
        ILdapConnectionEx connection, String baseDn, LdapScope scope, String objectCn, T object );
    void updateObject(
        ILdapConnectionEx connection, String objectDn, T object );

    LdapValue[] getObjectProperty(
        ILdapConnectionEx connection, String baseDn, LdapScope scope, String objectCn, String propertyName );

    void setObjectPropertyValue(
        ILdapConnectionEx connection, String baseDn,
        LdapScope scope, String objectCn,
        String propertyName, LdapValue[] value );
    void setObjectPropertyValue(
            ILdapConnectionEx connection, String objectDn,
            String propertyName, LdapValue[] value );
}

interface IPropertyGetterSetterBase<T, V>
{
    void SetLdapValue(V object, LdapValue[] value);
    LdapValue[] GetLdapValue(T object);
}

interface IPropertyGetterSetter<T> extends IPropertyGetterSetterBase<T, T>
{
}

// PropertyMapperMetaInfoBase<T, V> to accommodate two types
// T is the real object type used in the getter
// V is an intermediate representation, usually as the T.Builder type which is used in the setter
// Use PropertyMapperMetaInfo<T> if you want the getter and setter based on the same type T.
class PropertyMapperMetaInfoBase<T, V>
{
    private String _attributeName;
    private int _ctorOrder;
    private boolean _isSettableOnObject;
    private IPropertyGetterSetterBase<T, V> _propertyGetterSetter;
    private boolean _isUpdateableOnLdapServer;

    public PropertyMapperMetaInfoBase(
            String attributeName, int ctorOrder,
            boolean isSettableOnObject, IPropertyGetterSetterBase<T, V> propertyGetterSetter
    )
    {
        this( attributeName, ctorOrder, isSettableOnObject, propertyGetterSetter, isSettableOnObject );
    }

    public PropertyMapperMetaInfoBase(
            String attributeName, int ctorOrder,
            boolean isSettableOnObject, IPropertyGetterSetterBase<T, V> propertyGetterSetter,
            boolean isUpdateableOnLdapServer
    )
    {
        if ( (isSettableOnObject == true) || (ctorOrder >= 0 ) )
        {
            ValidateUtil.validateNotNull( propertyGetterSetter, "propertyGeterSetter" );
        }
        if ( ( isUpdateableOnLdapServer == true ) && ( isSettableOnObject == false ) )
        {
            throw new IllegalArgumentException("Attribute marked isUpdateableOnLdapServer must also be marked isSettableOnObject");
        }

        this._attributeName = attributeName;
        this._ctorOrder = ctorOrder;
        this._isSettableOnObject = isSettableOnObject;
        this._propertyGetterSetter = propertyGetterSetter;
        this._isUpdateableOnLdapServer = isUpdateableOnLdapServer;
    }

    public String getAttributeName() { return this._attributeName; }
    public int getCtorOrder() { return this._ctorOrder; }
    public boolean getSettableOnObject() { return this._isSettableOnObject; }
    public IPropertyGetterSetterBase<T, V> getPropertyGetterSetter() { return this._propertyGetterSetter; }
    public boolean getUpdateableOnLdapServer() { return this._isUpdateableOnLdapServer; }
}

class PropertyMapperMetaInfo<T> extends PropertyMapperMetaInfoBase<T, T> {

    public PropertyMapperMetaInfo(
            String attributeName, int ctorOrder,
            boolean isSettableOnObject, IPropertyGetterSetter<T> propertyGetterSetter
    )
    {
        this( attributeName, ctorOrder, isSettableOnObject, propertyGetterSetter, isSettableOnObject );
    }

    public PropertyMapperMetaInfo(
            String attributeName, int ctorOrder,
            boolean isSettableOnObject, IPropertyGetterSetter<T> propertyGetterSetter,
            boolean isUpdateableOnLdapServer
    )
    {
        super(attributeName, ctorOrder, isSettableOnObject, propertyGetterSetter, isUpdateableOnLdapServer);
    }
}

abstract class BaseLdapObjectBase<T, V> implements ILdapObject<T>
{
    private String _objectClass;
    private Map<String, IPropertyGetterSetterBase<T, V>> _attributesPropertyMapper;
    private String[] _ctorAttributes;
    private String[] _nonCtorSettableAttributes;
    private String[] _allSettableAttributes;
    // these are ldap attributes which can't be updated on ldap object on ldap server
    // a simplest example of the attribute which is not updateable is 'CN'
    private String[] _ldapUpdateableAttributes;

    protected static final String CN = "cn";

    protected BaseLdapObjectBase(String objectClass, PropertyMapperMetaInfoBase<T, V>[] propertiesMap)
    {
        this._objectClass = objectClass;
        this._attributesPropertyMapper = new HashMap<String, IPropertyGetterSetterBase<T, V>>(propertiesMap.length);

        ArrayList<String> nonCtorAttrib = new ArrayList<String>();
        ArrayList<String> allAttribs = new ArrayList<String>();
        ArrayList<PropertyMapperMetaInfoBase<T, V>> ctorAttribs = new ArrayList<PropertyMapperMetaInfoBase<T, V>>();
        ArrayList<String> ldapUpdateableAttribs = new ArrayList<String>();

        for(PropertyMapperMetaInfoBase<T, V> metaInfo : propertiesMap)
        {
            this._attributesPropertyMapper.put( metaInfo.getAttributeName(), metaInfo.getPropertyGetterSetter() );
            if(metaInfo.getCtorOrder() >=0 )
            {
                ctorAttribs.add(metaInfo);
            }
            if(metaInfo.getSettableOnObject() == true)
            {
                allAttribs.add(metaInfo.getAttributeName());
                if(metaInfo.getCtorOrder() < 0)
                {
                    nonCtorAttrib.add(metaInfo.getAttributeName());
                }
                if ( metaInfo.getUpdateableOnLdapServer() == true )
                {
                    ldapUpdateableAttribs.add(metaInfo.getAttributeName());
                }
            }
        }

        Collections.sort( ctorAttribs, new Comparator<PropertyMapperMetaInfoBase<T, V>>() {
            @Override
            public int compare(PropertyMapperMetaInfoBase<T, V> o1, PropertyMapperMetaInfoBase<T, V> o2)
            {
                if(o1 == o2)
                {
                    return 0;
                }
                else if(o1 == null)
                {
                    return -1;
                }
                else if(o2 == null)
                {
                    return 1;
                }
                else
                {
                    return o1.getCtorOrder() - o2.getCtorOrder();
                }
            }
        });

        this._ctorAttributes = new String[ctorAttribs.size()];
        int i = 0;
        for( PropertyMapperMetaInfoBase<T, V> meta : ctorAttribs )
        {
            this._ctorAttributes[i] = meta.getAttributeName();
            i++;
        }
        this._nonCtorSettableAttributes = new String[nonCtorAttrib.size()];
        this._nonCtorSettableAttributes = nonCtorAttrib.toArray(this._nonCtorSettableAttributes);
        this._allSettableAttributes = new String[ allAttribs.size() ];
        this._allSettableAttributes = allAttribs.toArray(this._allSettableAttributes);
        this._ldapUpdateableAttributes = new String[ ldapUpdateableAttribs.size() ];
        this._ldapUpdateableAttributes = ldapUpdateableAttribs.toArray(this._ldapUpdateableAttributes);

        if(this._attributesPropertyMapper.containsKey(CN) == false)
        {
            throw new IllegalArgumentException("cn attribute must be mapped!");
        }
    }

    @Override
    public String getCn(T object)
    {
        IPropertyGetterSetterBase<T, V> props = this._attributesPropertyMapper.get(CN);
        LdapValue[] value = props.GetLdapValue(object);

        return ServerUtils.getStringValue(value);
    }

    @Override
    public String getDnFromCn(String parentDn, String objectCn)
    {
        ValidateUtil.validateNotEmpty( parentDn, "parentDn" );
        ValidateUtil.validateNotEmpty( objectCn, "objectCn" );

        return String.format( "%s=%s,%s", CN, objectCn, parentDn );
    }

    @Override
    public String getDnFromObject(String parentDn, T object)
    {
        ValidateUtil.validateNotEmpty( parentDn, "parentDn" );
        ValidateUtil.validateNotNull(object, "object");

        return String.format( "%s=%s,%s", CN, this.getCn(object), parentDn );
    }

    @Override
    public String getInSetSearchFilter(String propertyName, String[] values )
    {
        ValidateUtil.validateNotEmpty( propertyName, "propertyName" );
        if(this._attributesPropertyMapper.containsKey( propertyName ) == false )
        {
            throw new IllegalArgumentException(
                String.format( "Unknown property '%s'.", propertyName )
            );
        }

        String filter = null;
        if ( ( values != null ) && (values.length > 0) )
        {
            filter = String.format( "(%s=%s)", propertyName /*attribute name*/, LdapFilterString.encode(values[0]));
            for(int i = 1; i < values.length; i++ )
            {
                filter = String.format( "(|%s(%s=%s))", filter, propertyName /*attribute name*/, LdapFilterString.encode(values[i]) );
            }
        }

        return filter;
    }

    @Override
    public void createObject( ILdapConnectionEx connection, String objectDn, T object )
    {
        ValidateUtil.validateNotNull( connection, "connection" );
        ValidateUtil.validateNotEmpty( objectDn, "objectDn" );
        ValidateUtil.validateNotNull( object, "object" );

        ArrayList<LdapMod> mods = new ArrayList<LdapMod>();

        // objectclass
        mods.add(
            new LdapMod(
                LdapMod.LdapModOperation.ADD,
                "objectClass",
                ServerUtils.getLdapValue( this._objectClass )
            )
        );

        IPropertyGetterSetterBase<T, V> props = null;
        LdapValue[] value = null;
        for(String attribute : this._allSettableAttributes)
        {
            props = this._attributesPropertyMapper.get(attribute);
            value = props.GetLdapValue( object );

            if ( value != null && value.length > 0)
            {
                mods.add(
                        new LdapMod( LdapMod.LdapModOperation.ADD, attribute, value )
                );
            }
        }

        connection.addObject( objectDn, mods );
    }

    @Override
    public void updateObject( ILdapConnectionEx connection, String objectDn, T object )
    {
        ValidateUtil.validateNotNull(connection, "connection");
        ValidateUtil.validateNotNull( objectDn, "objectDn" );
        ValidateUtil.validateNotNull( object, "object" );

        ArrayList<LdapMod> mods = new ArrayList<LdapMod>();
        IPropertyGetterSetterBase<T, V> props = null;
        LdapValue[] value = null;
        for(String attribute : this._ldapUpdateableAttributes)
        {
            props = this._attributesPropertyMapper.get(attribute);
            ValidateUtil.validateNotNull( props, "Invalid attributesPropertyMapper." );
            value = props.GetLdapValue( object );

            if ( value != null && value.length > 0)
            {
                mods.add(
                        new LdapMod( LdapMod.LdapModOperation.REPLACE, attribute, value )
                );
            }
            else
            {
                mods.add(
                        new LdapMod( LdapMod.LdapModOperation.DELETE, attribute, (LdapValue[])null )
                );
            }
        }

        connection.modifyObject( objectDn, mods );

    }

    @Override
    public void updateObject( ILdapConnectionEx connection, String baseDn, LdapScope scope, String objectCn, T object )
    {
        ValidateUtil.validateNotNull( connection, "connection" );
        ValidateUtil.validateNotEmpty( baseDn, "baseDn" );
        ValidateUtil.validateNotNull( object, "object" );

        String objectDn = this.lookupObject( connection, baseDn, scope, objectCn );
        if( ServerUtils.isNullOrEmpty(objectDn) == true )
        {
            throw new IllegalArgumentException(
                    String.format( "Object '%s' does not exist.", objectCn )
            );
        }

        this.updateObject(connection, objectDn, object);
    }

    @Override
    public void deleteObject( ILdapConnectionEx connection, String objectDn )
    {
        ValidateUtil.validateNotNull(connection, "connection");
        ValidateUtil.validateNotEmpty(objectDn, "objectDn");

        Stack<String> objects = new Stack<String>();
        HashSet<String> objectsWithChildrenProcessed = new HashSet<String>();
        objects.push(objectDn);
        String currentObject = null;

        while (objects.empty() == false)
        {
            currentObject = objects.pop();
            if( objectsWithChildrenProcessed.contains( currentObject ) )
            {
                connection.deleteObject( currentObject );
                objectsWithChildrenProcessed.remove(currentObject);
            }
            else
            {
                objects.push(currentObject);
                ILdapMessage message = connection.search(currentObject, LdapScope.SCOPE_ONE_LEVEL, "(objectClass=*)", new String[]{CN}, false);
                try
                {
                    ILdapEntry[] entries = message.getEntries();
                    if (( entries != null ) && (entries.length > 0) )
                    {
                        for(ILdapEntry entry : entries)
                        {
                            objects.push(entry.getDN());
                        }
                    }
                }
                finally
                {
                    message.close();
                }
                objectsWithChildrenProcessed.add(currentObject);
            }
        }
    }

    @Override
    public List<T> searchObjects(ILdapConnectionEx connection, String baseDn, LdapScope scope)
    {
        return this.searchObjects( connection, baseDn, scope, null );
    }

    @Override
    public List<T> searchObjects( ILdapConnectionEx connection, String baseDn, LdapScope scope, String additionalFilter )
    {
        ValidateUtil.validateNotNull(connection, "connection");
        ValidateUtil.validateNotEmpty( baseDn, "baseDn");

        ArrayList<T> objects = null;
        String filter = null;

        if( ServerUtils.isNullOrEmpty( additionalFilter ) == true )
        {
            filter = String.format("(objectClass=%s)", LdapFilterString.encode(this._objectClass) );
        }
        else
        {
            filter = String.format("(&(objectClass=%s)%s)", LdapFilterString.encode(this._objectClass), additionalFilter );
        }

        ILdapMessage message = connection.search( baseDn, scope, filter, this._allSettableAttributes, false );

        try
        {
            ILdapEntry[] entries = message.getEntries();
            if ( (entries != null) && (entries.length > 0) )
            {
                objects = new ArrayList<T>();

                for(ILdapEntry entry : entries)
                {
                    objects.add( this.createObject(entry));
                }
            }
        }
        finally
        {
            message.close();
        }

        return objects;
    }

    @Override
    public String lookupObject(ILdapConnectionEx connection, String baseDn, LdapScope scope, String objectCn)
    {
        return lookupObject(connection, baseDn, scope, objectCn, null);
    }

    @Override
    public String lookupObject(ILdapConnectionEx connection, String baseDn, LdapScope scope, String objectCn, String additionalFilter)
    {
        ValidateUtil.validateNotNull( connection, "connection");
        ValidateUtil.validateNotEmpty( baseDn, "baseDn");

        String objectDn = null;
        String searchFilter = null;

        if (ServerUtils.isNullOrEmpty(additionalFilter) == false)
        {
            searchFilter = (ServerUtils.isNullOrEmpty(objectCn) == false)
                ?
                String.format(
                    "(&(objectClass=%s)(cn=%s)%s)",
                    LdapFilterString.encode(this._objectClass),
                    LdapFilterString.encode(objectCn),
                    additionalFilter
                )
                :
                String.format(
                    "(&(objectClass=%s)%s)",
                    LdapFilterString.encode(this._objectClass),
                    additionalFilter
                );
        }
        else
        {
            searchFilter = (ServerUtils.isNullOrEmpty(objectCn) == false)
                    ?
                    String.format(
                        "(&(objectClass=%s)(cn=%s))",
                        LdapFilterString.encode(this._objectClass),
                        LdapFilterString.encode(objectCn)
                    )
                    :
                    String.format(
                        "(objectClass=%s)",
                        LdapFilterString.encode(this._objectClass)
                    );
        }

        ILdapMessage message = connection.search(
                baseDn, scope, searchFilter, new String[]{"cn"}, false );

        try
        {
            ILdapEntry[] entries = message.getEntries();
            if ( entries != null )
            {
                if( entries.length > 1)
                {
                    // TODO: logging
                    throw new IllegalStateException(
                            String.format(
                                    "Invalid lookup search: object '%s' under '%s' is not unique.",
                                    objectCn, baseDn
                            )
                    );
                }
                else if( entries.length == 1 )
                {
                    objectDn = entries[0].getDN();
                }
            }
        }
        finally
        {
            message.close();
        }

        return objectDn;
    }

    @Override
    public Set<String> lookupObjects(ILdapConnectionEx connection, String baseDn, LdapScope scope, String objectCn, String objectClass)
    {
        ValidateUtil.validateNotNull( connection, "connection");
        ValidateUtil.validateNotEmpty( baseDn, "baseDn");
        Set<String> Dns = new HashSet<String>();

        final String searchFilter =
            (ServerUtils.isNullOrEmpty(objectCn) == false)
            ?
            String.format(
                "(&(objectClass=%s)(cn=%s))",
                LdapFilterString.encode(objectClass),
                LdapFilterString.encode(objectCn)
            )
            :
            String.format(
                "(objectClass=%s)",
                LdapFilterString.encode(objectClass)
            );

        ILdapMessage message = connection.search(
                baseDn, scope, searchFilter, new String[]{"cn"}, false );

        try
        {
            ILdapEntry[] entries = message.getEntries();
            if ( entries != null && entries.length != 0)
            {
                for (ILdapEntry entry : entries)
                {
                    Dns.add(entry.getDN());
                }
            }
        }
        finally
        {
            message.close();
        }

        return Dns;
    }

    @Override
    public T retrieveObject(ILdapConnectionEx connection, String baseDn, LdapScope scope, String objectCn)
    {
        ValidateUtil.validateNotNull( connection, "connection");
        ValidateUtil.validateNotEmpty( baseDn, "baseDn");

        final String searchFilter =
                (ServerUtils.isNullOrEmpty(objectCn) == false)
                ?
                String.format(
                    "(&(objectClass=%s)(cn=%s))",
                    LdapFilterString.encode(this._objectClass),
                    LdapFilterString.encode(objectCn)
                )
                :
                String.format(
                    "(objectClass=%s)",
                    LdapFilterString.encode(this._objectClass)
                );

        T object = null;
        ILdapMessage message = connection.search(
                baseDn,
                scope,
                searchFilter,
                this._allSettableAttributes,
                false
        );

        try
        {
            ILdapEntry[] entries = message.getEntries();
            if (entries != null)
            {
                if(entries.length == 1)
                {
                    object = this.createObject(entries[0]);
                }
                else if(entries.length > 1)
                {
                    // todo: logging
                    throw new IllegalStateException(
                            String.format( "Invalid config state: Non-unique object '%s' under '%s'", objectCn, baseDn)
                    );
                }
            }
        }
        finally
        {
            message.close();
        }

        return object;
    }

    @Override
    public Set<T> retrieveObjects(ILdapConnectionEx connection, String baseDn, LdapScope scope)
    {
        ValidateUtil.validateNotNull( connection, "connection");
        ValidateUtil.validateNotEmpty( baseDn, "baseDn");

        final String searchFilter =
            String.format( "(objectClass=%s)", LdapFilterString.encode(this._objectClass));

        Set<T> objects = new HashSet<T>();
        ILdapMessage message = connection.search(
                baseDn,
                scope,
                searchFilter,
                this._allSettableAttributes,
                false
        );

        try
        {
            ILdapEntry[] entries = message.getEntries();
            if (entries != null && entries.length != 0)
            {
                for (ILdapEntry entry : entries)
                {
                    objects.add(this.createObject(entry));
                }
            }
        }
        finally
        {
            message.close();
        }

        return objects;
    }

    @Override
    public LdapValue[] getObjectProperty(ILdapConnectionEx connection, String baseDn, LdapScope scope, String objectCn, String propertyName)
    {
        ValidateUtil.validateNotNull( connection, "connection");
        ValidateUtil.validateNotEmpty( baseDn, "baseDn");
        ValidateUtil.validateNotEmpty( propertyName, "propertyName");

        if(this._attributesPropertyMapper.containsKey( propertyName ) == false )
        {
            throw new IllegalArgumentException(
                    String.format( "Unknown property '%s'.", propertyName )
            );
        }

        LdapValue[] propertyValue = null;

        final String searchFilter =
                (ServerUtils.isNullOrEmpty(objectCn) == false)
                ?
                String.format(
                    "(&(objectClass=%s)(cn=%s))",
                    LdapFilterString.encode(this._objectClass),
                    LdapFilterString.encode(objectCn)
                )
                :
                String.format(
                    "(objectClass=%s)",
                    LdapFilterString.encode(this._objectClass)
                );

        ILdapMessage message = connection.search(
                baseDn,
                scope,
                searchFilter,
                new String[] { propertyName },
                false
        );

        try
        {
            ILdapEntry[] entries = message.getEntries();
            if (entries != null)
            {
                if(entries.length == 1)
                {
                    HashSet<String> availableAttributes = getAvailableAttributes( entries[0] );
                    if( (availableAttributes != null ) && (availableAttributes.contains( propertyName ) ) )
                    {
                        propertyValue = entries[0].getAttributeValues(propertyName);
                    }
                }
                else if(entries.length > 1)
                {
                    // todo: logging
                    throw new IllegalStateException(
                            String.format( "Invalid config state: Non-unique object '%s' under '%s'", objectCn, baseDn)
                    );
                }
            }
        }
        finally
        {
            message.close();
        }

        return propertyValue;
    }

    @Override
    public void setObjectPropertyValue(
            ILdapConnectionEx connection, String objectDn,
            String propertyName, LdapValue[] value )
    {
        ValidateUtil.validateNotNull(connection, "connection");
        ValidateUtil.validateNotEmpty( objectDn, "objectDn");
        ValidateUtil.validateNotEmpty( propertyName, "propertyName");

        if(this._attributesPropertyMapper.containsKey( propertyName ) == false )
        {
            throw new IllegalArgumentException(
                    String.format( "Unknown property '%s'.", propertyName )
            );
        }

        if ( ( value != null ) && (value.length == 0) )
        {
            value = null;
        }
        connection.modifyObject(
                objectDn,
                new LdapMod(
                        (value != null) ? LdapMod.LdapModOperation.REPLACE : LdapMod.LdapModOperation.DELETE,
                        propertyName,
                        value
                )
        );
    }

    @Override
    public void setObjectPropertyValue(
            ILdapConnectionEx connection, String baseDn, LdapScope scope, String objectCn,
            String propertyName, LdapValue[] value )
    {
        ValidateUtil.validateNotNull( connection, "connection");
        ValidateUtil.validateNotEmpty( baseDn, "baseDn");

        String dn = this.lookupObject( connection, baseDn, scope, objectCn );
        if(ServerUtils.isNullOrEmpty(dn))
        {
            throw new IllegalArgumentException("Object does not exist.");
        }
        this.setObjectPropertyValue(connection, dn, propertyName, value);
    }

    protected abstract V createObject(List<LdapValue[]> ctorParams);
    protected abstract T createFinalObject(V object);

    private static HashSet<String> getAvailableAttributes( ILdapEntry entry )
    {
        HashSet<String> attributes = null;
        if( entry != null )
        {
            String[] returnedAttributes = entry.getAttributeNames();
            if ( (returnedAttributes!= null) && ( returnedAttributes.length > 0) )
            {
                attributes = new HashSet<String>(returnedAttributes.length);
                for(String attribute : returnedAttributes)
                {
                    attributes.add( attribute );
                }
            }
        }

        return attributes;
    }

    private T createObject(ILdapEntry entry)
    {
        V intermediateObject = null;

        HashSet<String> availableAttributes = getAvailableAttributes(entry);

        ArrayList<LdapValue[]> ctorArgs = new ArrayList<LdapValue[]>(this._ctorAttributes.length);
        for( int i = 0; i < this._ctorAttributes.length; i++)
        {
            LdapValue[] value = null;
            if ( (availableAttributes != null) && (availableAttributes.contains( this._ctorAttributes[i] )) )
            {
                value = entry.getAttributeValues(this._ctorAttributes[i]);
            }
            ctorArgs.add(value);
        }
        intermediateObject = this.createObject(ctorArgs);

        for( String str : this._nonCtorSettableAttributes)
        {
            IPropertyGetterSetterBase<T, V> props = this._attributesPropertyMapper.get(str);
            LdapValue[] value = null;
            if ( (availableAttributes != null) && (availableAttributes.contains( str )) )
            {
                value = entry.getAttributeValues(str);
            }

            props.SetLdapValue( intermediateObject, value );
        }

        return this.createFinalObject(intermediateObject);
    }
}

abstract class BaseLdapObject<T> extends BaseLdapObjectBase<T, T> {

    protected BaseLdapObject(String objectClass, PropertyMapperMetaInfo<T>[] propertiesMap) {
        super(objectClass, propertiesMap);
    }

    @Override
    protected T createFinalObject(T object) {
        return object;
    };
}

final class TenantAttributesLdapObject extends BaseLdapObject<TenantAttributes>
{
    private static TenantAttributesLdapObject _instance = new TenantAttributesLdapObject();
    public static TenantAttributesLdapObject getInstance() { return _instance; }

    public static final String PROPERTY_NAME = TenantLdapObject.PROPERTY_NAME;
    public static final String PROPERTY_GUID = TenantLdapObject.PROPERTY_GUID;
    public static final String PROPERTY_LONG_NAME = TenantLdapObject.PROPERTY_LONG_NAME;
    public static final String PROPERTY_ENTITY_ID = TenantLdapObject.PROPERTY_ENTITY_ID;
    public static final String PROPERTY_CLOCK_TOLERANCE = TenantLdapObject.PROPERTY_CLOCK_TOLERANCE;
    public static final String PROPERTY_SIGNATURE_ALGORITHM = TenantLdapObject.PROPERTY_SIGNATURE_ALGORITHM;
    public static final String PROPERTY_ISSUER_NAME = TenantLdapObject.PROPERTY_ISSUER_NAME;
    public static final String PROPERTY_DELEGATION_COUNT = TenantLdapObject.PROPERTY_DELEGATION_COUNT;
    public static final String PROPERTY_RENEW_COUNT = TenantLdapObject.PROPERTY_RENEW_COUNT;
    public static final String PROPERTY_MAX_BEARTOKEN_LIFETIME = TenantLdapObject.PROPERTY_MAX_BEARTOKEN_LIFETIME;
    public static final String PROPERTY_MAX_HOKTOKEN_LIFETIME = TenantLdapObject.PROPERTY_MAX_HOKTOKEN_LIFETIME;
    public static final String PROPERTY_DEFAULT_PROVIDER = TenantLdapObject.PROPERTY_DEFAULT_PROVIDER;
    public static final String PROPERTY_BRAND_NAME = TenantLdapObject.PROPERTY_BRAND_NAME;
    public static final String PROPERTY_LOGON_BANNER_TITLE = TenantLdapObject.PROPERTY_LOGON_BANNER_TITLE;
    public static final String PROPERTY_LOGON_BANNER_CONTENT = TenantLdapObject.PROPERTY_LOGON_BANNER_CONTENT;
    public static final String PROPERTY_LOGON_BANNER_ENABLE_CHECKBOX = TenantLdapObject.PROPERTY_LOGON_BANNER_ENABLE_CHECKBOX;
    public static final String PROPERTY_AUTHN_TYPES = "vmwSTSAuthnTypes";
    public static final String PROPERTY_MAX_BEARER_REFRESH_TOKEN_LIFETIME = TenantLdapObject.PROPERTY_MAX_BEARER_REFRESH_TOKEN_LIFETIME;
    public static final String PROPERTY_MAX_HOK_REFRESH_TOKEN_LIFETIME = TenantLdapObject.PROPERTY_MAX_HOK_REFRESH_TOKEN_LIFETIME;
    public static final String PROPERTY_ENABLE_IDP_SELECTION = TenantLdapObject.PROPERTY_ENABLE_IDP_SELECTION;
    public static final String PROPERTY_ALIAS = TenantLdapObject.PROPERTY_ALIAS;

    private static final String OBJECT_CLASS = "vmwSTSTenant";

    @SuppressWarnings("unchecked")
    private TenantAttributesLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<TenantAttributes>(
                                PROPERTY_NAME,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<TenantAttributes>(
                                PROPERTY_CLOCK_TOLERANCE,
                                0,
                                true,
                                new IPropertyGetterSetter<TenantAttributes>() {
                                    @Override
                                    public void SetLdapValue(TenantAttributes object, LdapValue[] value) {
                                        throw new IllegalStateException("property is not settable.");
                                    }

                                    @Override
                                    public LdapValue[] GetLdapValue(TenantAttributes object) {
                                        ValidateUtil.validateNotNull( object, "object" );
                                       return ServerUtils.getLdapValue( object.getTokenPolicy().getMaxTokenClockTolerance());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<TenantAttributes>(
                                PROPERTY_DELEGATION_COUNT,
                                1,
                                true,
                                new IPropertyGetterSetter<TenantAttributes>() {
                                    @Override
                                    public void SetLdapValue(TenantAttributes object, LdapValue[] value) {
                                        throw new IllegalStateException("property is not settable.");
                                    }

                                    @Override
                                    public LdapValue[] GetLdapValue(TenantAttributes object) {
                                        ValidateUtil.validateNotNull( object, "object" );
                                       return ServerUtils.getLdapValue( object.getTokenPolicy().getMaxTokenDelegationCount());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<TenantAttributes>(
                                PROPERTY_RENEW_COUNT,
                                2,
                                true,
                                new IPropertyGetterSetter<TenantAttributes>() {
                                    @Override
                                    public void SetLdapValue(TenantAttributes object, LdapValue[] value) {
                                        throw new IllegalStateException("property is not settable.");
                                    }

                                    @Override
                                    public LdapValue[] GetLdapValue(TenantAttributes object) {
                                        ValidateUtil.validateNotNull( object, "object" );
                                       return ServerUtils.getLdapValue( object.getTokenPolicy().getMaxTokenRenewCount());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<TenantAttributes>(
                                PROPERTY_MAX_BEARTOKEN_LIFETIME,
                                3,
                                true,
                                new IPropertyGetterSetter<TenantAttributes>() {
                                    @Override
                                    public void SetLdapValue(TenantAttributes object, LdapValue[] value) {
                                        throw new IllegalStateException("property is not settable.");
                                    }

                                    @Override
                                    public LdapValue[] GetLdapValue(TenantAttributes object) {
                                        ValidateUtil.validateNotNull( object, "object" );
                                       return ServerUtils.getLdapValue( object.getTokenPolicy().getMaxBearerTokenLifetime());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<TenantAttributes>(
                               PROPERTY_SIGNATURE_ALGORITHM,
                               4,
                               true,
                               new IPropertyGetterSetter<TenantAttributes>() {
                                   @Override
                                   public void SetLdapValue(TenantAttributes object, LdapValue[] value) {
                                       throw new IllegalStateException("property is not settable.");
                                   }

                                   @Override
                                   public LdapValue[] GetLdapValue(TenantAttributes object) {
                                       ValidateUtil.validateNotNull( object, "object" );
                                      return ServerUtils.getLdapValue( object.getSignatureAlgorithm());
                                   }
                               }
                        ),
                        new PropertyMapperMetaInfo<TenantAttributes>(
                                PROPERTY_BRAND_NAME,
                                5,
                                true,
                                new IPropertyGetterSetter<TenantAttributes>() {
                                    @Override
                                    public void SetLdapValue(TenantAttributes object, LdapValue[] value) {
                                        throw new IllegalStateException("property is not settable.");
                                    }

                                    @Override
                                    public LdapValue[] GetLdapValue(TenantAttributes object) {
                                        ValidateUtil.validateNotNull( object, "object" );
                                       return ServerUtils.getLdapValue( object.getBrandName() );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<TenantAttributes>(
                                PROPERTY_MAX_HOKTOKEN_LIFETIME,
                                6,
                                true,
                                new IPropertyGetterSetter<TenantAttributes>() {
                                    @Override
                                    public void SetLdapValue(TenantAttributes object, LdapValue[] value) {
                                        throw new IllegalStateException("property is not settable.");
                                    }

                                    @Override
                                    public LdapValue[] GetLdapValue(TenantAttributes object) {
                                        ValidateUtil.validateNotNull( object, "object" );
                                       return ServerUtils.getLdapValue( object.getTokenPolicy().getMaxHOKLifetime() );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<TenantAttributes>(
                            PROPERTY_ENTITY_ID,
                            7,
                            true,
                            new IPropertyGetterSetter<TenantAttributes>()
                            {
                                @Override
                                public void SetLdapValue(TenantAttributes object, LdapValue[] value)
                                {
                                    throw new IllegalStateException("property is not settable.");
                                }

                                @Override
                                public LdapValue[] GetLdapValue(TenantAttributes object)
                                {
                                     ValidateUtil.validateNotNull( object, "object" );
                                     return ServerUtils.getLdapValue( object.getEntityId() );
                                }
                            }
                        ),
                        new PropertyMapperMetaInfo<TenantAttributes>(
                                PROPERTY_LOGON_BANNER_TITLE,
                                8,
                                true,
                                new IPropertyGetterSetter<TenantAttributes>() {
                                    @Override
                                    public void SetLdapValue(TenantAttributes object, LdapValue[] value) {
                                        throw new IllegalStateException("property is not settable.");
                                    }

                                    @Override
                                    public LdapValue[] GetLdapValue(TenantAttributes object) {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        return ServerUtils.getLdapValue( object.getLogonBannerTitle() );
                                    }
                                }
                            ),
                            new PropertyMapperMetaInfo<TenantAttributes>(
                                    PROPERTY_LOGON_BANNER_CONTENT,
                                    9,
                                    true,
                                    new IPropertyGetterSetter<TenantAttributes>() {
                                        @Override
                                        public void SetLdapValue(TenantAttributes object, LdapValue[] value)
                                        {
                                            throw new IllegalStateException("property is not settable.");
                                        }
                                        @Override
                                        public LdapValue[] GetLdapValue(TenantAttributes object)
                                        {
                                            return ServerUtils.getLdapValue(object.getLogonBannerContent());
                                        }
                                    }
                            ),
                            new PropertyMapperMetaInfo<TenantAttributes>(
                                    PROPERTY_LOGON_BANNER_ENABLE_CHECKBOX,
                                    10,
                                    true,
                                    new IPropertyGetterSetter<TenantAttributes>() {
                                        @Override
                                        public void SetLdapValue(TenantAttributes object, LdapValue[] value)
                                        {
                                            throw new IllegalStateException("property is not settable.");
                                        }
                                        @Override
                                        public LdapValue[] GetLdapValue(TenantAttributes object)
                                        {
                                            return ServerUtils.getLdapValue(object.getLogonBannerCheckboxFlag());
                                        }
                                    }
                            ),
                            new PropertyMapperMetaInfo<TenantAttributes>(
                                    PROPERTY_AUTHN_TYPES,
                                    11,
                                    true,
                                    new IPropertyGetterSetter<TenantAttributes>() {
                                        @Override
                                        public void SetLdapValue(TenantAttributes object, LdapValue[] value) {
                                            throw new IllegalStateException("property is not settable.");
                                        }
                                        @Override
                                        public LdapValue[] GetLdapValue(TenantAttributes object) {
                                            ValidateUtil.validateNotNull( object, "object" );
                                            return ServerUtils.getLdapValue(object.getAuthnTypes());
                                        }
                                    }
                            ),
                            new PropertyMapperMetaInfo<TenantAttributes>(
                                    PROPERTY_MAX_BEARER_REFRESH_TOKEN_LIFETIME,
                                    12,
                                    true,
                                    new IPropertyGetterSetter<TenantAttributes>() {
                                        @Override
                                        public void SetLdapValue(TenantAttributes object, LdapValue[] value) {
                                            throw new IllegalStateException("property is not settable.");
                                    }

                                    @Override
                                    public LdapValue[] GetLdapValue(TenantAttributes object) {
                                        ValidateUtil.validateNotNull( object, "object" );
                                       return ServerUtils.getLdapValue( object.getTokenPolicy().getMaxBearerRefreshTokenLifetime() );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<TenantAttributes>(
                                PROPERTY_MAX_HOK_REFRESH_TOKEN_LIFETIME,
                                13,
                                true,
                                new IPropertyGetterSetter<TenantAttributes>() {
                                    @Override
                                    public void SetLdapValue(TenantAttributes object, LdapValue[] value) {
                                        throw new IllegalStateException("property is not settable.");
                                    }

                                    @Override
                                    public LdapValue[] GetLdapValue(TenantAttributes object) {
                                        ValidateUtil.validateNotNull( object, "object" );
                                       return ServerUtils.getLdapValue( object.getTokenPolicy().getMaxHoKRefreshTokenLifetime() );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<TenantAttributes>(
                                PROPERTY_ENABLE_IDP_SELECTION,
                                14,
                                true,
                                new IPropertyGetterSetter<TenantAttributes>() {
                                    @Override
                                    public void SetLdapValue(TenantAttributes object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException("property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(TenantAttributes object)
                                    {
                                        return ServerUtils.getLdapValue(object.isIDPSelectionEnabled());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<TenantAttributes>(
                                PROPERTY_ALIAS,
                                15,
                                true,
                                new IPropertyGetterSetter<TenantAttributes>() {
                                    @Override
                                    public void SetLdapValue(TenantAttributes object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException("property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(TenantAttributes object)
                                    {
                                        return ServerUtils.getLdapValue(object.getAlias());
                                    }
                                }
                        )
                }
        );
    }

    @Override
    protected TenantAttributes createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 16) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        long clockTolerance = ctorParams.get(0) == null ? IConfigStore.DEFAULT_CLOCK_TOLERANCE : ServerUtils.getNativeLongValue(ctorParams.get(0));
        int delegationCount = ctorParams.get(1) == null ? IConfigStore.DEFAULT_DELEGATION_COUNT : ServerUtils.getIntValue(ctorParams.get(1));
        int renewCount = ctorParams.get(2) == null ? IConfigStore.DEFAULT_RENEW_COUNT : ServerUtils.getIntValue(ctorParams.get(2));
        long bearerTokenLifeTime = ctorParams.get(3) == null ? IConfigStore.DEFAULT_MAX_BEARER_LIFETIME : ServerUtils.getNativeLongValue(ctorParams.get(3));
        String signatureAlgorithm = ServerUtils.getStringValue( ctorParams.get(4) );
        String brandName = ServerUtils.getStringValue( ctorParams.get(5));
        long hokTokenLifeTime = ctorParams.get(6) == null ? IConfigStore.DEFAULT_MAX_HOK_LIFETIME : ServerUtils.getNativeLongValue(ctorParams.get(6));
        String entityId = ServerUtils.getStringValue( ctorParams.get(7));
        String logonBannerTitle = ServerUtils.getStringValue(ctorParams.get(8));
        String logonBannerContent = ServerUtils.getStringValue(ctorParams.get(9));
        boolean logonBannerEnableCheckbox = ctorParams.get(10) == null ? false : ServerUtils.getBooleanValue(ctorParams.get(10));
        int[] authnTypes = ServerUtils.getMultiIntValue( ctorParams.get(11) );
        long bearerRefreshTokenLifeTime = ctorParams.get(12) == null ? IConfigStore.DEFAULT_MAX_BEARER_REFRESH_TOKEN_LIFETIME : ServerUtils.getNativeLongValue(ctorParams.get(12));
        long hokRefreshTokenLifeTime = ctorParams.get(13) == null ? IConfigStore.DEFAULT_MAX_HOK_REFRESH_TOKEN_LIFETIME : ServerUtils.getNativeLongValue(ctorParams.get(13));
        // by default, idp selection is enabled.
        boolean enableIdpSelection = ctorParams.get(14) == null ? true : ServerUtils.getBooleanValue(ctorParams.get(14));
        String alias = ServerUtils.getStringValue(ctorParams.get(15));

        return new TenantAttributes( new TokenPolicy(delegationCount, renewCount, clockTolerance, hokTokenLifeTime, bearerTokenLifeTime, bearerRefreshTokenLifeTime, hokRefreshTokenLifeTime),
                                         signatureAlgorithm,
                                         brandName, logonBannerTitle, logonBannerContent, logonBannerEnableCheckbox, entityId, alias, authnTypes, enableIdpSelection);
    }
}

final class TenantClientCertPolicy
{
    private final String _tenantName;
    private final ClientCertPolicy _policy;

    public TenantClientCertPolicy( String tenantName, ClientCertPolicy policy )
    {
        ValidateUtil.validateNotEmpty( tenantName, "tenantName" );
        ValidateUtil.validateNotNull( policy, "policy" );
        this._tenantName = tenantName;
        this._policy = policy;
    }

    public String getTenantName()
    {
        return this._tenantName;
    }

    public ClientCertPolicy getClientCertPolicy()
    {
        return this._policy;
    }
}

final class RSAAgentConfigLdapObject extends BaseLdapObject<RSAAgentConfig> {

    private static RSAAgentConfigLdapObject _instance = new RSAAgentConfigLdapObject();

    public static RSAAgentConfigLdapObject getInstance() {
        return _instance;
    }
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
            .getLogger(RSAAgentConfigLdapObject.class);

    public static final String PROPERTY_DEFAULT_NAME = "RsaConfig";
    public static final String PROPERTY_NAME = CN;
    private static final String OBJECT_CLASS = "vmwSTSTenantRsaAgentConfiguration";
    private static final String PROPERTY_LOGIN_GUIDENCE = "vmwSTSRsaLoginGuidence";
    private static final String PROPERTY_LOG_LEVEL = "vmwSTSRsaLogLevel";
    private static final String PROPERTY_LOG_FILE_SIZE = "vmwSTSRsaLogFileSize";
    private static final String PROPERTY_MAX_LOG_FILE_COUNT = "vmwSTSRsaMaxLogFileCount";
    private static final String PROPERTY_CONNECTION_TIME_OUT = "vmwSTSRsaConnectionTimeOut";
    private static final String PROPERTY_READ_TIME_OUT = "vmwSTSRsaReadTimeOut";
    private static final String PROPERTY_ENCRYPTION_ALG = "vmwSTSRsaEncryptionAlg";


    @SuppressWarnings("unchecked")
    private RSAAgentConfigLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<RSAAgentConfig>(
                                PROPERTY_NAME,
                                0,
                                true,
                                new IPropertyGetterSetter<RSAAgentConfig>() {
                                    @Override
                                    public void SetLdapValue(RSAAgentConfig object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException("name cannot be set on Tenant;");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(RSAAgentConfig object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object");
                                        return ServerUtils.getLdapValue( RSAAgentConfigLdapObject.PROPERTY_DEFAULT_NAME );
                                    }
                                },
                                false // cannot update in ldap
                         ),
                         new PropertyMapperMetaInfo<RSAAgentConfig>(
                                 PROPERTY_LOGIN_GUIDENCE,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<RSAAgentConfig>() {
                                     @Override
                                     public void SetLdapValue(RSAAgentConfig object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         object.set_loginGuide(ServerUtils.getStringValue(value));
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(RSAAgentConfig object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.get_loginGuide());
                                     }
                                 },
                                 true
                         ),

                         new PropertyMapperMetaInfo<RSAAgentConfig>(
                                 PROPERTY_LOG_LEVEL,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<RSAAgentConfig>() {
                                     @Override
                                     public void SetLdapValue(RSAAgentConfig object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         object.set_logLevel(
                                                 RSAAgentConfig.RSALogLevelType.valueOf(ServerUtils.getStringValue(value)));
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(RSAAgentConfig object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.get_logLevel().toString());
                                     }
                                 },
                                 true
                         ),
                         new PropertyMapperMetaInfo<RSAAgentConfig>(
                                 PROPERTY_LOG_FILE_SIZE,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<RSAAgentConfig>() {
                                     @Override
                                     public void SetLdapValue(RSAAgentConfig object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         Integer integerVal = ServerUtils.getIntegerValue(value);
                                         if (integerVal != null) {
                                             object.set_logFileSize(integerVal.intValue());
                                         }
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(RSAAgentConfig object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.get_logFileSize());
                                     }
                                 },
                                 true
                         ),
                         new PropertyMapperMetaInfo<RSAAgentConfig>(
                                 PROPERTY_MAX_LOG_FILE_COUNT,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<RSAAgentConfig>() {
                                     @Override
                                     public void SetLdapValue(RSAAgentConfig object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         Integer integerVal = ServerUtils.getIntegerValue(value);
                                         if (integerVal != null) {
                                             object.set_maxLogFileCount(integerVal.intValue());
                                         }
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(RSAAgentConfig object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.get_maxLogFileCount());
                                     }
                                 },
                                 true
                         ),
                         new PropertyMapperMetaInfo<RSAAgentConfig>(
                                 PROPERTY_CONNECTION_TIME_OUT,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<RSAAgentConfig>() {
                                     @Override
                                     public void SetLdapValue(RSAAgentConfig object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         Integer integerVal = ServerUtils.getIntegerValue(value);
                                         if (integerVal != null) {
                                             object.set_connectionTimeOut(integerVal.intValue());
                                         }
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(RSAAgentConfig object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.get_connectionTimeOut());
                                     }
                                 },
                                 true
                         ),
                         new PropertyMapperMetaInfo<RSAAgentConfig>(
                                 PROPERTY_READ_TIME_OUT,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<RSAAgentConfig>() {
                                     @Override
                                     public void SetLdapValue(RSAAgentConfig object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         Integer integerVal = ServerUtils.getIntegerValue(value);
                                         if (integerVal != null) {
                                             object.set_readTimeOut(integerVal.intValue());
                                         }
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(RSAAgentConfig object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.get_readTimeOut());
                                     }
                                 },
                                 true
                         ),
                         new PropertyMapperMetaInfo<RSAAgentConfig>(
                                 PROPERTY_ENCRYPTION_ALG,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<RSAAgentConfig>() {
                                     @Override
                                     public void SetLdapValue(RSAAgentConfig object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         Set<String> algSet = new HashSet<String>(Arrays.asList(ServerUtils.getMultiStringValue(value)));
                                         object.set_rsaEncAlgList(algSet);
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(RSAAgentConfig object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.get_rsaEncAlgList());
                                     }
                                 },
                                 true
                         )
                }
          );
    }
    @Override
    protected RSAAgentConfig createObject(List<LdapValue[]> ctorParams) {
        if ( (ctorParams == null) || (ctorParams.size() < 1) )
        {
            throw new IllegalArgumentException("ctorParams");
        }
        return new RSAAgentConfig();
    }
}



final class RSAInstanceLdapObject extends BaseLdapObject<RSAAMInstanceInfo> {

    private static RSAInstanceLdapObject _instance = new RSAInstanceLdapObject();

    public static RSAInstanceLdapObject getInstance() {
        return _instance;
    }
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
            .getLogger(RSAAgentConfigLdapObject.class);

    public static final String PROPERTY_NAME = CN;
    public static final String OBJECT_CLASS = "vmwSTSTenantRsaAgentInstance";
    private static final String PROPERTY_SITE_ID = "vmwSTSRsaSiteID";
    private static final String PROPERTY_AGENT_NAME = "vmwSTSRsaAgentName";
    private static final String PROPERTY_SDCONFIG_REC = "vmwSTSRsaSDConfigRec";
    private static final String PROPERTY_SDOPTS_REC = "vmwSTSRsaSDOptsRec";


    @SuppressWarnings("unchecked")
    private RSAInstanceLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<RSAAMInstanceInfo>(
                                PROPERTY_NAME,
                                0,
                                true,
                                new IPropertyGetterSetter<RSAAMInstanceInfo>() {
                                    @Override
                                    public void SetLdapValue(RSAAMInstanceInfo object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException("name cannot be set on Tenant;");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(RSAAMInstanceInfo object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object");
                                        return ServerUtils.getLdapValue( object.get_siteID() );
                                    }
                                },
                                false // cannot update in ldap
                         ),
                         new PropertyMapperMetaInfo<RSAAMInstanceInfo>(
                                 PROPERTY_SITE_ID,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<RSAAMInstanceInfo>() {
                                     @Override
                                     public void SetLdapValue(RSAAMInstanceInfo object, LdapValue[] value) {
                                            //do nothing
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(RSAAMInstanceInfo object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.get_siteID());
                                     }
                                 },
                                 false
                         ),
                         new PropertyMapperMetaInfo<RSAAMInstanceInfo>(
                                 PROPERTY_AGENT_NAME,
                                 1,
                                 true,
                                 new IPropertyGetterSetter<RSAAMInstanceInfo>() {
                                     @Override
                                     public void SetLdapValue(RSAAMInstanceInfo object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         try {
                                             object.set_agentName(ServerUtils.getStringValue(value));
                                         } catch (InvalidArgumentException e) {
                                             logger.error("Null or empty agent name string read from directory.");
                                         }
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(RSAAMInstanceInfo object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.get_agentName());
                                     }
                                 },
                                 true
                         ),

                         new PropertyMapperMetaInfo<RSAAMInstanceInfo>(
                                 PROPERTY_SDCONFIG_REC,
                                 2,
                                 true,
                                 new IPropertyGetterSetter<RSAAMInstanceInfo>() {
                                     @Override
                                     public void SetLdapValue(RSAAMInstanceInfo object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         try {
                                             object.set_sdconfRec(ServerUtils.getBinaryValue(value));
                                         } catch (InvalidArgumentException e) {
                                             logger.error("Null or empty sdconf_rec file read from directory");
                                         }
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(RSAAMInstanceInfo object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.get_sdconfRec());
                                     }
                                 },
                                 true
                         ),

                         new PropertyMapperMetaInfo<RSAAMInstanceInfo>(
                                 PROPERTY_SDOPTS_REC,
                                 3,
                                 true,
                                 new IPropertyGetterSetter<RSAAMInstanceInfo>() {
                                     @Override
                                     public void SetLdapValue(RSAAMInstanceInfo object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         object.set_sdoptsRec(ServerUtils.getBinaryValue(value));
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(RSAAMInstanceInfo object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.get_sdoptsRec());
                                     }
                                 },
                                 true
                         )

                }
          );
    }
    @Override
    protected RSAAMInstanceInfo createObject(List<LdapValue[]> ctorParams) {
        if ( (ctorParams == null) || (ctorParams.size() < 1) )
        {
            throw new IllegalArgumentException("ctorParams");
        }
        return new RSAAMInstanceInfo(
                ServerUtils.getStringValue(ctorParams.get(0)),
                ServerUtils.getStringValue(ctorParams.get(1)),
                ServerUtils.getBinaryValue(ctorParams.get(2)),
                ServerUtils.getBinaryValue(ctorParams.get(3))
                );
    }
}
final class ClientCertPolicyLdapObject extends BaseLdapObject<TenantClientCertPolicy> {
    private static ClientCertPolicyLdapObject _instance = new ClientCertPolicyLdapObject();

    public static ClientCertPolicyLdapObject getInstance() {
        return _instance;
    }

    public static final String ClientCertificatePolicyDefaultName = "Default";
    public static final String PROPERTY_NAME = CN;
    public static final String PROPERTY_CLIENT_CERT_CRL_CHECK_ENABLED = "vmwSTSClientCertRevocationCheckEnabled";
    public static final String PROPERTY_CLIENT_CERT_CRL_CHECK_OCSP_ENABLED = "vmwSTSClientCertOCSPEnabled";
    public static final String PROPERTY_CLIENT_CERT_CRL_CHECK_OCSP_USE_CRL_AS_FAIL_OVER = "vmwSTSClientCertUseCRLAsFailOver";
    public static final String PROPERTY_CLIENT_CERT_CRL_CHECK_OCSP_SEND_OCSP_NOUNCE = "vmwSTSClientCertSendOCSPNounce";
    public static final String PROPERTY_CLIENT_CERT_CRL_CHECK_OCSP_URL = "vmwSTSClientCertOCSPUrl";
    public static final String PROPERTY_CLIENT_CERT_CRL_CHECK_OCSP_CERT = "userCertificate";
    public static final String PROPERTY_CLIENT_CERT_CRL_CHECK_CRL_USE_CERT_CRL = "vmwSTSClientCertUseCertCRL";
    public static final String PROPERTY_CLIENT_CERT_CRL_CHECK_CRL_URL = "vmwSTSClientCertCRLUrl";
    public static final String PROPERTY_CLIENT_CERT_CRL_CHECK_CRL_CACHE_SIZE = "vmwSTSClientCertCRLCacheSize";
    public static final String PROPERTY_CLIENT_CERT_CRL_CHECK_CUSTOMER_POLICY_OID = "vmwSTSClientCertCustomCertPolicyOid";
    public static final String PROPERTY_CLIENT_CERT_ENABLE_HINT = "vmwSTSClientCertEnableUserNameHint";

    public static final String CONTAINER_CLIENT_CERT_TRUSTED_CA_CERTIFICATES = "ClientCertAuthnTrustedCAs";
    public static final String CLIENT_CERT_TRUSTED_CA_CERTIFICATES_DEFAULT_NAME = "DefaultClientCertCAStore";

    private static final String OBJECT_CLASS = "vmwSTSTenantClientCertificatePolicy";

    @SuppressWarnings("unchecked")
    private ClientCertPolicyLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<TenantClientCertPolicy>(
                                PROPERTY_NAME,
                                0,
                                true,
                                new IPropertyGetterSetter<TenantClientCertPolicy>() {
                                    @Override
                                    public void SetLdapValue(TenantClientCertPolicy object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException("name cannot be set on Tenant;");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(TenantClientCertPolicy object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object");
                                        return ServerUtils.getLdapValue( ClientCertificatePolicyDefaultName );
                                    }
                                },
                                false // cannot update in ldap
                         ),
                         new PropertyMapperMetaInfo<TenantClientCertPolicy>(
                                 PROPERTY_CLIENT_CERT_CRL_CHECK_ENABLED,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<TenantClientCertPolicy>() {
                                     @Override
                                     public void SetLdapValue(TenantClientCertPolicy object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         object.getClientCertPolicy().setRevocationCheckEnabled(ServerUtils.getBooleanValue(value));
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(TenantClientCertPolicy object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.getClientCertPolicy().revocationCheckEnabled());
                                     }
                                 }
                         ),
                         new PropertyMapperMetaInfo<TenantClientCertPolicy>(
                                 PROPERTY_CLIENT_CERT_CRL_CHECK_OCSP_ENABLED,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<TenantClientCertPolicy>() {
                                     @Override
                                     public void SetLdapValue(TenantClientCertPolicy object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         ValidateUtil.validateNotNull( object.getClientCertPolicy(), "object.getClientCertPolicy()" );
                                         object.getClientCertPolicy().setUseOCSP(ServerUtils.getBooleanValue(value));
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(TenantClientCertPolicy object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.getClientCertPolicy().useOCSP());
                                     }
                                 }
                         ),
                         new PropertyMapperMetaInfo<TenantClientCertPolicy>(
                                 PROPERTY_CLIENT_CERT_CRL_CHECK_OCSP_USE_CRL_AS_FAIL_OVER,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<TenantClientCertPolicy>() {
                                     @Override
                                     public void SetLdapValue(TenantClientCertPolicy object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         object.getClientCertPolicy().setUseCRLAsFailOver(ServerUtils.getBooleanValue(value));
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(TenantClientCertPolicy object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.getClientCertPolicy().useCRLAsFailOver());
                                     }
                                 }
                         ),
                         new PropertyMapperMetaInfo<TenantClientCertPolicy>(
                                 PROPERTY_CLIENT_CERT_CRL_CHECK_OCSP_SEND_OCSP_NOUNCE,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<TenantClientCertPolicy>() {
                                     @Override
                                     public void SetLdapValue(TenantClientCertPolicy object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         object.getClientCertPolicy().setSendOCSPNonce(ServerUtils.getBooleanValue(value));
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(TenantClientCertPolicy object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.getClientCertPolicy().sendOCSPNonce());
                                     }
                                 }
                         ),
                         new PropertyMapperMetaInfo<TenantClientCertPolicy>(  //obsoleted
                                 PROPERTY_CLIENT_CERT_CRL_CHECK_OCSP_URL,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<TenantClientCertPolicy>() {
                                     @Override
                                     public void SetLdapValue(TenantClientCertPolicy object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         String urlString = ServerUtils.getStringValue(value);
                                         URL url = null;
                                         if(urlString != null){
                                             try
                                             {
                                                 url = new URL(urlString);
                                             }
                                             catch( MalformedURLException ex ){
                                                 throw new IllegalStateException("OCSP Url is mal-formed.");
                                             }
                                         }
                                         object.getClientCertPolicy().setOCSPUrl(url);
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(TenantClientCertPolicy object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         URL url = object.getClientCertPolicy().getOCSPUrl();
                                         return ServerUtils.getLdapValue(url != null? url.toString(): null);
                                     }
                                 }
                         ),
                         new PropertyMapperMetaInfo<TenantClientCertPolicy>(  //obsoleted
                                 PROPERTY_CLIENT_CERT_CRL_CHECK_OCSP_CERT,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<TenantClientCertPolicy>() {
                                     @Override
                                     public void SetLdapValue(TenantClientCertPolicy object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         object.getClientCertPolicy().setOCSPResponderSigningCert(ServerUtils.getCertificateValue(value));
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(TenantClientCertPolicy object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.getClientCertPolicy().getOCSPResponderSigningCert());
                                     }
                                 }
                         ),
                         new PropertyMapperMetaInfo<TenantClientCertPolicy>(
                                 PROPERTY_CLIENT_CERT_CRL_CHECK_CRL_USE_CERT_CRL,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<TenantClientCertPolicy>() {
                                     @Override
                                     public void SetLdapValue(TenantClientCertPolicy object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         object.getClientCertPolicy().setUseCertCRL(ServerUtils.getBooleanValue(value));
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(TenantClientCertPolicy object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.getClientCertPolicy().useCertCRL());
                                     }
                                 }
                         ),
                         new PropertyMapperMetaInfo<TenantClientCertPolicy>(
                                 PROPERTY_CLIENT_CERT_CRL_CHECK_CRL_URL,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<TenantClientCertPolicy>() {
                                     @Override
                                     public void SetLdapValue(TenantClientCertPolicy object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         String urlString = ServerUtils.getStringValue(value);
                                         URL url = null;
                                         if(urlString != null){
                                             try
                                             {
                                                 url = new URL(urlString);
                                             }
                                             catch( MalformedURLException ex ){
                                                 throw new IllegalStateException("CRL Url is mal-formed.");
                                             }
                                         }
                                         object.getClientCertPolicy().setCRLUrl(url);
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(TenantClientCertPolicy object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         URL url = object.getClientCertPolicy().getCRLUrl();
                                         return ServerUtils.getLdapValue(url != null? url.toString(): null);
                                     }
                                 }
                         ),
                         new PropertyMapperMetaInfo<TenantClientCertPolicy>(
                                 PROPERTY_CLIENT_CERT_CRL_CHECK_CRL_CACHE_SIZE,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<TenantClientCertPolicy>() {
                                     @Override
                                     public void SetLdapValue(TenantClientCertPolicy object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         object.getClientCertPolicy().setCacheSize(ServerUtils.getIntValue(value));
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(TenantClientCertPolicy object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.getClientCertPolicy().getCacheSize());
                                     }
                                 }
                         ),
                         new PropertyMapperMetaInfo<TenantClientCertPolicy>(
                                 PROPERTY_CLIENT_CERT_CRL_CHECK_CUSTOMER_POLICY_OID,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<TenantClientCertPolicy>() {
                                     @Override
                                     public void SetLdapValue(TenantClientCertPolicy object, LdapValue[] value) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         object.getClientCertPolicy().setOIDs(ServerUtils.getMultiStringValue(value));
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(TenantClientCertPolicy object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         Collection<String> c = new HashSet<String>();
                                         if(object.getClientCertPolicy().getOIDs() != null) {
                                             for(String oid : object.getClientCertPolicy().getOIDs()){
                                                 c.add(oid);
                                             }
                                         }
                                         return ServerUtils.getLdapValue(c);
                                     }
                                 }
                         ),
                         new PropertyMapperMetaInfo<TenantClientCertPolicy>(
                             PROPERTY_CLIENT_CERT_ENABLE_HINT,
                             -1,
                             true,
                             new IPropertyGetterSetter<TenantClientCertPolicy>() {
                                 @Override
                                 public void SetLdapValue(TenantClientCertPolicy object, LdapValue[] value) {
                                     ValidateUtil.validateNotNull( object, "object" );
                                     object.getClientCertPolicy().setEnableHint(ServerUtils.getBooleanValue(value));
                                 }
                                 @Override
                                 public LdapValue[] GetLdapValue(TenantClientCertPolicy object) {
                                     ValidateUtil.validateNotNull( object, "object" );
                                     return ServerUtils.getLdapValue(object.getClientCertPolicy().getEnableHint());
                                 }
                             }
                     )
                }
          );
    }

    @Override
    protected TenantClientCertPolicy createObject(List<LdapValue[]> ctorParams) {
        if ( (ctorParams == null) || (ctorParams.size() < 1) )
        {
            throw new IllegalArgumentException("ctorParams");
        }
        String tenantName = ServerUtils.getStringValue(ctorParams.get(0));
        return new TenantClientCertPolicy(tenantName, new ClientCertPolicy());
    }
}

final class AlternativeOCSPListLdapObject extends BaseLdapObject<AlternativeOCSPList> {

    private static AlternativeOCSPListLdapObject _instance = new AlternativeOCSPListLdapObject();

    public static AlternativeOCSPListLdapObject getInstance() {
        return _instance;
    }
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
            .getLogger(AlternativeOCSPListLdapObject.class);

    public static final String PROPERTY_NAME = CN;
    public static final String OBJECT_CLASS = "vmwSTSTenantAltOCSPRespondersSite";
    private static final String PROPERTY_OCSP_SITEID = "vmwSTSPscSiteID";


    @SuppressWarnings("unchecked")
    private AlternativeOCSPListLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<AlternativeOCSPList>(
                                PROPERTY_NAME,
                                0,
                                true,
                                new IPropertyGetterSetter<AlternativeOCSPList>() {
                                    @Override
                                    public void SetLdapValue(AlternativeOCSPList object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException("name cannot be set on AlternativeOCSPList;");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(AlternativeOCSPList object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object");
                                        return ServerUtils.getLdapValue( object.get_siteID() );
                                    }
                                },
                                false // cannot update in ldap
                         ),
                         new PropertyMapperMetaInfo<AlternativeOCSPList>(
                                 PROPERTY_OCSP_SITEID,
                                 -1,
                                 true,
                                 new IPropertyGetterSetter<AlternativeOCSPList>() {
                                     @Override
                                     public void SetLdapValue(AlternativeOCSPList object, LdapValue[] value) {
                                         //do nothing
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(AlternativeOCSPList object) {
                                         ValidateUtil.validateNotNull( object, "object" );

                                         return ServerUtils.getLdapValue(object.get_siteID());
                                     }
                                 },
                                 false
                         ),

                }
          );
    }
    @Override
    protected AlternativeOCSPList createObject(List<LdapValue[]> ctorParams) {
        if ( (ctorParams == null) || (ctorParams.size() < 1) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        return new AlternativeOCSPList(
                ServerUtils.getStringValue(ctorParams.get(0)),
                null
                );
    }
}


final class TenantAlternativeOCSPLdapObject extends BaseLdapObject<TenantAlternativeOCSP> {

    private static TenantAlternativeOCSPLdapObject _instance = new TenantAlternativeOCSPLdapObject();

    public static TenantAlternativeOCSPLdapObject getInstance() {
        return _instance;
    }
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
            .getLogger(TenantAlternativeOCSPLdapObject.class);

    public static final String PROPERTY_NAME = CN;
    public static final String OBJECT_CLASS = "vmwSTSTenantAltOCSPResponder";
    private static final String PROPERTY_OCSP_URL = "vmwSTSClientCertOCSPUrl";
    private static final String PROPERTY_OCSP_CERT = "userCertificate";


    @SuppressWarnings("unchecked")
    private TenantAlternativeOCSPLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<TenantAlternativeOCSP>(
                                PROPERTY_NAME,
                                0,
                                true,
                                new IPropertyGetterSetter<TenantAlternativeOCSP>() {
                                    @Override
                                    public void SetLdapValue(TenantAlternativeOCSP object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException("name cannot be set on AlternativeOCSP;");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(TenantAlternativeOCSP object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object");
                                        return ServerUtils.getLdapValue( object.getCn() );
                                    }
                                },
                                false // cannot update in ldap
                         ),
                         new PropertyMapperMetaInfo<TenantAlternativeOCSP>(
                                 PROPERTY_OCSP_URL,
                                 1,
                                 true,
                                 new IPropertyGetterSetter<TenantAlternativeOCSP>() {
                                     @Override
                                     public void SetLdapValue(TenantAlternativeOCSP object, LdapValue[] value) {
                                         //do nothing
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(TenantAlternativeOCSP object) {
                                         ValidateUtil.validateNotNull( object, "object" );

                                         return ServerUtils.getLdapValue(object.getAlternativeOCSP().get_responderURL().toString());
                                     }
                                 },
                                 false
                         ),
                         new PropertyMapperMetaInfo<TenantAlternativeOCSP>(
                                 PROPERTY_OCSP_CERT,
                                 2,
                                 true,
                                 new IPropertyGetterSetter<TenantAlternativeOCSP>() {
                                     @Override
                                     public void SetLdapValue(TenantAlternativeOCSP object, LdapValue[] value) {
                                         throw new IllegalStateException( "property is not settable.");
                                     }
                                     @Override
                                     public LdapValue[] GetLdapValue(TenantAlternativeOCSP object) {
                                         ValidateUtil.validateNotNull( object, "object" );
                                         return ServerUtils.getLdapValue(object.getAlternativeOCSP().get_responderSigningCert());
                                     }
                                 },
                                 true
                         ),

                }
          );
    }
    @Override
    protected TenantAlternativeOCSP createObject(List<LdapValue[]> ctorParams) {
        if ( (ctorParams == null) || (ctorParams.size() != 3) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        URL ocspURL;
        try {
            ocspURL = new URL(ServerUtils.getStringValue(ctorParams.get(1)));
        } catch (MalformedURLException e) {
            logger.error("MalformedURLException thrown in creating URL from ldap value "+ServerUtils.getStringValue(ctorParams.get(0)));
            return null;
        }
        return new TenantAlternativeOCSP( ServerUtils.getStringValue(ctorParams.get(0)), new AlternativeOCSP(
                    ocspURL,
                    ServerUtils.getCertificateValue(ctorParams.get(2))
                    ));
    }
}
final class TenantAlternativeOCSP
{
    private final String _cn;
    private final AlternativeOCSP _ocsp;

    public TenantAlternativeOCSP( String cn, AlternativeOCSP ocsp )
    {
        ValidateUtil.validateNotEmpty( cn, "cn" );
        ValidateUtil.validateNotNull( ocsp, "ocsp" );
        this._ocsp = ocsp;
        this._cn = cn;
    }

    public String getCn()
    {
        return this._cn;
    }

    public AlternativeOCSP getAlternativeOCSP()
    {
        return this._ocsp;
    }
}
final class TenantLdapObject extends BaseLdapObject<Tenant>
{
    private static TenantLdapObject _instance = new TenantLdapObject();
    public static TenantLdapObject getInstance() { return _instance; }

    public static final String PROPERTY_NAME = CN;
    public static final String PROPERTY_GUID = "vmwSTSGuidIdentity";
    public static final String PROPERTY_LONG_NAME = "name";
    public static final String PROPERTY_ENTITY_ID = "vmwSTSEntityId";
    public static final String PROPERTY_ALIAS = "vmwSTSAlias";
    public static final String PROPERTY_CLOCK_TOLERANCE = "vmwSTSClockTolerance";
    public static final String PROPERTY_SIGNATURE_ALGORITHM = "vmwSTSSignatureAlgorithmIdentifier";
    public static final String PROPERTY_ISSUER_NAME = "vmwSTSIssuerName";
    public static final String PROPERTY_DELEGATION_COUNT = "vmwSTSDelegationCount";
    public static final String PROPERTY_RENEW_COUNT = "vmwSTSRenewCount";
    public static final String PROPERTY_MAX_BEARTOKEN_LIFETIME = "vmwSTSMaxBearerTokenLifetime";
    public static final String PROPERTY_MAX_HOKTOKEN_LIFETIME = "vmwSTSMaxHolderOfKeyTokenLifetime";
    public static final String PROPERTY_DEFAULT_PROVIDER = "vmwSTSDefaultIdentityProvider";
    public static final String PROPERTY_BRAND_NAME = "vmwSTSBrandName";
    public static final String PROPERTY_LOGON_BANNER_TITLE = "vmwSTSLogonBannerTitle";
    public static final String PROPERTY_LOGON_BANNER_CONTENT = "vmwSTSLogonBanner";
    public static final String PROPERTY_LOGON_BANNER_ENABLE_CHECKBOX = "vmwSTSLogonBannerEnableCheckbox";
    public static final String PROPERTY_AUTHN_TYPES = "vmwSTSAuthnTypes";
    public static final String PROPERTY_RSA_SITE_ID = "vmwSTSRsaSiteID";
    public static final String PROPERTY_RSA_AGENT_NAME = "vmwSTSRsaAgentName";
    public static final String PROPERTY_RSA_SDCONFIG_REC = "vmwSTSRsaSDConfigRec";
    public static final String PROPERTY_RSA_SDOPTS_REC = "vmwSTSRsaSDOptsRec";
    public static final String PROPERTY_RSA_LOG_LEVEL = "vmwSTSRsaLogLevel";
    public static final String PROPERTY_RSA_LOGFILE_SIZE = "vmwSTSRsaLogFileSize";
    public static final String PROPERTY_RSA_MAC_LOGFILE_COUNT = "vmwSTSRsaMaxLogFileCount";
    public static final String PROPERTY_RSA_CONNECTION_TIME_OUT = "vmwSTSRsaConnectionTimeOut";
    public static final String PROPERTY_RSA_READ_TIME_OUT = "vmwSTSRsaReadTimeOut";
    public static final String PROPERTY_RSA_ENCRYPTION_ALG = "vmwSTSRsaEncryptionAlg";

    public static final String PROPERTY_MAX_BEARER_REFRESH_TOKEN_LIFETIME = "vmwSTSMaxBearerRefreshTokenLifetime";
    public static final String PROPERTY_MAX_HOK_REFRESH_TOKEN_LIFETIME = "vmwSTSMaxHolderOfKeyRefreshTokenLifetime";
    public static final String PROPERTY_ENABLE_IDP_SELECTION = "vmwSTSEnableIdpSelection";

    public static final String PROPERTY_TENANT_KEY = "vmwSTSTenantKey";

    private static final String OBJECT_CLASS = "vmwSTSTenant";

    @SuppressWarnings("unchecked")
    private TenantLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_NAME,
                                0,
                                true,
                                new IPropertyGetterSetter<Tenant>() {
                                    @Override
                                    public void SetLdapValue(Tenant object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException("name cannot be set on Tenant;");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(Tenant object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object");
                                        return ServerUtils.getLdapValue( object.getName() );
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_GUID,
                                -1,
                                true,
                                new IPropertyGetterSetter<Tenant>() {
                                    @Override
                                    public void SetLdapValue(Tenant object, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( object, "object");
                                        object._guid = ServerUtils.getStringValue(value);
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(Tenant object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object");
                                        return ServerUtils.getLdapValue( object._guid );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_LONG_NAME,
                                1,
                                true,
                                new IPropertyGetterSetter<Tenant>() {
                                    @Override
                                    public void SetLdapValue(Tenant object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException("long name cannot be set on Tenant;");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(Tenant object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object");
                                        return ServerUtils.getLdapValue( object._longName );
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_TENANT_KEY,
                                2,
                                true,
                                new IPropertyGetterSetter<Tenant>() {
                                    @Override
                                    public void SetLdapValue(Tenant object, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( object, "object");
                                        object._tenantKey = ServerUtils.getStringValue(value);
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(Tenant object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object");
                                        return ServerUtils.getLdapValue( object._tenantKey );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_ISSUER_NAME,
                                -1,
                                true,
                                new IPropertyGetterSetter<Tenant>() {
                                    @Override
                                    public void SetLdapValue(Tenant object, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( object, "object");
                                        object._issuerName = ServerUtils.getStringValue(value);
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(Tenant object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object");
                                        return ServerUtils.getLdapValue( object._issuerName );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_CLOCK_TOLERANCE,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_DELEGATION_COUNT,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_RENEW_COUNT,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_MAX_BEARTOKEN_LIFETIME,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                           PROPERTY_SIGNATURE_ALGORITHM,
                           -1,
                           false,
                           null
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_BRAND_NAME,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_ENTITY_ID,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_MAX_HOKTOKEN_LIFETIME,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_DEFAULT_PROVIDER,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                        		PROPERTY_LOGON_BANNER_TITLE,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_LOGON_BANNER_CONTENT,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_LOGON_BANNER_ENABLE_CHECKBOX,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_AUTHN_TYPES,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_MAX_BEARER_REFRESH_TOKEN_LIFETIME,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_MAX_HOK_REFRESH_TOKEN_LIFETIME,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_ENABLE_IDP_SELECTION,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<Tenant>(
                                PROPERTY_ALIAS,
                                -1,
                                false,
                                null
                        )
                }
        );
    }

    @Override
    protected Tenant createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 3) )
        {
            throw new IllegalArgumentException("ctorParams");
        }
        Tenant tenant = null;

        tenant = new Tenant(ServerUtils.getStringValue(ctorParams.get(0)),
                            ServerUtils.getStringValue(ctorParams.get(1)),
                            ServerUtils.getStringValue(ctorParams.get(2)));

        return tenant;
    }
}

final class TenantPasswordExpiration
{
    private final String _tenantName;
    private final PasswordExpiration _passwordExpiration;

    public TenantPasswordExpiration( String tenantName, PasswordExpiration passwordExpiration )
    {
        ValidateUtil.validateNotEmpty( tenantName, "tenantName" );
        ValidateUtil.validateNotNull( passwordExpiration, "passwordExpiration" );

        this._tenantName = tenantName;
        this._passwordExpiration = passwordExpiration;
    }

    public String getTenantName()
    {
        return this._tenantName;
    }

    public PasswordExpiration getPasswordExpiration()
    {
        return this._passwordExpiration;
    }
}

final class TenantPasswordExpirationLdapObject extends BaseLdapObject<TenantPasswordExpiration>
{
    private static TenantPasswordExpirationLdapObject _instance = new TenantPasswordExpirationLdapObject();
    public static TenantPasswordExpirationLdapObject getInstance() { return _instance; }

    public static final String PROPERTY_NAME = CN;
    public static final String PROPERTY_PWDEXP_ENABLE_EMAIL_NOTIFICATION = "vmwSTSEnablePasswordExpirationEmailNotification";
    public static final String PROPERTY_PWDEXP_NOTIFICATION_DAYS = "vmwSTSPasswordExpirationNotificationDays";
    public static final String PROPERTY_PWDEXP_EMAIL_FROM = "vmwSTSPasswordExpirationFromEmail";
    public static final String PROPERTY_PWDEXP_EMAIL_SUBJECT = "vmwSTSPasswordExpirationEmailSubject";

    private static final String OBJECT_CLASS = "vmwSTSTenant";

    @SuppressWarnings("unchecked")
    private TenantPasswordExpirationLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<TenantPasswordExpiration>(
                                PROPERTY_NAME,
                                0,
                                true,
                                new IPropertyGetterSetter<TenantPasswordExpiration>() {
                                    @Override
                                    public void SetLdapValue(TenantPasswordExpiration object, LdapValue[] value) {
                                        // no op
                                    }

                                    @Override
                                    public LdapValue[] GetLdapValue(TenantPasswordExpiration object) {

                                        ValidateUtil.validateNotNull( object, "object" );
                                        return ServerUtils.getLdapValue(object.getTenantName());
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<TenantPasswordExpiration>(
                                PROPERTY_PWDEXP_ENABLE_EMAIL_NOTIFICATION,
                                1,
                                true,
                                new IPropertyGetterSetter<TenantPasswordExpiration>() {
                                    @Override
                                    public void SetLdapValue(TenantPasswordExpiration object, LdapValue[] value) {
                                        throw new IllegalStateException("property is not settable.");
                                    }

                                    @Override
                                    public LdapValue[] GetLdapValue(TenantPasswordExpiration object) {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        return ServerUtils.getLdapValue(
                                            (object.getPasswordExpiration().isEmailNotificationEnabled() == true)
                                            ? 1
                                            : 0
                                        );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<TenantPasswordExpiration>(
                                PROPERTY_PWDEXP_EMAIL_FROM,
                                2,
                                true,
                                new IPropertyGetterSetter<TenantPasswordExpiration>() {
                                    @Override
                                    public void SetLdapValue(TenantPasswordExpiration object, LdapValue[] value) {
                                        throw new IllegalStateException("property is not settable.");
                                    }

                                    @Override
                                    public LdapValue[] GetLdapValue(TenantPasswordExpiration object) {
                                        ValidateUtil.validateNotNull( object, "object" );
                                       return ServerUtils.getLdapValue( object.getPasswordExpiration().getEmailFrom());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<TenantPasswordExpiration>(
                                PROPERTY_PWDEXP_EMAIL_SUBJECT,
                                3,
                                true,
                                new IPropertyGetterSetter<TenantPasswordExpiration>() {
                                    @Override
                                    public void SetLdapValue(TenantPasswordExpiration object, LdapValue[] value) {
                                        throw new IllegalStateException("property is not settable.");
                                    }

                                    @Override
                                    public LdapValue[] GetLdapValue(TenantPasswordExpiration object) {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        return ServerUtils.getLdapValue(
                                            object.getPasswordExpiration().getEmailSubject()
                                        );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<TenantPasswordExpiration>(
                                PROPERTY_PWDEXP_NOTIFICATION_DAYS,
                                4,
                                true,
                                new IPropertyGetterSetter<TenantPasswordExpiration>() {
                                    @Override
                                    public void SetLdapValue(TenantPasswordExpiration object, LdapValue[] value) {
                                        throw new IllegalStateException("property is not settable.");
                                    }

                                    @Override
                                    public LdapValue[] GetLdapValue(TenantPasswordExpiration object) {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        return ServerUtils.getLdapValue(
                                                object.getPasswordExpiration().getNotificationDays());
                                    }
                                }
                        )
                }
        );
    }

    @Override
    protected TenantPasswordExpiration createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 5) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        try
        {
            boolean enabled = ServerUtils.getIntValue( ctorParams.get(1) ) == 1 ? true : false;
            String fromAddr = ServerUtils.getStringValue( ctorParams.get(2) );
            String subject = ServerUtils.getStringValue( ctorParams.get(3));
            int[] notificationTime = ServerUtils.getMultiIntValue( ctorParams.get(4) );

            ValidateUtil.validateNotEmpty(fromAddr, "fromAddr");
            ValidateUtil.validateNotEmpty(subject, "subject");
            ValidateUtil.validateNotNull(notificationTime, "notificationTime");

            PasswordExpiration expiration = new PasswordExpiration(enabled, fromAddr, subject, notificationTime);

            return new TenantPasswordExpiration( ServerUtils.getStringValue( ctorParams.get(0)), expiration );
        }
        catch(IllegalArgumentException ex)
        {
            // TODO: log
            throw new RuntimeException(
                    "Tenant's password expiration hasn't been configured yet."
            );
        }
    }
}

final class TenantCredentialsLdapObject extends BaseLdapObject<TenantCredentialInformation>
{
    private static TenantCredentialsLdapObject _instance = new TenantCredentialsLdapObject();
    public static TenantCredentialsLdapObject getInstance() { return _instance; }

    public static final String OBJECT_CLASS = "vmwSTSTenantCredential";

    public static final String PROPERTY_PRIVATE_KEY = "vmwSTSPrivateKey";

    public static final String PROPERTY_CERTIFICATE_CHAIN = "userCertificate";

    @SuppressWarnings("unchecked")
    private TenantCredentialsLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<TenantCredentialInformation>(
                                CN,
                                0,
                                true,
                                new IPropertyGetterSetter<TenantCredentialInformation>() {
                                    @Override
                                    public void SetLdapValue(TenantCredentialInformation object, LdapValue[] value)
                                    {
                                        // no-op
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(TenantCredentialInformation object)
                                    {
                                        return ServerUtils.getLdapValue( object.getTenantCredName() );
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<TenantCredentialInformation>(
                                PROPERTY_PRIVATE_KEY,
                                1,
                                true,
                                new IPropertyGetterSetter<TenantCredentialInformation>() {
                                    @Override
                                    public void SetLdapValue(TenantCredentialInformation object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(TenantCredentialInformation object)
                                    {
                                        return ServerUtils.getLdapValue(object.getPrivateKey());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<TenantCredentialInformation>(
                                PROPERTY_CERTIFICATE_CHAIN,
                                2,
                                true,
                                new IPropertyGetterSetter<TenantCredentialInformation>() {
                                    @Override
                                    public void SetLdapValue(TenantCredentialInformation object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(TenantCredentialInformation object)
                                    {
                                        return ServerUtils.getLdapValue(object.getCertificateChain());
                                    }
                                }
                        )
                }
        );
    }

    @Override
    protected TenantCredentialInformation createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 3) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        return new TenantCredentialInformation(ServerUtils.getStringValue(ctorParams.get(0)),
                                               ServerUtils.getPrivateKeyValue(ctorParams.get(1)),
                                               ServerUtils.getCertificateValues(ctorParams.get(2)));
    }
}

final class TenantTrustedCertChainLdapObject extends BaseLdapObject<TenantTrustedCertificateChain>
{
    private static TenantTrustedCertChainLdapObject _instance = new TenantTrustedCertChainLdapObject();
    public static TenantTrustedCertChainLdapObject getInstance() { return _instance; }

    public static final String OBJECT_CLASS = "vmwSTSTenantTrustedCertificateChain";

    public static final String PROPERTY_CERTIFICATE_CHAIN = "userCertificate";

    @SuppressWarnings("unchecked")
    private TenantTrustedCertChainLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<TenantTrustedCertificateChain>(
                                CN,
                                0,
                                true,
                                new IPropertyGetterSetter<TenantTrustedCertificateChain>() {
                                    @Override
                                    public void SetLdapValue(TenantTrustedCertificateChain object, LdapValue[] value)
                                    {
                                        // no-op
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(TenantTrustedCertificateChain object)
                                    {
                                        return ServerUtils.getLdapValue( object.getTenantTrustedCertChainName() );
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<TenantTrustedCertificateChain>(
                                PROPERTY_CERTIFICATE_CHAIN,
                                1,
                                true,
                                new IPropertyGetterSetter<TenantTrustedCertificateChain>() {
                                    @Override
                                    public void SetLdapValue(TenantTrustedCertificateChain object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(TenantTrustedCertificateChain object)
                                    {
                                        return ServerUtils.getLdapValue(object.getCertificateChain());
                                    }
                                }
                        )
                }
        );
    }

    @Override
    protected TenantTrustedCertificateChain createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 2) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        return new TenantTrustedCertificateChain(ServerUtils.getStringValue(ctorParams.get(0)),
                                                 ServerUtils.getCertificateValues(ctorParams.get(1)));
    }
}

final class IndexedObjectWrapper<T>
{
    private final int _index;
    private final T _wrappedObject;

    public IndexedObjectWrapper(T object, int index)
    {
        ValidateUtil.validateNotNull( object, "object" );
        ValidateUtil.validateNonNegativeNumber( index, "index" );

        this._wrappedObject = object;
        this._index = index;
    }

    public T getWrappedObject() { return this._wrappedObject; }
    public int getIndex() { return this._index; }
}

final class TenantAttributeLdapObject extends BaseLdapObject<IndexedObjectWrapper<Attribute>>
{
    private static TenantAttributeLdapObject _instance = new TenantAttributeLdapObject();
    public static TenantAttributeLdapObject getInstance() { return _instance; }

    private static final String OBJECT_CLASS = "vmwSTSAttribute";

    public static final String PROPERTY_ATTR_NAME = "name";
    public static final String PROPERTY_ATTR_FRIENDLY_NAME = "vmwSTSName";
    public static final String PROPERTY_ATTR_NAME_FORMAT = "vmwSTSNameFormat";

    @SuppressWarnings("unchecked")
    private TenantAttributeLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<IndexedObjectWrapper<Attribute>>(
                                CN,
                                -1,
                                true,
                                new IPropertyGetterSetter<IndexedObjectWrapper<Attribute>>() {
                                    @Override
                                    public void SetLdapValue(IndexedObjectWrapper<Attribute> object, LdapValue[] value)
                                    {
                                        // no-op
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IndexedObjectWrapper<Attribute> object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        return ServerUtils.getLdapValue(
                                            String.format("Attribute-%d", object.getIndex()) );
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<IndexedObjectWrapper<Attribute>>(
                                PROPERTY_ATTR_NAME,
                                0,
                                true,
                                new IPropertyGetterSetter<IndexedObjectWrapper<Attribute>>() {
                                    @Override
                                    public void SetLdapValue(IndexedObjectWrapper<Attribute> object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IndexedObjectWrapper<Attribute> object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        ValidateUtil.validateNotNull( object.getWrappedObject(), "object.getWrappedObject()" );
                                        return ServerUtils.getLdapValue(object.getWrappedObject().getName());
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<IndexedObjectWrapper<Attribute>>(
                                PROPERTY_ATTR_FRIENDLY_NAME,
                                -1,
                                true,
                                new IPropertyGetterSetter<IndexedObjectWrapper<Attribute>>() {
                                    @Override
                                    public void SetLdapValue(IndexedObjectWrapper<Attribute> object, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        ValidateUtil.validateNotNull( object.getWrappedObject(), "object.getWrappedObject()" );
                                        object.getWrappedObject().setFriendlyName( ServerUtils.getStringValue( value ) );
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IndexedObjectWrapper<Attribute> object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        ValidateUtil.validateNotNull( object.getWrappedObject(), "object.getWrappedObject()" );
                                        return ServerUtils.getLdapValue(object.getWrappedObject().getFriendlyName());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IndexedObjectWrapper<Attribute>>(
                                PROPERTY_ATTR_NAME_FORMAT,
                                -1,
                                true,
                                new IPropertyGetterSetter<IndexedObjectWrapper<Attribute>>() {
                                    @Override
                                    public void SetLdapValue(IndexedObjectWrapper<Attribute> object, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        ValidateUtil.validateNotNull( object.getWrappedObject(), "object.getWrappedObject()" );
                                        object.getWrappedObject().setNameFormat( ServerUtils.getStringValue( value ) );
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IndexedObjectWrapper<Attribute> object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        ValidateUtil.validateNotNull( object.getWrappedObject(), "object.getWrappedObject()" );
                                        return ServerUtils.getLdapValue( object.getWrappedObject().getNameFormat());
                                    }
                                }
                        )
                }
        );
    }

    @Override
    protected IndexedObjectWrapper<Attribute> createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 1) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        return new IndexedObjectWrapper<Attribute>(
                new Attribute(ServerUtils.getStringValue( ctorParams.get(0) )),
                0
        );
    }
}

final class IdmCertificateLdapObject extends BaseLdapObject<IdmCertificate>
{
    private static IdmCertificateLdapObject _instance = new IdmCertificateLdapObject();
    public static IdmCertificateLdapObject getInstance() { return _instance; }

    private static final String OBJECT_CLASS = "vmwSTSCertificate";

    private static final String PROPERTY_FINGERPRINT = "vmwSTSFingerprint";
    public static final String PROPERTY_CERT_TYPE = "vmwSTSCertificateType";
    public static final String PROPERTY_ATTR_CERTIFICATE = "userCertificate";

    @SuppressWarnings("unchecked")
    private IdmCertificateLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<IdmCertificate>(
                                CN,
                                -1,
                                true,
                                new IPropertyGetterSetter<IdmCertificate>() {
                                    @Override
                                    public void SetLdapValue(IdmCertificate object, LdapValue[] value)
                                    {
                                        // no op
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IdmCertificate cert)
                                    {
                                        ValidateUtil.validateNotNull(cert, "cert");

                                        String fingerprint = null;
                                        try
                                        {
                                            fingerprint = cert.getFingerprint();
                                        }
                                        catch(CertificateEncodingException ex)
                                        {
                                            throw new RuntimeException( ex.getMessage(), ex);
                                        }
                                        catch(NoSuchAlgorithmException ex)
                                        {
                                            throw new RuntimeException( ex.getMessage(), ex);
                                        }
                                        return ServerUtils.getLdapValue(fingerprint);
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<IdmCertificate>(
                                PROPERTY_FINGERPRINT,
                                -1,
                                true,
                                new IPropertyGetterSetter<IdmCertificate>() {
                                    @Override
                                    public void SetLdapValue(IdmCertificate object, LdapValue[] value)
                                    {
                                        // no op
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IdmCertificate cert)
                                    {
                                        ValidateUtil.validateNotNull(cert, "cert");

                                        String fingerprint = null;
                                        try
                                        {
                                            fingerprint = cert.getFingerprint();
                                        }
                                        catch(CertificateEncodingException ex)
                                        {
                                            throw new RuntimeException( ex.getMessage(), ex);
                                        }
                                        catch(NoSuchAlgorithmException ex)
                                        {
                                            throw new RuntimeException( ex.getMessage(), ex);
                                        }
                                        return ServerUtils.getLdapValue(fingerprint);
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IdmCertificate>(
                                PROPERTY_ATTR_CERTIFICATE,
                                0,
                                true,
                                new IPropertyGetterSetter<IdmCertificate>() {
                                    @Override
                                    public void SetLdapValue(IdmCertificate object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IdmCertificate cert)
                                    {
                                        ValidateUtil.validateNotNull(cert, "cert");
                                        return ServerUtils.getLdapValue( cert.getCertificate() );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IdmCertificate>(
                                PROPERTY_CERT_TYPE,
                                1,
                                true,
                                new IPropertyGetterSetter<IdmCertificate>() {
                                    @Override
                                    public void SetLdapValue(IdmCertificate cert, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull(cert, "cert");
                                        cert.setCertType( CertificateType.valueOf( ServerUtils.getStringValue(value) ) );
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IdmCertificate cert)
                                    {
                                        ValidateUtil.validateNotNull(cert, "cert");
                                        return ServerUtils.getLdapValue( cert.getCertType().toString());
                                    }
                                }
                        )
                }
        );
    }

    @Override
    protected IdmCertificate createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 2) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        return new IdmCertificate(
                ServerUtils.getCertificateValue(ctorParams.get(0)),
                CertificateType.valueOf(ServerUtils.getStringValue(ctorParams.get(1)))
        );
    }
}

final class IdentityProviderLdapObject extends BaseLdapObject<IIdentityStoreData>
{
    private static IdentityProviderLdapObject _instance = new IdentityProviderLdapObject();
    public static IdentityProviderLdapObject getInstance() { return _instance; }

    private static final String OBJECT_CLASS = "vmwSTSIdentityStore";

    private static final String PROPERTY_DOMAIN_NAME = "vmwSTSDomainName";
    public static final String PROPERTY_NAME = CN;
    public static final String PROPERTY_DOMAIN_TYPE = "vmwSTSDomainType";
    public static final String PROPERTY_AUTHENTICATION_TYPE = "vmwSTSAuthenticationType";
    public static final String PROPERTY_PROVIDER_TYPE = "vmwSTSProviderType";
    public static final String PROPERTY_FRIENDLY_NAME = "vmwSTSName";
    public static final String PROPERTY_ALIAS = "vmwSTSAlias";
    public static final String PROPERTY_USER_NAME = "vmwSTSUserName";
    public static final String PROPERTY_USE_MACHINE_ACCT = "vmwSTSServiceUseMachineAccount";
    public static final String PROPERTY_SPN_NAME = "vmwSTSServicePrincipalName";
    public static final String PROPERTY_PASSWORD = "vmwSTSPassword";
    public static final String PROPERTY_SEARCH_TIMEOUT = "vmwSTSTimeout";
    public static final String PROPERTY_USER_BASEDN = "vmwSTSUserBaseDN";
    public static final String PROPERTY_GROUP_BASEDN = "vmwSTSGroupBaseDN";
    public static final String PROPERTY_CONNECTION_STR_LIST = "vmwSTSConnectionStrings";
    public static final String PROPERTY_UPN_SUFFIXES = "vmwSTSUpnSuffixes";
    public static final String PROPERTY_FLAGS = "vmwSTSIdentityStoreFlags";
    public static final String PROPERTY_TRUSTED_CERTIFICATES = "userCertificate";
    public static final String PROPERTY_AUTHN_TYPES = "vmwSTSAuthnTypes";
    public static final String PROPERTY_CERT_AUTHN_HINT_ATTR_NAME = "vmwSTSUserHintAttributeName";
    public static final String PROPERTY_CERT_AUTHN_LINKING_USE_UPN = "vmwSTSCertificateAccountLinkingUseUPN";

    @SuppressWarnings("unchecked")
    private IdentityProviderLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_DOMAIN_TYPE,
                                0,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                    @Override
                                    public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        return ServerUtils.getLdapValue( object.getDomainType().toString() );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_NAME,
                                1,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                    @Override
                                    public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        return ServerUtils.getLdapValue( object.getName() );
                                    }
                                },
                                false // cannot update in ldap
                        ),

                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                            PROPERTY_CERT_AUTHN_HINT_ATTR_NAME,
                            -1,
                            true,
                            new IPropertyGetterSetter<IIdentityStoreData>() {
                                @Override
                                public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                {
                                    ServerIdentityStoreData serverIdentityStoreData =
                                            getServerIdentityStoreData(object);

                                    serverIdentityStoreData.setHintAttributeName(
                                            ServerUtils.getStringValue( value )
                                    );
                                }
                                @Override
                                public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                {
                                    IIdentityStoreDataEx storeData = getIIdentityStoreDataEx( object );
                                    return ServerUtils.getLdapValue(
                                            (storeData != null) ? storeData.getCertUserHintAttributeName() : null );
                                }

                            },
                            false // cannot update in ldap
                        ),

                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                            PROPERTY_CERT_AUTHN_LINKING_USE_UPN,
                            -1,
                            true,
                            new IPropertyGetterSetter<IIdentityStoreData>() {
                                @Override
                                public void SetLdapValue(IIdentityStoreData object, LdapValue[] value) {
                                    ValidateUtil.validateNotNull( object, "object" );
                                    ServerIdentityStoreData serverIdentityStoreData =
                                        getServerIdentityStoreData(object);

                                    serverIdentityStoreData.setAccountLinkingUseUPN(ServerUtils.getBooleanValue(value));
                                }
                                @Override
                                public LdapValue[] GetLdapValue(IIdentityStoreData object) {
                                    ValidateUtil.validateNotNull( object, "object" );
                                    return ServerUtils.getLdapValue(object.getExtendedIdentityStoreData().getCertLinkingUseUPN());
                                }
                            }
                        ),

                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_DOMAIN_NAME,
                                -1,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                    @Override
                                    public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                    {
                                        // no op
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        return ServerUtils.getLdapValue( object.getName() );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_ALIAS,
                                -1,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                    @Override
                                    public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                    {
                                        ServerIdentityStoreData serverIdentityStoreData =
                                                getServerIdentityStoreData(object);

                                        serverIdentityStoreData.setAlias( ServerUtils.getStringValue( value ) );
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                    {
                                        IIdentityStoreDataEx storeData = getIIdentityStoreDataEx( object );
                                        return ServerUtils.getLdapValue( (storeData != null) ? storeData.getAlias() : null );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_PROVIDER_TYPE,
                                -1,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                    @Override
                                    public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                    {
                                        ServerIdentityStoreData serverIdentityStoreData =
                                                getServerIdentityStoreData(object);

                                        serverIdentityStoreData.setProviderType(
                                                IdentityStoreType.valueOf( ServerUtils.getStringValue( value ).toUpperCase() )
                                        );
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                    {
                                        IIdentityStoreDataEx storeData = getIIdentityStoreDataEx( object );
                                        return ServerUtils.getLdapValue(
                                                (storeData != null) ? storeData.getProviderType().toString() : null );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_AUTHENTICATION_TYPE,
                                -1,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                    @Override
                                    public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                    {
                                        ServerIdentityStoreData serverIdentityStoreData =
                                                getServerIdentityStoreData(object);

                                        serverIdentityStoreData.setAuthenticationType(
                                                AuthenticationType.valueOf(
                                                        ServerUtils.getStringValue( value ).toUpperCase() )
                                        );
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                    {
                                        IIdentityStoreDataEx storeData = getIIdentityStoreDataEx( object );
                                        return ServerUtils.getLdapValue(
                                                (storeData != null) ? storeData.getAuthenticationType().toString() : null );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_FRIENDLY_NAME,
                                -1,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                    @Override
                                    public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                    {
                                        ServerIdentityStoreData serverIdentityStoreData =
                                                getServerIdentityStoreData(object);

                                        serverIdentityStoreData.setFriendlyName(
                                                ServerUtils.getStringValue( value )
                                        );
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                    {
                                        IIdentityStoreDataEx storeData = getIIdentityStoreDataEx( object );
                                        return ServerUtils.getLdapValue(
                                                (storeData != null) ? storeData.getFriendlyName() : null );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_SEARCH_TIMEOUT,
                                -1,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                    @Override
                                    public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                    {
                                        ServerIdentityStoreData serverIdentityStoreData =
                                                getServerIdentityStoreData(object);

                                        Integer integerValue = ServerUtils.getIntegerValue( value );
                                        if(integerValue != null)
                                        {
                                            serverIdentityStoreData.setSearchTimeoutSeconds( integerValue.intValue() );
                                        }
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                    {
                                        IIdentityStoreDataEx storeData = getIIdentityStoreDataEx(object);
                                        return (
                                                (storeData != null)
                                                ?
                                                ServerUtils.getLdapValue(storeData.getSearchTimeoutSeconds())
                                                : null
                                        );
                                   }
                                }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_USER_NAME,
                                -1,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                    @Override
                                    public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                    {
                                        ServerIdentityStoreData serverIdentityStoreData =
                                                getServerIdentityStoreData(object);

                                        serverIdentityStoreData.setUserName( ServerUtils.getStringValue( value ) );
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                    {
                                        IIdentityStoreDataEx storeData = getIIdentityStoreDataEx( object );
                                        return ServerUtils.getLdapValue(
                                                (storeData != null) ? storeData.getUserName() : null );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_USE_MACHINE_ACCT,
                                -1,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                    @Override
                                    public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                    {
                                        ServerIdentityStoreData serverIdentityStoreData =
                                                getServerIdentityStoreData(object);

                                        serverIdentityStoreData.setUseMachineAccount( ServerUtils.getBooleanValue( value ) );
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                    {
                                        IIdentityStoreDataEx storeData = getIIdentityStoreDataEx( object );
                                        return ServerUtils.getLdapValue(
                                                (storeData != null) ? storeData.useMachineAccount() ? "true" : "false" : null );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_SPN_NAME,
                                -1,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                    @Override
                                    public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                    {
                                        ServerIdentityStoreData serverIdentityStoreData =
                                                getServerIdentityStoreData(object);

                                        serverIdentityStoreData.setServicePrincipalName( ServerUtils.getStringValue( value ) );
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                    {
                                        IIdentityStoreDataEx storeData = getIIdentityStoreDataEx( object );
                                        return ServerUtils.getLdapValue(
                                                (storeData != null) ? storeData.getServicePrincipalName() : null );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_PASSWORD,
                                -1,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                    @Override
                                    public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                    {
                                        ServerIdentityStoreData serverIdentityStoreData =
                                                getServerIdentityStoreData(object);

                                        serverIdentityStoreData.setPassword( ServerUtils.getStringValue( value ) );

                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                    {
                                        IIdentityStoreDataEx storeData = getIIdentityStoreDataEx( object );
                                        return ServerUtils.getLdapValue(
                                                (storeData != null) ? storeData.getPassword() : null );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_CONNECTION_STR_LIST,
                                -1,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                    @Override
                                    public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                    {
                                        ServerIdentityStoreData serverIdentityStoreData =
                                                getServerIdentityStoreData(object);

                                        serverIdentityStoreData.setConnectionStrings(
                                                ServerUtils.getMultiStringValueAsCollection( value )
                                        );

                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                    {
                                        IIdentityStoreDataEx storeData = getIIdentityStoreDataEx( object );
                                        return ServerUtils.getLdapValue(
                                                (storeData != null) ? storeData.getConnectionStrings() : null );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_AUTHN_TYPES,
                                -1,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                    @Override
                                    public void SetLdapValue(IIdentityStoreData object, LdapValue[] value) {
                                        if(value != null && value.length > 0) {
                                            ServerIdentityStoreData serverIdentityStoreData =
                                                    getServerIdentityStoreData(object);

                                            serverIdentityStoreData.setAuthnTypes(
                                                    ServerUtils.getMultiIntValue(value)
                                            );
                                        }
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IIdentityStoreData object) {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        IIdentityStoreDataEx storeData = getIIdentityStoreDataEx( object );
                                        return ServerUtils.getLdapValue(
                                                (storeData != null) ? storeData.getAuthnTypes() : null );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                              PROPERTY_TRUSTED_CERTIFICATES,
                              -1,
                              true,
                              new IPropertyGetterSetter<IIdentityStoreData>() {
                                 @Override
                                 public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                 {
                                    ServerIdentityStoreData serverIdentityStoreData =
                                          getServerIdentityStoreData(object);
                                    if (value != null)
                                    {
                                       serverIdentityStoreData.setCertificates(ServerUtils
                                             .getX509CertificateValues(value));
                                    }
                                 }
                                 @Override
                                 public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                 {
                                     IIdentityStoreDataEx storeData = getIIdentityStoreDataEx(object);
                                     if (storeData != null && storeData.getCertificates() != null) {
                                         ArrayList<X509Certificate> certs = new ArrayList<>(storeData.getCertificates());
                                         return ServerUtils.getLdapValue(certs);
                                     }
                                     return null;
                                 }
                              }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                              PROPERTY_UPN_SUFFIXES,
                              -1,
                              true,
                              new IPropertyGetterSetter<IIdentityStoreData>() {
                                 @Override
                                 public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                 {
                                    ServerIdentityStoreData serverIdentityStoreData = getServerIdentityStoreData(object);

                                    serverIdentityStoreData.setUpnSuffixes(ServerUtils.getMultiStringValueAsCollection(value));
                                 }
                                 @Override
                                 public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                 {
                                    IIdentityStoreDataEx storeData = getIIdentityStoreDataEx(object);
                                    return ServerUtils.getLdapValue(storeData == null? null : storeData.getUpnSuffixes());
                                 }
                              }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_USER_BASEDN,
                                -1,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                    @Override
                                    public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                    {
                                        ServerIdentityStoreData serverIdentityStoreData =
                                                getServerIdentityStoreData(object);

                                        serverIdentityStoreData.setUserBaseDn(
                                                ServerUtils.getStringValue( value )
                                        );
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                    {
                                        IIdentityStoreDataEx storeData = getIIdentityStoreDataEx( object );
                                        return ServerUtils.getLdapValue(
                                                (storeData != null) ? storeData.getUserBaseDn() : null );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_GROUP_BASEDN,
                                -1,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                    @Override
                                    public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                    {
                                        ServerIdentityStoreData serverIdentityStoreData =
                                                getServerIdentityStoreData(object);

                                        serverIdentityStoreData.setGroupBaseDn(
                                                ServerUtils.getStringValue( value )
                                        );
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                    {
                                        IIdentityStoreDataEx storeData = getIIdentityStoreDataEx( object );
                                        return ServerUtils.getLdapValue(
                                                (storeData != null) ? storeData.getGroupBaseDn() : null );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IIdentityStoreData>(
                                PROPERTY_FLAGS,
                                -1,
                                true,
                                new IPropertyGetterSetter<IIdentityStoreData>() {
                                   @Override
                                   public void SetLdapValue(IIdentityStoreData object, LdapValue[] value)
                                   {
                                      ServerIdentityStoreData serverIdentityStoreData = getServerIdentityStoreData(object);

                                      Integer flags = ServerUtils.getIntegerValue(value);
                                      if (flags != null)
                                      {
                                          serverIdentityStoreData.setFlags(flags.intValue());
                                      }
                                   }
                                   @Override
                                   public LdapValue[] GetLdapValue(IIdentityStoreData object)
                                   {
                                      IIdentityStoreDataEx storeData = getIIdentityStoreDataEx(object);
                                      return ServerUtils.getLdapValue(storeData == null ? null : storeData.getFlags());
                                   }
                                }
                          )
                 }
                );
    }

    @Override
    protected ServerIdentityStoreData createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 2) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        return new ServerIdentityStoreData(
                DomainType.valueOf( ServerUtils.getStringValue(ctorParams.get(0)).toUpperCase()),
                ServerUtils.getStringValue( ctorParams.get(1) )
        );
    }

    private static IIdentityStoreDataEx getIIdentityStoreDataEx(IIdentityStoreData object)
    {
        if( object == null )
        {
            throw new IllegalArgumentException("object");
        }

        return object.getExtendedIdentityStoreData();
    }

    private static ServerIdentityStoreData getServerIdentityStoreData(IIdentityStoreData object)
    {
        if(object instanceof ServerIdentityStoreData )
        {
            return (ServerIdentityStoreData)object;
        }
        else
        {
            throw new IllegalArgumentException( "object" );
        }
    }
}

final class IdentityProviderAlias
{
    private final String _providerDn;
    private final String _providerAlias;

    IdentityProviderAlias(String providerDn, String providerAlias)
    {
        ValidateUtil.validateNotEmpty( providerDn, "providerName" );
        ValidateUtil.validateNotEmpty( providerAlias, "providerAlias" );

        this._providerDn = providerDn;
        this._providerAlias = providerAlias;
    }

    public String getProviderDn()
    {
        return this._providerDn;
    }

    public String getProviderAlias()
    {
        return this._providerAlias;
    }
}

final class IdentityProviderAliasLdapObject extends BaseLdapObject<IdentityProviderAlias>
{
    private static IdentityProviderAliasLdapObject _instance = new IdentityProviderAliasLdapObject();
    public static IdentityProviderAliasLdapObject getInstance() { return _instance; }

    private static final String OBJECT_CLASS = "vmwSTSIdentityStoreAlias";

    public static final String PROPERTY_PROVIDER_ALIAS = CN;
    public static final String PROPERTY_PROVIDER_DN = "aliasedObjectName";

    @SuppressWarnings("unchecked")
    private IdentityProviderAliasLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<IdentityProviderAlias>(
                                PROPERTY_PROVIDER_DN,
                                0,
                                true,
                                new IPropertyGetterSetter<IdentityProviderAlias>() {
                                    @Override
                                    public void SetLdapValue(IdentityProviderAlias object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IdentityProviderAlias object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        return ServerUtils.getLdapValue( object.getProviderDn() );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IdentityProviderAlias>(
                                PROPERTY_PROVIDER_ALIAS,
                                1,
                                true,
                                new IPropertyGetterSetter<IdentityProviderAlias>() {
                                    @Override
                                    public void SetLdapValue(IdentityProviderAlias object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IdentityProviderAlias object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        return ServerUtils.getLdapValue( object.getProviderAlias() );
                                    }
                                },
                                false // cannot update in ldap
                        )
                }
        );
    }

    @Override
    protected IdentityProviderAlias createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 2) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        return new IdentityProviderAlias(
                ServerUtils.getStringValue( ctorParams.get(0) ),
                ServerUtils.getStringValue(ctorParams.get(1))
        );
    }
}

final class ContainerLdapObject extends BaseLdapObject<String>
{
    private static ContainerLdapObject _instance = new ContainerLdapObject();
    public static ContainerLdapObject getInstance() { return _instance; }

    private static final String OBJECT_CLASS = "container";

    public static final String CONTAINER_SERVICES = "Services";
    public static final String CONTAINER_IDENTITY_MANAGER = "IdentityManager";
    public static final String CONTAINER_ATTRIBUTES = "Attributes";
    public static final String CONTAINER_ATRIBUTE_MAP = "AttributesMap";
    public static final String CONTAINER_IDENTITY_STORE_SCHEMA_MAPPING = "SchemaMapping";
    public static final String CONTAINER_IDM_CERTIFICATES = "IdmCertificates";
    public static final String CONTAINER_RELYING_PARTIES = "RelyingParties";
    public static final String CONTAINER_IDENTITY_PROVIDERS = "IdentityProviders";
    public static final String CONTAINER_ASSERTION_COMSUMER_SERVICES = "AssertionConsumerServices";
    public static final String CONTAINER_ATTRIBUTE_COMSUMER_SERVICES = "AttributeConsumerServices";
    public static final String CONTAINER_EXTERNAL_IDP_CONFIGS = "ExternalIdpConfigs";
    public static final String CONTAINER_SINGLE_LOGOUT_SERVICES = "SingleLogoutServices";
    public static final String CONTAINER_SIGNATURE_ALGORITHMS_SERVICES = "SignatureAlgorithms";
    public static final String CONTAINER_TRUSTED_CERTIFICATE_CHAINS = "TrustedCertificateChains";
    public static final String CONTAINER_OIDC_CLIENTS = "OIDCClients";
    public static final String CONTAINER_RESOURCE_SERVERS = "ResourceServers";

    //ExternalIdp containers
    public static final String CONTAINER_EXTERNAL_IDP_SSO_SERVICES = "SSOServices";
    public static final String CONTAINER_EXTERNAL_IDP_SLO_SERVICES = "SLOServices";
    public static final String CONTAINER_EXTERNAL_IDP_CERTIFICATE_CHAINS = "TrustedCertificateChains";
    public static final String CONTAINER_EXTERNAL_IDP_SUBJECT_FORMAT_MAPPINGS = "SubjectFormatMap";
    public static final String CONTAINER_EXTERNAL_IDP_GROUP_ATTRIBUTE_MAPPINGS = "ClaimGroupMap";

    // Client Certificate Policy container
    public static final String CONTAINER_CLIENT_CERT_POLICIES = "ClientCertificatePolicies";

    // Alternative OCSP lists container
    public static final String CONTAINER_CLIENT_CERT_OCSP_LISTS = "ClientCertificateOcspLists";

    // Alternative OCSP container
    public static final String CONTAINER_OCSPs = "SiteOcspResponders";

    //RSA agent configuration containers
    public static final String CONTAINER_RSA_CONFIGURATIONS = "RSAAgentConfigurations";
    public static final String CONTAINER_RSA_INSTANCES = "RSAAMInstances";
    public static final String CONTAINER_RSA_IDS_USERID_ATTRIBUTE_MAPS = "RSAAgentIDSAtributeMaps";

    @SuppressWarnings("unchecked")
    private ContainerLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<String>(
                                CN,
                                0,
                                true,
                                new IPropertyGetterSetter<String>() {
                                    @Override
                                    public void SetLdapValue(String object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(String object)
                                    {
                                        return ServerUtils.getLdapValue( object );
                                    }
                                },
                                false // cannot update in ldap
                        )
                }
        );
    }

    @Override
    protected String createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 1) )
        {
            throw new IllegalArgumentException("ctorParams");
        }
        return ServerUtils.getStringValue(ctorParams.get(0));
    }
}

final class TenantsContainerLdapObject extends BaseLdapObject<String>
{
    private static TenantsContainerLdapObject _instance = new TenantsContainerLdapObject();
    public static TenantsContainerLdapObject getInstance() { return _instance; }

    private static final String OBJECT_CLASS = "vmwSTSTenantsContainer";

    public static final String CONTAINER_TENANTS = "Tenants";

    //public static final String PROPERTY_DEFAULT_TENANT = "DEFAULT_TENANT";
    public static final String PROPERTY_NAME = CN;
    public static final String PROPERTY_DEFAULT_TENANT = "vmwSTSDefaultTenant";
    public static final String PROPERTY_SYSTEM_TENANT = "vmwSTSSystemTenant";

    @SuppressWarnings("unchecked")
    private TenantsContainerLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<String>(
                                PROPERTY_NAME,
                                0,
                                true,
                                new IPropertyGetterSetter<String>() {
                                    @Override
                                    public void SetLdapValue(String object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(String object)
                                    {
                                        return ServerUtils.getLdapValue("Tenants");
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<String>(
                                PROPERTY_DEFAULT_TENANT,
                                -1,
                                false,
                                null
                        ),
                        new PropertyMapperMetaInfo<String>(
                                PROPERTY_SYSTEM_TENANT,
                                -1,
                                false,
                                null
                        )
                }
        );
    }

    @Override
    protected String createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 1) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        return ServerUtils.getStringValue(ctorParams.get(0));
    }
}

final class AttributeMapping
{
    private final String _attributeFrom;
    private final String _attributeTo;
    private final int _index;

    public AttributeMapping( String attributeFrom, String attributeTo, int index )
    {
        ValidateUtil.validateNotEmpty( attributeFrom, "attributeFrom" );
        ValidateUtil.validateNotEmpty( attributeTo, "attributeTo" );
        ValidateUtil.validateNonNegativeNumber( index, "index" );

        this._attributeFrom = attributeFrom;
        this._attributeTo = attributeTo;
        this._index = index;
    }

    public String getAttributeFrom()
    {
        return this._attributeFrom;
    }

    public String getAttributeTo()
    {
        return this._attributeTo;
    }

    public int getIndex()
    {
        return this._index;
    }
}

final class AttributeMappingLdapObject extends BaseLdapObject<AttributeMapping>
{
    private static AttributeMappingLdapObject _instance = new AttributeMappingLdapObject();
    public static AttributeMappingLdapObject getInstance() { return _instance; }

    private static final String OBJECT_CLASS = "vmwSTSAttributeMap";

    public static final String PROPERTY_ATTRIBUTE_FROM = "vmwSTSMapKey";
    public static final String PROPERTY_ATTRIBUTE_TO = "vmwSTSMapValue";

    @SuppressWarnings("unchecked")
    private AttributeMappingLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<AttributeMapping>(
                                CN,
                                -1,
                                true,
                                new IPropertyGetterSetter<AttributeMapping>() {
                                    @Override
                                    public void SetLdapValue(AttributeMapping object, LdapValue[] value)
                                    {
                                        // no op
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(AttributeMapping object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        return ServerUtils.getLdapValue(
                                            String.format("AttributeMapping-%d", object.getIndex() )
                                        );
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<AttributeMapping>(
                                PROPERTY_ATTRIBUTE_FROM,
                                0,
                                true,
                                new IPropertyGetterSetter<AttributeMapping>() {
                                    @Override
                                    public void SetLdapValue(AttributeMapping object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(AttributeMapping object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        return ServerUtils.getLdapValue(object.getAttributeFrom());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<AttributeMapping>(
                                PROPERTY_ATTRIBUTE_TO,
                                1,
                                true,
                                new IPropertyGetterSetter<AttributeMapping>() {
                                    @Override
                                    public void SetLdapValue(AttributeMapping object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(AttributeMapping object)
                                    {
                                        ValidateUtil.validateNotNull( object, "object" );
                                        return ServerUtils.getLdapValue(object.getAttributeTo());
                                    }
                                }
                        )
                }
        );
    }

    @Override
    protected AttributeMapping createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 2) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        return new AttributeMapping(
                ServerUtils.getStringValue( ctorParams.get(0) ),
                ServerUtils.getStringValue( ctorParams.get(1) ),
                0 // it is not saved or read from outside
        );
    }
}

final class RelyingPartyLdapObject extends BaseLdapObject<RelyingParty>
{
    private static RelyingPartyLdapObject _instance = new RelyingPartyLdapObject();
    public static RelyingPartyLdapObject getInstance() { return _instance; }

    private static final String OBJECT_CLASS = "vmwSTSRelyingParty";

    public static final String PROPERTY_NAME = CN;
    private static final String NAME_ATTRIBUTE = "name";
    public static final String PROPERTY_URL = "vmwSTSRelyingPartyURL";
    public static final String PROPERTY_CERTIFICATE = "userCertificate";
    // unused for now, but will be used in the future
    //private static final String PROPERTY_DIGEST_METHODS = "vmwSTSDigestMethods";
    //private static final String PROPERTY_NAME_ID_FORMAT = "vmwSTSNameIDFormat";
    //private static final String PROPERTY_ONE_TIME_USE = "vmwSTSOneTimeUse";
    public static final String PROPERTY_DEFAULT_ASSERTION_COMSUMER_SERVICE = "vmwSTSDefaultAssertionConsumerService";
    public static final String PROPERTY_DEFAULT_ATTRIBUTE_COMSUMER_SERVICE = "vmwSTSDefaultAttributeConsumerService";
    public static final String PROPERTY_AUTHN_REQUESTS_SIGNED = "vmwSTSAuthnRequestsSigned";

    @SuppressWarnings("unchecked")
    private RelyingPartyLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<RelyingParty>(
                                PROPERTY_NAME,
                                0,
                                true,
                                new IPropertyGetterSetter<RelyingParty>() {
                                    @Override
                                    public void SetLdapValue(RelyingParty object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(RelyingParty relyingParty)
                                    {
                                        ValidateUtil.validateNotNull( relyingParty, "relyingParty" );
                                        return ServerUtils.getLdapValue( relyingParty.getName() );
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<RelyingParty>(
                                NAME_ATTRIBUTE,
                                -1,
                                true,
                                new IPropertyGetterSetter<RelyingParty>() {
                                    @Override
                                    public void SetLdapValue(RelyingParty object, LdapValue[] value)
                                    {
                                        // no op
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(RelyingParty relyingParty)
                                    {
                                        ValidateUtil.validateNotNull( relyingParty, "relyingParty" );
                                        return ServerUtils.getLdapValue( relyingParty.getName() );
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<RelyingParty>(
                                PROPERTY_URL,
                                -1,
                                true,
                                new IPropertyGetterSetter<RelyingParty>() {
                                    @Override
                                    public void SetLdapValue(RelyingParty relyingParty, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( relyingParty, "relyingParty" );
                                        relyingParty.setUrl( ServerUtils.getStringValue( value ));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(RelyingParty relyingParty)
                                    {
                                        ValidateUtil.validateNotNull( relyingParty, "relyingParty" );
                                        return ServerUtils.getLdapValue(relyingParty.getUrl());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<RelyingParty>(
                                PROPERTY_CERTIFICATE,
                                -1,
                                true,
                                new IPropertyGetterSetter<RelyingParty>() {
                                    @Override
                                    public void SetLdapValue(RelyingParty relyingParty, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( relyingParty, "relyingParty" );
                                        relyingParty.setCertificate( ServerUtils.getCertificateValue( value ));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(RelyingParty relyingParty)
                                    {
                                        ValidateUtil.validateNotNull( relyingParty, "relyingParty" );
                                        return ServerUtils.getLdapValue( relyingParty.getCertificate() );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<RelyingParty>(
                                PROPERTY_DEFAULT_ASSERTION_COMSUMER_SERVICE,
                                -1,
                                true,
                                new IPropertyGetterSetter<RelyingParty>() {
                                    @Override
                                    public void SetLdapValue(RelyingParty relyingParty, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( relyingParty, "relyingParty" );
                                        relyingParty.setDefaultAssertionConsumerService(
                                            ServerUtils.getStringValue( value ));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(RelyingParty relyingParty)
                                    {
                                        ValidateUtil.validateNotNull( relyingParty, "relyingParty" );
                                        return ServerUtils.getLdapValue(
                                            relyingParty.getDefaultAssertionConsumerService() );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<RelyingParty>(
                                PROPERTY_DEFAULT_ATTRIBUTE_COMSUMER_SERVICE,
                                -1,
                                true,
                                new IPropertyGetterSetter<RelyingParty>() {
                                    @Override
                                    public void SetLdapValue(RelyingParty relyingParty, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( relyingParty, "relyingParty" );
                                        relyingParty.setDefaultAttributeConsumerService(
                                                ServerUtils.getStringValue( value )
                                        );
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(RelyingParty relyingParty)
                                    {
                                        ValidateUtil.validateNotNull( relyingParty, "relyingParty" );
                                        return ServerUtils.getLdapValue(
                                                relyingParty.getDefaultAttributeConsumerService() );
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<RelyingParty>(
                                PROPERTY_AUTHN_REQUESTS_SIGNED,
                                -1,
                                true,
                                new IPropertyGetterSetter<RelyingParty>() {
                                    @Override
                                    public void SetLdapValue(RelyingParty relyingParty, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( relyingParty, "relyingParty" );
                                        Integer ldapval = ServerUtils.getIntegerValue(value);
                                        relyingParty.setAuthnRequestsSigned(
                                                ldapval != null && ldapval != 0);
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(RelyingParty relyingParty)
                                    {
                                        ValidateUtil.validateNotNull( relyingParty, "relyingParty" );
                                        return ServerUtils.getLdapValue(
                                                relyingParty.isAuthnRequestsSigned() ? 1 : 0);
                                    }
                                }
                        )
                }
        );
    }

    @Override
    protected RelyingParty createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 1) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        return new RelyingParty( ServerUtils.getStringValue(ctorParams.get(0)) );
    }
}

final class AssertionConsumerServiceLdapObject extends BaseLdapObject<AssertionConsumerService>
{
    private static AssertionConsumerServiceLdapObject _instance = new AssertionConsumerServiceLdapObject();
    public static AssertionConsumerServiceLdapObject getInstance() { return _instance; }

    private static final String OBJECT_CLASS = "vmwSTSAssertionConsumerService";

    public static final String PROPERTY_NAME = CN;
    public static final String PROPERTY_BINDING = "vmwSTSBinding";
    public static final String PROPERTY_INDEX = "vmwSTSIndex";
    public static final String PROPERTY_ENDPOINT = "vmwSTSEndpoint";

    @SuppressWarnings("unchecked")
    private AssertionConsumerServiceLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<AssertionConsumerService>(
                                PROPERTY_NAME,
                                0,
                                true,
                                new IPropertyGetterSetter<AssertionConsumerService>() {
                                    @Override
                                    public void SetLdapValue(AssertionConsumerService object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(AssertionConsumerService assertionConsumerService)
                                    {
                                        ValidateUtil.validateNotNull( assertionConsumerService, "assertionConsumerService" );
                                        return ServerUtils.getLdapValue( assertionConsumerService.getName() );
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<AssertionConsumerService>(
                                PROPERTY_ENDPOINT,
                                -1,
                                true,
                                new IPropertyGetterSetter<AssertionConsumerService>() {
                                    @Override
                                    public void SetLdapValue(AssertionConsumerService assertionConsumerService, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( assertionConsumerService, "assertionConsumerService" );
                                        assertionConsumerService.setEndpoint( ServerUtils.getStringValue( value ));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(AssertionConsumerService assertionConsumerService)
                                    {
                                        ValidateUtil.validateNotNull( assertionConsumerService, "assertionConsumerService" );
                                        return ServerUtils.getLdapValue(assertionConsumerService.getEndpoint());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<AssertionConsumerService>(
                                PROPERTY_BINDING,
                                -1,
                                true,
                                new IPropertyGetterSetter<AssertionConsumerService>() {
                                    @Override
                                    public void SetLdapValue(AssertionConsumerService assertionConsumerService, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( assertionConsumerService, "assertionConsumerService" );
                                        assertionConsumerService.setBinding( ServerUtils.getStringValue( value ));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(AssertionConsumerService assertionConsumerService)
                                    {
                                        ValidateUtil.validateNotNull( assertionConsumerService, "assertionConsumerService" );
                                        return ServerUtils.getLdapValue(assertionConsumerService.getBinding());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<AssertionConsumerService>(
                                PROPERTY_INDEX,
                                -1,
                                true,
                                new IPropertyGetterSetter<AssertionConsumerService>() {
                                    @Override
                                    public void SetLdapValue(AssertionConsumerService assertionConsumerService, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( assertionConsumerService, "assertionConsumerService" );
                                        assertionConsumerService.setIndex( ServerUtils.getIntValue( value ));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(AssertionConsumerService assertionConsumerService)
                                    {
                                        ValidateUtil.validateNotNull( assertionConsumerService, "assertionConsumerService" );
                                        return ServerUtils.getLdapValue(assertionConsumerService.getIndex());
                                    }
                                }
                        )
                }
        );
    }

    @Override
    protected AssertionConsumerService createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 1) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        return new AssertionConsumerService( ServerUtils.getStringValue(ctorParams.get(0)) );
    }
}

final class AttributeConsumerServiceLdapObject extends BaseLdapObject<AttributeConsumerService>
{
    private static AttributeConsumerServiceLdapObject _instance = new AttributeConsumerServiceLdapObject();
    public static AttributeConsumerServiceLdapObject getInstance() { return _instance; }

    private static final String OBJECT_CLASS = "vmwSTSAttributeConsumerService";

    public static final String PROPERTY_NAME = CN;
    public static final String PROPERTY_INDEX = "vmwSTSIndex";

    @SuppressWarnings("unchecked")
    private AttributeConsumerServiceLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<AttributeConsumerService>(
                                PROPERTY_NAME,
                                0,
                                true,
                                new IPropertyGetterSetter<AttributeConsumerService>() {
                                    @Override
                                    public void SetLdapValue(AttributeConsumerService object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(AttributeConsumerService attributeConsumerService)
                                    {
                                        ValidateUtil.validateNotNull( attributeConsumerService, "attributeConsumerService" );
                                        return ServerUtils.getLdapValue( attributeConsumerService.getName() );
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<AttributeConsumerService>(
                                PROPERTY_INDEX,
                                -1,
                                true,
                                new IPropertyGetterSetter<AttributeConsumerService>() {
                                    @Override
                                    public void SetLdapValue(AttributeConsumerService attributeConsumerService, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( attributeConsumerService, "attributeConsumerService" );
                                        attributeConsumerService.setIndex( ServerUtils.getIntValue( value ));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(AttributeConsumerService attributeConsumerService)
                                    {
                                        ValidateUtil.validateNotNull( attributeConsumerService, "attributeConsumerService" );
                                        return ServerUtils.getLdapValue(attributeConsumerService.getIndex());
                                    }
                                }
                        )
                }
        );
    }

    @Override
    protected AttributeConsumerService createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 1) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        return new AttributeConsumerService( ServerUtils.getStringValue(ctorParams.get(0)) );
    }
}

final class IDPConfigLdapObject extends BaseLdapObject<IDPConfig>
{
    private static IDPConfigLdapObject _instance = new IDPConfigLdapObject();
    public static IDPConfigLdapObject getInstance() { return _instance; }

    private static final String OBJECT_CLASS = "vmwSTSExternalIdp";

    public static final String PROPERTY_NAME = CN;
    public static final String PROPERTY_ENTITY_ID = "vmwSTSEntityId";
    public static final String PROPERTY_ALIAS = "vmwSTSAlias";
    public static final String PROPERTY_NAME_ID_FORMAT = "vmwSTSNameIDFormat";
    public static final String PROPERTY_JIT_FORMAT = "vmwSTSExternalIdpEnableJit";
    public static final String PROPERTY_UPN_SUFFIX = "vmwSTSExternalIdpUpnSuffix";

    @SuppressWarnings("unchecked")
    private IDPConfigLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[] {
                        new PropertyMapperMetaInfo<IDPConfig>(
                                PROPERTY_NAME,
                                0,
                                true,
                                new IPropertyGetterSetter<IDPConfig>() {
                                    @Override
                                    public void SetLdapValue(IDPConfig idpConfig, LdapValue[] value)
                                    {
                                        throw new UnsupportedOperationException("property cn is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IDPConfig idpConfig)
                                    {
                                        ValidateUtil.validateNotNull(idpConfig, "config");
                                        return ServerUtils.getLdapValue(idpConfig.getEntityID());
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<IDPConfig>(
                                PROPERTY_ENTITY_ID,
                                -1,
                                true,
                                new IPropertyGetterSetter<IDPConfig>() {
                                    @Override
                                    public void SetLdapValue(IDPConfig idpConfig, LdapValue[] value)
                                    {
                                        //no-Op
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IDPConfig config)
                                    {
                                        ValidateUtil.validateNotNull(config, "config");
                                        return ServerUtils.getLdapValue(config.getEntityID());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IDPConfig>(
                                PROPERTY_ALIAS,
                                -1,
                                true,
                                new IPropertyGetterSetter<IDPConfig>() {
                                    @Override
                                    public void SetLdapValue(IDPConfig idpConfig, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull(idpConfig, "idpConfig");
                                        idpConfig.setAlias(ServerUtils.getStringValue(value));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IDPConfig config)
                                    {
                                        ValidateUtil.validateNotNull(config, "config");
                                        return ServerUtils.getLdapValue(config.getAlias());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IDPConfig>(
                                PROPERTY_NAME_ID_FORMAT,
                                -1,
                                true,
                                new IPropertyGetterSetter<IDPConfig>() {
                                    @Override
                                    public void SetLdapValue(IDPConfig idpConfig, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull(idpConfig, "idpConfig");
                                        idpConfig.setNameIDFormats(ServerUtils.getMultiStringValueAsCollection(value));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IDPConfig config)
                                    {
                                        ValidateUtil.validateNotNull(config, "config");
                                        return ServerUtils.getLdapValue(config.getNameIDFormats());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IDPConfig>(
                                PROPERTY_JIT_FORMAT,
                                -1,
                                true,
                                new IPropertyGetterSetter<IDPConfig>() {
                                    @Override
                                    public void SetLdapValue(IDPConfig idpConfig, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull(idpConfig, "idpConfig");
                                        idpConfig.setJitAttribute(ServerUtils.getBooleanValue(value));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IDPConfig config)
                                    {
                                        ValidateUtil.validateNotNull(config, "config");
                                        return ServerUtils.getLdapValue(config.getJitAttribute());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IDPConfig>(
                                PROPERTY_UPN_SUFFIX,
                                -1,
                                true,
                                new IPropertyGetterSetter<IDPConfig>() {
                                    @Override
                                    public void SetLdapValue(IDPConfig idpConfig, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull(idpConfig, "idpConfig");
                                        idpConfig.setUpnSuffix(ServerUtils.getStringValue(value));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IDPConfig config)
                                    {
                                        ValidateUtil.validateNotNull(config, "config");
                                        return ServerUtils.getLdapValue(config.getUpnSuffix());
                                    }
                                }
                        )
                }
            );
    }

    /* (non-Javadoc)
     * @see com.vmware.identity.idm.server.config.directory.BaseLdapObject#createObject(java.util.List)
     */
    @Override
    protected IDPConfig createObject(List<LdapValue[]> ctorParams)
    {
        if (ctorParams == null || ctorParams.size() != 1)
        {
            throw new IllegalArgumentException("size of ctorParams needs to be 1 for class IDPConfig");
        }
        return new IDPConfig(ServerUtils.getStringValue(ctorParams.get(0)));
    }
}

final class SingleSignOnServiceLdapObject extends BaseLdapObject<ServiceEndpoint>
{
    private static SingleSignOnServiceLdapObject _instance = new SingleSignOnServiceLdapObject();
    public static SingleSignOnServiceLdapObject getInstance() { return _instance; }

    private static final String OBJECT_CLASS = "vmwSTSSingleSignOnService";

    public static final String PROPERTY_NAME = CN;
    public static final String PROPERTY_BINDING = "vmwSTSBinding";
    public static final String PROPERTY_ENDPOINT = "vmwSTSEndpoint";

    @SuppressWarnings("unchecked")
    private SingleSignOnServiceLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<ServiceEndpoint>(
                                PROPERTY_NAME,
                                0,
                                true,
                                new IPropertyGetterSetter<ServiceEndpoint>() {
                                    @Override
                                    public void SetLdapValue(ServiceEndpoint object, LdapValue[] value)
                                    {
                                        throw new UnsupportedOperationException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(ServiceEndpoint singleSignOnService)
                                    {
                                        ValidateUtil.validateNotNull( singleSignOnService, "singleSignOnService" );
                                        return ServerUtils.getLdapValue( singleSignOnService.getName() );
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<ServiceEndpoint>(
                                PROPERTY_ENDPOINT,    // mapped to 'location' field in object model
                                -1,
                                true,
                                new IPropertyGetterSetter<ServiceEndpoint>() {
                                    @Override
                                    public void SetLdapValue(ServiceEndpoint singleSignOnService, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( singleSignOnService, "singleSignOnService" );
                                        singleSignOnService.setEndpoint( ServerUtils.getStringValue( value ));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(ServiceEndpoint singleSignOnService)
                                    {
                                        ValidateUtil.validateNotNull( singleSignOnService, "singleSignOnService" );
                                        return ServerUtils.getLdapValue(singleSignOnService.getEndpoint());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<ServiceEndpoint>(
                                PROPERTY_BINDING,
                                -1,
                                true,
                                new IPropertyGetterSetter<ServiceEndpoint>() {
                                    @Override
                                    public void SetLdapValue(ServiceEndpoint singleSignOnService, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( singleSignOnService, "singleSignOnService" );
                                        singleSignOnService.setBinding( ServerUtils.getStringValue( value ));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(ServiceEndpoint singleSignOnService)
                                    {
                                        ValidateUtil.validateNotNull( singleSignOnService, "singleSignOnService" );
                                        return ServerUtils.getLdapValue(singleSignOnService.getBinding());
                                    }
                                }
                        ),
                }
        );
    }

    @Override
    protected ServiceEndpoint createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 1) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        return new ServiceEndpoint( ServerUtils.getStringValue(ctorParams.get(0)) );
    }
}

final class SingleLogoutServiceLdapObject extends BaseLdapObject<ServiceEndpoint>
{
    private static SingleLogoutServiceLdapObject _instance = new SingleLogoutServiceLdapObject();
    public static SingleLogoutServiceLdapObject getInstance() { return _instance; }

    private static final String OBJECT_CLASS = "vmwSTSSingleLogoutService";

    public static final String PROPERTY_NAME = CN;
    public static final String PROPERTY_BINDING = "vmwSTSBinding";
    public static final String PROPERTY_ENDPOINT = "vmwSTSEndpoint";
    public static final String PROPERTY_RESPONSE_ENDPOINT = "vmwSTSResponseEndpoint";

    @SuppressWarnings("unchecked")
    private SingleLogoutServiceLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<ServiceEndpoint>(
                                PROPERTY_NAME,
                                0,
                                true,
                                new IPropertyGetterSetter<ServiceEndpoint>() {
                                    @Override
                                    public void SetLdapValue(ServiceEndpoint object, LdapValue[] value)
                                    {
                                        throw new IllegalStateException( "property is not settable.");
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(ServiceEndpoint singleLogoutService)
                                    {
                                        ValidateUtil.validateNotNull( singleLogoutService, "singleLogoutService" );
                                        return ServerUtils.getLdapValue( singleLogoutService.getName() );
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<ServiceEndpoint>(
                                PROPERTY_ENDPOINT,
                                -1,
                                true,
                                new IPropertyGetterSetter<ServiceEndpoint>() {
                                    @Override
                                    public void SetLdapValue(ServiceEndpoint singleLogoutService, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( singleLogoutService, "singleLogoutService" );
                                        singleLogoutService.setEndpoint( ServerUtils.getStringValue( value ));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(ServiceEndpoint singleLogoutService)
                                    {
                                        ValidateUtil.validateNotNull( singleLogoutService, "singleLogoutService" );
                                        return ServerUtils.getLdapValue(singleLogoutService.getEndpoint());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<ServiceEndpoint>(
                                PROPERTY_RESPONSE_ENDPOINT,
                                -1,
                                true,
                                new IPropertyGetterSetter<ServiceEndpoint>() {
                                    @Override
                                    public void SetLdapValue(ServiceEndpoint singleLogoutService, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( singleLogoutService, "singleLogoutService" );
                                        singleLogoutService.setResponseEndpoint( ServerUtils.getStringValue( value ));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(ServiceEndpoint singleLogoutService)
                                    {
                                        ValidateUtil.validateNotNull( singleLogoutService, "singleLogoutService" );
                                        return ServerUtils.getLdapValue(singleLogoutService.getResponseEndpoint());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<ServiceEndpoint>(
                                PROPERTY_BINDING,
                                -1,
                                true,
                                new IPropertyGetterSetter<ServiceEndpoint>() {
                                    @Override
                                    public void SetLdapValue(ServiceEndpoint singleLogoutService, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( singleLogoutService, "singleLogoutService" );
                                        singleLogoutService.setBinding( ServerUtils.getStringValue( value ));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(ServiceEndpoint singleLogoutService)
                                    {
                                        ValidateUtil.validateNotNull( singleLogoutService, "singleLogoutService" );
                                        return ServerUtils.getLdapValue(singleLogoutService.getBinding());
                                    }
                                }
                        ),
                }
        );
    }

    @Override
    protected ServiceEndpoint createObject(List<LdapValue[]> ctorParams)
    {
        if ( (ctorParams == null) || (ctorParams.size() != 1) )
        {
            throw new IllegalArgumentException("ctorParams");
        }

        return new ServiceEndpoint( ServerUtils.getStringValue(ctorParams.get(0)) );
    }
}

final class SignatureAlgorithmsLdapObject extends BaseLdapObject<IndexedObjectWrapper<SignatureAlgorithm>>
{
    private static SignatureAlgorithmsLdapObject _instance = new SignatureAlgorithmsLdapObject();
    public static SignatureAlgorithmsLdapObject getInstance() { return _instance; }

    private static final String OBJECT_CLASS = "vmwSTSSignatureAlgorithm";

    public static final String PROPERTY_MAXIMUM_KEY_SIZE = "vmwSTSMaximumKeySize";
    public static final String PROPERTY_MINIMUM_KEY_SIZE = "vmwSTSMinimumKeySize";
    public static final String PROPERTY_PRIORITY = "vmwSTSPriority";

    @SuppressWarnings("unchecked")
    private SignatureAlgorithmsLdapObject()
    {
        super(
                OBJECT_CLASS,
                new PropertyMapperMetaInfo[]{
                        new PropertyMapperMetaInfo<IndexedObjectWrapper<SignatureAlgorithm>>(
                                CN,
                                -1,
                                true,
                                new IPropertyGetterSetter<IndexedObjectWrapper<SignatureAlgorithm>>() {
                                    @Override
                                    public void SetLdapValue(IndexedObjectWrapper<SignatureAlgorithm> object, LdapValue[] value)
                                    {
                                        // no-op
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IndexedObjectWrapper<SignatureAlgorithm> signatureAlgorithm)
                                    {
                                        ValidateUtil.validateNotNull( signatureAlgorithm, "signatureAlgorithm" );
                                        return ServerUtils.getLdapValue(
                                            String.format("SignatureAlgorithm-%d", signatureAlgorithm.getIndex() )
                                        );
                                    }
                                },
                                false // cannot update in ldap
                        ),
                        new PropertyMapperMetaInfo<IndexedObjectWrapper<SignatureAlgorithm>>(
                                PROPERTY_MAXIMUM_KEY_SIZE,
                                -1,
                                true,
                                new IPropertyGetterSetter<IndexedObjectWrapper<SignatureAlgorithm>>() {
                                    @Override
                                    public void SetLdapValue(IndexedObjectWrapper<SignatureAlgorithm> signatureAlgorithm, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( signatureAlgorithm, "signatureAlgorithm" );
                                        signatureAlgorithm.getWrappedObject().setMaximumKeySize(
                                            ServerUtils.getIntValue( value ));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IndexedObjectWrapper<SignatureAlgorithm> signatureAlgorithm)
                                    {
                                        ValidateUtil.validateNotNull( signatureAlgorithm, "signatureAlgorithm" );
                                        return ServerUtils.getLdapValue(
                                            signatureAlgorithm.getWrappedObject().getMaximumKeySize());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IndexedObjectWrapper<SignatureAlgorithm>>(
                                PROPERTY_MINIMUM_KEY_SIZE,
                                -1,
                                true,
                                new IPropertyGetterSetter<IndexedObjectWrapper<SignatureAlgorithm>>() {
                                    @Override
                                    public void SetLdapValue(IndexedObjectWrapper<SignatureAlgorithm> signatureAlgorithm, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( signatureAlgorithm, "signatureAlgorithm" );
                                        signatureAlgorithm.getWrappedObject().setMinimumKeySize(
                                                ServerUtils.getIntValue( value ));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IndexedObjectWrapper<SignatureAlgorithm> signatureAlgorithm)
                                    {
                                        ValidateUtil.validateNotNull( signatureAlgorithm, "signatureAlgorithm" );
                                        return ServerUtils.getLdapValue(
                                                signatureAlgorithm.getWrappedObject().getMinimumKeySize());
                                    }
                                }
                        ),
                        new PropertyMapperMetaInfo<IndexedObjectWrapper<SignatureAlgorithm>>(
                                PROPERTY_PRIORITY,
                                -1,
                                true,
                                new IPropertyGetterSetter<IndexedObjectWrapper<SignatureAlgorithm>>() {
                                    @Override
                                    public void SetLdapValue(IndexedObjectWrapper<SignatureAlgorithm> signatureAlgorithm, LdapValue[] value)
                                    {
                                        ValidateUtil.validateNotNull( signatureAlgorithm, "signatureAlgorithm" );
                                        signatureAlgorithm.getWrappedObject().setPriority(
                                                ServerUtils.getIntValue( value ));
                                    }
                                    @Override
                                    public LdapValue[] GetLdapValue(IndexedObjectWrapper<SignatureAlgorithm> signatureAlgorithm)
                                    {
                                        ValidateUtil.validateNotNull( signatureAlgorithm, "signatureAlgorithm" );
                                        return ServerUtils.getLdapValue(
                                                signatureAlgorithm.getWrappedObject().getPriority());
                                    }
                                }
                        )
                }
        );
}

    @Override
    protected IndexedObjectWrapper<SignatureAlgorithm> createObject(List<LdapValue[]> ctorParams)
    {
        return new IndexedObjectWrapper<SignatureAlgorithm>(
                new SignatureAlgorithm(),
                0
        );
    }
}
