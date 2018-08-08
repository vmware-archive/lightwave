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

import com.vmware.identity.websso.test.util.processor.response.IResponseProcessor;
import com.vmware.identity.websso.test.util.processor.response.ResponseProcessorFactory;
import com.vmware.identity.websso.test.util.processor.response.ResponseProcessorType;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.http.client.RedirectStrategy;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class RequestSequence implements Iterable<ReqRespPair> {
  protected static final Logger log = LoggerFactory.getLogger(RequestSequence.class);

  private List<ReqRespPair> reqRespList;

  public RequestSequence() {
    reqRespList = new ArrayList<ReqRespPair>();
  }

  public Iterator<ReqRespPair> iterator() {
    return reqRespList.iterator();
  }

  public void addRequestResponse(
    RequestState reqState, ResponseState respState, long responseTime
  ) {
    reqRespList.add(new ReqRespPair(reqState, respState, responseTime));
  }

  public void addRequestResponse(ReqRespPair pair) {
    reqRespList.add(pair);
  }

  public ReqRespPair getLastRequestResponsePair() {
    if (reqRespList.size() > 0) {
      return reqRespList.get(reqRespList.size() - 1);
    } else {
      return null;
    }
  }

  public IResponseProcessor executeRequestHelper(
    RequestState reqState, ResponseProcessorType type, RedirectStrategy strategy
  ) throws IOException {
    IResponseProcessor rsp = null;

    TestHttpClient t = new TestHttpClient();

    if (strategy != null) {
      t.setRedirectStrategy(strategy);
    } ReqRespPair pair = reqState.executeRequest(t); if (pair == null) {
      log.error("Request execution failed"); return null;
    }
    addRequestResponse(pair);
    if (type != null) {
      rsp = ResponseProcessorFactory.getInstance(type);
      boolean testResult = rsp.handleResponse(this);
      if (!testResult) {
        log.error(String.format("%s returned false\n", rsp.getClass().toString()));
      }
    }
    return rsp;
  }
}
