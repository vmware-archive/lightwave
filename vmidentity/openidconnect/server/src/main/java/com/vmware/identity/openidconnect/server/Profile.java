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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import com.nimbusds.jose.JWSAlgorithm;
import com.nimbusds.oauth2.sdk.GrantType;
import com.nimbusds.oauth2.sdk.ResponseType;
import com.nimbusds.oauth2.sdk.Scope;
import com.nimbusds.oauth2.sdk.auth.ClientAuthenticationMethod;
import com.nimbusds.openid.connect.sdk.OIDCResponseTypeValue;
import com.nimbusds.openid.connect.sdk.SubjectType;

/**
 * @author Jun Sun
 */
public final class Profile {

    public static final List<SubjectType> SUBJECT_TYPES = new ArrayList<SubjectType>(Arrays.asList(
            SubjectType.PUBLIC));

    public static final List<ResponseType> RESPONSE_TYPES = new ArrayList<ResponseType>(Arrays.asList(
            new ResponseType(ResponseType.Value.CODE),
            new ResponseType(OIDCResponseTypeValue.ID_TOKEN),
            new ResponseType(OIDCResponseTypeValue.ID_TOKEN, ResponseType.Value.TOKEN)));

    public static final List<JWSAlgorithm> ID_TOKEN_JWS_ALGS = new ArrayList<JWSAlgorithm>(Arrays.asList(
            JWSAlgorithm.RS256));

    public static final List<GrantType> GRANT_TYPES = new ArrayList<GrantType>(Arrays.asList(
            GrantType.AUTHORIZATION_CODE,
            GrantType.IMPLICIT,
            GrantType.PASSWORD,
            GrantType.REFRESH_TOKEN));

    public static final List<ClientAuthenticationMethod> TOKEN_ENDPOINT_AUTH_METHODS = new ArrayList<ClientAuthenticationMethod>(Arrays.asList(
            ClientAuthenticationMethod.PRIVATE_KEY_JWT));

    public static final List<JWSAlgorithm> TOKEN_ENDPOINT_JWS_ALGS = new ArrayList<JWSAlgorithm>(Arrays.asList(
            JWSAlgorithm.RS256));

    public static final Scope SCOPES = new Scope(
            "openid",
            "offline_access",
            "id_groups",
            "at_groups",
            "rs_admin_server");

    public static final List<String> CLAIMS = new ArrayList<String>(Arrays.asList(
            "sub",
            "exp",
            "aud",
            "iss",
            "iat",
            "jti",
            "given_name",
            "family_name",
            "token_class",
            "token_type",
            "nonce",
            "hotk",
            "groups",
            "admin_server_role"));
}
