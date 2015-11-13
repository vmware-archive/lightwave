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

import java.util.ArrayList;
import java.util.Collection;

import com.vmware.identity.idm.SignatureAlgorithm;
import com.vmware.identity.rest.idm.data.SignatureAlgorithmDTO;

/**
 * Mapper utility to map objects from {@link SignatureAlgorithm} to {@link SignatureAlgorithmDTO} and vice-versa.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class SignatureAlgorithmMapper {

    public static SignatureAlgorithmDTO getSignatureAlgorithmDTO(SignatureAlgorithm signAlgo) {
        return SignatureAlgorithmDTO.builder()
                                    .withMaxKeySize(signAlgo.getMaximumKeySize())
                                    .withPriority(signAlgo.getPriority())
                                    .withMinKeySize(signAlgo.getMinimumKeySize())
                                    .build();
    }

    public static SignatureAlgorithm getSignatureAlgorithm(SignatureAlgorithmDTO signAlgoDTO) {
        SignatureAlgorithm signatureAlgo = new SignatureAlgorithm();
        signatureAlgo.setMaximumKeySize(signAlgoDTO.getMaxKeySize());
        signatureAlgo.setMinimumKeySize(signAlgoDTO.getMinKeySize());
        signatureAlgo.setPriority(signAlgoDTO.getPriority());
        return signatureAlgo;
    }

    public static Collection<SignatureAlgorithm> getSignatureAlgorithms(Collection<SignatureAlgorithmDTO> signatureAlgorithmDTOs) {
        Collection<SignatureAlgorithm> signAlgorithms = new ArrayList<SignatureAlgorithm>();
        for (SignatureAlgorithmDTO algoDTO : signatureAlgorithmDTOs) {
            signAlgorithms.add(getSignatureAlgorithm(algoDTO));
        }
        return signAlgorithms;
    }

    public static Collection<SignatureAlgorithmDTO> getSignatureAlgorithmDTOs(Collection<SignatureAlgorithm> signatureAlgorithms) {
        Collection<SignatureAlgorithmDTO> signAlgorithmDTOs = new ArrayList<SignatureAlgorithmDTO>();
        for (SignatureAlgorithm signAlgo : signatureAlgorithms) {
            signAlgorithmDTOs.add(getSignatureAlgorithmDTO(signAlgo));
        }
        return signAlgorithmDTOs;
    }

}
