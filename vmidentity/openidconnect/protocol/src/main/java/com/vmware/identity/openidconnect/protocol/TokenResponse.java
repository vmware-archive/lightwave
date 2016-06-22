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

import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.StatusCode;

/**
 * @author Yehia Zayour
 */
public abstract class TokenResponse extends ProtocolResponse {
    public static TokenResponse parse(HttpResponse httpResponse) throws ParseException {
        Validate.notNull(httpResponse, "httpResponse");

        return (httpResponse.getStatusCode() ==  StatusCode.OK) ?
                TokenSuccessResponse.parse(httpResponse) :
                TokenErrorResponse.parse(httpResponse);
    }
}