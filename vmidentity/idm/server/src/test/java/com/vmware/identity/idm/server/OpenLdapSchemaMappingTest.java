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

import java.util.HashMap;
import java.util.Map;

import org.junit.Test;

import org.junit.Assert;

import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.IdentityStoreObjectMapping.ObjectIds;
import com.vmware.identity.idm.IdentityStoreSchemaMapping;
import com.vmware.identity.idm.server.provider.ldap.OpenLdapSchemaMapping;

public class OpenLdapSchemaMappingTest {

    @Test
    public void testUnmappedSchema()
    {
        OpenLdapSchemaMapping olSchemaMapping = new OpenLdapSchemaMapping(null);

        SchemaMappingTestUtils.verifySchemaMapping(olSchemaMapping, expectedDefaultMapping);

        String userGroupLinkAttribute = olSchemaMapping.getUserAttribute(
            IdentityStoreAttributeMapping.AttributeIds.UserAttributeGroupMembersListLink);
        String groupGroupLinkAttribute = olSchemaMapping.getGroupAttribute(
            IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupMembersListLink);
        Assert.assertTrue("For default mapping user link attribute should be dn", olSchemaMapping.isDnAttribute(userGroupLinkAttribute));
        Assert.assertTrue("For default mapping user link attribute should exist", olSchemaMapping.doesLinkExist(userGroupLinkAttribute));
        Assert.assertTrue("For default mapping group link attribute should be dn", olSchemaMapping.isDnAttribute(groupGroupLinkAttribute));
        Assert.assertTrue("For default mapping group link attribute should exist", olSchemaMapping.doesLinkExist(groupGroupLinkAttribute));
    }

    @Test
    public void testCustomMappedSchema()
    {
        IdentityStoreSchemaMapping idsMapping = SchemaMappingTestUtils.getIdentityStoreSchemaMapping(expectedCustomizedSchema);

        OpenLdapSchemaMapping olSchemaMapping = new OpenLdapSchemaMapping(idsMapping);

        SchemaMappingTestUtils.verifySchemaMapping(olSchemaMapping, expectedCustomizedSchema);

        String userGroupLinkAttribute = olSchemaMapping.getUserAttribute(
            IdentityStoreAttributeMapping.AttributeIds.UserAttributeGroupMembersListLink);
        String groupGroupLinkAttribute = olSchemaMapping.getGroupAttribute(
            IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupMembersListLink);
        Assert.assertFalse("For custom mapped schema user link attribute should not be dn", olSchemaMapping.isDnAttribute(userGroupLinkAttribute));
        Assert.assertTrue("For custom mapped schema user link attribute should exist", olSchemaMapping.doesLinkExist(userGroupLinkAttribute));
        Assert.assertFalse("For custom mapped schema group link attribute should not be dn", olSchemaMapping.isDnAttribute(groupGroupLinkAttribute));
        Assert.assertTrue("For custom mapped schema link attribute should exist", olSchemaMapping.doesLinkExist(groupGroupLinkAttribute));
    }

    @Test
    public void testFullyMappedSchema()
    {
        IdentityStoreSchemaMapping idsMapping = SchemaMappingTestUtils.getIdentityStoreSchemaMapping(expectedCustomMapping);

        OpenLdapSchemaMapping olSchemaMapping = new OpenLdapSchemaMapping(idsMapping);

        SchemaMappingTestUtils.verifySchemaMapping(olSchemaMapping, expectedCustomMapping);

        String userGroupLinkAttribute = olSchemaMapping.getUserAttribute(
            IdentityStoreAttributeMapping.AttributeIds.UserAttributeGroupMembersListLink);
        String groupGroupLinkAttribute = olSchemaMapping.getGroupAttribute(
            IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupMembersListLink);
        Assert.assertFalse("For fully mapped schema user link attribute should not be dn", olSchemaMapping.isDnAttribute(userGroupLinkAttribute));
        Assert.assertTrue("For fully mapped schema user link attribute should exist", olSchemaMapping.doesLinkExist(userGroupLinkAttribute));
        Assert.assertFalse("For fully mapped schema group link attribute should not be dn", olSchemaMapping.isDnAttribute(groupGroupLinkAttribute));
        Assert.assertFalse("For fully mapped schema group link attribute should not exist", olSchemaMapping.doesLinkExist(groupGroupLinkAttribute));
    }

    private static final Map<String, String> expectedDefaultMapping;
    private static final Map<String, String> expectedCustomMapping;
    private static final Map<String, String> expectedCustomizedSchema;

    static
    {
        expectedDefaultMapping = new HashMap<String, String> ();
        expectedDefaultMapping.put(ObjectIds.ObjectIdUser, "inetOrgPerson");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl, "userAccountControl");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName, "uid");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeCommonName, "cn");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription, "description");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDisplayName, "displayName");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail, "mail");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName, "givenName");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName, "sn");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime, "pwdAccountLockedTime");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeMemberOf, null);
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId, "entryUUID");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject, null);
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrimaryGroupId, null);
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName, null);
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet, null);
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeGroupMembersListLink, "dn");
        expectedDefaultMapping.put(ObjectIds.ObjectIdGroup, "groupOfUniqueNames");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName, "cn");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription, "description");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMemberOf, null);
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList, "uniqueMember");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId, "entryUUID");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupMembersListLink, "dn");
        expectedDefaultMapping.put(ObjectIds.ObjectIdPasswordSettings, "pwdPolicy");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.PasswordSettingsAttributeMaximumPwdAge, null);
        expectedDefaultMapping.put(ObjectIds.ObjectIdDomain, null);
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.DomainAttributeMaxPwdAge, null);

        expectedDefaultMapping.put(SchemaMappingTestUtils.UserQueryByUpn, null);
        expectedDefaultMapping.put(SchemaMappingTestUtils.UserQueryByAccountNameOrUpn, null);
        expectedDefaultMapping.put(SchemaMappingTestUtils.UserQueryByAccountName, "(&(objectClass=inetOrgPerson)(uid=%s))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.UserQueryByObjectUniqueId, "(&(objectClass=inetOrgPerson)(entryUUID=%s))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.UserQueryByCriteria, "(&(objectClass=inetOrgPerson)(|(uid=*%1$s*)(sn=*%1$s*)(givenName=*%1$s*)(displayName=*%1$s*)(description=*%1$s*)))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.UserQueryByCriteriaForName, "(&(objectClass=inetOrgPerson)(|(uid=%1$s*)(mail=%1$s*)(sn=%1$s*)(givenName=%1$s*)(cn=%1$s*)(displayName=%1$s*)))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.AllUsersQuery, "(objectClass=inetOrgPerson)");
        expectedDefaultMapping.put(SchemaMappingTestUtils.AllDisabledUsersQuery, "(objectClass=inetOrgPerson)");
        expectedDefaultMapping.put(SchemaMappingTestUtils.AllGroupsQuery, "(objectClass=groupOfUniqueNames)");
        expectedDefaultMapping.put(SchemaMappingTestUtils.DirectParentGroupsQuery, "(&(objectClass=groupOfUniqueNames)(uniqueMember=%s))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.NestedParentGroupsQuery, null);
        expectedDefaultMapping.put(SchemaMappingTestUtils.GroupQueryByObjectUniqueId, "(&(objectClass=groupOfUniqueNames)(entryUUID=%s))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.GroupQueryByAccountName, "(&(objectClass=groupOfUniqueNames)(cn=%s))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.GroupQueryByCriteria, "(&(objectClass=groupOfUniqueNames)(|(cn=*%1$s*)(description=*%1$s*)(entryUUID=*%1$s*)))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.GroupQueryByCriteriaForName, "(&(objectClass=groupOfUniqueNames)(cn=%1$s*))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.UserOrGroupQueryByAccountNameOrUpn, null);
        expectedDefaultMapping.put(SchemaMappingTestUtils.UserOrGroupQueryByAccountName, "(|(&(objectClass=inetOrgPerson)(uid=%1$s))(&(objectClass=groupOfUniqueNames)(cn=%1$s)))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.PasswordSettingsQuery, "(objectClass=pwdPolicy)");
        expectedDefaultMapping.put(SchemaMappingTestUtils.DomainObjectQuery, null);

        expectedCustomMapping = new HashMap<String, String> ();

        expectedCustomMapping.put(ObjectIds.ObjectIdUser, "my-inetOrgPerson");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl, "my-userAccountControl");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName, "my-user-cn");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeCommonName, "my-user-cn");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription, "my-user-description");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDisplayName, "my-displayName");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail, "my-mail");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName, "my-givenName");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName, "my-sn");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime, "pwdAccountLockedTime");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeMemberOf, null);
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId, "my-user-entryUUID");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject, null);
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrimaryGroupId, null);
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName, null);
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet, null);
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeGroupMembersListLink, "uid");
        expectedCustomMapping.put(ObjectIds.ObjectIdGroup, "my-groupOfUniqueNames");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName, "my-group-cn");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription, "my-group-description");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMemberOf, null);
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList, "my-uniqueMember");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId, "my-group-entryUUID");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupMembersListLink, IdentityStoreAttributeMapping.NO_LINK_ATTRIBUTE_MAPPING);
        expectedCustomMapping.put(ObjectIds.ObjectIdPasswordSettings, "pwdPolicy");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.PasswordSettingsAttributeMaximumPwdAge, null);
        expectedCustomMapping.put(ObjectIds.ObjectIdDomain, null);
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.DomainAttributeMaxPwdAge, null);

        expectedCustomMapping.put(SchemaMappingTestUtils.UserQueryByUpn, null);
        expectedCustomMapping.put(SchemaMappingTestUtils.UserQueryByAccountNameOrUpn, null);
        expectedCustomMapping.put(SchemaMappingTestUtils.UserQueryByAccountName, "(&(objectClass=my-inetOrgPerson)(my-user-cn=%s))");
        expectedCustomMapping.put(SchemaMappingTestUtils.UserQueryByObjectUniqueId, "(&(objectClass=my-inetOrgPerson)(my-user-entryUUID=%s))");
        expectedCustomMapping.put(SchemaMappingTestUtils.UserQueryByCriteria, "(&(objectClass=my-inetOrgPerson)(|(my-user-cn=*%1$s*)(my-sn=*%1$s*)(my-givenName=*%1$s*)(my-displayName=*%1$s*)(my-user-description=*%1$s*)))");
        expectedCustomMapping.put(SchemaMappingTestUtils.UserQueryByCriteriaForName, "(&(objectClass=my-inetOrgPerson)(|(my-user-cn=%1$s*)(my-mail=%1$s*)(my-sn=%1$s*)(my-givenName=%1$s*)(my-user-cn=%1$s*)(my-displayName=%1$s*)))");
        expectedCustomMapping.put(SchemaMappingTestUtils.AllUsersQuery, "(objectClass=my-inetOrgPerson)");
        expectedCustomMapping.put(SchemaMappingTestUtils.AllDisabledUsersQuery, "(objectClass=my-inetOrgPerson)");
        expectedCustomMapping.put(SchemaMappingTestUtils.AllGroupsQuery, "(objectClass=my-groupOfUniqueNames)");
        expectedCustomMapping.put(SchemaMappingTestUtils.DirectParentGroupsQuery, "(&(objectClass=my-groupOfUniqueNames)(my-uniqueMember=%s))");
        expectedCustomMapping.put(SchemaMappingTestUtils.NestedParentGroupsQuery, null);
        expectedCustomMapping.put(SchemaMappingTestUtils.GroupQueryByObjectUniqueId, "(&(objectClass=my-groupOfUniqueNames)(my-group-entryUUID=%s))");
        expectedCustomMapping.put(SchemaMappingTestUtils.GroupQueryByAccountName, "(&(objectClass=my-groupOfUniqueNames)(my-group-cn=%s))");
        expectedCustomMapping.put(SchemaMappingTestUtils.GroupQueryByCriteria, "(&(objectClass=my-groupOfUniqueNames)(|(my-group-cn=*%1$s*)(my-group-description=*%1$s*)(my-group-entryUUID=*%1$s*)))");
        expectedCustomMapping.put(SchemaMappingTestUtils.GroupQueryByCriteriaForName, "(&(objectClass=my-groupOfUniqueNames)(my-group-cn=%1$s*))");
        expectedCustomMapping.put(SchemaMappingTestUtils.UserOrGroupQueryByAccountNameOrUpn, null);
        expectedCustomMapping.put(SchemaMappingTestUtils.UserOrGroupQueryByAccountName, "(|(&(objectClass=my-inetOrgPerson)(my-user-cn=%1$s))(&(objectClass=my-groupOfUniqueNames)(my-group-cn=%1$s)))");
        expectedCustomMapping.put(SchemaMappingTestUtils.PasswordSettingsQuery, "(objectClass=pwdPolicy)");
        expectedCustomMapping.put(SchemaMappingTestUtils.DomainObjectQuery, null);

        expectedCustomizedSchema = new HashMap<String, String> ();
        expectedCustomizedSchema.put(ObjectIds.ObjectIdUser, "my-inetOrgPerson");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl, "my-userAccountControl");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName, "my-cn");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeCommonName, "my-cn");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription, "my-description");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDisplayName, "my-displayName");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail, "my-mail");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName, "givenName");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName, "sn");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime, "pwdAccountLockedTime");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeMemberOf, null);
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId, "my-entryUUID");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject, null);
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrimaryGroupId, null);
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName, null);
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet, null);
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeGroupMembersListLink, "uid");
        expectedCustomizedSchema.put(ObjectIds.ObjectIdGroup, "my-groupOfUniqueNames");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName, "my-cn");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription, "my-description");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMemberOf, null);
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList, "uniqueMember");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId, "my-entryUUID");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupMembersListLink, "cn");
        expectedCustomizedSchema.put(ObjectIds.ObjectIdPasswordSettings, "pwdPolicy");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.PasswordSettingsAttributeMaximumPwdAge, null);
        expectedCustomizedSchema.put(ObjectIds.ObjectIdDomain, null);
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.DomainAttributeMaxPwdAge, null);

        expectedCustomizedSchema.put(SchemaMappingTestUtils.UserQueryByUpn, null);
        expectedCustomizedSchema.put(SchemaMappingTestUtils.UserQueryByAccountNameOrUpn, null);
        expectedCustomizedSchema.put(SchemaMappingTestUtils.UserQueryByAccountName, "(&(objectClass=my-inetOrgPerson)(my-cn=%s))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.UserQueryByObjectUniqueId, "(&(objectClass=my-inetOrgPerson)(my-entryUUID=%s))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.UserQueryByCriteria, "(&(objectClass=my-inetOrgPerson)(|(my-cn=*%1$s*)(sn=*%1$s*)(givenName=*%1$s*)(my-displayName=*%1$s*)(my-description=*%1$s*)))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.UserQueryByCriteriaForName, "(&(objectClass=my-inetOrgPerson)(|(my-cn=%1$s*)(my-mail=%1$s*)(sn=%1$s*)(givenName=%1$s*)(my-cn=%1$s*)(my-displayName=%1$s*)))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.AllUsersQuery, "(objectClass=my-inetOrgPerson)");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.AllDisabledUsersQuery, "(objectClass=my-inetOrgPerson)");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.AllGroupsQuery, "(objectClass=my-groupOfUniqueNames)");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.DirectParentGroupsQuery, "(&(objectClass=my-groupOfUniqueNames)(uniqueMember=%s))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.NestedParentGroupsQuery, null);
        expectedCustomizedSchema.put(SchemaMappingTestUtils.GroupQueryByObjectUniqueId, "(&(objectClass=my-groupOfUniqueNames)(my-entryUUID=%s))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.GroupQueryByAccountName, "(&(objectClass=my-groupOfUniqueNames)(my-cn=%s))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.GroupQueryByCriteria, "(&(objectClass=my-groupOfUniqueNames)(|(my-cn=*%1$s*)(my-description=*%1$s*)(my-entryUUID=*%1$s*)))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.GroupQueryByCriteriaForName, "(&(objectClass=my-groupOfUniqueNames)(my-cn=%1$s*))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.UserOrGroupQueryByAccountNameOrUpn, null);
        expectedCustomizedSchema.put(SchemaMappingTestUtils.UserOrGroupQueryByAccountName, "(&(|(objectClass=my-inetOrgPerson)(objectClass=my-groupOfUniqueNames))(my-cn=%1$s))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.PasswordSettingsQuery, "(objectClass=pwdPolicy)");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.DomainObjectQuery, null);

    }
}
