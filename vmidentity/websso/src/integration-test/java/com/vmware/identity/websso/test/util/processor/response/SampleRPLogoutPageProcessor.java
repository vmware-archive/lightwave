/*
 * Copyright (c) 2018 VMware, Inc. All Rights Reserved.
 */
package com.vmware.identity.websso.test.util.processor.response;

import com.vmware.identity.websso.test.util.RequestSequence;
import com.vmware.identity.websso.test.util.common.Assert;
import static com.vmware.identity.websso.test.util.common.SSOConstants.UTF8_CHARSET;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

public class SampleRPLogoutPageProcessor extends PageTextResponseProcessor implements IResponseProcessor {
  private String reLoginLink = null;

  public String getReloginLink() {
    return reLoginLink;
  }

  @Override
  public boolean handleResponse(RequestSequence sequence) {
    String responseContent = sequence.getLastRequestResponsePair().responseState.getResponseContent();
    Document document = Jsoup.parse(responseContent, UTF8_CHARSET);
    Document parse = Jsoup.parse(document.html());
    Elements eList = parse.select("h1");
    Assert.assertTrue(eList.size() == 1, "Failed to find Heading in content page");
    String logoutString = eList.first().wholeText();
    Assert.assertTrue(
      logoutString != null && logoutString.trim().equalsIgnoreCase(": You are logged out."),
      "Logout Header does not match"
    );
    Element linkElement = parse.select("a").first();
    Assert.assertNotNull(linkElement, "No re-login link found");
    reLoginLink = linkElement.attr("href");
    Assert.assertTrue(reLoginLink != null && !reLoginLink.isEmpty(), "Invalid re-login link");
    setSuccess();
    return true;
  }
}
