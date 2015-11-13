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

import java.util.Collection;
import java.util.Map;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.IdentityStoreObjectMapping;
import com.vmware.identity.idm.IdentityStoreSchemaMapping;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.interop.ldap.LdapFilterString;

public abstract class BaseLdapSchemaMapping implements ILdapSchemaMapping
{
    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(BaseLdapSchemaMapping.class);

    private IdentityStoreSchemaMapping _schemaMapping;
    private Map<String, String> _defaultAttributes;
    private String _userQueryByAccountNameOrUpn;
    private String _userQueryByUpn;
    private String _userQueryByAccountName;
    private String _userQueryByObjectUniqueId;
    private String _userQueryByCriteria;
    private String _userQueryByCriteriaForName;
    private String _allUsersQuery;
    private String _allDisabledUsersQuery;
    private String _groupQueryByCriteria;
    private String _groupQueryByCriteriaForName;
    private String _allGroupsQuery;
    private String _directParentGroupsQuery;
    private String _nestedParentGroupsQuery;
    private String _groupQueryByObjectUniqueId;
    private String _groupQueryByAccountName;
    private String _userOrGroupQueryByAccountNameOrUpn;
    private String _userOrGroupQueryByAccountName;
    private String _passwordSettingsQuery;
    private String _domainObjectQuery;
    private Boolean _builtPasswordSettingsQuery;
    private Boolean _builtDomainObjectQuery;

    protected static final String DN_ATTRIBUTE = "dn";

    protected BaseLdapSchemaMapping( IdentityStoreSchemaMapping schemaMapping, Map<String, String> defaultAttributes )
    {
        ValidateUtil.validateNotNull(defaultAttributes, "defaultAttributes");
        this._schemaMapping = schemaMapping;
        this._defaultAttributes = defaultAttributes;
        this._userQueryByAccountNameOrUpn = null;
        this._userQueryByAccountName = null;
        this._userQueryByUpn = null;
        this._userQueryByObjectUniqueId = null;
        this._userQueryByCriteria = null;
        this._userQueryByCriteriaForName = null;
        this._allUsersQuery = null;
        this._allDisabledUsersQuery = null;
        this._groupQueryByCriteria = null;
        this._groupQueryByCriteriaForName = null;
        this._allGroupsQuery = null;
        this._directParentGroupsQuery = null;
        this._nestedParentGroupsQuery = null;
        this._groupQueryByObjectUniqueId = null;
        this._groupQueryByAccountName = null;
        this._userOrGroupQueryByAccountNameOrUpn = null;
        this._userOrGroupQueryByAccountName = null;
        this._passwordSettingsQuery = null;
        this._domainObjectQuery = null;
        this._builtPasswordSettingsQuery = false;
        this._builtDomainObjectQuery = false;
    }

    @Override
    public String getUserQueryByAccountNameOrUpn()
    {
        if(ServerUtils.isNullOrEmpty(this._userQueryByAccountNameOrUpn))
        {
            this._userQueryByAccountNameOrUpn = buildUserQueryByAccountNameOrUpn();
            log.debug(
                String.format("Built UserQueryByAccountNameOrUpn: [%s]", this._userQueryByAccountNameOrUpn)
            );
        }

        return this._userQueryByAccountNameOrUpn;
    }

    @Override
    public String getUserQueryByUpn()
    {
        if(ServerUtils.isNullOrEmpty(this._userQueryByUpn))
        {
            this._userQueryByUpn = buildUserQueryByUpn();
            log.debug(
                String.format("Built UserQueryByUpn: [%s]", this._userQueryByUpn)
            );
        }

        return this._userQueryByUpn;
    }

    @Override
    public String getUserQueryByAccountName()
    {
        if(ServerUtils.isNullOrEmpty(this._userQueryByAccountName))
        {
            this._userQueryByAccountName = buildUserQueryByAccountName();
            log.debug(
                String.format("Built UserQueryByAccountName: [%s]", this._userQueryByAccountName)
            );
        }

        return this._userQueryByAccountName;
    }

    @Override
    public String getUserQueryByObjectUniqueId()
    {
        if(ServerUtils.isNullOrEmpty(this._userQueryByObjectUniqueId))
        {
            this._userQueryByObjectUniqueId = buildUserQueryByObjectUniqueId();
            log.debug(
                String.format("Built UserQueryByObjectUniqueId: [%s]", this._userQueryByObjectUniqueId)
            );
        }

        return this._userQueryByObjectUniqueId;
    }

    @Override
    public String getUserQueryByCriteria()
    {
        if(ServerUtils.isNullOrEmpty(this._userQueryByCriteria))
        {
            this._userQueryByCriteria = buildUserQueryByCriteria();
            log.debug(
                String.format("Built UserQueryByCriteria: [%s]", this._userQueryByCriteria)
            );
        }

        return this._userQueryByCriteria;
    }

    @Override
    public String getUserQueryByCriteriaForName()
    {
        if(ServerUtils.isNullOrEmpty(this._userQueryByCriteriaForName))
        {
            this._userQueryByCriteriaForName = buildUserQueryByCriteriaForName();
            log.debug(
                String.format("Built UserQueryByCriteriaForName: [%s]", this._userQueryByCriteriaForName)
            );
        }

        return this._userQueryByCriteriaForName;
    }

    @Override
    public String getAllUsersQuery()
    {
        if(ServerUtils.isNullOrEmpty(this._allUsersQuery))
        {
            this._allUsersQuery = buildAllUsersQuery();
            log.debug(
                String.format("Built AllUsersQuery: [%s]", this._allUsersQuery)
            );
        }

        return this._allUsersQuery;
    }

    @Override
    public String getAllDisabledUsersQuery()
    {
        if(ServerUtils.isNullOrEmpty(this._allDisabledUsersQuery))
        {
            this._allDisabledUsersQuery = buildAllDisabledUsersQuery();
            log.debug(
                String.format("Built AllDisabledUsersQuery: [%s]", this._allDisabledUsersQuery)
            );
        }

        return this._allDisabledUsersQuery;
    }

    @Override
    public String getGroupQueryByCriteria()
    {
        if(ServerUtils.isNullOrEmpty(this._groupQueryByCriteria))
        {
            this._groupQueryByCriteria = buildGroupQueryByCriteria();
            log.debug(
                String.format("Built GroupQueryByCriteria: [%s]", this._groupQueryByCriteria)
            );
        }

        return this._groupQueryByCriteria;
    }

    @Override
    public String getGroupQueryByCriteriaForName()
    {
        if(ServerUtils.isNullOrEmpty(this._groupQueryByCriteriaForName))
        {
            this._groupQueryByCriteriaForName = buildGroupQueryByCriteriaForName();
            log.debug(
                String.format("Built GroupQueryByCriteriaForName: [%s]", this._groupQueryByCriteriaForName)
            );
        }

        return this._groupQueryByCriteriaForName;
    }

    @Override
    public String getAllGroupsQuery()
    {
        if(ServerUtils.isNullOrEmpty(this._allGroupsQuery))
        {
            this._allGroupsQuery = buildAllGroupsQuery();
            log.debug(
                String.format("Built AllGroupsQuery: [%s]", this._allGroupsQuery)
            );
        }

        return this._allGroupsQuery;
    }

    @Override
    public String getDirectParentGroupsQuery()
    {
        if(ServerUtils.isNullOrEmpty(this._directParentGroupsQuery))
        {
            this._directParentGroupsQuery = buildDirectParentGroupsQuery();
            log.debug(
                String.format("Built DirectParentGroupsQuery: [%s]", this._directParentGroupsQuery)
            );
        }

        return this._directParentGroupsQuery;
    }

    @Override
    public String getNestedParentGroupsQuery()
    {
        if(ServerUtils.isNullOrEmpty(this._nestedParentGroupsQuery))
        {
            this._nestedParentGroupsQuery = buildNestedParentGroupsQuery();
            log.debug(
                String.format("Built NestedParentGroupsQuery: [%s]", this._nestedParentGroupsQuery)
            );
        }

        return this._nestedParentGroupsQuery;
    }

    @Override
    public String getGroupQueryByObjectUniqueId()
    {
        if(ServerUtils.isNullOrEmpty(this._groupQueryByObjectUniqueId))
        {
            this._groupQueryByObjectUniqueId = buildGroupQueryByObjectUniqueId();
            log.debug(
                String.format("Built GroupQueryByObjectUniqueId: [%s]", this._groupQueryByObjectUniqueId)
            );
        }

        return this._groupQueryByObjectUniqueId;
    }

    @Override
    public String getGroupQueryByAccountName()
    {
        if(ServerUtils.isNullOrEmpty(this._groupQueryByAccountName))
        {
            this._groupQueryByAccountName = buildGroupQueryByAccountName();
            log.debug(
                String.format("Built GroupQueryByAccountName: [%s]", this._groupQueryByAccountName)
            );
        }

        return this._groupQueryByAccountName;
    }

    @Override
    public String getUserOrGroupQueryByAccountNameOrUpn()
    {
        if(ServerUtils.isNullOrEmpty(this._userOrGroupQueryByAccountNameOrUpn))
        {
            this._userOrGroupQueryByAccountNameOrUpn = buildUserOrGroupQueryByAccountNameOrUpn();
            log.debug(
                String.format("Built UserOrGroupQueryByAccountNameOrUpn: [%s]", this._userOrGroupQueryByAccountNameOrUpn)
            );
        }

        return this._userOrGroupQueryByAccountNameOrUpn;
    }

    @Override
    public String getUserOrGroupQueryByAccountName()
    {
        if(ServerUtils.isNullOrEmpty(this._userOrGroupQueryByAccountName))
        {
            this._userOrGroupQueryByAccountName = buildUserOrGroupQueryByAccountName();
            log.debug(
                String.format("Built UserOrGroupQueryByAccountName: [%s]", this._userOrGroupQueryByAccountName)
            );
        }

        return this._userOrGroupQueryByAccountName;
    }

    @Override
    public String getPasswordSettingsQuery()
    {
        if( this._builtPasswordSettingsQuery == false)
        {
            this._passwordSettingsQuery = buildPasswordSettingsQuery();
            this._builtPasswordSettingsQuery = true;
            log.debug(
                String.format("Built PasswordSettingsQuery: [%s]", this._passwordSettingsQuery)
            );
        }

        return this._passwordSettingsQuery;
    }

    @Override
    public String getDomainObjectQuery()
    {
        if( this._builtDomainObjectQuery == false)
        {
            this._domainObjectQuery = buildDomainObjectQuery();
            this._builtDomainObjectQuery = true;
            log.debug(
                String.format("Built DomainObjectQuery: [%s]", this._domainObjectQuery)
            );
        }

        return this._domainObjectQuery;
    }

    @Override
    public String getUserAttribute( String attribute )
    {
        return this.getAttribute(IdentityStoreObjectMapping.ObjectIds.ObjectIdUser, attribute);
    }

    @Override
    public String getGroupAttribute( String attribute )
    {
        return this.getAttribute(IdentityStoreObjectMapping.ObjectIds.ObjectIdGroup, attribute);
    }

    @Override
    public String getPwdObjectAttribute( String attribute )
    {
        return this.getAttribute(IdentityStoreObjectMapping.ObjectIds.ObjectIdPasswordSettings, attribute);
    }

    @Override
    public String getDomainObjectAttribute( String attribute )
    {
        return this.getAttribute(IdentityStoreObjectMapping.ObjectIds.ObjectIdDomain, attribute);
    }

    @Override
    public String getDNFilter( String filter, Collection<String> memberDNs )
    {
       StringBuilder dnListFilter = new StringBuilder();
       dnListFilter.append("(&");
       dnListFilter.append(filter);

       dnListFilter.append("(|");
       for (String memberDn : memberDNs)
       {
           dnListFilter.append("(distinguishedName=");
           dnListFilter.append(LdapFilterString.encode(memberDn));
           dnListFilter.append(")");
       }

       dnListFilter.append("))");

       return dnListFilter.toString();
    }

    @Override
    public boolean doesLinkExist(String mappedAttributeName)
    {
        return ((IdentityStoreAttributeMapping.NO_LINK_ATTRIBUTE_MAPPING.equalsIgnoreCase(mappedAttributeName)) == false);
    }

    @Override
    public boolean isDnAttribute(String attributeName)
    {
        return DN_ATTRIBUTE.equalsIgnoreCase(attributeName);
    }

    protected String getUserObjectClassValue()
    {
        return this.getObjectClassValue(IdentityStoreObjectMapping.ObjectIds.ObjectIdUser);
    }

    protected String getGroupObjectClassValue()
    {
        return this.getObjectClassValue(IdentityStoreObjectMapping.ObjectIds.ObjectIdGroup);
    }

    protected String getPasswordSettingsObjectClassValue()
    {
        return this.getObjectClassValue(IdentityStoreObjectMapping.ObjectIds.ObjectIdPasswordSettings);
    }

    protected String getDomainObjectClassValue()
    {
        return this.getObjectClassValue(IdentityStoreObjectMapping.ObjectIds.ObjectIdDomain);
    }

    protected abstract String buildUserQueryByAccountNameOrUpn();

    protected abstract String buildUserQueryByUpn();

    protected abstract String buildUserQueryByAccountName();

    protected abstract String buildUserQueryByObjectUniqueId( );

    protected abstract String buildUserQueryByCriteria();

    protected abstract String buildUserQueryByCriteriaForName();

    protected abstract String buildAllUsersQuery();

    protected abstract String buildAllDisabledUsersQuery();

    protected abstract String buildGroupQueryByCriteria();

    protected abstract String buildGroupQueryByCriteriaForName();

    protected abstract String buildAllGroupsQuery();

    protected abstract String buildDirectParentGroupsQuery();

    protected abstract String buildNestedParentGroupsQuery();

    protected abstract String buildGroupQueryByObjectUniqueId( );

    protected abstract String buildGroupQueryByAccountName();

    protected abstract String buildUserOrGroupQueryByAccountNameOrUpn();

    protected abstract String buildUserOrGroupQueryByAccountName();

    protected abstract String buildPasswordSettingsQuery();

    protected abstract String buildDomainObjectQuery();

    private String getObjectClassValue(String objectCl)
    {
        String objectClass = null;
        if(this._schemaMapping != null)
        {
            IdentityStoreObjectMapping objectMapping = this._schemaMapping.getObjectMapping(objectCl);
            if(objectMapping != null)
            {
                objectClass = objectMapping.getObjectClass();
            }
        }

        if( ServerUtils.isNullOrEmpty(objectClass) )
        {
            objectClass = this._defaultAttributes.get(objectCl);
        }

        return objectClass;
    }

    private String getAttribute( String objectClass, String attributeName )
    {
        String attribute = null;
        if( this._schemaMapping != null)
        {
            IdentityStoreObjectMapping objectMapping = this._schemaMapping.getObjectMapping(objectClass);

            if( objectMapping != null )
            {
                IdentityStoreAttributeMapping attributeMapping = objectMapping.getAttributeMapping(attributeName);
                if( attributeMapping != null )
                {
                    attribute = attributeMapping.getAttributeName();
                }
            }
        }

        if( ServerUtils.isNullOrEmpty(attribute) )
        {
            attribute = this._defaultAttributes.get(attributeName);
        }

        return attribute;
    }
}
