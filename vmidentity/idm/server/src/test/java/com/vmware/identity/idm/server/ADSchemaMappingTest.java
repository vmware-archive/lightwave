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

import org.junit.Assert;
import org.junit.Test;

import com.vmware.identity.idm.IdentityStoreObjectMapping.ObjectIds;
import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.IdentityStoreSchemaMapping;
import com.vmware.identity.idm.server.provider.activedirectory.ADSchemaMapping;

public class ADSchemaMappingTest {

    @Test
    public void testUnmappedSchema()
    {
        ADSchemaMapping adSchemaMapping = new ADSchemaMapping(null);

        SchemaMappingTestUtils.verifySchemaMapping(adSchemaMapping, expectedDefaultMapping);
    }

    @Test
    public void testCustomMappedSchema()
    {
        IdentityStoreSchemaMapping idsMapping = SchemaMappingTestUtils.getIdentityStoreSchemaMapping(expectedCustomizedSchema);

        ADSchemaMapping adSchemaMapping = new ADSchemaMapping(idsMapping);

        SchemaMappingTestUtils.verifySchemaMapping(adSchemaMapping, expectedCustomizedSchema);
    }

    @Test
    public void testFullyMappedSchema()
    {
        IdentityStoreSchemaMapping idsMapping = SchemaMappingTestUtils.getIdentityStoreSchemaMapping(expectedCustomMapping);

        ADSchemaMapping adSchemaMapping = new ADSchemaMapping(idsMapping);

        SchemaMappingTestUtils.verifySchemaMapping(adSchemaMapping, expectedCustomMapping);
    }

    @Test
    public void testSchemaMapping() throws Exception
    {
        IdentityStoreAttributeMapping attrMapping = null;

        String [] validAttrs = new String[]
            {
                "1.3.6.1.4.1.1466.115.121.1.38",
                "1.0.6.1.4.1.1466.115.121.1.38",
                "1.0.6.1.4.1.1466.115.1210.1.38",
                "0.0",
                "10.0",
                "myAttribute",
                "MyAttribute",
                "m123",
                "m-123"
            };

        String [] inValidAttrs = new String[]
                {
                    "01.3.6.1.4.1.1466.115.121.1.38",
                    "1.3.6.1.4.1.01466.115.121.1.38",
                    ".1.3",
                    "1.3.",
                    "01",
                    "",
                    null,
                    "_myAttribute",
                    "1m123",
                    "-m-123"
                };

        for( String validName : validAttrs )
        {
            attrMapping = new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName, validName);
            Assert.assertNotNull(attrMapping);
            Assert.assertEquals("Attribute name must match", validName, attrMapping.getAttributeName());
            Assert.assertEquals("Attribute id must match", IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName, attrMapping.getAttributeId());
            attrMapping = null;
        }
        for( String invalidName : inValidAttrs )
        {
            try
            {
                attrMapping = new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName, invalidName);
                Assert.fail("expected to throw");
            }
            catch(IllegalArgumentException ex)
            {
                // expected
            }
            catch(Exception ex)
            {
                Assert.fail(String.format("unexpected exception : [%s]", ex.toString()));
            }
        }
    }
    private static final Map<String, String> expectedDefaultMapping;
    private static final Map<String, String> expectedCustomMapping;
    private static final Map<String, String> expectedCustomizedSchema;

    static
    {
        expectedDefaultMapping = new HashMap<String, String> ();
        expectedDefaultMapping.put(ObjectIds.ObjectIdUser, "user");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl, "userAccountControl");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeCommonName, "cn");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName, "sAMAccountName");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription, "description");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDisplayName, "displayname");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail, "mail");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName, "givenName");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName, "sn");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime, "lockoutTime");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeMemberOf, "memberof");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId, "objectSid");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject, "msDS-ResultantPSO");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrimaryGroupId, "primaryGroupID");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName, "userPrincipalName");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet, "pwdLastSet");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeGroupMembersListLink, "dn");
        expectedDefaultMapping.put(ObjectIds.ObjectIdGroup, "group");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName, "sAMAccountName");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription, "description");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMemberOf, "memberof");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList, "member");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId, "objectSid");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupMembersListLink, "dn");
        expectedDefaultMapping.put(ObjectIds.ObjectIdPasswordSettings, "msDS-PasswordSettings");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.PasswordSettingsAttributeMaximumPwdAge, "msDS-MaximumPasswordAge");
        expectedDefaultMapping.put(ObjectIds.ObjectIdDomain, "domain");
        expectedDefaultMapping.put(IdentityStoreAttributeMapping.AttributeIds.DomainAttributeMaxPwdAge, "maxPwdAge");

        expectedDefaultMapping.put(SchemaMappingTestUtils.UserQueryByUpn, "(&(userPrincipalName=%s)(objectClass=user))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.UserQueryByAccountNameOrUpn, "(&(|(sAMAccountName=%1$s)(userPrincipalName=%2$s))(objectClass=user))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.UserQueryByAccountName, "(&(sAMAccountName=%s)(objectClass=user))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.UserQueryByObjectUniqueId, "(&(objectClass=user)(objectSid=%s))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.UserQueryByCriteria, "(&(objectClass=user)(|(sAMAccountName=*%1$s*)(sn=*%1$s*)(givenName=*%1$s*)(cn=*%1$s*)(name=*%1$s*)(displayname=*%1$s*)))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.UserQueryByCriteriaForName, "(&(objectClass=user)(|(sAMAccountName=%1$s*)(sn=%1$s*)(givenName=%1$s*)(cn=%1$s*)(displayname=%1$s*)(userPrincipalName=%1$s*)))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.AllUsersQuery, "(objectClass=user)");
        expectedDefaultMapping.put(SchemaMappingTestUtils.AllDisabledUsersQuery, "(&(objectclass=user)(!(userAccountControl=0)))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.AllGroupsQuery, "(objectClass=group)");
        expectedDefaultMapping.put(SchemaMappingTestUtils.DirectParentGroupsQuery, "(&(objectClass=group)(member=%s))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.NestedParentGroupsQuery, "(&(objectClass=group)(member:1.2.840.113556.1.4.1941:=%s))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.GroupQueryByObjectUniqueId, "(&(objectClass=group)(objectSid=%s))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.GroupQueryByAccountName, "(&(sAMAccountName=%s)(objectClass=group))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.GroupQueryByCriteria, "(&(objectClass=group)(|(sAMAccountName=*%1$s*)(description=*%1$s*)(objectSid=*%1$s*)))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.GroupQueryByCriteriaForName, "(&(objectClass=group)(|(sAMAccountName=%1$s*)(cn=%1$s*)))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.UserOrGroupQueryByAccountNameOrUpn, "(|(&(sAMAccountName=%1$s)(|(objectClass=user)(objectClass=group)))(&(userPrincipalName=%2$s)(objectClass=user)))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.UserOrGroupQueryByAccountName, "(&(sAMAccountName=%1$s)(|(objectClass=user)(objectClass=group)))");
        expectedDefaultMapping.put(SchemaMappingTestUtils.PasswordSettingsQuery, "(objectclass=msDS-PasswordSettings)");
        expectedDefaultMapping.put(SchemaMappingTestUtils.DomainObjectQuery, "(objectclass=domain)");

        expectedCustomMapping = new HashMap<String, String> ();

        expectedCustomMapping.put(ObjectIds.ObjectIdUser, "my-user");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl, "my-userAccountControl");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName, "my-user-sAMAccountName");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeCommonName, "my-user-cn");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription, "my-user-description");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDisplayName, "my-user-displayname");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail, "my-mail");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName, "my-givenName");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName, "my-sn");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime, "my-lockoutTime");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeMemberOf, "my-user-memberof");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId, "my-user-objectSid");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject, "my-msDS-ResultantPSO");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrimaryGroupId, "my-primaryGroupID");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName, "my-userPrincipalName");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet, "my-pwdLastSet");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeGroupMembersListLink, "dn");
        expectedCustomMapping.put(ObjectIds.ObjectIdGroup, "my-group");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName, "my-group-sAMAccountName");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription, "my-group-description");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMemberOf, "my-group-memberof");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList, "my-member");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId, "my-group-objectSid");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupMembersListLink, "dn");
        expectedCustomMapping.put(ObjectIds.ObjectIdPasswordSettings, "my-msDS-PasswordSettings");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.PasswordSettingsAttributeMaximumPwdAge, "my-msDS-MaximumPasswordAge");
        expectedCustomMapping.put(ObjectIds.ObjectIdDomain, "my-domain");
        expectedCustomMapping.put(IdentityStoreAttributeMapping.AttributeIds.DomainAttributeMaxPwdAge, "my-maxPwdAge");

        expectedCustomMapping.put(SchemaMappingTestUtils.UserQueryByUpn, "(&(my-userPrincipalName=%s)(objectClass=my-user))");
        expectedCustomMapping.put(SchemaMappingTestUtils.UserQueryByAccountNameOrUpn, "(&(|(my-user-sAMAccountName=%1$s)(my-userPrincipalName=%2$s))(objectClass=my-user))");
        expectedCustomMapping.put(SchemaMappingTestUtils.UserQueryByAccountName, "(&(my-user-sAMAccountName=%s)(objectClass=my-user))");
        expectedCustomMapping.put(SchemaMappingTestUtils.UserQueryByObjectUniqueId, "(&(objectClass=my-user)(my-user-objectSid=%s))");
        expectedCustomMapping.put(SchemaMappingTestUtils.UserQueryByCriteria, "(&(objectClass=my-user)(|(my-user-sAMAccountName=*%1$s*)(my-sn=*%1$s*)(my-givenName=*%1$s*)(cn=*%1$s*)(name=*%1$s*)(my-user-displayname=*%1$s*)))");
        expectedCustomMapping.put(SchemaMappingTestUtils.UserQueryByCriteriaForName, "(&(objectClass=my-user)(|(my-user-sAMAccountName=%1$s*)(my-sn=%1$s*)(my-givenName=%1$s*)(my-user-cn=%1$s*)(my-user-displayname=%1$s*)(my-userPrincipalName=%1$s*)))");
        expectedCustomMapping.put(SchemaMappingTestUtils.AllUsersQuery, "(objectClass=my-user)");
        expectedCustomMapping.put(SchemaMappingTestUtils.AllDisabledUsersQuery, "(&(objectclass=my-user)(!(my-userAccountControl=0)))");
        expectedCustomMapping.put(SchemaMappingTestUtils.AllGroupsQuery, "(objectClass=my-group)");
        expectedCustomMapping.put(SchemaMappingTestUtils.DirectParentGroupsQuery, "(&(objectClass=my-group)(my-member=%s))");
        expectedCustomMapping.put(SchemaMappingTestUtils.NestedParentGroupsQuery, "(&(objectClass=my-group)(my-member:1.2.840.113556.1.4.1941:=%s))");
        expectedCustomMapping.put(SchemaMappingTestUtils.GroupQueryByObjectUniqueId, "(&(objectClass=my-group)(my-group-objectSid=%s))");
        expectedCustomMapping.put(SchemaMappingTestUtils.GroupQueryByAccountName, "(&(my-group-sAMAccountName=%s)(objectClass=my-group))");
        expectedCustomMapping.put(SchemaMappingTestUtils.GroupQueryByCriteria, "(&(objectClass=my-group)(|(my-group-sAMAccountName=*%1$s*)(my-group-description=*%1$s*)(my-group-objectSid=*%1$s*)))");
        expectedCustomMapping.put(SchemaMappingTestUtils.GroupQueryByCriteriaForName, "(&(objectClass=my-group)(|(my-group-sAMAccountName=%1$s*)(cn=%1$s*)))");
        expectedCustomMapping.put(SchemaMappingTestUtils.UserOrGroupQueryByAccountNameOrUpn, "(|(&(|(my-user-sAMAccountName=%1$s)(my-userPrincipalName=%2$s))(objectClass=my-user))(&(my-group-sAMAccountName=%1$s)(objectClass=my-group)))");
        expectedCustomMapping.put(SchemaMappingTestUtils.UserOrGroupQueryByAccountName, "(|(&(my-user-sAMAccountName=%1$s)(objectClass=my-user))(&(my-group-sAMAccountName=%1$s)(objectClass=my-group)))");
        expectedCustomMapping.put(SchemaMappingTestUtils.PasswordSettingsQuery, "(objectclass=my-msDS-PasswordSettings)");
        expectedCustomMapping.put(SchemaMappingTestUtils.DomainObjectQuery, "(objectclass=my-domain)");

        expectedCustomizedSchema = new HashMap<String, String>();
        expectedCustomizedSchema.put(ObjectIds.ObjectIdUser, "my-user");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl, "my-userAccountControl");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName, "my-sAMAccountName");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeCommonName, "my-cn");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription, "my-description");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDisplayName, "my-displayname");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail, "my-mail");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName, "givenName");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName, "sn");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime, "lockoutTime");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeMemberOf, "my-memberof");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId, "my-objectSid");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject, "my-msDS-ResultantPSO");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrimaryGroupId, "my-primaryGroupID");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName, "my-userPrincipalName");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet, "my-pwdLastSet");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeGroupMembersListLink, "dn");
        expectedCustomizedSchema.put(ObjectIds.ObjectIdGroup, "my-group");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName, "my-sAMAccountName");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription, "my-description");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMemberOf, "my-memberof");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList, "my-member");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId, "objectSid");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupMembersListLink, "dn");
        expectedCustomizedSchema.put(ObjectIds.ObjectIdPasswordSettings, "my-msDS-PasswordSettings");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.PasswordSettingsAttributeMaximumPwdAge, "my-msDS-MaximumPasswordAge");
        expectedCustomizedSchema.put(ObjectIds.ObjectIdDomain, "my-domain");
        expectedCustomizedSchema.put(IdentityStoreAttributeMapping.AttributeIds.DomainAttributeMaxPwdAge, "my-maxPwdAge");

        expectedCustomizedSchema.put(SchemaMappingTestUtils.UserQueryByUpn, "(&(my-userPrincipalName=%s)(objectClass=my-user))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.UserQueryByAccountNameOrUpn, "(&(|(my-sAMAccountName=%1$s)(my-userPrincipalName=%2$s))(objectClass=my-user))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.UserQueryByAccountName, "(&(my-sAMAccountName=%s)(objectClass=my-user))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.UserQueryByObjectUniqueId, "(&(objectClass=my-user)(my-objectSid=%s))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.UserQueryByCriteria, "(&(objectClass=my-user)(|(my-sAMAccountName=*%1$s*)(sn=*%1$s*)(givenName=*%1$s*)(cn=*%1$s*)(name=*%1$s*)(my-displayname=*%1$s*)))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.UserQueryByCriteriaForName, "(&(objectClass=my-user)(|(my-sAMAccountName=%1$s*)(sn=%1$s*)(givenName=%1$s*)(my-cn=%1$s*)(my-displayname=%1$s*)(my-userPrincipalName=%1$s*)))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.AllUsersQuery, "(objectClass=my-user)");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.AllDisabledUsersQuery, "(&(objectclass=my-user)(!(my-userAccountControl=0)))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.AllGroupsQuery, "(objectClass=my-group)");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.DirectParentGroupsQuery, "(&(objectClass=my-group)(my-member=%s))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.NestedParentGroupsQuery, "(&(objectClass=my-group)(my-member:1.2.840.113556.1.4.1941:=%s))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.GroupQueryByObjectUniqueId, "(&(objectClass=my-group)(objectSid=%s))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.GroupQueryByAccountName, "(&(my-sAMAccountName=%s)(objectClass=my-group))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.GroupQueryByCriteria, "(&(objectClass=my-group)(|(my-sAMAccountName=*%1$s*)(my-description=*%1$s*)(objectSid=*%1$s*)))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.GroupQueryByCriteriaForName, "(&(objectClass=my-group)(|(my-sAMAccountName=%1$s*)(cn=%1$s*)))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.UserOrGroupQueryByAccountNameOrUpn, "(|(&(my-sAMAccountName=%1$s)(|(objectClass=my-user)(objectClass=my-group)))(&(my-userPrincipalName=%2$s)(objectClass=my-user)))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.UserOrGroupQueryByAccountName, "(&(my-sAMAccountName=%1$s)(|(objectClass=my-user)(objectClass=my-group)))");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.PasswordSettingsQuery, "(objectclass=my-msDS-PasswordSettings)");
        expectedCustomizedSchema.put(SchemaMappingTestUtils.DomainObjectQuery, "(objectclass=my-domain)");

    }
}
