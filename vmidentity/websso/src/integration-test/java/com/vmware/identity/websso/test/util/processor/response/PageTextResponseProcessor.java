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
import com.vmware.identity.websso.test.util.ReqRespPair;
import com.vmware.identity.websso.test.util.RequestSequence;

import java.io.IOException;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/*************
 * This class checks the HTTP page response contains a list of strings
 * specified in pageText
 * @author shenoyk
 *
 */
public class PageTextResponseProcessor extends ResponseProcessorBase
    implements IResponseProcessor {
    private static final Logger log = LoggerFactory.getLogger(PageTextResponseProcessor.class);

    private List<String> pageText = null;
    protected String ResponseContent;

    public void init(List<String> pageText) {
        this.pageText = pageText;
    }

    @Override public boolean handleResponse(RequestSequence sequence)
        throws IllegalStateException {
        ReqRespPair pair = sequence.getLastRequestResponsePair();
        super.process(pair.requestState, pair.responseState);
        ResponseContent = pair.responseState.getResponseContent();
        if (ResponseContent == null || ResponseContent.isEmpty()) {
            log.error("Response Content is null or empty");
            return false;
        }
        Assert.assertNotNull(pageText, "pageText not initialized");
        for (String str : pageText) {
            if (ResponseContent.indexOf(str) == -1) {
                log.error("Response does not contain " + str);
                return false;
            }
        }
        setSuccess();
        return true;
    }
}
