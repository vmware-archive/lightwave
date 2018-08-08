/*
 *  Copyright (c) 2012-2018 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.websso.test.util;

import com.vmware.identity.websso.test.util.common.Assert;
import com.vmware.identity.websso.test.util.common.SAMLConstants;

import java.io.IOException;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;

import org.apache.http.HttpHost;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.RedirectStrategy;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpUriRequest;
import org.apache.http.client.protocol.HttpClientContext;
import org.apache.http.config.Registry;
import org.apache.http.config.RegistryBuilder;
import org.apache.http.conn.HttpClientConnectionManager;
import org.apache.http.conn.socket.ConnectionSocketFactory;
import org.apache.http.conn.ssl.AllowAllHostnameVerifier;
import org.apache.http.conn.ssl.SSLConnectionSocketFactory;
import org.apache.http.impl.client.BasicCookieStore;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClientBuilder;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.impl.conn.BasicHttpClientConnectionManager;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class TestHttpClient {
  private static final Logger log = LoggerFactory.getLogger(TestHttpClient.class);

  SSLConnectionSocketFactory sslFactory;
  RedirectStrategy redirectStrategy;
  CloseableHttpClient client = null;
  BasicCookieStore cookieStore = null;
  HttpClientContext context = null;
  boolean requestSubmitted = false;
  HttpHost httpProxy = null;

  public TestHttpClient() {
  }

  public void setCookieStore(BasicCookieStore cookieStore) {
    Assert.assertFalse(requestSubmitted, "Request already submitted using this HttpClient");
    this.cookieStore = cookieStore;
  }

  public void setSSLConnectionSocketFactory(SSLConnectionSocketFactory sslFactory) {
    this.sslFactory = sslFactory;
  }

  public void setRedirectStrategy(RedirectStrategy redirectStrategy) {
    this.redirectStrategy = redirectStrategy;
  }

  protected HttpClientContext getHttpClientContext() {
    return context;
  }

  public CloseableHttpResponse execute(HttpUriRequest request) throws IOException {
    build();
    int retryCount = 0;
    CloseableHttpResponse response = null;
    while (retryCount++ < 6 && response == null) {
      try {
        context = HttpClientContext.create();
        if (cookieStore != null && context != null) {
          context.setCookieStore(cookieStore);
        }
        response = client.execute(request, context);
        requestSubmitted = true;
      } catch (org.apache.http.conn.HttpHostConnectException e) {
        try {
          Thread.sleep(retryCount * 1000 * 30);
        } catch (InterruptedException e1) {

        }
      }
    }
    return response;
  }

  private void build() {
    // Request has already been submitted. Create a new HttpClient to submit request.
    Assert.assertFalse(requestSubmitted, "Request already submitted using this HttpClient");

    try {
      TestX509TrustManager x509tm = new TestX509TrustManager();
      SSLContext sslContext = SSLContext.getInstance("TLS");
      sslContext.init(null, new TrustManager[]{x509tm}, null);

      if (sslFactory == null) {
        sslFactory = new SSLConnectionSocketFactory(sslContext, new AllowAllHostnameVerifier());
      }

      Registry<ConnectionSocketFactory> reg = RegistryBuilder.<ConnectionSocketFactory>create()
          .register("https", sslFactory)
          .build();

      HttpClientConnectionManager ccm = new BasicHttpClientConnectionManager(reg);
      HttpClientBuilder builder = HttpClients.custom();

      if (cookieStore != null) {
        builder.setDefaultCookieStore(cookieStore);
      }
      if (ccm != null) {
        builder.setConnectionManager(ccm);
      }
      if (redirectStrategy != null) {
        builder.setRedirectStrategy(redirectStrategy);
      }
      if (httpProxy != null) {
        // Not able to correctly set proxy settings
        throw new UnsupportedOperationException();
      }
      // Setting user-agent by default to that emitted by Chrome browser
      builder.setUserAgent(SAMLConstants.ChromeUserAgent);
      client = builder.build();
      Assert.assertNotNull(client, "HttpClientBuilder returned null HttpClient");
    } catch (KeyManagementException exc) {
      HttpClientUtils.logException(log, exc);
    } catch (NoSuchAlgorithmException exc) {
      HttpClientUtils.logException(log, exc);
    }
  }
}
