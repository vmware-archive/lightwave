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
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.nio.charset.UnsupportedCharsetException;
import java.security.KeyFactory;
import java.security.interfaces.RSAPublicKey;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.X509EncodedKeySpec;

import org.apache.http.HttpEntity;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpRequestBase;
import org.apache.http.entity.ContentType;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.StatusCode;
import com.vmware.identity.openidconnect.protocol.Base64Utils;
import com.vmware.identity.openidconnect.protocol.HttpRequest;
import com.vmware.identity.openidconnect.protocol.JSONUtils;

import net.minidev.json.JSONObject;

public class CSPTokenPublicKey implements FederatedTokenPublicKey {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(CSPTokenPublicKey.class);

    private String _issuer;
    private RSAPublicKey _publicKey;

    private CSPTokenPublicKey(String issuer, RSAPublicKey publicKey) {
        this._issuer = issuer;
        this._publicKey = publicKey;
    }

    @Override
    public String getIssuer() {
      return _issuer;
    }

    @Override
    public RSAPublicKey getPublicKey() {
      return _publicKey;
    }

    public static CSPTokenPublicKey build(String jwkUri) throws InvalidKeySpecException, Exception {
        URI target = new URI(jwkUri);
        HttpRequest request = HttpRequest.createGetRequest(target);
        HttpRequestBase httpRequestBase = request.toHttpTask();

        try (CloseableHttpClient client = HttpClients.createDefault();
             CloseableHttpResponse response = client.execute(httpRequestBase);) {
          int statusCodeInt = response.getStatusLine().getStatusCode();
          StatusCode statusCode;
          try {
            statusCode = StatusCode.parse(statusCodeInt);
            if (statusCode != StatusCode.OK) {
              ErrorObject errorObject =
                  ErrorObject.serverError(
                      String.format(
                          "Failed to retrieve public key from [%s]",
                          jwkUri
                      )
                  );
              LoggerUtils.logFailedRequest(logger, errorObject);
              throw new ServerException(errorObject);
            }
          } catch (ParseException e) {
            ErrorObject errorObject =
                ErrorObject.serverError(
                  String.format(
                      "failed to parse status code. %s:%s",
                        e.getClass().getName(),
                        e.getMessage()
                  )
                );
            throw new ServerException(errorObject);
          }

          HttpEntity httpEntity = response.getEntity();
          if (httpEntity == null) {
            ErrorObject errorObject =
                ErrorObject.serverError(
                        "failed to retrieve public key"
                );
            throw new ServerException(errorObject);
          }

          ContentType contentType;
          try {
            contentType = ContentType.get(httpEntity);
          } catch (UnsupportedCharsetException | org.apache.http.ParseException e) {
            ErrorObject errorObject =
                ErrorObject.serverError(
                    "Error in setting content type in HTTP response"
                );
            throw new ServerException(errorObject);
          }

          Charset charset = contentType.getCharset();
          if (charset != null && !StandardCharsets.UTF_8.equals(charset)) {
            ErrorObject errorObject =
                ErrorObject.serverError(
                    String.format("unsupported charset: %s", charset)
                );
            throw new ServerException(errorObject);
          }

          if (!ContentType.APPLICATION_JSON.getMimeType().equalsIgnoreCase(contentType.getMimeType())) {
            ErrorObject errorObject =
                ErrorObject.serverError(
                    String.format(
                        "unsupported mime type: %s",
                        contentType.getMimeType()
                    )
                );
            throw new ServerException(errorObject);
          }

          try {
            String content = EntityUtils.toString(httpEntity);
            JSONObject jsonContent = JSONUtils.parseJSONObject(content);
            // Get the Issuer
            String issuer = JSONUtils.getString(jsonContent, "issuer");
            if (issuer == null || issuer.isEmpty()) {
              ErrorObject errorObject = ErrorObject.serverError(
                  "Error: Invalid Issuer found for public key"
              );
              throw new ServerException(errorObject);
            }
            // Get the Algorithm
            String alg = JSONUtils.getString(jsonContent, "alg");
            if (alg == null || (!alg.equals("RSA") && !alg.equals("SHA256withRSA"))) {
              ErrorObject errorObject = ErrorObject.serverError(
                  String.format(
                      "Error: No Handler for enclosed Key's Algorithm (%s)",
                      alg
                  )
              );
              throw new ServerException(errorObject);
            }
            // Get the key material
            String val = JSONUtils.getString(jsonContent, "value");
            if (val == null || val.isEmpty()) {
              ErrorObject errorObject = ErrorObject.serverError(
                      "Error: Invalid key material for public key"
              );
              throw new ServerException(errorObject);
            }
            val = val.replaceAll("-----BEGIN PUBLIC KEY-----", "")
                     .replaceAll("-----END PUBLIC KEY-----", "")
                     .replaceAll("[\n\r]", "")
                     .trim();
            byte[] decoded = Base64Utils.decodeToBytes(val);
            X509EncodedKeySpec spec = new X509EncodedKeySpec(decoded);
            KeyFactory kf = KeyFactory.getInstance("RSA");
            return new CSPTokenPublicKey(issuer, (RSAPublicKey) kf.generatePublic(spec));
          } catch (ParseException e) {
            ErrorObject errorObject = ErrorObject.serverError(
                String.format(
                    "failed to get public key from federatd IDP. %s:%s",
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
