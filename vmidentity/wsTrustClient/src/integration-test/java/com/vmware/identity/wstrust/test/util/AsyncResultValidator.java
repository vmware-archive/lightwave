/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.wstrust.test.util;

import com.vmware.identity.wstrust.client.AsyncHandler;

/**
 * Call-back used to verify the correctness of the result from making
 * asynchronous method call
 */
public class AsyncResultValidator<T> implements AsyncHandler<T> {

  private final boolean _failOnResponse;
  private boolean _correctHandlerExecuted = false;
  private boolean _failTest = false;

  public AsyncResultValidator(boolean failOnResponse) {
    _failOnResponse = failOnResponse;
  }

  @Override
  public void handleException(Exception exception) {
    if (_failOnResponse) {
      _correctHandlerExecuted = true;
    } else {
      _failTest = true;
    }
  }

  @Override
  public void handleResponse(T response) {
    if (_failOnResponse) {
      _failTest = true;
    } else {
      _correctHandlerExecuted = true;
    }
  }

  public void verifyCorrectExecution() {
    Assert.assertFalse(_failTest, "Test Failed as incorect AsyncHandler is called ");
    Assert.assertTrue(_correctHandlerExecuted, "Test Failed as incorect AsyncHandler is called ");
  }
}
