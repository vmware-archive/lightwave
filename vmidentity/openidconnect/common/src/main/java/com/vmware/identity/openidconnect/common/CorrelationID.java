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

import org.apache.commons.lang3.StringUtils;
import org.apache.commons.lang3.Validate;

import com.nimbusds.oauth2.sdk.id.Identifier;

/**
 * @author Yehia Zayour
 */
public class CorrelationID extends Identifier {
    private static final long serialVersionUID = 1L;

    public CorrelationID() {
        super(); // this will initialize the value
    }

    public CorrelationID(String value) {
        super(value);
    }

    public static CorrelationID get(HttpRequest httpRequest) {
        Validate.notNull(httpRequest, "httpRequest");
        String correlationIdString = httpRequest.getParameters().get("correlation_id");
        return StringUtils.isBlank(correlationIdString) ? new CorrelationID() : new CorrelationID(correlationIdString);
    }

    @Override
    public boolean equals(Object object) {
        return object instanceof CorrelationID && this.toString().equals(object.toString());
    }
}
