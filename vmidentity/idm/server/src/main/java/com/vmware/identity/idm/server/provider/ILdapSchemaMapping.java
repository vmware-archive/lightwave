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

import com.vmware.identity.idm.IdentityStoreObjectMapping;

/**
 * Exposes attribute names and query templates for an ldap store.
 */
public interface ILdapSchemaMapping
{
    /**
     * @return User query template parameterized with
     * - arg1 samAccount
     * - arg2 upn
     */
    String getUserQueryByAccountNameOrUpn();

    /**
     * @return User query template parameterized with user principal name.
     */
    String getUserQueryByUpn();

    /**
     * @return User query template parameterized with user samAccount name.
     */
    String getUserQueryByAccountName();

    /**
      * @return User query template parameterized with user object id.
      */
    String getUserQueryByObjectUniqueId( );

    /**
      * @return User query template parameterized with search string (criteria).
      */
    String getUserQueryByCriteria();

    /**
     * @return User query template parameterized with search string (criteria)  limiting only searching on accountNames
     */
   String getUserQueryByCriteriaForName();

    /**
      * @return User query.
      */
    String getAllUsersQuery();

    /**
      * @return Disabled user query.
      * (results might need additional processing wrt disabled state).
      */
    String getAllDisabledUsersQuery();

    /**
     * @return Group query template parameterized with search string (criteria).
     */
    String getGroupQueryByCriteria();

    /**
     * @return Group query template parameterized with search string (criteria) limiting only searching on accountNames
     */
    String getGroupQueryByCriteriaForName();

    /**
      * @return Group query.
      */
    String getAllGroupsQuery();

    /**
     * @return Direct parent groups query template parameterized with GroupAttributeMembersList.
     */
    String getDirectParentGroupsQuery();

    /**
     * @return Nested parent groups query template parameterized with member's dn.
     */
    String getNestedParentGroupsQuery();

    /**
     * @return Group query template parameterized with group object id.
     */
    String getGroupQueryByObjectUniqueId( );

    /**
     * @return Group query template parameterized with group account name.
     */
    String getGroupQueryByAccountName();

    /**
     * @return Query template for (user or group) parameterized with
     * - arg1 SamAccountName
     * - arg2 upn
     */
    String getUserOrGroupQueryByAccountNameOrUpn();

    /**
     * @return Query template for (user or group) parameterized with
     * - arg1 SamAccountName
     */
    String getUserOrGroupQueryByAccountName();

    /**
     * @return Password settings object query.
     */
    String getPasswordSettingsQuery();

    /**
     * @return Domain object query.
     */
    String getDomainObjectQuery();

    /**
     * @param attributeId Id of an attribute whose name to retrieve.
     *        @see IdentityStoreObjectMapping.AttributeIds
     * @return Specified user attribute's name.
     */
    String getUserAttribute( String attributeId );

    /**
     * @param attributeId Id of an attribute whose name to retrieve.
     *        @see IdentityStoreObjectMapping.AttributeIds
     * @return Specified group attribute's name.
     */
    String getGroupAttribute( String attributeId );

    /**
     * @param attributeId Id of an attribute whose name to retrieve.
     *        @see IdentityStoreObjectMapping.AttributeIds
     * @return Specified password settings attribute's name.
     */
    String getPwdObjectAttribute( String attributeId );

    /**
     * @param attributeId Id of an attribute whose name to retrieve.
     *        @see IdentityStoreObjectMapping.AttributeIds
     * @return Specified domain object attribute's name.
     */
    String getDomainObjectAttribute( String attributeId );

    String getDNFilter( String filter, Collection<String> memberDNs );

    boolean doesLinkExist(String mappedAttributeName);

    boolean isDnAttribute(String attributeName);

    /**
     * @return Query template for (user or group) parameterized with
     * - arg1 attribute arg2 attribute value
     */
    String getUserQueryByAttribute();
}
