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
import java.security.cert.CertificateEncodingException;
import java.util.ArrayList;
import java.util.List;
import java.util.Collection;

import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.CertificateChainDTO;

/**
 * Mapper for certificates
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 *
 */
public class CertificateMapper {

    /**
     * Converts collection of  {@link Certificate} objects to {@link CertificateDTO}
     * @throws CertificateEncodingException
     */
    public static List<CertificateDTO> getCertificateDTOs(Collection<? extends Certificate> certs) {
        List<CertificateDTO> certificateDTOs = new ArrayList<CertificateDTO>();
        if (certs != null)
        {
            for (Certificate cert : certs) {
                try {
                    certificateDTOs.add(new CertificateDTO((X509Certificate) cert));
                } catch (CertificateEncodingException e) {
                    throw new DTOMapperException("Error converting the certificate", e);
                }
            }
        }
        return certificateDTOs;
    }

    /**
     * Converts collection of {@link CertificateDTO} to {@link Certificate} objects
     */
    public static List<Certificate> getCertificates(Collection<CertificateDTO> certificateDTOs) {
        List<Certificate> certificates = new ArrayList<Certificate>();
        for (CertificateDTO certDTO : certificateDTOs) {
            certificates.add(certDTO.getX509Certificate());
        }
        return certificates;
    }

    /**
     * Converts collection of {@link CertificateDTO} to {@link X509Certificate} objects
     */
    public static Collection<X509Certificate> getX509Certificates(Collection<CertificateDTO> certificateDTOs) {
        List<X509Certificate> certificates = new ArrayList<X509Certificate>();
        if (certificateDTOs != null)
        {
            for (CertificateDTO certDTO : certificateDTOs) {
                certificates.add(certDTO.getX509Certificate());
            }
        }
        return certificates;
    }

    public static CertificateChainDTO getCertificateChainDTO(List<Certificate> certs) {
        CertificateChainDTO.Builder builder = CertificateChainDTO.builder();
        builder.withCertificates(getCertificateDTOs(certs));
        return builder.build();
    }
}
