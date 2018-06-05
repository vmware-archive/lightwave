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

import com.vmware.identity.idm.OperatorAccessPolicy;
import com.vmware.identity.rest.idm.data.OperatorsAccessPolicyDTO;

/**
 * Mapper for Operators access policy
 *
 */
public class OperatorsAccessPolicyMapper {

    public static OperatorsAccessPolicyDTO getOperatorsAccessPolicyDTO(OperatorAccessPolicy policy) {

        OperatorsAccessPolicyDTO policyDto = null;

        if (policy != null) {

            OperatorsAccessPolicyDTO.Builder builder = new OperatorsAccessPolicyDTO.Builder();
            policyDto = builder.withEnabled(policy.enabled())
               .withUserBaseDn(policy.userBaseDn())
               .withGroupBaseDn(policy.groupBaseDn())
               .build();
        }

        return policyDto;
    }

    public static OperatorAccessPolicy getOperatorsAccessPolicy(OperatorsAccessPolicyDTO policyDTO) {
        OperatorAccessPolicy policy = null;
        if (policyDTO != null)
        {
            policy = new OperatorAccessPolicy.Builder()
                        .withEnabled(policyDTO.getEnabled()!= null ? policyDTO.getEnabled().booleanValue() : false )
                        .withUserBaseDn(policyDTO.getUserBaseDn())
                        .withGroupBaseDn(policyDTO.getGroupBaseDn())
                        .build();
        }
        return policy;
    }
}
