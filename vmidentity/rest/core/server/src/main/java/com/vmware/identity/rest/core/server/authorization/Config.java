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

package com.vmware.identity.rest.core.server.authorization;

/**
 * Generic configuration class - mostly just contains a great deal of static values
 *
 * TODO make this configurable
 */
public class Config {

    public static final String RESOURCE_SERVER_AUDIENCE = "rs_admin_server";

    public static final String X_SSL_SECURE_HEADER = "X-SSL-SECURE";
    public static final String CORRELATION_ID_HEADER = "id";
    public static final String ACCESS_TOKEN_HEADER = "Authorization";

    public static final String ACCESS_TOKEN_PARAMETER = "access_token";
    public static final String TOKEN_TYPE_PARAMETER = "token_type";
    public static final String TOKEN_SIGNATURE_PARAMETER = "token_signature";

    public static final String JWT_TYPE_FIELD = "token_type";
    public static final String JWT_ROLE_FIELD = "admin_server_role";
    public static final String JWT_GROUPS_FIELD = "groups";
    public static final String JWT_HOK_FIELD = "hotk";

    public static final String LOCALIZATION_PACKAGE_NAME = "i18n.authorization";

}