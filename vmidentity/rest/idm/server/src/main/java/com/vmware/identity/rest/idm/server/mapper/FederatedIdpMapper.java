/*
 *  Copyright (c) 2012-2017 VMware, Inc.  All Rights Reserved.
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


import com.vmware.identity.idm.AttributeConfig;
import com.vmware.identity.idm.ExternalIDPCertChainInvalidTrustedPathException;
import com.vmware.identity.idm.ExternalIDPExtraneousCertsInCertChainException;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.OidcConfig;
import com.vmware.identity.idm.ServiceEndpoint;
import com.vmware.identity.idm.TokenClaimAttribute;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.core.server.util.Validate;
import com.vmware.identity.rest.idm.data.FederatedIdpDTO;
import com.vmware.identity.rest.idm.data.FederatedOidcConfigDTO;
import com.vmware.identity.rest.idm.data.FederatedSamlConfigDTO;
import com.vmware.identity.rest.idm.data.ServiceEndpointDTO;
import com.vmware.identity.rest.idm.data.TokenClaimGroupDTO;

import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Mapper for Federated Identity providers
 *
 * @author Sriram Nambakam
 *
 */
public class FederatedIdpMapper {

    @SuppressWarnings("unchecked")
    public static FederatedIdpDTO getFederatedIdpDTO(IDPConfig config) {
        FederatedIdpDTO.Builder builder = FederatedIdpDTO.builder();
        builder.withEntityID(config.getEntityID());
        builder.withProtocol(config.getProtocol());
        builder.withAlias(config.getAlias());
        builder.withJitEnabled(config.getJitAttribute());
        builder.withUpnSuffix(config.getUpnSuffix());
        String protocol = config.getProtocol();
        if (protocol.equals(IDPConfig.IDP_PROTOCOL_SAML_2_0)) {
            builder.withSamlConfig(getSamlConfigDTO(config));
        } else if (protocol.equals(IDPConfig.IDP_PROTOCOL_OAUTH_2_0)) {
            builder.withOidcConfig(getOidcConfigDTO(config));
        }
        return builder.build();
    }

    public static Collection<FederatedIdpDTO> getFederatedIdpDTOs(Collection<IDPConfig> configs) {
        Collection<FederatedIdpDTO> dtos = new ArrayList<FederatedIdpDTO>();

        if (configs != null) {
            for (IDPConfig config : configs) {
                dtos.add(getFederatedIdpDTO(config));
            }
        }
        return dtos;
    }

    public static ServiceEndpointDTO getServiceEndpointDTO(ServiceEndpoint endpoint) {
        ServiceEndpointDTO.Builder builder = ServiceEndpointDTO.builder();
        builder.withName(endpoint.getName());
        builder.withBinding(endpoint.getBinding());
        builder.withEndpoint(endpoint.getEndpoint());
        return builder.build();
    }

    public static Collection<ServiceEndpointDTO> getServiceEndpointDTOs(Collection<ServiceEndpoint> endpoints) {
        Collection<ServiceEndpointDTO> dtos = new ArrayList<ServiceEndpointDTO>();

        if (endpoints != null) {
            for (ServiceEndpoint endpoint : endpoints) {
                dtos.add(getServiceEndpointDTO(endpoint));
            }
        }
        return dtos;
    }

    public static FederatedSamlConfigDTO getSamlConfigDTO(IDPConfig config) {
        FederatedSamlConfigDTO.Builder samlConfigBuilder = new FederatedSamlConfigDTO.Builder();

        samlConfigBuilder.withNameIDFormats(config.getNameIDFormats());
        samlConfigBuilder.withSsoServices(getServiceEndpointDTOs(config.getSsoServices()));
        samlConfigBuilder.withSloServices(getServiceEndpointDTOs(config.getSloServices()));
        samlConfigBuilder.withSigningCertificates(CertificateMapper.getCertificateChainDTO(
                (List<Certificate>)(List<?>) config.getSigningCertificateChain())
        );
        samlConfigBuilder.withSubjectFormats(getSubjectFormats(config.getSubjectFormatMappings()));
        samlConfigBuilder.withTokenClaimGroups(getTokenClaimGroupDTOs(config.getTokenClaimGroupMappings()));

        return samlConfigBuilder.build();
    }

    public static FederatedOidcConfigDTO getOidcConfigDTO(IDPConfig config) {
        OidcConfig oidcConfig = config.getOidcConfig();

        Validate.notNull(oidcConfig);

        return new FederatedOidcConfigDTO.Builder()
                        .withIssuerType(oidcConfig.getIssuerType())
                        .withAuthorizeEndpoint(oidcConfig.getAuthorizeRedirectURI())
                        .withClientId(oidcConfig.getClientId())
                        .withClientSecret(oidcConfig.getClientSecret())
                        .withJwksEndpoint(oidcConfig.getJwksURI())
                        .withLogoutEndpoint(oidcConfig.getLogoutURI())
                        .withMetadataEndpoint(oidcConfig.getMetadataURI())
                        .withPostLogoutEndpoint(oidcConfig.getPostLogoutURI())
                        .withRedirectURL(oidcConfig.getRedirectURI())
                        .withTokenEndpoint(oidcConfig.getTokenRedirectURI())
                        .build();
    }

    public static ServiceEndpoint getServiceEndpoint(ServiceEndpointDTO endpoint) {
        return new ServiceEndpoint(endpoint.getName(), endpoint.getEndpoint(), endpoint.getBinding());
    }

    public static Collection<ServiceEndpoint> getServiceEndpoints(Collection<ServiceEndpointDTO> endpoints) {
        Collection<ServiceEndpoint> serviceEndpoints = new ArrayList<ServiceEndpoint>();

        if (endpoints != null) {
            for (ServiceEndpointDTO endpoint : endpoints) {
                serviceEndpoints.add(getServiceEndpoint(endpoint));
            }
        }
        return serviceEndpoints;
    }

    public static OidcConfig getOidcConfig(FederatedOidcConfigDTO oidcConfig) {
        Validate.notNull(oidcConfig);

        return new OidcConfig()
                    .setIssuerType(oidcConfig.getIssuerType())
                    .setAuthorizeRedirectURI(oidcConfig.getAuthorizeEndpoint())
                    .setClientId(oidcConfig.getClientId())
                    .setClientSecret(oidcConfig.getClientSecret())
                    .setJwksURI(oidcConfig.getJwksEndpoint())
                    .setLogoutURI(oidcConfig.getLogoutEndpoint())
                    .setMetadataURI(oidcConfig.getMetadataEndpoint())
                    .setPostLogoutURI(oidcConfig.getPostLogoutEndpoint())
                    .setTokenRedirectURI(oidcConfig.getTokenEndpoint())
                    .setRedirectURI(oidcConfig.getRedirectURL());
    }

    public static Map<String, String> getSubjectFormats(AttributeConfig[] attributes) {
        Map<String, String> formats = new HashMap<String, String>();

        if (attributes != null) {
            for (AttributeConfig attribute : attributes) {
                formats.put(attribute.getStoreAttribute(), attribute.getTokenSubjectFormat());
            }
        }
        return formats;
    }

    public static AttributeConfig[] getAttributeConfigs(Map<String, String> formats) {
        AttributeConfig[] attributes = null;

        if (formats != null) {
            attributes = new AttributeConfig[formats.size()];

            int i = 0;
            for (Map.Entry<String, String> entry : formats.entrySet()) {
                attributes[i++] = new AttributeConfig(entry.getValue(), entry.getKey());
            }
        }
        return attributes;
    }

    public static TokenClaimGroupDTO getTokenClaimGroupDTO(TokenClaimAttribute attribute, List<String> groups) {
        TokenClaimGroupDTO.Builder builder = TokenClaimGroupDTO.builder();
        builder.withClaimName(attribute.getClaimName());
        builder.withClaimValue(attribute.getClaimValue());
        builder.withGroups(groups);
        return builder.build();
    }

    public static List<TokenClaimGroupDTO> getTokenClaimGroupDTOs(Map<TokenClaimAttribute, List<String>> mapping) {
        List<TokenClaimGroupDTO> dtos = new ArrayList<TokenClaimGroupDTO>();

        if (mapping != null) {
            for (Map.Entry<TokenClaimAttribute, List<String>> entry : mapping.entrySet()) {
                dtos.add(getTokenClaimGroupDTO(entry.getKey(), entry.getValue()));
            }
        }
        return dtos;
    }

    public static Map<TokenClaimAttribute, List<String>> getTokenClaimGroupMappings(List<TokenClaimGroupDTO> claimGroups) {
        Map<TokenClaimAttribute, List<String>> mapping = new HashMap<TokenClaimAttribute, List<String>>();

        if (claimGroups != null) {
            for (TokenClaimGroupDTO claim : claimGroups) {
                mapping.put(new TokenClaimAttribute(claim.getClaimName(), claim.getClaimValue()), claim.getGroups());
            }
        }
        return mapping;
    }

    @SuppressWarnings("unchecked")
    public static IDPConfig
    getIDPConfig(
            FederatedIdpDTO federatedIdp
    ) throws ExternalIDPExtraneousCertsInCertChainException, ExternalIDPCertChainInvalidTrustedPathException {
        IDPConfig config = null;
        try {
            config = new IDPConfig(federatedIdp.getEntityID(), federatedIdp.getProtocol());
            config.setAlias(federatedIdp.getAlias());
            String protocol = config.getProtocol();
            if (protocol.equals(IDPConfig.IDP_PROTOCOL_SAML_2_0)) {
                FederatedSamlConfigDTO samlConfig = federatedIdp.getSamlConfig();
                config.setNameIDFormats(samlConfig.getNameIDFormats());
                config.setSsoServices(getServiceEndpoints(samlConfig.getSsoServices()));
                config.setSloServices(getServiceEndpoints(samlConfig.getSloServices()));
                config.setSigningCertificateChain(
                        (List<X509Certificate>) (List<?>) CertificateMapper.getCertificates(
                                samlConfig.getSigningCertificates().getCertificates()
                                )
                );
                config.setSubjectFormatMappings(getAttributeConfigs(samlConfig.getSubjectFormats()));
                config.setTokenClaimGroupMappings(getTokenClaimGroupMappings(samlConfig.getTokenClaimGroups()));
            } else if (protocol.equals(IDPConfig.IDP_PROTOCOL_OAUTH_2_0)) {
                config.setOidcConfig(getOidcConfig(federatedIdp.getOidcConfig()));
            }
            config.setJitAttribute(federatedIdp.isJitEnabled() == null ? false : federatedIdp.isJitEnabled());
            config.setUpnSuffix(federatedIdp.getUpnSuffix());
        } catch (Exception e) {
            throw new DTOMapperException("Failed to map FederatedIdpDTO to IDPConfig", e);
        }
        return config;
    }
}
