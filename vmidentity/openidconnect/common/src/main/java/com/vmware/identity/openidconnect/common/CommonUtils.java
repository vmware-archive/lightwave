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

import java.net.URI;
import java.net.URISyntaxException;

import org.apache.commons.lang3.Validate;
import org.apache.commons.validator.routines.UrlValidator;
import org.apache.http.client.utils.URIBuilder;

/**
 * @author Yehia Zayour
 */
public class CommonUtils {
    private CommonUtils() {
    }

    public static String appendQueryParameter(URI uri, String parameterName, String parameterValue) {
        Validate.notNull(uri, "uri");
        Validate.notEmpty(parameterName, "parameterName");
        Validate.notEmpty(parameterValue, "parameterValue");

        String result;
        URIBuilder uriBuilder = new URIBuilder(uri);
        uriBuilder.addParameter(parameterName, parameterValue);
        try {
            result = uriBuilder.build().toString();
        } catch (URISyntaxException e) {
            String error = String.format("failed to add parameter [%s]=[%s] to uri [%s]", parameterName, parameterValue, uri);
            throw new IllegalArgumentException(error, e);
        }
        return result;
    }

    public static boolean isValidUri(URI uri) {
        Validate.notNull(uri, "uri");

        String[] schemes = { "https" };
        UrlValidator urlValidator = new UrlValidator(schemes);
        return urlValidator.isValid(uri.toString());
    }
}
