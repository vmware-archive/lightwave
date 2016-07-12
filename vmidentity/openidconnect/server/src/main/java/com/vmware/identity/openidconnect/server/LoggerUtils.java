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

package com.vmware.identity.openidconnect.server;

import org.apache.commons.lang3.StringUtils;
import org.apache.commons.lang3.Validate;

import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.openidconnect.common.CorrelationID;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.protocol.HttpRequest;

/**
 * @author Yehia Zayour
 */
public class LoggerUtils {
    private LoggerUtils() {
    }

    public static void logFailedRequest(IDiagnosticsLogger logger, ServerException e) {
        Validate.notNull(logger, "logger");
        Validate.notNull(e, "e");
        logFailedRequest(logger, e.getErrorObject(), e.getCause());
    }

    public static void logFailedRequest(IDiagnosticsLogger logger, ErrorObject errorObject) {
        Validate.notNull(logger, "logger");
        Validate.notNull(errorObject, "errorObject");
        logFailedRequest(logger, errorObject, (Throwable) null);
    }

    public static void logFailedRequest(IDiagnosticsLogger logger, ErrorObject errorObject, Throwable cause) {
        Validate.notNull(logger, "logger");
        Validate.notNull(errorObject, "errorObject");
        // nullable cause
        logger.info(
                "request failed: error_code [{}] error_description [{}] exception [{}]",
                errorObject.getErrorCode().getValue(),
                errorObject.getDescription(),
                cause);
    }

    public static CorrelationID getCorrelationID(HttpRequest httpRequest) {
        Validate.notNull(httpRequest, "httpRequest");
        String correlationIdParameter = httpRequest.getParameters().get("correlation_id");
        return StringUtils.isEmpty(correlationIdParameter) ? new CorrelationID() : new CorrelationID(correlationIdParameter);
    }
}