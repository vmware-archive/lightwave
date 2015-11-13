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

import com.vmware.identity.idm.LockoutPolicy;
import com.vmware.identity.rest.idm.data.LockoutPolicyDTO;

/**
 * Mapper for LockoutPolicy
 * 
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class LockoutPolicyMapper {

    public static LockoutPolicyDTO getLockoutPolicyDTO(LockoutPolicy policy) {
        return LockoutPolicyDTO.builder()
                               .withDescription(policy.getDescription())
                               .withFailedAttemptIntervalSec(policy.getFailedAttemptIntervalSec())
                               .withMaxFailedAttempts(policy.getMaxFailedAttempts())
                               .withAutoUnlockIntervalSec(policy.getAutoUnlockIntervalSec())
                               .build();
    }

    public static LockoutPolicy getLockoutPolicy(LockoutPolicyDTO lockoutPolicyDTO) {
        return new LockoutPolicy(lockoutPolicyDTO.getDescription(),
                                 lockoutPolicyDTO.getFailedAttemptIntervalSec(),
                                 lockoutPolicyDTO.getMaxFailedAttempts(),
                                 lockoutPolicyDTO.getAutoUnlockIntervalSec());
    }

}
