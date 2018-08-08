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

package com.vmware.identity.websso.test.integration;

import com.vmware.identity.websso.test.util.CustomHttpRedirectStrategy;
import com.vmware.identity.websso.test.util.HttpClientUtils;
import com.vmware.identity.websso.test.util.ReqRespPair;
import com.vmware.identity.websso.test.util.RequestSequence;
import com.vmware.identity.websso.test.util.RequestState;
import com.vmware.identity.websso.test.util.TestHttpClient;
import com.vmware.identity.websso.test.util.WebSSOTestBase;
import com.vmware.identity.websso.test.util.common.Assert;
import com.vmware.identity.websso.test.util.config.SampleRPConfiguration;
import com.vmware.identity.websso.test.util.config.TestConfig;
import com.vmware.identity.websso.test.util.processor.request.RequestType;
import com.vmware.identity.websso.test.util.processor.response.ResponseProcessorFactory;
import com.vmware.identity.websso.test.util.processor.response.ResponseProcessorType;
import com.vmware.identity.websso.test.util.processor.response.STSResponseProcessor;
import com.vmware.identity.websso.test.util.processor.response.SampleRPContentPageProcessor;
import com.vmware.identity.websso.test.util.processor.response.SampleRPLogoutPageProcessor;

import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;

import org.apache.http.client.utils.URIBuilder;
import org.apache.http.cookie.Cookie;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.Test;

import org.junit.rules.TestName;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

@RunWith(Parameterized.class)
public class WebssoSampleLibIT extends WebSSOTestBase {
  public static final Logger log = LoggerFactory.getLogger(WebssoSampleLibIT.class);

  @Rule
  public TestName name = new TestName();

  @Parameterized.Parameter // first data value (0) is default
  public TestParam param;

  private static SampleRPConfiguration samlRP;

  @Parameterized.Parameters
  public static Collection<TestParam> data() {
    TestConfig properties1 = new TestConfig(); ArrayList<TestParam> testParams = new ArrayList<TestParam>();
    testParams.add(new TestParam(properties1.getProperty(WebSSOTestBase.KEY_ORG_1)));
    testParams.add(new TestParam(properties1.getProperty(WebSSOTestBase.KEY_ORG_2))); return testParams;
  }

  @BeforeClass
  public static void testSetUp() throws Exception {
    WebSSOTestBase.setup(WebssoSampleLibIT.class.getName()); samlRP = WebSSOTestBase.getSAMLRPConfiguration();
  }

  @Test
  public void testSSORedirectOnAppRequest() throws Exception {
    logger.info(String.format("Testing SSO Redirect for Org (%s)", param.getOrgName()));

    RequestSequence sequence = new RequestSequence();

    sampleAppRequestCounter.reset();

    URI sampleAppURI = GetSampleAppURI(param.getOrgName());

    // Step 1: Request to Sample APP URL
    doSampleAppRequest(sampleAppURI, sequence);
    // Step 2: Request to STS
    doWebSSORequest(sequence);
    // Step 3: Fill in the Login Page, submit and get SAML Token
    STSResponseProcessor stsResponseProcessor = doWebSSOLogin(sequence);

    String encodedSAMLToken = stsResponseProcessor.getEncodedSamlData();
    Assert.assertNotNull(encodedSAMLToken, "Invalid SAML Token");

    String redirectURL = stsResponseProcessor.getRedirectURL();
    Assert.assertNotNull(redirectURL, "Invalid redirect URL to Service Provider");
    // Step 4: Navigate to Service Provider URL. Get content URL
    String contentURL = doServiceProviderRequest(redirectURL, encodedSAMLToken, sequence);
    Assert.assertNotNull(contentURL, "Content URL is null");
    ReqRespPair contentURLResponse = sequence.getLastRequestResponsePair();
    // Step 5: Navigate to content URL
    URI logoutURL = doContentRequest(contentURL, encodedSAMLToken, stsResponseProcessor.getExpectedUPN(), sequence);
    // Step 6: Logout
    String reLoginLink = doLogout(logoutURL, contentURLResponse.responseState.getAllCookies(), sequence);
    Assert.assertTrue(sampleAppURI.getPath().equals(reLoginLink), "Re-login link does not match");
  }

  @AfterClass
  public static void testCleanUp() {
  }

  private void doSampleAppRequest(URI sampleAppURI, RequestSequence sequence) throws IOException {
    TestHttpClient client = new TestHttpClient();
    client.setRedirectStrategy(
      new CustomHttpRedirectStrategy(
        new ReqRespPair(),
        false // autoRedirect
      )
    );

    RequestState req = new RequestState("Request to WebSSO SampleApp");
    req.setRequestURI(sampleAppURI);

    ReqRespPair pair = req.executeRequest(client);

    sampleAppRequestCounter.addValue(pair.getExecTime());

    sequence.addRequestResponse(pair);

    logger.debug(pair.toString());

    String ssoSAMLRequestURL = pair.responseState.getLocation();

    Assert.assertNotNull(ssoSAMLRequestURL, "SampleAppRequest did not result in SSO redirect");
    // Validate result. Expecting to get redirect to STS Login URL
    Assert.assertTrue(
      ResponseProcessorFactory.getInstance(ResponseProcessorType.RedirectResponseProcessor).handleResponse(sequence),
      "RedirectResponseProcessor validation failed"
    );
  }

  private void doWebSSORequest(RequestSequence sequence) throws URISyntaxException, IOException {
    RequestState stsReq = new RequestState(
                      "Request to WebSSO STS",
                      sequence.getLastRequestResponsePair().requestState,
                      sequence.getLastRequestResponsePair().responseState
    );

    ReqRespPair pair = stsReq.executeRequest(new TestHttpClient());

    sampleAppRequestCounter.addValue(pair.getExecTime());

    logger.debug(pair.toString());

    sequence.addRequestResponse(pair);
    // Validate result.
    Assert.assertTrue(ResponseProcessorFactory.getInstance(ResponseProcessorType.LightwaveLoginPageResponseProcessor)
                                              .handleResponse(sequence),
                      "Failed to validate Lightwave Login Page Response"
    );
  }

  private STSResponseProcessor doWebSSOLogin(RequestSequence sequence) throws URISyntaxException, IOException {
    RequestState loginReq = new RequestState("Login",
                                             sequence.getLastRequestResponsePair().requestState,
                                             sequence.getLastRequestResponsePair().responseState
    );
    loginReq.setVerb(RequestType.POST);
    loginReq.addRequestHeader("Cache-Control", "no-cache");
    loginReq.addRequestHeader("Pragma", "no-cache");
    loginReq.addRequestHeader("Content-type", "application/x-www-form-urlencoded");
    HashMap<String, String> queryParams = new HashMap<String, String>();
    queryParams.put(
      "CastleAuthorization",
      String.format(
        "Basic %s",
        HttpClientUtils.ConvertToBase64(
          String.format(
            "%s:%s",
            properties.getProperty("system.admin.username"),
            properties.getProperty("system.admin.password")
          )
        )
      )
    );
    loginReq.setQueryParams(queryParams);

    ReqRespPair pair = loginReq.executeRequest(new TestHttpClient());

    sampleAppRequestCounter.addValue(pair.getExecTime());

    logger.debug(pair.toString());

    sequence.addRequestResponse(pair);

    STSResponseProcessor stsResponseProcessor =
      (STSResponseProcessor) ResponseProcessorFactory.getInstance(
                                ResponseProcessorType.STSResponseProcessor
                              );
    stsResponseProcessor.setExpectedUPN(properties.getProperty("system.admin.username"));
    stsResponseProcessor.setExpectedDomain(properties.getProperty("system.domain"));
    Assert.assertTrue(stsResponseProcessor.handleResponse(sequence), "LoginPageProcessor validation failed");
    return stsResponseProcessor;
  }

  private String doServiceProviderRequest(
    String serviceProviderURL, String encodedSAMLToken, RequestSequence sequence
  ) throws URISyntaxException, IOException {
    RequestState reqWithToken = new RequestState("Request to WebSSO SampleApp with SAML Token",
                                                 sequence.getLastRequestResponsePair().requestState,
                                                 sequence.getLastRequestResponsePair().responseState
    );
    reqWithToken.setRequestURI(serviceProviderURL);
    reqWithToken.addFormVariable("SAMLResponse", encodedSAMLToken);
    reqWithToken.setVerb(RequestType.POST);

    TestHttpClient client = new TestHttpClient();
    client.setRedirectStrategy(
      new CustomHttpRedirectStrategy(
        new ReqRespPair(),
        false // autoRedirect
      )
    );

    ReqRespPair pair = reqWithToken.executeRequest(client);

    sampleAppRequestCounter.addValue(pair.getExecTime());

    logger.debug(pair.toString());

    sequence.addRequestResponse(pair);

    return pair.responseState.getLocation();
  }

  private URI doContentRequest(
    String contentURL, String encodedSAMLToken, String expectedUPN, RequestSequence sequence
  ) throws URISyntaxException, IOException {
    RequestState contentRequest = new RequestState(
      "Content request",
                 sequence.getLastRequestResponsePair().requestState,
                 sequence.getLastRequestResponsePair().responseState
    );
    contentRequest.setRequestURI(contentURL);

    ReqRespPair pair = contentRequest.executeRequest(new TestHttpClient());

    sampleAppRequestCounter.addValue(pair.getExecTime());

    logger.debug(pair.toString());

    sequence.addRequestResponse(pair);

    SampleRPContentPageProcessor sampleRPContentPage =
      (SampleRPContentPageProcessor) ResponseProcessorFactory.getInstance(
        ResponseProcessorType.SampleRPContentPageProcessor
      );
    sampleRPContentPage.setExpectedUPN(expectedUPN);
    Assert.assertTrue(
      sampleRPContentPage.handleResponse(sequence),
      "Failed to validate content URL"
    );
    return new URIBuilder(pair.requestState.getRequestURI())
                .setPath(sampleRPContentPage.getLogoutLink())
                .build();
  }

  private String doLogout(
    URI logoutURI, List<Cookie> cookies, RequestSequence sequence
  ) throws URISyntaxException, IOException {
    RequestState contentRequest = new RequestState(
      "Logout request",
      sequence.getLastRequestResponsePair().requestState,
      sequence.getLastRequestResponsePair().responseState
    );
    contentRequest.setRequestURI(logoutURI);
    contentRequest.addCookies(cookies);

    ReqRespPair pair = contentRequest.executeRequest(new TestHttpClient());

    sampleAppRequestCounter.addValue(pair.getExecTime());

    logger.debug(pair.toString());

    sequence.addRequestResponse(pair);

    SampleRPLogoutPageProcessor sampleRPLogoutPage =
      (SampleRPLogoutPageProcessor) ResponseProcessorFactory.getInstance(
        ResponseProcessorType.SampleRPLogoutPageProcessor
      );
    Assert.assertTrue(
      sampleRPLogoutPage.handleResponse(sequence),
      "Failed to validate logout page"
    );

    return sampleRPLogoutPage.getReloginLink();
  }

  private static class TestParam {
    private String orgName;

    public TestParam(String orgName) {
      this.orgName = orgName;
    }

    public String getOrgName() {
      return orgName;
    }
  }
}
