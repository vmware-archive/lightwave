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

public class ReqRespPair {
  public RequestState requestState;
  public ResponseState responseState;
  private long execMillis;

  public ReqRespPair() {
    requestState = null;
    responseState = null;
  }

  ReqRespPair(RequestState reqState, ResponseState respState, long execMillis) {
    Assert.assertTrue(
      reqState != null && respState != null,
      "RequestState or ResponseState is null"
    );

    requestState = reqState;
    responseState = respState;
    this.execMillis = execMillis;
  }

  public long getExecTime() {
    return execMillis;
  }

  @Override
  public String toString() {
    StringBuilder sb = new StringBuilder();
    if (requestState != null) {
      sb.append(requestState.toString());
    }
    if (responseState != null) {
      sb.append(responseState.toString());
    }
    return sb.toString();
  }
}
