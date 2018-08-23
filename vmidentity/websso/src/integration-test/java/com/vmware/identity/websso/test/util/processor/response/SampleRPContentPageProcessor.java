/*
 * Copyright (c) 2018 VMware, Inc. All Rights Reserved.
 */
package com.vmware.identity.websso.test.util.processor.response;

import com.vmware.identity.websso.test.util.RequestSequence;
import com.vmware.identity.websso.test.util.common.Assert;
import static com.vmware.identity.websso.test.util.common.SSOConstants.UTF8_CHARSET;

import org.apache.http.client.utils.URIBuilder;
import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

import java.net.URI;
import java.net.URISyntaxException;

public class SampleRPContentPageProcessor extends PageTextResponseProcessor implements IResponseProcessor {
  private String logoutLink = null;
  private String expectedUPN = "";

  public String getLogoutLink() {
    return logoutLink;
  }

  public void setExpectedUPN(String expectedUPN) {
    this.expectedUPN = expectedUPN;
  }

  @Override
  public boolean handleResponse(RequestSequence sequence) {
    String responseContent = sequence.getLastRequestResponsePair().responseState.getResponseContent();
    Document document = Jsoup.parse(responseContent, UTF8_CHARSET);
    Document parse = Jsoup.parse(document.html());
    Elements eList = parse.select("h1");
    Assert.assertTrue(eList.size() == 1, "Failed to find Heading in content page");
    String welcomeString = eList.first().wholeText();
    String expectedWelcomeString = String.format("Welcome %s", expectedUPN);
    Assert.assertTrue(
      welcomeString != null && welcomeString.trim().equalsIgnoreCase(expectedWelcomeString),
      "Content Header does not match"
    );
    Element linkElement = parse.select("a").first();
    Assert.assertNotNull(linkElement, "No logout link found");
    logoutLink = linkElement.attr("href");
    Assert.assertTrue(logoutLink != null && !logoutLink.isEmpty(), "Invalid logout link");
    setSuccess();
    return true;
  }
}
