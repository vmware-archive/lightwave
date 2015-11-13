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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertNotNull;

import java.util.Calendar;
import java.util.Collection;
import java.util.Date;
import java.util.GregorianCalendar;

import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.TestConstants;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;

/**
 * Tests for Session structure.
 *
 */
public class SessionTest {
    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(SessionTest.class);

    private static PrincipalId principal;
    private static Date date;

    @BeforeClass
    public static void setUp() throws Exception {
        principal = new PrincipalId(TestConstants.USER, TestConstants.DOMAIN);
        Calendar calendar = new GregorianCalendar();
        date = calendar.getTime();
    }

    /**
     * Test method for {@link com.vmware.identity.session.Session#Session(com.vmware.identity.idm.PrincipalId, java.util.Date)}.
     */
    @Test
    public final void testSession() throws Exception {
        Session session = new Session(principal, date, AuthnMethod.KERBEROS);

        assertEquals(session.getPrincipalId(), principal);
        assertEquals(session.getExpireDate(), date);
        assertFalse(session.isValid());
        assertEquals(session.getAuthnMethod(), AuthnMethod.KERBEROS);
        assertNotNull(session.getId());
        assertTrue(session.getSessionParticipants().isEmpty());
    }

    @Test
    public final void testEnsureParticipant() throws Exception {
        Session session = new Session(principal, date, AuthnMethod.KERBEROS);
        String participantSessionId =
                session.ensureSessionParticipant(TestConstants.RELYING_PARTY);

        assertNotNull(participantSessionId);
        Collection<SessionParticipant> participants = session.getSessionParticipants();
        log.debug("Participant count " + participants.size());
        for (SessionParticipant sp : participants) {
            log.debug("PARTICIPANT " + sp);
        }
        assertEquals(participants.size(), 1);

        String participantSessionId2 =
                session.ensureSessionParticipant(TestConstants.RELYING_PARTY);
        // should get same participant session
        assertEquals(participantSessionId, participantSessionId2);
        assertEquals(session.getSessionParticipants().size(), 1);
    }

    @Test
    public final void testEquals() throws Exception {
        Session session1 = new Session(principal, date, AuthnMethod.KERBEROS);
        Session session2 = new Session(principal, date, AuthnMethod.KERBEROS);

        // must have different session ids, so won't be equal
        assertFalse(session1.equals(session2));
    }
}
