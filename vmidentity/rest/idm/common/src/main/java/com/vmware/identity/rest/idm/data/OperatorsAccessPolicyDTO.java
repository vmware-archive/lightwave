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
package com.vmware.identity.rest.idm.data;

import java.util.List;
import java.util.Map;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code OperatorsAccessPolicyDTO} class contains the configuration details related
 * to enabling operators access within tenant.
 */
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = OperatorsAccessPolicyDTO.Builder.class)
public class OperatorsAccessPolicyDTO extends DTO {

    private final Boolean enabled;
    private final String userBaseDn;
    private final String groupBaseDn;

    /**
     * Construct a {@code OperatorsAccessPolicyDTO} with its details.
     *
     * @param enabled
     *            Enable/disable operators access.
     * @param userBaseDn
     *            base dn of users objects (optional); can only be set by system tenant
     * @param groupBaseDn
     *            base dn of group objects (optional); can only be set by system tenant
     */
    public OperatorsAccessPolicyDTO(Boolean enabled, String userBaseDn, String groupBaseDn) {
        this.enabled = enabled;
        this.userBaseDn = userBaseDn;
        this.groupBaseDn = groupBaseDn;
    }

    /**
     * whether operators access is enabled.
     *
     * @return {@code true} whether operators access is enabled.
     */
    public Boolean getEnabled() {
        return this.enabled;
    }

    /**
     * return base dn for users
     *
     * @return base dn for users
     */
    public String getUserBaseDn() {
        return this.userBaseDn;
    }

    /**
     * return base dn for groups
     *
     * @return base dn for groups
     */
    public String getGroupBaseDn() {
        return this.groupBaseDn;
    }

    /**
     * The JSON POJO Builder for this class. The builder class is meant mostly for usage when constructing the object from its JSON string and thus only accepts content for the canonical fields of the
     * JSON representation. Other constructors may exist that provide greater convenience.
     */
    @JsonIgnoreProperties(ignoreUnknown = true)
    @JsonPOJOBuilder
    public static class Builder {

        private Boolean enabled;
        private String userBaseDn;
        private String groupBaseDn;

        public Builder() {}

        public Builder withEnabled(Boolean enabled) {
            this.enabled = enabled;
            return this;
        }

        public Builder withUserBaseDn(String userBaseDn) {
            this.userBaseDn = userBaseDn;
            return this;
        }

        public Builder withGroupBaseDn(String groupBaseDn) {
            this.groupBaseDn = groupBaseDn;
            return this;
        }

        public OperatorsAccessPolicyDTO build() {
            return new OperatorsAccessPolicyDTO(
                this.enabled, this.userBaseDn, this.groupBaseDn);
        }
    }

}
