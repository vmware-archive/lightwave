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

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code SignatureAlgorithmDTO} class represents a signature algorithm used
 * by a {@link RelyingPartyDTO}.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = SignatureAlgorithmDTO.Builder.class)
public class SignatureAlgorithmDTO extends DTO {

    private final Integer maxKeySize;
    private final Integer minKeySize;
    private final Integer priority;

    /**
     * Construct a {@code SignatureAlgorithmDTO} with a min and max key size as well as
     * a priority.
     *
     * @param maxKeySize the maximum key size.
     * @param minKeySize the minimum key size.
     * @param priority the signature priority.
     */
    public SignatureAlgorithmDTO(Integer maxKeySize, Integer minKeySize, Integer priority) {
        this.maxKeySize = maxKeySize;
        this.minKeySize = minKeySize;
        this.priority = priority;
    }

    /**
     * Get the maximum key size.
     *
     * @return the maximum key size.
     */
    public Integer getMaxKeySize() {
        return maxKeySize;
    }

    /**
     * Get the minimum key size.
     *
     * @return the minimum key size.
     */
    public Integer getMinKeySize() {
        return minKeySize;
    }

    /**
     * Get the signature priority.
     *
     * @return the signature priority.
     */
    public Integer getPriority() {
        return priority;
    }

    /**
     * Creates an instance of the {@link SignatureAlgorithmDTO.Builder} class.
     *
     * @return a new {@code SignatureAlgorithmDTO.Builder}.
     */
    public static Builder builder() {
        return new Builder();
    }

    /**
     * The JSON POJO Builder for this class. The builder class is meant mostly for
     * usage when constructing the object from its JSON string and thus only accepts
     * content for the canonical fields of the JSON representation. Other constructors
     * may exist that provide greater convenience.
     */
    @JsonIgnoreProperties(ignoreUnknown=true)
    @JsonPOJOBuilder
    public static class Builder {

        private Integer maxKeySize;
        private Integer minKeySize;
        private Integer priority;

        public Builder withMaxKeySize(Integer maxKeySize) {
            this.maxKeySize = maxKeySize;
            return this;
        }

        public Builder withMinKeySize(Integer minKeySize) {
            this.minKeySize = minKeySize;
            return this;
        }

        public Builder withPriority(Integer priority) {
            this.priority = priority;
            return this;
        }

        public SignatureAlgorithmDTO build() {
            return new SignatureAlgorithmDTO(maxKeySize, minKeySize, priority);
        }
    }

}
