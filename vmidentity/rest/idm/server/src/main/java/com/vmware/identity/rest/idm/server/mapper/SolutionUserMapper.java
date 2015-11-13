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

import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.PrincipalDTO;
import com.vmware.identity.rest.idm.data.SolutionUserDTO;

/**
 * Maps {@link SolutionUser.class} to {@link SolutionUserDTO.class} and vice-versa.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public final class SolutionUserMapper {

    public static SolutionUserDTO getSolutionUserDTO(SolutionUser solutionUser) {
        PrincipalDTO alias = null;
        if (solutionUser.getAlias() != null) {
            alias = new PrincipalDTO(solutionUser.getAlias().getName(), solutionUser.getAlias().getDomain());
        }

        try {
            X509Certificate cert = (X509Certificate) solutionUser.getCert();
            SolutionUserDTO.Builder builder = new SolutionUserDTO.Builder();
            builder.withName(solutionUser.getId().getName())
                   .withDomain(solutionUser.getId().getDomain())
                   .withDescription(solutionUser.getDetail().getDescription())
                   .withAlias(alias);

            if (cert != null) {
                builder.withCertificate(new CertificateDTO(cert));
            }

            builder.withDisabled(solutionUser.isDisabled())
                   .withObjectId(solutionUser.getObjectId());

            return builder.build();
        } catch (CertificateException e) {
            String errMessage = String.format("Failed to map object from %s to %s", SolutionUser.class.getName(), SolutionUserDTO.class.getName());
            throw new DTOMapperException(errMessage, e);
        }
    }

    public static Set<SolutionUserDTO> getSolutionUserDTOs(Collection<SolutionUser> solutionUsers) throws DTOMapperException {
        Set<SolutionUserDTO> solutionUserDTOs = new HashSet<SolutionUserDTO>();
        for(SolutionUser solutionUser : solutionUsers){
            solutionUserDTOs.add(getSolutionUserDTO(solutionUser));
        }
        return solutionUserDTOs;
    }
}
