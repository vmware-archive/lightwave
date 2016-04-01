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

import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.vmware.identity.idm.AttributeConfig;
import com.vmware.identity.idm.ExternalIDPCertChainInvalidTrustedPathException;
import com.vmware.identity.idm.ExternalIDPExtraneousCertsInCertChainException;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.ServiceEndpoint;
import com.vmware.identity.idm.TokenClaimAttribute;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.ExternalIDPDTO;
import com.vmware.identity.rest.idm.data.ServiceEndpointDTO;
import com.vmware.identity.rest.idm.data.TokenClaimGroupDTO;

/**
 * Mapper for External Identity providers such as ADFS, Shibboleth etc
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 *
 */
public class ExternalIDPMapper {

    @SuppressWarnings("unchecked")
    public static ExternalIDPDTO getExternalIDPDTO(IDPConfig config) {
        ExternalIDPDTO.Builder builder = ExternalIDPDTO.builder();
        builder.withEntityID(config.getEntityID());
        builder.withAlias(config.getAlias());
        builder.withNameIDFormats(config.getNameIDFormats());
        builder.withSsoServices(getServiceEndpointDTOs(config.getSsoServices()));
        builder.withSloServices(getServiceEndpointDTOs(config.getSloServices()));

        builder.withSigningCertificates(CertificateMapper.getCertificateChainDTO((List<Certificate>)(List<?>) config.getSigningCertificateChain()));

        builder.withSubjectFormats(getSubjectFormats(config.getSubjectFormatMappings()));
        builder.withTokenClaimGroups(getTokenClaimGroupDTOs(config.getTokenClaimGroupMappings()));
        builder.withJitEnabled(config.getJitAttribute());
        builder.withUpnSuffix(config.getUpnSuffix());

        return builder.build();
    }

    public static Collection<ExternalIDPDTO> getExternalIDPDTOs(Collection<IDPConfig> configs) {
        Collection<ExternalIDPDTO> dtos = new ArrayList<ExternalIDPDTO>();

        if (configs != null) {
            for (IDPConfig config : configs) {
                dtos.add(getExternalIDPDTO(config));
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
    public static IDPConfig getIDPConfig(ExternalIDPDTO externalIDP) throws ExternalIDPExtraneousCertsInCertChainException, ExternalIDPCertChainInvalidTrustedPathException {
        IDPConfig config = null;
        try {
            config = new IDPConfig(externalIDP.getEntityID());
            config.setAlias(externalIDP.getAlias());
            config.setNameIDFormats(externalIDP.getNameIDFormats());
            config.setSsoServices(getServiceEndpoints(externalIDP.getSsoServices()));
            config.setSloServices(getServiceEndpoints(externalIDP.getSloServices()));
            config.setSigningCertificateChain((List<X509Certificate>)(List<?>) CertificateMapper.getCertificates(externalIDP.getSigningCertificates().getCertificates()));
            config.setSubjectFormatMappings(getAttributeConfigs(externalIDP.getSubjectFormats()));
            config.setTokenClaimGroupMappings(getTokenClaimGroupMappings(externalIDP.getTokenClaimGroups()));
            config.setJitAttribute(externalIDP.isJitEnabled() == null ? false : externalIDP.isJitEnabled());
            config.setUpnSuffix(externalIDP.getUpnSuffix());
        } catch (Exception e) {
            throw new DTOMapperException("Failed to map ExternalIDPDTO to IDPConfig", e);
        }
        return config;
    }

}
