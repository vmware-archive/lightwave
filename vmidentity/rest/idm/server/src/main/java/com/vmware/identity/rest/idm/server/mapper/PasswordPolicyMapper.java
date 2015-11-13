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

import com.vmware.identity.idm.PasswordPolicy;
import com.vmware.identity.rest.idm.data.PasswordPolicyDTO;

/**
 * Mapper for password policy entity
 * 
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class PasswordPolicyMapper {

    public static PasswordPolicyDTO getPasswordPolicyDTO(PasswordPolicy policy) {
        return PasswordPolicyDTO.builder()
                                .withDescription(policy.getDescription())
                                .withMaxIdenticalAdjacentCharacters(policy.getMaximumAdjacentIdenticalCharacterCount())
                                .withMaxLength(policy.getMaximumLength())
                                .withMinAlphabeticCount(policy.getMinimumAlphabetCount())
                                .withMinLength(policy.getMinimumLength())
                                .withMinLowercaseCount(policy.getMinimumLowercaseCount())
                                .withMinNumericCount(policy.getMinimumNumericCount())
                                .withMinSpecialCharCount(policy.getMinimumSpecialCharacterCount())
                                .withMinUppercaseCount(policy.getMinimumUppercaseCount())
                                .withPasswordLifetimeDays(policy.getPasswordLifetimeDays())
                                .withProhibitedPreviousPasswordCount(policy.getProhibitedPreviousPasswordsCount())
                                .build();
    }

    public static PasswordPolicy getPasswordPolicy(PasswordPolicyDTO pwdPolicyDTO){
        return new PasswordPolicy(pwdPolicyDTO.getDescription(),
                                  pwdPolicyDTO.getProhibitedPreviousPasswordCount(),
                                  pwdPolicyDTO.getMinLength(),
                                  pwdPolicyDTO.getMaxLength(),
                                  pwdPolicyDTO.getMinAlphabeticCount(),
                                  pwdPolicyDTO.getMinUppercaseCount(),
                                  pwdPolicyDTO.getMinLowercaseCount(),
                                  pwdPolicyDTO.getMinNumericCount(),
                                  pwdPolicyDTO.getMinSpecialCharCount(),
                                  pwdPolicyDTO.getMaxIdenticalAdjacentCharacters(),
                                  pwdPolicyDTO.getPasswordLifetimeDays());
    }

}
