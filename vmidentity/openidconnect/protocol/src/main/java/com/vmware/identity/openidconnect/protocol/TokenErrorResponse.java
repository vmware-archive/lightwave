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

package com.vmware.identity.openidconnect.protocol;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.StatusCode;

/**
 * @author Yehia Zayour
 */
public final class TokenErrorResponse extends TokenResponse {
    private final ErrorObject errorObject;

    public TokenErrorResponse(ErrorObject errorObject) {
        Validate.notNull(errorObject, "errorObject");
        this.errorObject = errorObject;
    }

    public ErrorObject getErrorObject() {
        return this.errorObject;
    }

    @Override
    public HttpResponse toHttpResponse() {
        return HttpResponse.createJsonResponse(this.errorObject);
    }

    public static TokenErrorResponse parse(HttpResponse httpResponse) throws ParseException {
        Validate.notNull(httpResponse, "httpResponse");

        if (httpResponse.getJsonContent() == null) {
            throw new ParseException("expecting json http response");
        }

        if (httpResponse.getStatusCode() == StatusCode.OK) {
            throw new ParseException("expecting status code not OK");
        }

        ErrorObject errorObject = ErrorObjectMapper.parse(httpResponse.getJsonContent(), httpResponse.getStatusCode());
        return new TokenErrorResponse(errorObject);
    }
}