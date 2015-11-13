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

import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;

import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.data.RelyingPartyDTO;

/**
 * Mapper utility to map objects from {@link RelyingParty} to {@link RelyingPartyDTO} and vice-versa.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class RelyingPartyMapper {

    public static RelyingParty getRelyingParty(RelyingPartyDTO relyingPartyDTO) {
        RelyingParty rp = new RelyingParty(relyingPartyDTO.getName());
        rp.setUrl(relyingPartyDTO.getUrl());
        rp.setAssertionConsumerServices(AssertionConsumerServiceMapper.getAssertionCsConfigs(relyingPartyDTO.getAssertionConsumerServices()));
        rp.setAttributeConsumerServices(AttributeConsumerServiceMapper.getAttributeCsConfigs(relyingPartyDTO.getAttributeConsumerServices()));
        if (relyingPartyDTO.isAuthnRequestsSigned() != null) {
            rp.setAuthnRequestsSigned(relyingPartyDTO.isAuthnRequestsSigned());
        }
        rp.setCertificate(relyingPartyDTO.getCertificate() == null ? null : relyingPartyDTO.getCertificate().getX509Certificate());
        rp.setDefaultAssertionConsumerService(relyingPartyDTO.getDefaultAssertionConsumerService());
        rp.setDefaultAttributeConsumerService(relyingPartyDTO.getDefaultAttributeConsumerService());
        rp.setSignatureAlgorithms(relyingPartyDTO.getSignatureAlgorithms() == null ? null : SignatureAlgorithmMapper.getSignatureAlgorithms(relyingPartyDTO.getSignatureAlgorithms()));
        rp.setSingleLogoutServices(relyingPartyDTO.getSingleLogoutServices() == null ? null : ServiceEndPointMapper.getServiceEndPoints(relyingPartyDTO.getSingleLogoutServices()));
        return rp;
    }

    public static RelyingPartyDTO getRelyingPartyDTO(RelyingParty relyingParty) throws CertificateEncodingException {
        return RelyingPartyDTO.builder()
                              .withName(relyingParty.getName())
                              .withDefaultAssertionConsumerService(relyingParty.getDefaultAssertionConsumerService())
                              .withDefaultAttributeConsumerService(relyingParty.getDefaultAttributeConsumerService())
                              .withAssertionConsumerServices(AssertionConsumerServiceMapper.getAssertionCsDTOs(relyingParty.getAssertionConsumerServices()))
                              .withAttributeConsumerServices(AttributeConsumerServiceMapper.getAttributeCsDTOs(relyingParty.getAttributeConsumerServices()))
                              .withAuthnRequestsSigned(relyingParty.isAuthnRequestsSigned())
                              .withCertificate(new CertificateDTO((X509Certificate) relyingParty.getCertificate()))
                              .withSignatureAlgorithms(SignatureAlgorithmMapper.getSignatureAlgorithmDTOs(relyingParty.getSignatureAlgorithms()))
                              .withSingleLogoutServices(ServiceEndPointMapper.getServiceEndPointDTOs(relyingParty.getSingleLogoutServices()))
                              .withUrl(relyingParty.getUrl())
                              .build();
    }

    public static Collection<RelyingPartyDTO> getRelyingPartyDTOs(Collection<RelyingParty> relyingParties) throws CertificateEncodingException {
        Collection<RelyingPartyDTO> relyingPartyDTOs = new ArrayList<RelyingPartyDTO>();
        for (RelyingParty rp : relyingParties) {
            relyingPartyDTOs.add(getRelyingPartyDTO(rp));
        }
        return relyingPartyDTOs;
    }
}
