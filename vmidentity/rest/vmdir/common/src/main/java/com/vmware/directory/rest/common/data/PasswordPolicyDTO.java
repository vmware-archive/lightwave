package com.vmware.directory.rest.common.data;

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

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code PasswordPolicyDTO} class describes the password policy configuration for a tenant.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = PasswordPolicyDTO.Builder.class)
public class PasswordPolicyDTO extends DTO {

    private final String description;
    private final Integer maxIdenticalAdjacentCharacters;
    private final Integer maxLength;
    private final Integer minAlphabeticCount;
    private final Integer minLength;
    private final Integer minLowercaseCount;
    private final Integer minNumericCount;
    private final Integer minSpecialCharCount;
    private final Integer minUppercaseCount;
    private final Integer passwordLifetimeDays;
    private final Integer prohibitedPreviousPasswordCount;

    /**
     * Construct a {@code PasswordPolicyDTO} with its various details.
     *
     * @param description a description of the password policy.
     * @param prohibitedPreviousPasswordsCount the number of previous passwords to disallow
     *  when setting new passwords.
     * @param minLength minimum password length.
     * @param maxLength maximum password length.
     * @param minAlphabeticCount minimum number of alphabetic characters in a password.
     * @param minUppercaseCount minimum number of uppercase characters in a password.
     * @param minLowercaseCount minumum number of lowercase characters in a password.
     * @param minNumericCount minimum number of numerical characters in a password.
     * @param minSpecialCharCount minimum number of special characters in a password.
     * @param maxIdenticalAdjacentCharacters maximum number of identical adjacent characters
     *  in a password.
     * @param passwordLifetimeDays number of days allowed in a password's lifetime.
     */
    public PasswordPolicyDTO(String description, Integer prohibitedPreviousPasswordsCount,
            Integer minLength, Integer maxLength, Integer minAlphabeticCount,
            Integer minUppercaseCount, Integer minLowercaseCount, Integer minNumericCount,
            Integer minSpecialCharCount, Integer maxIdenticalAdjacentCharacters, Integer passwordLifetimeDays) {
        this.description = description;
        this.prohibitedPreviousPasswordCount = prohibitedPreviousPasswordsCount;
        this.minLength = minLength;
        this.maxLength = maxLength;
        this.minAlphabeticCount = minAlphabeticCount;
        this.minUppercaseCount = minUppercaseCount;
        this.minLowercaseCount = minLowercaseCount;
        this.minNumericCount = minNumericCount;
        this.minSpecialCharCount = minSpecialCharCount;
        this.maxIdenticalAdjacentCharacters = maxIdenticalAdjacentCharacters;
        this.passwordLifetimeDays = passwordLifetimeDays;
    }

    /**
     * Get the detailed description of the password policy.
     *
     * @return the description for the password policy.
     */
    public String getDescription() {
        return description;
    }

    /**
     * Get the maximum allowed number of identical adjacent characters in a password.
     *
     * @return a non-negative number indicating the maximum number of identical adjacent
     *  characters allowed in a password.
     */
    public Integer getMaxIdenticalAdjacentCharacters() {
        return maxIdenticalAdjacentCharacters;
    }

    /**
     * Get the maximum allowed character length of passwords.
     *
     * @return a non-negative number indicating the maximum number of characters allowed
     *  in a password.
     */
    public Integer getMaxLength() {
        return maxLength;
    }

    /**
     * Get the minimum number of alphabetic characters required in a password.
     *
     * @return a non-negative number indicating the minimum number of alphabetic characters
     *  required in a password.
     */
    public Integer getMinAlphabeticCount() {
        return minAlphabeticCount;
    }

    /**
     * Get the minimum allowed character length of passwords.
     *
     * @return a non-negative number indicating the minimum number of characters allowed
     *  in a password.
     */
    public Integer getMinLength() {
        return minLength;
    }

    /**
     * Get the minumum number of lowercase characters required in a password.
     *
     * @return a non-negative number indicating the minimum number of lowercase characters
     *  required in a password.
     */
    public Integer getMinLowercaseCount() {
        return minLowercaseCount;
    }

    /**
     * Get the minimum number of numeric characters required in a password.
     *
     * @return a non-negative number indicating the minimum number of numeric characters
     *  required in a password.
     */
    public Integer getMinNumericCount() {
        return minNumericCount;
    }

    /**
     * Get the minimum number of special characters required in a password.
     *
     * @return a non-negative number indicating the minimum number of special characters
     *  required in a password.
     */
    public Integer getMinSpecialCharCount() {
        return minSpecialCharCount;
    }

    /**
     * Get the minimum number of uppercase characters required in a password.
     *
     * @return a non-negative number indicating the minimum number of uppercase characters
     *  required in a password.
     */
    public Integer getMinUppercaseCount() {
        return minUppercaseCount;
    }

    /**
     * Get the maximum age of a password in days.
     *
     * @return a non-negative number indicating the maximum number of days that a password
     *  can remain unchanged.
     */
    public Integer getPasswordLifetimeDays() {
        return passwordLifetimeDays;
    }

    /**
     * Get the number of previous passwords that are disallowed when changing passwords.
     *
     * @return a non-negative number indicating the number of previous passwords that are
     *  disallowed when changing passwords.
     */
    public Integer getProhibitedPreviousPasswordCount() {
        return prohibitedPreviousPasswordCount;
    }

    /**
     * Creates an instance of the {@link PasswordPolicyDTO.Builder} class.
     *
     * @return a new {@code PasswordPolicyDTO.Builder}.
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
        private String description;
        private Integer maxIdenticalAdjacentCharacters;
        private Integer maxLength;
        private Integer minAlphabeticCount;
        private Integer minLength;
        private Integer minLowercaseCount;
        private Integer minNumericCount;
        private Integer minSpecialCharCount;
        private Integer minUppercaseCount;
        private Integer passwordLifetimeDays;
        private Integer prohibitedPreviousPasswordCount;

        public Builder withDescription(String description) {
            this.description = description;
            return this;
        }

        public Builder withMaxIdenticalAdjacentCharacters(Integer maxIdenticalAdjacentCharacters) {
            this.maxIdenticalAdjacentCharacters = maxIdenticalAdjacentCharacters;
            return this;
        }

        public Builder withMaxLength(Integer maxLength) {
            this.maxLength = maxLength;
            return this;
        }

        public Builder withMinAlphabeticCount(Integer minAlphabeticCount) {
            this.minAlphabeticCount = minAlphabeticCount;
            return this;
        }

        public Builder withMinLength(Integer minLength) {
            this.minLength = minLength;
            return this;
        }

        public Builder withMinLowercaseCount(Integer minLowercaseCount) {
            this.minLowercaseCount = minLowercaseCount;
            return this;
        }

        public Builder withMinNumericCount(Integer minNumericCount) {
            this.minNumericCount = minNumericCount;
            return this;
        }

        public Builder withMinSpecialCharCount(Integer minSpecialCharCount) {
            this.minSpecialCharCount = minSpecialCharCount;
            return this;
        }

        public Builder withMinUppercaseCount(Integer minUppercaseCount) {
            this.minUppercaseCount = minUppercaseCount;
            return this;
        }

        public Builder withPasswordLifetimeDays(Integer passwordLifetimeDays) {
            this.passwordLifetimeDays = passwordLifetimeDays;
            return this;
        }

        public Builder withProhibitedPreviousPasswordCount(Integer prohibitedPreviousPasswordCount) {
            this.prohibitedPreviousPasswordCount = prohibitedPreviousPasswordCount;
            return this;
        }

        public PasswordPolicyDTO build() {
            return new PasswordPolicyDTO(description, prohibitedPreviousPasswordCount, minLength, maxLength, minAlphabeticCount, minUppercaseCount, minLowercaseCount, minNumericCount, minSpecialCharCount, maxIdenticalAdjacentCharacters,
                            passwordLifetimeDays);
        }
    }

}
