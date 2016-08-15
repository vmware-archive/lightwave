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

package com.vmware.identity.openidconnect.common;

import org.apache.commons.lang3.Validate;

/**
 * @author Yehia Zayour
 */
public final class ErrorObject {
    private final ErrorCode errorCode;
    private final String description;
    private final StatusCode statusCode;

    public ErrorObject(ErrorCode errorCode, String description, StatusCode statusCode) {
        Validate.notNull(errorCode, "errorCode");
        Validate.notEmpty(description, "description");
        Validate.notNull(statusCode, "statusCode");
        this.errorCode = errorCode;
        this.description = description;
        this.statusCode = statusCode;
    }

    public static ErrorObject invalidRequest(String description) {
        Validate.notEmpty(description, "description");
        return new ErrorObject(ErrorCode.INVALID_REQUEST, description, StatusCode.BAD_REQUEST);
    }

    public static ErrorObject invalidScope(String description) {
        Validate.notEmpty(description, "description");
        return new ErrorObject(ErrorCode.INVALID_SCOPE, description, StatusCode.BAD_REQUEST);
    }

    public static ErrorObject invalidGrant(String description) {
        Validate.notEmpty(description, "description");
        return new ErrorObject(ErrorCode.INVALID_GRANT, description, StatusCode.BAD_REQUEST);
    }

    public static ErrorObject invalidClient(String description) {
        Validate.notEmpty(description, "description");
        return new ErrorObject(ErrorCode.INVALID_CLIENT, description, StatusCode.UNAUTHORIZED);
    }

    public static ErrorObject unauthorizedClient(String description) {
        Validate.notEmpty(description, "description");
        return new ErrorObject(ErrorCode.UNAUTHORIZED_CLIENT, description, StatusCode.BAD_REQUEST);
    }

    public static ErrorObject unsupportedResponseType(String description) {
        Validate.notEmpty(description, "description");
        return new ErrorObject(ErrorCode.UNSUPPORTED_RESPONSE_TYPE, description, StatusCode.BAD_REQUEST);
    }

    public static ErrorObject unsupportedGrantType(String description) {
        Validate.notEmpty(description, "description");
        return new ErrorObject(ErrorCode.UNSUPPORTED_GRANT_TYPE, description, StatusCode.BAD_REQUEST);
    }

    public static ErrorObject accessDenied(String description) {
        Validate.notEmpty(description, "description");
        return new ErrorObject(ErrorCode.ACCESS_DENIED, description, StatusCode.FORBIDDEN);
    }

    public static ErrorObject serverError(String description) {
        Validate.notEmpty(description, "description");
        return new ErrorObject(ErrorCode.SERVER_ERROR, description, StatusCode.INTERNAL_SERVER_ERROR);
    }

    public ErrorCode getErrorCode() {
        return this.errorCode;
    }

    public String getDescription() {
        return this.description;
    }

    public StatusCode getStatusCode() {
        return this.statusCode;
    }
}