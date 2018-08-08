/*
 * Copyright (c) 2018 VMware, Inc. All Rights Reserved.
 */
package com.vmware.identity.websso.test.util.processor.response;

import com.vmware.identity.websso.test.util.RequestSequence;

import static com.vmware.identity.websso.test.util.common.SSOConstants.UTF8_CHARSET;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.HashMap;
import java.util.Map;

public class LightwaveLoginPageResponseProcessor extends PageTextResponseProcessor implements IResponseProcessor {
  private static final Logger logger = LoggerFactory.getLogger(LightwaveLoginPageResponseProcessor.class);

  private static final String INPUT_NAME_ATTR = "input";
  private static final String ID_ATTR = "id";
  private static final String VALUE_ATTR = "value";
  private static final String USERNAME_ATTR = "username";
  private static final String PASSWORD_ATTR = "password";

  @Override public boolean handleResponse(RequestSequence sequence) {
    boolean result = false;
    String responseContent = sequence.getLastRequestResponsePair().responseState.getResponseContent();
    Document document = Jsoup.parse(responseContent, UTF8_CHARSET);
    Document parse = Jsoup.parse(document.html());
    Elements eList = parse.select(INPUT_NAME_ATTR);
    Map<String, String> loginPageFormVars = new HashMap<String, String>();
    for (Element e : eList) {
      logger.debug("Login page input variable - " + e.attr(ID_ATTR));
      loginPageFormVars.put(e.attr(ID_ATTR), e.attr(VALUE_ATTR));
    } if (loginPageFormVars.containsKey(USERNAME_ATTR) && loginPageFormVars.containsKey(PASSWORD_ATTR)) {
      setSuccess();
      result = true;
    }
    return result;
  }
}
