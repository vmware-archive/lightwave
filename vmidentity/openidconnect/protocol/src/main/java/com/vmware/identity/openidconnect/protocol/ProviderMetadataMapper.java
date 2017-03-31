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

import java.net.URI;
import java.util.Arrays;

import net.minidev.json.JSONObject;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.Issuer;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.ProviderMetadata;

/**
 * @author Yehia Zayour
 */
public final class ProviderMetadataMapper {
    public static JSONObject toJSONObject(ProviderMetadata providerMetadata) {
        Validate.notNull(providerMetadata, "providerMetadata");

        JSONObject json = new JSONObject();

        json.put("issuer",                      providerMetadata.getIssuer().getValue());
        json.put("authorization_endpoint",      providerMetadata.getAuthorizationEndpointURI().toString());
        json.put("token_endpoint",              providerMetadata.getTokenEndpointURI().toString());
        json.put("end_session_endpoint",        providerMetadata.getEndSessionEndpointURI().toString());
        json.put("jwks_uri",                    providerMetadata.getJWKSetURI().toString());
        json.put("response_types_supported",    providerMetadata.getResponseTypesSupported());
        json.put("subject_types_supported",     providerMetadata.getSubjectTypesSupported());
        json.put("id_token_signing_alg_values_supported", providerMetadata.getIDTokenSigningAlgorithmValuesSupported());

        return json;
    }

    public static ProviderMetadata parse(JSONObject json) throws ParseException {
        Validate.notNull(json, "json");

        URI issuerURI                   = JSONUtils.getURI(json, "issuer");
        URI authorizationEndpointURI    = JSONUtils.getURI(json, "authorization_endpoint");
        URI tokenEndpointURI            = JSONUtils.getURI(json, "token_endpoint");
        URI endSessionEndpointURI       = JSONUtils.getURI(json, "end_session_endpoint");
        URI jwkSetURI                   = JSONUtils.getURI(json, "jwks_uri");
        String[] responseTypesSupported = JSONUtils.getStringArray(json, "response_types_supported");
        String[] subjectTypesSupported  = JSONUtils.getStringArray(json, "subject_types_supported");
        String[] idTokenSigningAlgorithmValuesSupported = JSONUtils.getStringArray(json, "id_token_signing_alg_values_supported");

        return new ProviderMetadata.Builder(new Issuer(issuerURI.toString())).
                authorizationEndpointURI(authorizationEndpointURI).
                tokenEndpointURI(tokenEndpointURI).
                endSessionEndpointURI(endSessionEndpointURI).
                jwkSetURI(jwkSetURI).
                responseTypesSupported(Arrays.asList(responseTypesSupported)).
                subjectTypesSupported(Arrays.asList(subjectTypesSupported)).
                idTokenSigningAlgorithmValuesSupported(Arrays.asList(idTokenSigningAlgorithmValuesSupported)).build();
    }
}