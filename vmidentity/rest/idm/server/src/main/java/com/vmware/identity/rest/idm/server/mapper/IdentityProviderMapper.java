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
package com.vmware.identity.rest.idm.server.mapper;

import java.util.Collection;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IIdentityStoreDataEx;
import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.IdentityStoreData;
import com.vmware.identity.idm.IdentityStoreObjectMapping;
import com.vmware.identity.idm.IdentityStoreSchemaMapping;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.IdentityProviderDTO;
import com.vmware.identity.rest.idm.data.SchemaObjectMappingDTO;
import com.vmware.identity.rest.idm.data.attributes.IdentityProviderType;

/**
 * Mapper for identity provider entity.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class IdentityProviderMapper {

    /**
     * Constants used for bit masking - These translates to boolean flags on {@link IdentityProviderDTO.class}
     */
    public static final int MATCHING_RULE_IN_CHAIN_ENABLED = 0x1;
    public static final int BASE_DN_FOR_NESTED_GROUPS_DISABLED = 0x2;
    public static final int DIRECT_GROUPS_SEARCH_ENABLED = 0x4;
    public static final int SITE_AFFINITY_ENABLED = 0x8;

    /**
     *  Map {@link IIdentityStoreData} object to {@link IdentityProviderDTO}
     *  This maps complete information on identity provider including external identity provider with schema mappings.
     *
     */
    public static IdentityProviderDTO getIdentityProviderDTO(IIdentityStoreData identityStore) throws DTOMapperException {
        IdentityProviderDTO.Builder providerBuilder = IdentityProviderDTO.builder();
        try {
            DomainType domain = identityStore.getDomainType();
            String domainName = identityStore.getName();
            providerBuilder.withDomainType(domain.name())
                           .withName(domainName);
            // Retrieve external identity provider attributes.
            if (domain == DomainType.EXTERNAL_DOMAIN) {
                IIdentityStoreDataEx extendedIdentityStoreData = identityStore.getExtendedIdentityStoreData();
                providerBuilder.withAlias(extendedIdentityStoreData.getAlias())
                       .withType(extendedIdentityStoreData.getProviderType().name())
                       .withAuthenticationType(extendedIdentityStoreData.getAuthenticationType().name())
                       .withFriendlyName(extendedIdentityStoreData.getFriendlyName())
                       .withUsername(extendedIdentityStoreData.getUserName())
                       .withSearchTimeOutInSeconds((long) extendedIdentityStoreData.getSearchTimeoutSeconds())
                       .withMachineAccount(extendedIdentityStoreData.useMachineAccount())
                       .withServicePrincipalName(extendedIdentityStoreData.getServicePrincipalName())
                       .withUserBaseDN(extendedIdentityStoreData.getUserBaseDn())
                       .withGroupBaseDN(extendedIdentityStoreData.getGroupBaseDn())
                       .withConnectionStrings(extendedIdentityStoreData.getConnectionStrings())
                       .withAttributesMap(extendedIdentityStoreData.getAttributeMap())
                       .withUPNSuffixes(extendedIdentityStoreData.getUpnSuffixes())
                       .withMatchingRuleInChainEnabled(isMatchingRuleInChainEnabled(extendedIdentityStoreData.getFlags()))
                       .withBaseDnForNestedGroupsEnabled(isBaseDnForNestedGroupsEnabled(extendedIdentityStoreData.getFlags()))
                       .withDirectGroupsSearchEnabled(isDirectGroupsSearchEnabled(extendedIdentityStoreData.getFlags()))
                       .withSiteAffinityEnabled(isSiteAffinityEnabled(extendedIdentityStoreData.getFlags()))
                       .withCertificates(CertificateMapper.getCertificateDTOs(extendedIdentityStoreData.getCertificates()))
                       .withLinkAccountWithUPN(extendedIdentityStoreData.getCertLinkingUseUPN())
                       .withHintAttributeName(extendedIdentityStoreData.getCertUserHintAttributeName());
                // Set schema mappings if any.
                IdentityStoreSchemaMapping idmSchemaMappings = extendedIdentityStoreData.getIdentityStoreSchemaMapping();
                if (idmSchemaMappings != null) {
                    Map<String, SchemaObjectMappingDTO> schemaMappings = populateObjectSchema(idmSchemaMappings.getObjectMappings());
                    providerBuilder.withSchema(schemaMappings);
                }
            }

            if (domain == DomainType.LOCAL_OS_DOMAIN) {
                if (identityStore.getExtendedIdentityStoreData() != null) {
                    providerBuilder.withAlias(identityStore.getExtendedIdentityStoreData().getAlias());
                }
            }
        } catch (Exception e) {
            throw new DTOMapperException(String.format("Failed to map object from %s to %s", IIdentityStoreData.class.getName(), IdentityProviderDTO.class.getName()), e);
        }
        return providerBuilder.build();
    }

    /**
     * Populate Identity store object schema mapping. This helper method is used to transform IDM based schema object mapping to
     * REST based schema mapping object.
     * @throws DTOMapperException
     */
    private static Map<String, SchemaObjectMappingDTO> populateObjectSchema(Collection<IdentityStoreObjectMapping> objectMappings) throws DTOMapperException {
        Map<String,SchemaObjectMappingDTO> objectSchemaMappings = new HashMap<String,SchemaObjectMappingDTO>();
        try {
            for (IdentityStoreObjectMapping objectMapping : objectMappings) {
                String objectId = objectMapping.getObjectId();
                String objectClass = objectMapping.getObjectClass();
                Map<String, String> attributeMappings = new HashMap<String, String>();
                for (IdentityStoreAttributeMapping attributeMapping : objectMapping.getAttributeMappings()) {
                    attributeMappings.put(attributeMapping.getAttributeId(), attributeMapping.getAttributeName());
                }
                objectSchemaMappings.put(objectId, new SchemaObjectMappingDTO(objectClass, attributeMappings));
            }
        } catch (Exception ex) {
            throw new DTOMapperException("Failed to map identity provider schema");
        }
        return objectSchemaMappings;
    }

    public static Set<IdentityProviderDTO> getIdentityProviderDTOs(Collection<IIdentityStoreData> idmStoreObjects) throws DTOMapperException {
        Set<IdentityProviderDTO> schemaMappings = new LinkedHashSet<IdentityProviderDTO>();
        for (IIdentityStoreData ids : idmStoreObjects) {
            schemaMappings.add(getIdentityProviderDTO(ids));
        }
        return schemaMappings;
    }

    /**
     * Maps REST based identity provider object {@link IdentityProviderDTO} to IDM based identity provider object {@link IIdentityStoreData}
     * This transformation logic is used while creating/adding a new identity provider via REST API.
     */
    public static IIdentityStoreData getIdentityStoreData(IdentityProviderDTO identityProvider) throws DTOMapperException {
        IdentityProviderType identityProviderType = IdentityProviderType.valueOf(identityProvider.getType());
        IIdentityStoreData identityStore = null;

        // NOTE : We do not support create/updates on vmdir(vsphere.local)
        switch (identityProviderType) {

            case IDENTITY_STORE_TYPE_LOCAL_OS:
                identityStore = IdentityStoreData.CreateLocalOSIdentityStoreData(identityProvider.getName(), identityProvider.getAlias());
                break;

            case IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY:
                // TODO : Cleanup IDM code to customize configurable parameters.
                identityStore = IdentityStoreData.createActiveDirectoryIdentityStoreDataWithPIVControls(identityProvider.getName(),
                                identityProvider.getUsername(),
                                identityProvider.isMachineAccount() != null ? identityProvider.isMachineAccount() : false,
                                identityProvider.getServicePrincipalName(),
                                identityProvider.getPassword(),
                                identityProvider.getAttributesMap(),
                                identityProvider.getSchema() != null ? populateObjectMappingData(identityProvider.getSchema()) : null,
                                0, //flags
                                null, //authnTypes
                                identityProvider.getHintAttributeName(),
                                identityProvider.getLinkAccountWithUPN()
                                );

                 break;

            case IDENTITY_STORE_TYPE_LDAP:
               identityStore = IdentityStoreData.CreateExternalIdentityStoreData(identityProvider.getName(),
                                                                                  identityProvider.getAlias(),
                                                                                  IdentityStoreType.IDENTITY_STORE_TYPE_LDAP,
                                                                                  AuthenticationType.valueOf(identityProvider.getAuthenticationType()),
                                                                                  identityProvider.getFriendlyName(),
                                                                                  (int) (identityProvider.getSearchTimeOutInSeconds() == null ? 0 : identityProvider.getSearchTimeOutInSeconds().longValue()),
                                                                                  identityProvider.getUsername(),
                                                                                  identityProvider.isMachineAccount() != null ? identityProvider.isMachineAccount() : false,
                                                                                  identityProvider.getServicePrincipalName(),
                                                                                  identityProvider.getPassword(),
                                                                                  identityProvider.getUserBaseDN(),
                                                                                  identityProvider.getGroupBaseDN(),
                                                                                  identityProvider.getConnectionStrings(),
                                                                                  identityProvider.getAttributesMap(),
                                                                                  identityProvider.getSchema() != null ? populateObjectMappingData(identityProvider.getSchema()) : null,
                                                                                  identityProvider.getUpnSuffixes(),
                                                                                  getFlag(identityProvider.isMatchingRuleInChainEnabled(),
                                                                                          identityProvider.isBaseDnForNestedGroupsEnabled(),
                                                                                          identityProvider.isDirectGroupsSearchEnabled(),
                                                                                          identityProvider.isSiteAffinityEnabled()),
                                                                                  CertificateMapper.getX509Certificates(identityProvider.getCertificates()),
                                                                                  null,
                                                                                  identityProvider.getHintAttributeName(),
                                                                                  identityProvider.getLinkAccountWithUPN());
                break;

            case IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING:
                identityStore = IdentityStoreData.CreateExternalIdentityStoreData(identityProvider.getName(),
                                                                                     identityProvider.getAlias(),
                                                                                     IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING,
                                                                                     AuthenticationType.valueOf(identityProvider.getAuthenticationType()),
                                                                                     identityProvider.getFriendlyName(),
                                                                                     (int) (identityProvider.getSearchTimeOutInSeconds() == null ? 0 : identityProvider.getSearchTimeOutInSeconds().longValue()),
                                                                                     identityProvider.getUsername(),
                                                                                     identityProvider.isMachineAccount() != null ? identityProvider.isMachineAccount() : false,
                                                                                     identityProvider.getServicePrincipalName(),
                                                                                     identityProvider.getPassword(),
                                                                                     identityProvider.getUserBaseDN(),
                                                                                     identityProvider.getGroupBaseDN(),
                                                                                     identityProvider.getConnectionStrings(),
                                                                                     identityProvider.getAttributesMap(),
                                                                                     identityProvider.getSchema() != null ? populateObjectMappingData(identityProvider.getSchema()) : null,
                                                                                     identityProvider.getUpnSuffixes(),
                                                                                     getFlag(identityProvider.isMatchingRuleInChainEnabled(),
                                                                                          identityProvider.isBaseDnForNestedGroupsEnabled(),
                                                                                          identityProvider.isDirectGroupsSearchEnabled(),
                                                                                          identityProvider.isSiteAffinityEnabled()),
                                                                                     CertificateMapper.getX509Certificates(identityProvider.getCertificates()),
                                                                                     null,
                                                                                     identityProvider.getHintAttributeName(),
                                                                                     identityProvider.getLinkAccountWithUPN());
                break;

            default:
                throw new DTOMapperException(String.format("Unsupported identity provider. Failed to map object from '%s' to '%s'", IIdentityStoreData.class.getName(), IdentityProviderDTO.class.getName()));
        }
        return identityStore;
    }

    private static int getFlag(Boolean matchingRuleInChainEnabled, Boolean baseDnForNestedGroupsEnabled, Boolean directGroupsSearchEnabled, Boolean siteAffinityEnabled) {
        int flag = 0;

        if (matchingRuleInChainEnabled != null && matchingRuleInChainEnabled) {
            flag = flag | MATCHING_RULE_IN_CHAIN_ENABLED;
        }

        if (baseDnForNestedGroupsEnabled != null && !baseDnForNestedGroupsEnabled) {
            flag = flag | BASE_DN_FOR_NESTED_GROUPS_DISABLED;
        }

        if (directGroupsSearchEnabled != null && directGroupsSearchEnabled) {
            flag = flag | DIRECT_GROUPS_SEARCH_ENABLED;
        }

        if (siteAffinityEnabled != null && siteAffinityEnabled) {
            flag = flag | SITE_AFFINITY_ENABLED;
        }

        return flag;
    }

    /**
     * This helper method is used while transforming REST based schema object mappings to IDM based schema object mappings.
     */
    private static IdentityStoreSchemaMapping populateObjectMappingData(Map<String,SchemaObjectMappingDTO> objectSchemaMappings) throws DTOMapperException {
        IdentityStoreSchemaMapping.Builder schemaMapBuilder = new IdentityStoreSchemaMapping.Builder();
        for (Entry<String, SchemaObjectMappingDTO> objectSchema : objectSchemaMappings.entrySet()) {
            String objectId = objectSchema.getKey();
            SchemaObjectMappingDTO objectMapping = objectSchema.getValue();
            String objectClass = objectMapping.getObjectClass();
            Map<String, String> attributeMappings = objectMapping.getAttributeMappings();
            IdentityStoreObjectMapping.Builder objectSchemaBuilder = new IdentityStoreObjectMapping.Builder(objectId);
            objectSchemaBuilder.setObjectClass(objectClass);
            for (Entry<String, String> attributeMapping : attributeMappings.entrySet()) {
                objectSchemaBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(attributeMapping.getKey(), attributeMapping.getValue()));
            }
            schemaMapBuilder.addObjectMappings(objectSchemaBuilder.buildObjectMapping());
        }
        return schemaMapBuilder.buildSchemaMapping();
    }

    public static Boolean isMatchingRuleInChainEnabled(int flag) {
        return (flag & MATCHING_RULE_IN_CHAIN_ENABLED) != 0;
    }

    public static Boolean isBaseDnForNestedGroupsEnabled(int flag) {
        return (flag & BASE_DN_FOR_NESTED_GROUPS_DISABLED) == 0;
    }

    public static Boolean isDirectGroupsSearchEnabled(int flag) {
        return (flag & DIRECT_GROUPS_SEARCH_ENABLED) != 0;
    }

    public static Boolean isSiteAffinityEnabled(int flag) {
        return (flag & SITE_AFFINITY_ENABLED) != 0;
    }

    public static IdentityProviderDTO sanitizeProbeResponse(IdentityProviderDTO identityProvider) {
        return IdentityProviderDTO.builder()
                                  .withAuthenticationType(identityProvider.getAuthenticationType())
                                  .withType(identityProvider.getType())
                                  .withConnectionStrings(identityProvider.getConnectionStrings())
                                  .build();
    }
}
