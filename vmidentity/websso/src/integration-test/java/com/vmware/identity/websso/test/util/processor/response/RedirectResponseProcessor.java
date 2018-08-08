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

import com.vmware.identity.websso.test.util.ReqRespPair;
import com.vmware.identity.websso.test.util.RequestSequence;

import java.io.IOException;

public class RedirectResponseProcessor extends ResponseProcessorBase
    implements IResponseProcessor {

    @Override
    public boolean handleResponse(RequestSequence sequence)
        throws IllegalStateException {
        boolean result = false;
        ReqRespPair pair = sequence.getLastRequestResponsePair();
        String redirectLocation = pair.responseState.getLocation();
        // TO DO validate RedirectLocation
        if (redirectLocation != null && !redirectLocation.isEmpty()) {
            setSuccess();
            result = true;
        }
        return result;
    }
}
