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
package com.vmware.identity.session;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;

import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import com.vmware.identity.TestConstants;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.session.impl.SessionManagerImpl;

/**
 * Test SessionCleanupWrapper functionality (verify that expired sessions are removed)
 *
 */
@Ignore // ignored due to IDM process to library change, see PR 1780279.
public class SessionCleanupWrapperTest {
    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(SessionCleanupWrapperTest.class);

    private static PrincipalId principal;
    private static Date date;
    private static SessionManager sessionManager;
    private static SessionCleanupWrapper sessionCleanupWrapper;

    @BeforeClass
    public static void setUp() throws Exception {
        principal = new PrincipalId(TestConstants.USER, TestConstants.DOMAIN);
        Calendar calendar = new GregorianCalendar();
        date = calendar.getTime();
        sessionManager = new SessionManagerImpl();
        sessionCleanupWrapper = new SessionCleanupWrapper();
        sessionCleanupWrapper.setSessionManager(sessionManager);
        (new Thread(sessionCleanupWrapper)).start();
    }

    @Test
    public final void testSessionCleanup() throws Exception {
        Session session1 = new Session(principal, date, AuthnMethod.KERBEROS); // first session will be expired
        Calendar calendar = new GregorianCalendar();
        calendar.add(Calendar.YEAR, 1);
        Session session2 = new Session(principal, calendar.getTime(), AuthnMethod.KERBEROS); // second session will live on

        sessionManager.add(session1);
        sessionManager.add(session2);

        log.debug("Sleeping for " + (SessionCleanupWrapper.SLEEP_SECONDS+1) + " seconds");
        Thread.sleep((SessionCleanupWrapper.SLEEP_SECONDS + 1)*1000);

        // check that session2 is there and session1 is not there
        assertNotNull(sessionManager.get(session2.getId()));
        assertNull(sessionManager.get(session1.getId()));
    }
}
