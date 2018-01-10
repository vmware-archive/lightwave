/*
 *  Copyright (c) 2018 VMware, Inc.  All Rights Reserved.
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

import java.net.URI;

import org.apache.commons.lang.Validate;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.ContentType;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.StatusCode;
import com.vmware.identity.openidconnect.protocol.JSONUtils;

import net.minidev.json.JSONObject;

public class CSPLogoutProcessor implements FederatedIdentityLogoutProcessor {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(CSPLogoutProcessor.class);

    @Override
    public String process(FederatedIdentityProviderInfo federatedIdpInfo, SessionManager.Entry session) throws Exception {
        Validate.notEmpty(session.getExternalJWTContent(), "external idp token");

        URI logoutUri = new URI(federatedIdpInfo.getLogoutURI());
        HttpPost httpPost = new HttpPost(logoutUri);
        httpPost.setHeader("accept", "application/json");

        JSONObject jsonObject = JSONUtils.parseJSONObject(session.getExternalJWTContent());
        String idToken = JSONUtils.getString(jsonObject, "id_token");
        String logoutIdTokenContent = "{\"idToken\": \"" + idToken + "\"}";
        httpPost.setEntity(new StringEntity(logoutIdTokenContent, ContentType.create(ContentType.APPLICATION_JSON.getMimeType(), "UTF-8")));

        try (CloseableHttpClient client = HttpClients.createDefault();
                CloseableHttpResponse response = client.execute(httpPost);) {
            int statusCodeInt = response.getStatusLine().getStatusCode();
            StatusCode statusCode;
            try {
              statusCode = StatusCode.parse(statusCodeInt);
              if (statusCode != StatusCode.OK) {
                ErrorObject errorObject =
                    ErrorObject.serverError(
                        String.format(
                            "Failed to log out from external idp [%s] with status code [%s]",
                            federatedIdpInfo.getIssuer(), statusCode
                        )
                    );
                LoggerUtils.logFailedRequest(logger, errorObject);
                throw new ServerException(errorObject);
              }

              String content = EntityUtils.toString(response.getEntity());
              JSONObject jsonContent = JSONUtils.parseJSONObject(content);

              return JSONUtils.getString(jsonContent, "url");
            } catch (ParseException e) {
              ErrorObject errorObject = ErrorObject.serverError(
                  String.format(
                      "Failed to parse response from external idp. %s:%s",
                      e.getClass().getName(),
                      e.getMessage()
                  )
              );
              LoggerUtils.logFailedRequest(logger, errorObject, e);
              throw new ServerException(errorObject);
            }
        }
    }
}
