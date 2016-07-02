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
import java.util.Map;

import net.minidev.json.JSONObject;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ErrorCode;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.StatusCode;

/**
 * @author Yehia Zayour
 */
public final class ErrorObjectMapper {
    public static JSONObject toJSONObject(ErrorObject errorObject) {
        Validate.notNull(errorObject, "errorObject");

        JSONObject json = new JSONObject();
        json.put("error", errorObject.getErrorCode().getValue());
        json.put("error_description", errorObject.getDescription());
        return json;
    }

    public static ErrorObject parse(Map<String, String> parameters, StatusCode statusCode) throws ParseException {
        Validate.notNull(parameters, "parameters");
        Validate.notNull(statusCode, "statusCode");

        ErrorCode errorCode = ErrorCode.parse(ParameterMapUtils.getString(parameters, "error"));
        String description = ParameterMapUtils.getString(parameters, "error_description");

        return new ErrorObject(errorCode, description, statusCode);
    }

    public static ErrorObject parse(JSONObject jsonObject, StatusCode statusCode) throws ParseException {
        Validate.notNull(jsonObject, "jsonObject");
        Validate.notNull(statusCode, "statusCode");

        ErrorCode errorCode = ErrorCode.parse(JSONUtils.getString(jsonObject, "error"));
        String description = JSONUtils.getString(jsonObject, "error_description");

        return new ErrorObject(errorCode, description, statusCode);
    }
}