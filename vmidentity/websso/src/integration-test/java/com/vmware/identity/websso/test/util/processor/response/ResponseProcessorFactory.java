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

import com.vmware.identity.websso.test.util.common.Assert;

public class ResponseProcessorFactory {

  public static IResponseProcessor getInstance(ResponseProcessorType type) {
    IResponseProcessor rsp = null;
    switch (type) {
      case STSResponseProcessor:
        rsp = new STSResponseProcessor();
        break;
      case RedirectResponseProcessor:
        rsp = new RedirectResponseProcessor();
        break;
      case PageTextResponseProcessor:
        rsp = new PageTextResponseProcessor();
        break;
      case LightwaveLoginPageResponseProcessor:
        rsp = new LightwaveLoginPageResponseProcessor();
        break;
      case SampleRPContentPageProcessor:
        rsp = new SampleRPContentPageProcessor();
        break;
      case SampleRPLogoutPageProcessor:
        rsp = new SampleRPLogoutPageProcessor();
        break;
      default:
        throw new UnsupportedOperationException();

    }
    Assert.assertNotNull(rsp, "Could not instantiate ResponseProcessor");
    return rsp;
  }
}
