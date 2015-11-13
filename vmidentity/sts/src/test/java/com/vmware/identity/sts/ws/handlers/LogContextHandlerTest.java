/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.sts.ws.handlers;

import static org.junit.Assert.assertEquals;

import org.junit.Test;

public class LogContextHandlerTest {

    @Test
    public void testRemoveNewline() throws Exception {
        assertEquals(null,          LogContextHandler.removeNewline(null));
        assertEquals("",            LogContextHandler.removeNewline(""));
        assertEquals("a",           LogContextHandler.removeNewline("a"));
        assertEquals("abcdefgh",    LogContextHandler.removeNewline("ab\ncd\ref\n\rgh"));
    }

    @Test
    public void testTruncate() throws Exception {
        assertEquals(null,      LogContextHandler.truncate(null,   2));
        assertEquals("",        LogContextHandler.truncate("",     2));
        assertEquals("a",       LogContextHandler.truncate("a",    2));
        assertEquals("ab",      LogContextHandler.truncate("ab",   2));
        assertEquals("ab...",   LogContextHandler.truncate("abc",  2));
        assertEquals("ab...",   LogContextHandler.truncate("abcd", 2));
    }
}
