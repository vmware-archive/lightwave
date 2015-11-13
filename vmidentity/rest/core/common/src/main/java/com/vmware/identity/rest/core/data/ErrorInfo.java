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
package com.vmware.identity.rest.core.data;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;

/**
 * The {@code ErrorInfo} class represents REST SSO errors as sent from the server.
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder=ErrorInfo.Builder.class)
public class ErrorInfo extends DTO {

    private final String error;
    private final String details;
    private final String cause;

    /**
     * Construct an {@code ErrorInfo} from an error string and a details string.
     *
     * @param error a string representing the type of error that occurred.
     * @param details a message representing the details of the error.
     */
    public ErrorInfo(String error, String details) {
        this(error, details, null);
    }

    /**
     * Construct an {@code ErrorInfo} from an error string, a details string, and a
     * cause string.
     *
     * @param error a string representing the type of error that occurred.
     * @param details a message representing the details of the error.
     * @param cause a message representing the cause of the error.
     */
    public ErrorInfo(String error, String details, String cause) {
        this.error = error;
        this.details = details;
        this.cause = cause;
    }

    /**
     * Get the error string representing the type of error that occurred.
     *
     * @return the error string.
     */
    public String getError() {
        return error;
    }

    /**
     * Get the details string representing the detailed message of the error that
     * occurred.
     *
     * @return the details string.
     */
    public String getDetails() {
        return details;
    }

    /**
     * Get the cause string representing the underlying cause of the error that occurred.
     *
     * @return the cause string.
     */
    public String getCause() {
        return cause;
    }

    /**
     * Create an instance of the {@link ErrorInfo.Builder} class.
     *
     * @return a new {@code ErrorInfo.Builder}.
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
        private String error;
        private String details;
        private String cause;

        public Builder withError(String error) {
            this.error = error;
            return this;
        }

        public Builder withDetails(String details) {
            this.details = details;
            return this;
        }

        public Builder withCause(String cause) {
            this.cause = cause;
            return this;
        }

        public ErrorInfo build() {
            return new ErrorInfo(error, details, cause);
        }
    }

}
