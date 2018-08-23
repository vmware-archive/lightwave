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

package com.vmware.identity.websso.test.util.processor.response;

import com.vmware.identity.websso.test.util.HttpClientUtils;
import com.vmware.identity.websso.test.util.ReqRespPair;
import com.vmware.identity.websso.test.util.RequestSequence;
import com.vmware.identity.websso.test.util.common.SamlTokenValidator;

import java.io.IOException;
import java.util.ArrayList;

import org.junit.Assert;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class STSResponseProcessor extends ResponseProcessorBase implements IResponseProcessor {

  protected static final Logger log = LoggerFactory.getLogger(STSResponseProcessor.class);

  private String decodedSamlData = null;
  private String encodedSamlStr = null;
  private boolean isValidateGroups = false;
  private boolean isValidateAudience = false;
  private SamlTokenValidator stv = null;
  private String expectedUPN = null;
  private String domain = null;
  private String redirectURL = null;

  public void setExpectedUPN(String upn) {
    this.expectedUPN = upn;
  }

  public String getExpectedUPN() {
    return this.expectedUPN;
  }

  public void setExpectedDomain(String domain) {
    this.domain = domain;
  }

  @Override public boolean handleResponse(RequestSequence sequence) throws IllegalStateException {
    ReqRespPair pair = sequence.getLastRequestResponsePair(); super.process(pair.requestState, pair.responseState);

    if (200 != pair.responseState.getStatusCode()) {
      log.error(String.format("Response status code=%d expected %d\n", pair.responseState.getStatusCode(), 200));
      return false;
    }
    // TODO parse SAMLResponse to get SAML token and validate SAMLToken
    String ResponseContent = pair.responseState.getResponseContent();
    if (ResponseContent == null || ResponseContent.isEmpty()) {
      log.error(String.format("ResponseContent is (%s)\n", ResponseContent == null ? "null" : "empty"));
      return false;
    }
    // Response should contain a SAMLResponse in the response field
    if (ResponseContent.indexOf("SAMLResponse") == -1) {
      log.error("ResponseContent does not contain SAMLResponse element");
      return false;
    }
    // Parses the HTML response and extracts the samldata.
    encodedSamlStr = HttpClientUtils.getEncodedSamlToken(ResponseContent);
    decodedSamlData = HttpClientUtils.base64Decode(encodedSamlStr);
    redirectURL = HttpClientUtils.getRedirectURL(ResponseContent);
    boolean result = validateSamlToken();
    if (result) {
      setSuccess();
    }
    return result;
  }

  /**
   * Validate the SamlToken for
   * 1). UPN (Subject or NameID)
   * 2). The user group membership includes all the groups for which the user
   * is a member of.
   * 3). The SAML token audience should match the configured Relying party
   * address in config
   */
  private boolean validateSamlToken() {
    stv = new SamlTokenValidator(decodedSamlData);

    if (!stv.isValidSamlToken()) {
      return false;
    }

    boolean isValidTkn = true;

    // 1). Validate the UPN
    boolean validated = stv.validateNameID(expectedUPN);
    if (!validated) {
      log.error(String.format("Failed to validate UPN Expected(%s) Actual (%s)\n", expectedUPN, stv.getNameID()));
    }

    isValidTkn &= validated;

    // 2). The user group membership includes all the groups for which
    // the user is a member of.
    if (isValidateGroups) {
      Assert.assertNotNull(domain, "Expected domain is not set");
      ArrayList<String> expectedGroups = new ArrayList<String>();
      //TODO: Populated the expected membership from the request/config
      // file.
      expectedGroups.add(domain + "\\Users");
      expectedGroups.add(domain + "\\Administrators");
      expectedGroups.add(domain + "\\Everyone");

      validated = stv.validateGroupMembership(expectedGroups);
      if (!validated) {
        log.error("Failed to validate the group membership");
      }

      isValidTkn &= validated;
    }
    // 3). The SAML token audience should match the configured Relying
    // party address in config
    if (isValidateAudience) {
      String expectedAudience = "https://havcupgr1.ssolabs" + ".com:9443/vsphere-client/saml/websso/metadata";
      validated = stv.validateAudience(expectedAudience);
      if (!validated) {
        log.error(String.format("Failed to validate the Audience Expected:(%s) Actual(%s)" + "\n", expectedAudience,
                                stv.getAudience()
        ));
      }
      isValidTkn &= validated;
    }

    if (isValidTkn) {
      setSuccess();
      return true;
    } else {
      return false;
    }
  }

  public String getDecodedSamlData() {
    return decodedSamlData;
  }

  public String getEncodedSamlData() {
    return encodedSamlStr;
  }

  public String getRedirectURL() {
    return redirectURL;
  }

  public SamlTokenValidator getValidator() {
    return stv;
  }
}
