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

import java.net.MalformedURLException;
import java.net.URL;
import java.security.cert.Certificate;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.vmware.identity.idm.AlternativeOCSPList;
import com.vmware.identity.idm.ClientCertPolicy;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.AlternativeOCSPConnectionsDTO;
import com.vmware.identity.rest.idm.data.ClientCertificatePolicyDTO;
import com.vmware.identity.rest.idm.data.AlternativeOCSPListDTO;

/**
 * Mapper for client certificate policies
 * 
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class ClientCertificatePolicyMapper {

    public static ClientCertificatePolicyDTO getClientCertificatePolicyDTO(
            ClientCertPolicy clientCertPolicy) {
        List<String> objectIdentifiers = null;
        List<CertificateDTO> trustedCertificates = null;
        String ocspUrl = null;
        String crlUrl = null;

        if (clientCertPolicy.getOIDs() != null) {
            objectIdentifiers = Arrays.asList(clientCertPolicy.getOIDs());
        }
        if (clientCertPolicy.getTrustedCAs() != null) {
            trustedCertificates = CertificateMapper.getCertificateDTOs(Arrays
                    .asList(clientCertPolicy.getTrustedCAs()));
        }
        if (clientCertPolicy.getOCSPUrl() != null) {
            ocspUrl = clientCertPolicy.getOCSPUrl().toExternalForm();
        }
        if (clientCertPolicy.getCRLUrl() != null) {
            crlUrl = clientCertPolicy.getCRLUrl().toExternalForm();
        }

        Map<String, AlternativeOCSPList> alternativeOCSPConnections = clientCertPolicy.get_siteOCSPList();
        AlternativeOCSPConnectionsDTO altOCSPConnectionsDTO = null;

        if (alternativeOCSPConnections != null) {
            altOCSPConnectionsDTO = AlternativeOCSPConnectionsMapper.getAlternativeOCSPConnectionsDTO(alternativeOCSPConnections);
        }

        return ClientCertificatePolicyDTO
                .builder()
                .withCertPolicyOIDs(objectIdentifiers)
                .withTrustedCACertificates(trustedCertificates)
                .withOcspEnabled(clientCertPolicy.useOCSP())
                .withFailOverToCrlEnabled(clientCertPolicy.useCRLAsFailOver())
                .withOcspUrlOverride(ocspUrl)
                .withRevocationCheckEnabled(
                        clientCertPolicy.revocationCheckEnabled())
                .withCrlDistributionPointUsageEnabled(
                        clientCertPolicy.useCertCRL())
                .withCrlDistributionPointOverride(crlUrl)
                .withUserNameHintEnabled(clientCertPolicy.getEnableHint())
                .withAltOCSPConnections(altOCSPConnectionsDTO)
                .build();
    }

    public static ClientCertPolicy getClientCertificatePolicy(
            ClientCertificatePolicyDTO certPolicyDTO) {

        ClientCertPolicy certPolicy = new ClientCertPolicy();

        if (certPolicyDTO.getCertPolicyOIDs() != null) {
            // object identifiers
            int totalObjectIds = certPolicyDTO.getCertPolicyOIDs().size();
            String[] objectIdentifiers = certPolicyDTO.getCertPolicyOIDs()
                    .toArray(new String[totalObjectIds]);
            certPolicy.setOIDs(objectIdentifiers);
        }

        if (certPolicyDTO.getTrustedCACertificates() != null) {
            // trusted certificates
            int totalTrustedCertificates = certPolicyDTO
                    .getTrustedCACertificates().size();
            List<Certificate> certs = CertificateMapper
                    .getCertificates(certPolicyDTO.getTrustedCACertificates());
            Certificate[] trustedCertificates = new Certificate[totalTrustedCertificates];
            for (int i = 0; i < totalTrustedCertificates; i++) {
                trustedCertificates[i] = certs.get(i);
            }
            certPolicy.setTrustedCAs(trustedCertificates);
        }

        AlternativeOCSPConnectionsDTO altOCSPConnectionsDTO = certPolicyDTO.getAltOCSPConnections();
        if (altOCSPConnectionsDTO != null) {
            HashMap<String, AlternativeOCSPList> altOCSPConnectionsMap =
                    AlternativeOCSPConnectionsMapper.getAlternativeOCSPConnections(altOCSPConnectionsDTO);
            certPolicy.set_siteOCSPMap(altOCSPConnectionsMap);
        }

        try {
            // ocsp related settings
            certPolicy
                    .setUseOCSP(certPolicyDTO.isOcspEnabled() != null ? certPolicyDTO
                            .isOcspEnabled() : false);
            certPolicy.setUseCRLAsFailOver(certPolicyDTO
                    .isFailOverToCrlEnabled() != null ? certPolicyDTO
                    .isFailOverToCrlEnabled() : false);
            if (certPolicyDTO.getOcspUrlOverride() != null) {
                certPolicy.setOCSPUrl(new URL(certPolicyDTO
                        .getOcspUrlOverride()));
            }

            // crl related settings
            certPolicy.setRevocationCheckEnabled(certPolicyDTO
                    .isRevocationCheckEnabled() != null ? certPolicyDTO
                    .isRevocationCheckEnabled() : true);
            certPolicy
                    .setUseCertCRL(certPolicyDTO
                            .isCrlDistributionPointUsageEnabled() != null ? certPolicyDTO
                            .isCrlDistributionPointUsageEnabled() : true);
            if (certPolicyDTO.getCrlDistributionPointOverride() != null) {
                certPolicy.setCRLUrl(new URL(certPolicyDTO
                        .getCrlDistributionPointOverride()));
            }

            // other settings
            certPolicy.setEnableHint(certPolicyDTO.isUserNameHintEnabled());
        } catch (MalformedURLException e) {
            throw new DTOMapperException(
                    "Failed to convert ClientCertPolicyDTO to ClientCertPolicy",
                    e);
        }

        return certPolicy;
    }
}
