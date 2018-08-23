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

import org.apache.http.HttpRequest;
import org.apache.http.HttpResponse;
import org.apache.http.client.RedirectStrategy;
import org.apache.http.client.methods.HttpUriRequest;
import org.apache.http.protocol.HttpContext;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class CustomHttpRedirectStrategy implements RedirectStrategy {
  static final Logger log = LoggerFactory.getLogger(CustomHttpRedirectStrategy.class);

  ReqRespPair pair;
  boolean autoRedirect = true;
  HttpRequest savedRequest = null;

  // Using the boolean autoRedirect to decide if we need to automatically
  // redirect. True by default
  public CustomHttpRedirectStrategy(ReqRespPair pair, boolean autoRedirect) {
    this.pair = pair; this.autoRedirect = autoRedirect;
  }

  @Override public HttpUriRequest getRedirect(
    HttpRequest req, HttpResponse resp, HttpContext ctxt
  ) {
    return (HttpUriRequest) savedRequest;
  }

  @Override public boolean isRedirected(
    HttpRequest request, HttpResponse response, HttpContext ctxt
  ) {
    // TODO Auto-generated method stub
    savedRequest = request;
    RequestState requestState = RequestState.buildFromHttpRequest("RedirectRequest1", request);
    ResponseState responseState = null;
    try {
      responseState = new ResponseState("RedirectResponse1");
      // No ClientContext provided so ClientContext is null
      responseState.buildFromHttpResponse(response, null);
    } catch (IllegalStateException e) {
      HttpClientUtils.logException(log, e);
    }
    log.debug(requestState.toString());
    log.debug(responseState.toString());
    pair.requestState = requestState;
    pair.responseState = responseState;
    // TODO No easy way to measure the request response time in a redirect callback
    return autoRedirect;
  }
}
