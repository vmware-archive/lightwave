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

package com.vmware.identity.idm.server;

import java.util.Arrays;
import java.util.Collection;
import java.util.Map;

import junit.framework.Assert;

import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.IdentityStoreObjectMapping;
import com.vmware.identity.idm.IdentityStoreSchemaMapping;
import com.vmware.identity.idm.server.provider.ILdapSchemaMapping;

public class SchemaMappingTestUtils {

    public static void verifySchemaMapping(ILdapSchemaMapping schemaMapping, Map<String, String> expectedValues)
    {
        Assert.assertNotNull("schemaMapping must not be null", schemaMapping);
        Assert.assertNotNull("expectedValues must not be null", expectedValues);

        for( String userAttribute : SchemaMappingTestUtils.getUserAttributes() )
        {
            verifyUserAttribute(userAttribute, schemaMapping, expectedValues);
        }

        for( String groupAttribute : SchemaMappingTestUtils.getGroupAttributes() )
        {
            verifyGroupAttribute(groupAttribute, schemaMapping, expectedValues);
        }

        for( String pwdSettingsAttribute : SchemaMappingTestUtils.getPwdSettingsAttributes() )
        {
            verifyPasswordSettingsAttribute(pwdSettingsAttribute, schemaMapping, expectedValues);
        }

        for( String domainAttribute : SchemaMappingTestUtils.getDomainAttributes() )
        {
            verifyDomainAttribute(domainAttribute, schemaMapping, expectedValues);
        }

        verifyQueries( schemaMapping, expectedValues );
    }

    public static IdentityStoreSchemaMapping getIdentityStoreSchemaMapping(Map<String, String> expectedValues) {

        IdentityStoreSchemaMapping.Builder schemaBuilder = new IdentityStoreSchemaMapping.Builder();
        IdentityStoreObjectMapping.Builder objectBuilder = null;

        // user
        objectBuilder = new IdentityStoreObjectMapping.Builder(IdentityStoreObjectMapping.ObjectIds.ObjectIdUser);
        objectBuilder.setObjectClass(expectedValues.get(IdentityStoreObjectMapping.ObjectIds.ObjectIdUser));
        for( String userAttribute : SchemaMappingTestUtils.getUserAttributes() )
        {
            if( expectedValues.get(userAttribute) != null )
            {
                objectBuilder.addAttributeMapping(
                        new IdentityStoreAttributeMapping(
                            userAttribute,
                            expectedValues.get(userAttribute)));
            }
        }
        schemaBuilder.addObjectMappings(objectBuilder.buildObjectMapping());

        // group
        objectBuilder = new IdentityStoreObjectMapping.Builder(IdentityStoreObjectMapping.ObjectIds.ObjectIdGroup);
        objectBuilder.setObjectClass(expectedValues.get(IdentityStoreObjectMapping.ObjectIds.ObjectIdGroup));
        for( String groupAttribute : SchemaMappingTestUtils.getGroupAttributes() )
        {
            if( expectedValues.get(groupAttribute) != null )
            {
                objectBuilder.addAttributeMapping(
                        new IdentityStoreAttributeMapping(
                            groupAttribute,
                            expectedValues.get(groupAttribute)));
            }
        }
        schemaBuilder.addObjectMappings(objectBuilder.buildObjectMapping());

        // passwordSettings
        if( ServerUtils.isNullOrEmpty(expectedValues.get(IdentityStoreObjectMapping.ObjectIds.ObjectIdPasswordSettings)) == false )
        {
            objectBuilder = new IdentityStoreObjectMapping.Builder(IdentityStoreObjectMapping.ObjectIds.ObjectIdPasswordSettings);
            objectBuilder.setObjectClass(expectedValues.get(IdentityStoreObjectMapping.ObjectIds.ObjectIdPasswordSettings));
            for( String pwdSettingsAttribute : SchemaMappingTestUtils.getPwdSettingsAttributes() )
            {
                if( expectedValues.get(pwdSettingsAttribute) != null )
                {
                    objectBuilder.addAttributeMapping(
                            new IdentityStoreAttributeMapping(
                                pwdSettingsAttribute,
                                expectedValues.get(pwdSettingsAttribute)));
                }
            }
            schemaBuilder.addObjectMappings(objectBuilder.buildObjectMapping());
        }

        // domain
        if( ServerUtils.isNullOrEmpty(expectedValues.get(IdentityStoreObjectMapping.ObjectIds.ObjectIdDomain)) == false )
        {
            objectBuilder = new IdentityStoreObjectMapping.Builder(IdentityStoreObjectMapping.ObjectIds.ObjectIdDomain);
            objectBuilder.setObjectClass(expectedValues.get(IdentityStoreObjectMapping.ObjectIds.ObjectIdDomain));
            for( String domainAttribute : SchemaMappingTestUtils.getDomainAttributes() )
            {
                if( expectedValues.get(domainAttribute) != null )
                {
                    objectBuilder.addAttributeMapping(
                            new IdentityStoreAttributeMapping(
                                domainAttribute,
                                expectedValues.get(domainAttribute)));
                }
            }
            schemaBuilder.addObjectMappings(objectBuilder.buildObjectMapping());
        }

        return schemaBuilder.buildSchemaMapping();
    }

    private static void verifyUserAttribute( String attributeName, ILdapSchemaMapping schemaMapping, Map<String, String> expectedValues )
    {
        Boolean exceptionExpected = false;
        String actualAttribute = null;
        if(expectedValues.get(attributeName) == null)
        {
            exceptionExpected = true;
        }
        try
        {
            actualAttribute = schemaMapping.getUserAttribute(attributeName);
        }
        catch(RuntimeException ex)
        {
            if(exceptionExpected == false)
            {
                throw ex;
            }
        }
        Assert.assertEquals(
            String.format("User attribute [%s] expected to match", attributeName),
            expectedValues.get(attributeName),
            actualAttribute);
    }

    private static void verifyGroupAttribute( String attributeName, ILdapSchemaMapping schemaMapping, Map<String, String> expectedValues )
    {
        Boolean exceptionExpected = false;
        String actualAttribute = null;
        if(expectedValues.get(attributeName) == null)
        {
            exceptionExpected = true;
        }
        try
        {
            actualAttribute = schemaMapping.getGroupAttribute(attributeName);
        }
        catch(RuntimeException ex)
        {
            if(exceptionExpected == false)
            {
                throw ex;
            }
        }
        Assert.assertEquals(
            String.format("Group attribute [%s] expected to match", attributeName),
            expectedValues.get(attributeName),
            actualAttribute);
    }

    private static void verifyPasswordSettingsAttribute( String attributeName, ILdapSchemaMapping schemaMapping, Map<String, String> expectedValues )
    {
        Boolean exceptionExpected = false;
        String actualAttribute = null;
        if(expectedValues.get(attributeName) == null)
        {
            exceptionExpected = true;
        }
        try
        {
            actualAttribute = schemaMapping.getPwdObjectAttribute(attributeName);
        }
        catch(RuntimeException ex)
        {
            if(exceptionExpected == false)
            {
                throw ex;
            }
        }
        Assert.assertEquals(
            String.format("PasswordSettings attribute [%s] expected to match", attributeName),
            expectedValues.get(attributeName),
            actualAttribute);
    }

    private static void verifyDomainAttribute( String attributeName, ILdapSchemaMapping schemaMapping, Map<String, String> expectedValues )
    {
        Boolean exceptionExpected = false;
        String actualAttribute = null;
        if(expectedValues.get(attributeName) == null)
        {
            exceptionExpected = true;
        }
        try
        {
            actualAttribute = schemaMapping.getDomainObjectAttribute(attributeName);
        }
        catch(RuntimeException ex)
        {
            if(exceptionExpected == false)
            {
                throw ex;
            }
        }
        Assert.assertEquals(
            String.format("Domain attribute [%s] expected to match", attributeName),
            expectedValues.get(attributeName),
            actualAttribute);
    }

    private static void verifyQueries(ILdapSchemaMapping schemaMapping, Map<String, String> expectedValues)
    {
        Assert.assertEquals(
                "UserQueryByAccountNameOrUpn query expected to match",
                expectedValues.get(UserQueryByAccountNameOrUpn),
                schemaMapping.getUserQueryByAccountNameOrUpn());

        Assert.assertEquals(
                "UserQueryByUpn query expected to match",
                expectedValues.get(UserQueryByUpn),
                schemaMapping.getUserQueryByUpn());

        Assert.assertEquals(
                "UserQueryByAccountName query expected to match",
                expectedValues.get(UserQueryByAccountName),
                schemaMapping.getUserQueryByAccountName());

        Assert.assertEquals(
                "UserQueryByObjectUniqueId query expected to match",
                expectedValues.get(UserQueryByObjectUniqueId),
                schemaMapping.getUserQueryByObjectUniqueId());

        Assert.assertEquals(
                "UserQueryByCriteria query expected to match",
                expectedValues.get(UserQueryByCriteria),
                schemaMapping.getUserQueryByCriteria());

        Assert.assertEquals(
                "UserQueryByCriteriaForName query expected to match",
                expectedValues.get(UserQueryByCriteriaForName),
                schemaMapping.getUserQueryByCriteriaForName());

        Assert.assertEquals(
                "AllUsersQuery query expected to match",
                expectedValues.get(AllUsersQuery),
                schemaMapping.getAllUsersQuery());

        Assert.assertEquals(
                "AllDisabledUsersQuery query expected to match",
                expectedValues.get(AllDisabledUsersQuery),
                schemaMapping.getAllDisabledUsersQuery());

        Assert.assertEquals(
                "AllGroupsQuery query expected to match",
                expectedValues.get(AllGroupsQuery),
                schemaMapping.getAllGroupsQuery());

        Assert.assertEquals(
                "DirectParentGroupsQuery query expected to match",
                expectedValues.get(DirectParentGroupsQuery),
                schemaMapping.getDirectParentGroupsQuery());

        Assert.assertEquals(
                "NestedParentGroupsQuery query expected to match",
                expectedValues.get(NestedParentGroupsQuery),
                schemaMapping.getNestedParentGroupsQuery());

        Assert.assertEquals(
                "GroupQueryByObjectUniqueId query expected to match",
                expectedValues.get(GroupQueryByObjectUniqueId),
                schemaMapping.getGroupQueryByObjectUniqueId());

        Assert.assertEquals(
                "GroupQueryByAccountName query expected to match",
                expectedValues.get(GroupQueryByAccountName),
                schemaMapping.getGroupQueryByAccountName());

        Assert.assertEquals(
                "GroupQueryByCriteria query expected to match",
                expectedValues.get(GroupQueryByCriteria),
                schemaMapping.getGroupQueryByCriteria());

        Assert.assertEquals(
                "GroupQueryByCriteriaForName query expected to match",
                expectedValues.get(GroupQueryByCriteriaForName),
                schemaMapping.getGroupQueryByCriteriaForName());

        Assert.assertEquals(
                "UserOrGroupQueryByAccountNameOrUpn query expected to match",
                expectedValues.get(UserOrGroupQueryByAccountNameOrUpn),
                schemaMapping.getUserOrGroupQueryByAccountNameOrUpn());

        Assert.assertEquals(
                "UserOrGroupQueryByAccountName query expected to match",
                expectedValues.get(UserOrGroupQueryByAccountName),
                schemaMapping.getUserOrGroupQueryByAccountName());

        Assert.assertEquals(
                "PasswordSettingsQuery query expected to match",
                expectedValues.get(PasswordSettingsQuery),
                schemaMapping.getPasswordSettingsQuery());

        Assert.assertEquals(
                "DomainObjectQuery query expected to match",
                expectedValues.get(DomainObjectQuery),
                schemaMapping.getDomainObjectQuery());
    }

    private static Collection<String> getUserAttributes()
    {
        Collection<String> userAttributes =
            Arrays.asList(
                IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName,
                IdentityStoreAttributeMapping.AttributeIds.UserAttributeCommonName,
                IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl,
                IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription,
                IdentityStoreAttributeMapping.AttributeIds.UserAttributeDisplayName,
                IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail,
                IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName,
                IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName,
                IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime,
                IdentityStoreAttributeMapping.AttributeIds.UserAttributeMemberOf,
                IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId,
                IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject,
                IdentityStoreAttributeMapping.AttributeIds.UserAttributePrimaryGroupId,
                IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName,
                IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet,
                IdentityStoreAttributeMapping.AttributeIds.UserAttributeGroupMembersListLink
            );

        return userAttributes;
    }

    private static Collection<String> getGroupAttributes()
    {
        Collection<String> userAttributes =
            Arrays.asList(
                IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName,
                IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription,
                IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMemberOf,
                IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList,
                IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId,
                IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupMembersListLink
            );

        return userAttributes;
    }

    private static Collection<String> getPwdSettingsAttributes()
    {
        Collection<String> userAttributes =
            Arrays.asList(
                IdentityStoreAttributeMapping.AttributeIds.PasswordSettingsAttributeMaximumPwdAge
            );

        return userAttributes;
    }

    private static Collection<String> getDomainAttributes()
    {
        Collection<String> userAttributes =
            Arrays.asList(
                IdentityStoreAttributeMapping.AttributeIds.DomainAttributeMaxPwdAge
            );

        return userAttributes;
    }

    public static final String UserQueryByUpn = "QUERY_UserQueryByUpn";
    public static final String UserQueryByAccountName = "QUERY_UserQueryByAccountName";
    public static final String UserQueryByAccountNameOrUpn = "QUERY_UserQueryByAccountNameOrUpn";
    public static final String UserQueryByObjectUniqueId = "QUERY_UserQueryByObjectUniqueId";
    public static final String UserQueryByCriteria = "QUERY_UserQueryByCriteria";
    public static final String UserQueryByCriteriaForName = "QUERY_UserQueryByCriteriaForName";
    public static final String AllUsersQuery = "QUERY_AllUsersQuery";
    public static final String AllDisabledUsersQuery = "QUERY_AllDisabledUsersQuery";
    public static final String AllGroupsQuery = "QUERY_AllGroupsQuery";
    public static final String DirectParentGroupsQuery = "QUERY_DirectParentGroupsQuery";
    public static final String NestedParentGroupsQuery = "QUERY_NestedParentGroupsQuery";
    public static final String GroupQueryByObjectUniqueId = "QUERY_GroupQueryByObjectUniqueId";
    public static final String GroupQueryByAccountName = "QUERY_GroupQueryByAccountName";
    public static final String GroupQueryByCriteria = "QUERY_GroupQueryByCriteria";
    public static final String GroupQueryByCriteriaForName = "QUERY_GroupQueryByCriteriaForName";
    public static final String UserOrGroupQueryByAccountNameOrUpn = "QUERY_UserOrGroupQueryByAccountNameOrUpn";
    public static final String UserOrGroupQueryByAccountName = "QUERY_UserOrGroupQueryByAccountName";
    public static final String PasswordSettingsQuery = "QUERY_PasswordSettingsQuery";
    public static final String DomainObjectQuery = "QUERY_DomainObjectQuery";
}
