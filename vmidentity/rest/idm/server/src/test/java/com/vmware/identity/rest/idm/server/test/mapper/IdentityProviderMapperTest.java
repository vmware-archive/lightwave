/*
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
 */
package com.vmware.identity.rest.idm.server.test.mapper;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;

import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.junit.Test;

import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.IdentityStoreData;
import com.vmware.identity.idm.IdentityStoreObjectMapping;
import com.vmware.identity.idm.IdentityStoreObjectMapping.ObjectIds;
import com.vmware.identity.idm.IdentityStoreSchemaMapping;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.IdentityProviderDTO;
import com.vmware.identity.rest.idm.data.SchemaObjectMappingDTO;
import com.vmware.identity.rest.idm.data.attributes.IdentityProviderType;
import com.vmware.identity.rest.idm.data.attributes.ObjectClass;
import com.vmware.identity.rest.idm.server.mapper.IdentityProviderMapper;

/**
 *
 * Unit tests for IdentityProviderMapper
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class IdentityProviderMapperTest {

    // Constants - Common
    private static final String USER_NAME = "testUser@domain";
    private static final String PASSWORD = "UnitTest123!";
    private static final String SYSTEM_TENANT = "test.local";
    private static final String HINT_ATTR_NAME = "userNameAttribute";
    private static final Boolean ACCOUNT_LINKING_WITH_UPN = true;

    //Schema Mapping related constants
    private static final String OBJECT_CLASS_USER = "user";
    private static final String OBJECT_CLASS_GROUP = "group";
    private static final String OBJECT_CLASS_DOMAIN = "domain";
    private static final String OBJECT_CLASS_PWD_SETTINGS = "msDS-PasswordSettings";
    private static final String USER_ATTRIBUTE_ACCOUNT_NAME = "sAMAccountName";
    private static final String USER_ATTRIBUTE_DESC = "description";
    private static final String USER_ATTRIBUTE_DISPLAY_NAME = "displayName";
    private static final String USER_ATTRIBUTE_EMAIL = "mail";
    private static final String GROUP_ATTRIBUTE_ACCOUNT_NAME = "sAMAccountName";
    private static final String GROUP_ATTRIBUTE_DESC = "description";
    private static final String GROUP_ATTRIBUTE_MEMBER_OF = "memberOf";
    private static final String GROUP_ATTRIBUTE_MEMBER = "member";
    private static final String PWD_ATTRBIUTE_MAX_AGE = "msDS-MaximumPasswordAge";
    private static final String DOMAIN_ATTRIBUTE_MAX_PWD_AGE = "maxPwdAge";

    // AD -related constants
    private static final String AD_PROVIDER_NAME = "ad.ssolabs.eng.wmare.com";
    private static final String AD_SPN = "spn.ad.ssolabs.eng.vmware.com";

    // LDAP related constants
    private static final String LDAP_NAME = "sso.labs.eng.vmware.com";
    private static final String LDAP_ALIAS = "ssolabs";
    private static final String USER_BASE_DN = "cn=users,dc=ssolabs,dc=eng,dc=vmware,dc=com";
    private static final String GROUP_BASE_DN = "dc=ssolabs,dc=eng,dc=vmware,dc=com";
    private static final String FRIENDLY_NAME = "ssolabs";
    private static final String LDAP_UN = "cn=admin,dc=ssolabs,dc=eng,dc=vmware,dc=com";

    // Local OS related constants
    private static final String LOCALOS_NAME = "localOS";
    private static final String LOCALOS_ALIAS = "localOS-Alias";

    private static final Map<String, String> attributesMap = new HashMap<String, String>() {
        {
            put("http://vmware.com/schemas/attr-names/2011/07/isSolution", "subjectType");
            put("http://schemas.xmlsoap.org/claims/UPN", "userPrincipalName");
            put("http://schemas.xmlsoap.org/ws/2005/05/identity/claims/surname", "sn");
        }
    };

    private static final HashSet<String> UPN_SUFFIXES = new HashSet<String>() {
        {
            add("sso.labs.eng.vmware.com");
            add("foo.sso.labs.eng.vmware.com");
        }
    };

    private static final List<String> CONNECTION_STRINGS = new LinkedList<String>() {
        {
            add("dc=ssolabs,dc=eng,dc=vmware,dc=com");
        }
    };

    @Test
    public void testGetIdentityProviderDTO_WithoutSchemaMapping() throws DTOMapperException {
        IIdentityStoreData identityStore = getTestADIdentityProvider();
        IdentityProviderDTO identityProviderDTO = IdentityProviderMapper.getIdentityProviderDTO(identityStore);
        assertEquals(AD_PROVIDER_NAME, identityProviderDTO.getName());
        assertFalse(identityProviderDTO.isMachineAccount());
        assertEquals(AD_SPN, identityProviderDTO.getServicePrincipalName());
        assertEquals(3, identityProviderDTO.getAttributesMap().size());
        assertEquals("Failed asserting linkAccountWithUPN", ACCOUNT_LINKING_WITH_UPN, identityProviderDTO.getLinkAccountWithUPN());
        assertEquals("Failed asserting hintAttributeName", HINT_ATTR_NAME, identityProviderDTO.getHintAttributeName());
    }

    @Test
    public void testGetIdentityProviderDTO_NonExternalDomain() {
        IIdentityStoreData systemIDP = IdentityStoreData.CreateSystemIdentityStoreData(SYSTEM_TENANT);
        IdentityProviderDTO identityProviderDTO = IdentityProviderMapper.getIdentityProviderDTO(systemIDP);
        assertEquals(DomainType.SYSTEM_DOMAIN.name(), identityProviderDTO.getDomainType());
        assertEquals(SYSTEM_TENANT, identityProviderDTO.getName());
    }

    @Test(expected = DTOMapperException.class)
    public void testGetIdentityProviderDTOWithInvalidInput() {
        IdentityProviderMapper.getIdentityProviderDTO(null);
    }

    @Test
    public void testGetIdentityStoreData_LDAP(){
         IdentityProviderDTO ldapIDP = IdentityProviderDTO.builder()
                                                          .withDomainType(DomainType.EXTERNAL_DOMAIN.name())
                                                          .withName(LDAP_NAME)
                                                          .withAlias(LDAP_ALIAS)
                                                          .withType(IdentityProviderType.IDENTITY_STORE_TYPE_LDAP.name())
                                                          .withAuthenticationType(AuthenticationType.PASSWORD.name())
                                                          .withUsername(LDAP_UN)
                                                          .withPassword(PASSWORD)
                                                          .withAttributesMap(attributesMap)
                                                          .withUserBaseDN(USER_BASE_DN)
                                                          .withSearchTimeOutInSeconds(0L)
                                                          .withGroupBaseDN(GROUP_BASE_DN)
                                                          .withSchema(null)
                                                          .withFriendlyName(FRIENDLY_NAME)
                                                          .withUPNSuffixes(UPN_SUFFIXES)
                                                          .withConnectionStrings(CONNECTION_STRINGS)
                                                          .withSiteAffinityEnabled(true)
                                                          .withMatchingRuleInChainEnabled(true)
                                                          .withBaseDnForNestedGroupsEnabled(false)
                                                          .withDirectGroupsSearchEnabled(false)
                                                          .withHintAttributeName(HINT_ATTR_NAME)
                                                          .withLinkAccountWithUPN(ACCOUNT_LINKING_WITH_UPN)
                                                          .build();

        IIdentityStoreData identityStoreData = IdentityProviderMapper.getIdentityStoreData(ldapIDP);

        assertEquals(DomainType.EXTERNAL_DOMAIN.name(), identityStoreData.getDomainType().name());
        assertEquals(LDAP_NAME, identityStoreData.getName());
        assertEquals(LDAP_ALIAS, identityStoreData.getExtendedIdentityStoreData().getAlias());
        assertEquals(USER_BASE_DN,identityStoreData.getExtendedIdentityStoreData().getUserBaseDn());
        assertEquals(GROUP_BASE_DN,identityStoreData.getExtendedIdentityStoreData().getGroupBaseDn());
        assertEquals(LDAP_UN, identityStoreData.getExtendedIdentityStoreData().getUserName());
        assertEquals(2, identityStoreData.getExtendedIdentityStoreData().getUpnSuffixes().size());
        assertNull(identityStoreData.getExtendedIdentityStoreData().getIdentityStoreSchemaMapping());
        assertEquals(IdentityStoreType.IDENTITY_STORE_TYPE_LDAP.name(), identityStoreData.getExtendedIdentityStoreData().getProviderType().name());
        assertEquals(FRIENDLY_NAME, identityStoreData.getExtendedIdentityStoreData().getFriendlyName());
        assertEquals(0, identityStoreData.getExtendedIdentityStoreData().getSearchTimeoutSeconds());
        assertEquals(3, identityStoreData.getExtendedIdentityStoreData().getAttributeMap().size());
        assertEquals(1,identityStoreData.getExtendedIdentityStoreData().getConnectionStrings().size());
        assertEquals("Failed asserting linkAccountWithUPN", ACCOUNT_LINKING_WITH_UPN, identityStoreData.getExtendedIdentityStoreData().getCertLinkingUseUPN());
        assertEquals("Failed asserting hintAttributeName", HINT_ATTR_NAME, identityStoreData.getExtendedIdentityStoreData().getCertUserHintAttributeName());
        assertEquals(IdentityProviderMapper.MATCHING_RULE_IN_CHAIN_ENABLED,
                identityStoreData.getExtendedIdentityStoreData().getFlags() & IdentityProviderMapper.MATCHING_RULE_IN_CHAIN_ENABLED);
        assertEquals(IdentityProviderMapper.SITE_AFFINITY_ENABLED,
                identityStoreData.getExtendedIdentityStoreData().getFlags() & IdentityProviderMapper.SITE_AFFINITY_ENABLED);
        assertEquals(IdentityProviderMapper.BASE_DN_FOR_NESTED_GROUPS_DISABLED,
                identityStoreData.getExtendedIdentityStoreData().getFlags() & IdentityProviderMapper.BASE_DN_FOR_NESTED_GROUPS_DISABLED);
    }

    @Test
    public void testGetIdentityProviderDTO_LocalOS() {
        IIdentityStoreData localOSProvider = IdentityStoreData.CreateLocalOSIdentityStoreData(LOCALOS_NAME, LOCALOS_ALIAS);
        IdentityProviderDTO localOSDTO = IdentityProviderMapper.getIdentityProviderDTO(localOSProvider);
        assertEquals(LOCALOS_NAME, localOSDTO.getName());
        assertEquals(LOCALOS_ALIAS, localOSDTO.getAlias());
    }

    @Test
    public void testGetIdentityStoreData_LocalOS() {
        IdentityProviderDTO localOsProvider = IdentityProviderDTO.builder()
                                                                 .withName(LOCALOS_NAME)
                                                                 .withAlias(LOCALOS_ALIAS)
                                                                 .withType(IdentityProviderType.IDENTITY_STORE_TYPE_LOCAL_OS.name())
                                                                 .build();

        IIdentityStoreData localOsStore = IdentityProviderMapper.getIdentityStoreData(localOsProvider);
        assertEquals(LOCALOS_NAME, localOsStore.getName());
        assertEquals(LOCALOS_ALIAS, localOsStore.getExtendedIdentityStoreData().getAlias());
    }

    @Test
    public void testGetIdentityStoreData_LDAPWithADMapping(){
         IdentityProviderDTO ldapIDP = IdentityProviderDTO.builder()
                                                          .withDomainType(DomainType.EXTERNAL_DOMAIN.name())
                                                          .withName(LDAP_NAME)
                                                          .withAlias(LDAP_ALIAS)
                                                          .withType(IdentityProviderType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING.name())
                                                          .withAuthenticationType(AuthenticationType.PASSWORD.name())
                                                          .withUsername(LDAP_UN)
                                                          .withPassword(PASSWORD)
                                                          .withAttributesMap(attributesMap)
                                                          .withUserBaseDN(USER_BASE_DN)
                                                          .withSearchTimeOutInSeconds(0L)
                                                          .withGroupBaseDN(GROUP_BASE_DN)
                                                          .withSchema(null)
                                                          .withFriendlyName(FRIENDLY_NAME)
                                                          .withUPNSuffixes(UPN_SUFFIXES)
                                                          .withConnectionStrings(CONNECTION_STRINGS)
                                                          .withHintAttributeName(HINT_ATTR_NAME)
                                                          .withLinkAccountWithUPN(ACCOUNT_LINKING_WITH_UPN)
                                                          .build();
        IIdentityStoreData identityStoreData = IdentityProviderMapper.getIdentityStoreData(ldapIDP);
        assertEquals(DomainType.EXTERNAL_DOMAIN.name(), identityStoreData.getDomainType().name());
        assertEquals(LDAP_NAME, identityStoreData.getName());
        assertEquals(LDAP_ALIAS, identityStoreData.getExtendedIdentityStoreData().getAlias());
        assertEquals(USER_BASE_DN,identityStoreData.getExtendedIdentityStoreData().getUserBaseDn());
        assertEquals(GROUP_BASE_DN,identityStoreData.getExtendedIdentityStoreData().getGroupBaseDn());
        assertEquals(LDAP_UN, identityStoreData.getExtendedIdentityStoreData().getUserName());
        assertEquals(2, identityStoreData.getExtendedIdentityStoreData().getUpnSuffixes().size());
        assertNull(identityStoreData.getExtendedIdentityStoreData().getIdentityStoreSchemaMapping());
        assertEquals(IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING.name(), identityStoreData.getExtendedIdentityStoreData().getProviderType().name());
        assertEquals(FRIENDLY_NAME, identityStoreData.getExtendedIdentityStoreData().getFriendlyName());
        assertEquals(0, identityStoreData.getExtendedIdentityStoreData().getSearchTimeoutSeconds());
        assertEquals(3, identityStoreData.getExtendedIdentityStoreData().getAttributeMap().size());
        assertEquals(1,identityStoreData.getExtendedIdentityStoreData().getConnectionStrings().size());
        assertEquals(HINT_ATTR_NAME, identityStoreData.getExtendedIdentityStoreData().getCertUserHintAttributeName());
        assertEquals(ACCOUNT_LINKING_WITH_UPN, identityStoreData.getExtendedIdentityStoreData().getCertLinkingUseUPN());
    }

    @Test
    public void testGetIdentityProviderDTO_WithSchemaMapping() throws DTOMapperException {

        IIdentityStoreData identityStore = getTestADWithSchemaMapping();
        IdentityProviderDTO identityProvider = IdentityProviderMapper.getIdentityProviderDTO(identityStore);
        assertEquals(AD_PROVIDER_NAME, identityProvider.getName());
        assertEquals(AD_SPN, identityProvider.getServicePrincipalName());
        assertEquals(HINT_ATTR_NAME, identityProvider.getHintAttributeName());
        assertEquals(ACCOUNT_LINKING_WITH_UPN, identityProvider.getLinkAccountWithUPN());

        assertEquals(3, identityProvider.getAttributesMap().size());
        assertEquals(IdentityProviderType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY.name(), identityProvider.getType());
        assertEquals(AuthenticationType.USE_KERBEROS.name(), identityProvider.getAuthenticationType());
        assertFalse(identityProvider.isMachineAccount());
        assertEquals(4,identityProvider.getSchema().size());
    }

    @Test
    public void testGetIdentityStoreData_WithSchemaMapping() throws DTOMapperException {
        IIdentityStoreData identityStoreData = IdentityProviderMapper.getIdentityStoreData(getTestADIdentityProviderDTO());

        assertEquals(DomainType.EXTERNAL_DOMAIN, identityStoreData.getDomainType());
        assertEquals(AD_PROVIDER_NAME, identityStoreData.getName());
        assertNull(identityStoreData.getExtendedIdentityStoreData().getAlias());
        assertEquals(IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY, identityStoreData.getExtendedIdentityStoreData().getProviderType());
        assertEquals(300, identityStoreData.getExtendedIdentityStoreData().getSearchTimeoutSeconds()); // Default
        assertEquals(AD_SPN, identityStoreData.getExtendedIdentityStoreData().getServicePrincipalName());
        assertEquals(HINT_ATTR_NAME, identityStoreData.getExtendedIdentityStoreData().getCertUserHintAttributeName());
        assertEquals(ACCOUNT_LINKING_WITH_UPN, identityStoreData.getExtendedIdentityStoreData().getCertLinkingUseUPN());

        // Assert Schema
        IdentityStoreObjectMapping userSchema = identityStoreData.getExtendedIdentityStoreData().getIdentityStoreSchemaMapping().getObjectMapping(ObjectClass.User.getAttributeName());
        assertEquals(4, userSchema.getAttributeMappings().size());
        IdentityStoreObjectMapping groupSchema = identityStoreData.getExtendedIdentityStoreData().getIdentityStoreSchemaMapping().getObjectMapping(ObjectClass.Group.getAttributeName());
        assertEquals(4, groupSchema.getAttributeMappings().size());
        IdentityStoreObjectMapping pwdSchema = identityStoreData.getExtendedIdentityStoreData().getIdentityStoreSchemaMapping().getObjectMapping(ObjectClass.PasswordSettings.getAttributeName());
        assertEquals(1, pwdSchema.getAttributeMappings().size());
        IdentityStoreObjectMapping domainSchema = identityStoreData.getExtendedIdentityStoreData().getIdentityStoreSchemaMapping().getObjectMapping(ObjectClass.Domain.getAttributeName());
        assertEquals(1, domainSchema.getAttributeMappings().size());
    }

    private IdentityStoreData getTestADIdentityProvider() {
        return IdentityStoreData.createActiveDirectoryIdentityStoreDataWithPIVControls(AD_PROVIDER_NAME,
                                                                        USER_NAME,
                                                                        false,
                                                                        AD_SPN,
                                                                        PASSWORD,
                                                                        attributesMap,
                                                                        null,
                                                                        0, //flags
                                                                        null,
                                                                        HINT_ATTR_NAME,
                                                                        ACCOUNT_LINKING_WITH_UPN);
    }

    private IdentityStoreData getTestADWithSchemaMapping() {
        return IdentityStoreData.createActiveDirectoryIdentityStoreDataWithPIVControls(AD_PROVIDER_NAME,
                                                                        USER_NAME,
                                                                        false,
                                                                        AD_SPN,
                                                                        PASSWORD,
                                                                        attributesMap,
                                                                        getADSchemaMapping(),
                                                                        0, //flags
                                                                        null,
                                                                        HINT_ATTR_NAME,
                                                                        ACCOUNT_LINKING_WITH_UPN
                                                                        );
    }

    private IdentityProviderDTO getTestADIdentityProviderDTO() {

        Map<String,SchemaObjectMappingDTO> schemaMappings = new HashMap<String,SchemaObjectMappingDTO>();
        // User schema mapping
        Map<String, String> userAttributeMappings = new HashMap<String, String>();
        userAttributeMappings.put("UserAttributeAccountName",USER_ATTRIBUTE_ACCOUNT_NAME);
        userAttributeMappings.put("UserAttributeDescription", USER_ATTRIBUTE_DESC);
        userAttributeMappings.put("UserAttributeDisplayName", USER_ATTRIBUTE_DISPLAY_NAME);
        userAttributeMappings.put("UserAttributeEmail", USER_ATTRIBUTE_EMAIL);
        SchemaObjectMappingDTO userSchemaMapping = SchemaObjectMappingDTO.builder()
                                                                         .withObjectClass(OBJECT_CLASS_USER)
                                                                         .withAttributeMappings(userAttributeMappings)
                                                                         .build();
        schemaMappings.put(ObjectClass.User.getAttributeName(), userSchemaMapping);

        // Group schema mapping
        Map<String, String> groupAttributeMappings = new HashMap<String, String>();
        groupAttributeMappings.put("GroupAttributeAccountName",GROUP_ATTRIBUTE_ACCOUNT_NAME);
        groupAttributeMappings.put("GroupAttributeDescription", GROUP_ATTRIBUTE_DESC);
        groupAttributeMappings.put("GroupAttributeMemberOf", GROUP_ATTRIBUTE_MEMBER_OF);
        groupAttributeMappings.put("GroupAttributeMembersList", GROUP_ATTRIBUTE_MEMBER);
        SchemaObjectMappingDTO groupSchemaMapping = SchemaObjectMappingDTO.builder()
                        .withObjectClass(OBJECT_CLASS_GROUP)
                        .withAttributeMappings(groupAttributeMappings)
                        .build();
        schemaMappings.put(ObjectClass.Group.getAttributeName(), groupSchemaMapping);

        // Pwd Setting schema mapping
        Map<String, String> pwdAttributeMappings = new HashMap<String, String>();
        pwdAttributeMappings.put("PasswordSettingsAttributeMaximumPwdAge", PWD_ATTRBIUTE_MAX_AGE);
        SchemaObjectMappingDTO pwdSchemaMapping = SchemaObjectMappingDTO.builder()
                        .withObjectClass(OBJECT_CLASS_PWD_SETTINGS)
                        .withAttributeMappings(pwdAttributeMappings)
                        .build();
        schemaMappings.put(ObjectClass.PasswordSettings.getAttributeName(),pwdSchemaMapping);

        // Domain schema mapping
        Map<String, String> domainAttributeMappings = new HashMap<String, String>();
        domainAttributeMappings.put("DomainAttributeMaxPwdAge", DOMAIN_ATTRIBUTE_MAX_PWD_AGE);
        SchemaObjectMappingDTO domainSchema = SchemaObjectMappingDTO.builder()
                        .withObjectClass(OBJECT_CLASS_DOMAIN)
                        .withAttributeMappings(pwdAttributeMappings)
                        .build();
        schemaMappings.put(ObjectClass.Domain.getAttributeName(), domainSchema);

        return IdentityProviderDTO.builder().withDomainType(DomainType.EXTERNAL_DOMAIN.name())
               .withName(AD_PROVIDER_NAME)
               .withType(IdentityProviderType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY.name())
               .withAuthenticationType(AuthenticationType.USE_KERBEROS.name())
               .withUsername(USER_NAME)
               .withMachineAccount(true)
               .withServicePrincipalName(AD_SPN)
               .withPassword(PASSWORD)
               .withAttributesMap(attributesMap)
               .withSchema(schemaMappings)
               .withHintAttributeName(HINT_ATTR_NAME)
               .withLinkAccountWithUPN(ACCOUNT_LINKING_WITH_UPN)
               .build();
    }

    private IdentityStoreSchemaMapping getADSchemaMapping() {
        IdentityStoreSchemaMapping.Builder schemaMapBuilder = new IdentityStoreSchemaMapping.Builder();
        IdentityStoreObjectMapping.Builder objectMapBuilder = null;
        objectMapBuilder = new IdentityStoreObjectMapping.Builder(ObjectIds.ObjectIdUser);
        objectMapBuilder.setObjectClass("user");
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl, "userAccountControl"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName, "sAMAccountName"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription, "description"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDisplayName, "displayname"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail, "mail"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName, "givenName"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName, "sn"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime, "lockoutTime"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeMemberOf, "memberof"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId, "objectSid"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject, "msDS-ResultantPSO"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrimaryGroupId, "primaryGroupID"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName, "userPrincipalName"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet, "pwdLastSet"));
        schemaMapBuilder.addObjectMappings(objectMapBuilder.buildObjectMapping());

        objectMapBuilder = new IdentityStoreObjectMapping.Builder(ObjectIds.ObjectIdGroup);
        objectMapBuilder.setObjectClass("group");
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName, "sAMAccountName"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription, "description"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMemberOf, "memberof"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList, "member"));
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId, "objectSid"));
        schemaMapBuilder.addObjectMappings(objectMapBuilder.buildObjectMapping());

        objectMapBuilder = new IdentityStoreObjectMapping.Builder(ObjectIds.ObjectIdPasswordSettings);
        objectMapBuilder.setObjectClass("msDS-PasswordSettings");
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.PasswordSettingsAttributeMaximumPwdAge, "msDS-MaximumPasswordAge"));
        schemaMapBuilder.addObjectMappings(objectMapBuilder.buildObjectMapping());

        objectMapBuilder = new IdentityStoreObjectMapping.Builder(ObjectIds.ObjectIdDomain);
        objectMapBuilder.setObjectClass("domain");
        objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.DomainAttributeMaxPwdAge, "maxPwdAge"));
        schemaMapBuilder.addObjectMappings(objectMapBuilder.buildObjectMapping());

        return schemaMapBuilder.buildSchemaMapping();
    }

}
