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
package com.vmware.identity.session.impl;

import java.security.NoSuchAlgorithmException;
import java.util.Calendar;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.samlservice.SamlServiceException;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.session.Session;
import com.vmware.identity.session.SessionManager;
import com.vmware.identity.session.SessionParticipant;

/**
 * @author root
 *
 */
public class SessionManagerImpl implements SessionManager {
    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(SessionManagerImpl.class);

    private Map<String, Session> sessions;
    private Map<String, String> sessionParticipants; // relying party session id
                                                     // --> session id
    private Map<String, String> sessionRequests; // logout request id -->
                                                 // session id
    private Lock lock;

    /**
     * Construct the object
     */
    public SessionManagerImpl() {
        log.debug("SessionManagerImpl created");

        this.sessions = new HashMap<String, Session>();
        this.sessionParticipants = new HashMap<String, String>();
        this.sessionRequests = new HashMap<String, String>();
        this.lock = new ReentrantLock();
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.vmware.identity.session.SessionManager#add(com.vmware.identity.session
     * .Session)
     */
    @Override
    public void add(Session session) {
        Validate.notNull(session);
        log.debug("Adding " + session.toString());

        this.lock.lock();

        try {
            this.sessions.put(session.getId(), session);
            for (SessionParticipant participant : session.getSessionParticipants()) {
                this.sessionParticipants.put(participant.getSessionId(),
                        session.getId());
            }
            if (session.getLogoutRequestData() != null
                    && session.getLogoutRequestData().getCurrentRequestId() != null) {
                this.sessionRequests.put(session.getLogoutRequestData()
                        .getCurrentRequestId(), session.getId());
            }
        } finally {
            this.lock.unlock();
        }
    }

    /**
     * createSession() create a new session
     * @param externalIDPSessionId  -  optional external idp session id,
     *  only used for external authentication workflow.
     * @throws SamlServiceException
     */
    @Override
    public Session createSession(PrincipalId principal, AuthnMethod authMethod,
            String externalIDPSessionId, String idpEntId)
                    throws SamlServiceException {

        Validate.notNull(principal, "user principalId");

        // create a new session here
        Calendar calendar = new GregorianCalendar();
        calendar.add(Calendar.MINUTE, Shared.SESSION_LIFETIME_MINUTES);
        Date sessionEndTime = calendar.getTime();
        try {
            Session currentSession =
                new Session(principal, sessionEndTime
                        , authMethod);
            if (externalIDPSessionId != null) {
                Validate.notEmpty(idpEntId, "Need idpEntId for creating external authenticated session");

                currentSession.setUsingExtIDP(true);
                currentSession.setExtIDPToUsed(new IDPConfig(idpEntId));
                currentSession.setExtIDPSessionID(externalIDPSessionId);
             }
            add(currentSession);
            return currentSession;
        } catch (NoSuchAlgorithmException e) {
            throw new SamlServiceException(e);
        }
    }

    /*
     * (non-Javadoc)
     *
     * @see com.vmware.identity.session.SessionManager#getAll()
     */
    @Override
    public Collection<Session> getAll() {
        log.debug("Returning all sessions");

        this.lock.lock();

        try {
            return Collections.unmodifiableCollection(this.sessions.values());
        } finally {
            this.lock.unlock();
        }
    }

    /*
     * (non-Javadoc)
     *
     * @see com.vmware.identity.session.SessionManager#get(java.lang.String)
     */
    @Override
    public Session get(String sessionId) {
        Validate.notNull(sessionId);
        log.debug("Querying " + sessionId);

        this.lock.lock();

        try {
            Session retval = null;
            if (this.sessions.containsKey(sessionId)) {
                retval = this.sessions.get(sessionId);
            }

            log.debug("Found " + retval);
            return retval;
        } finally {
            this.lock.unlock();
        }
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.vmware.identity.session.SessionManager#getByParticipant(java.lang
     * .String)
     */
    @Override
    public Session getByParticipant(String participantSessionId) {
        Validate.notNull(participantSessionId);
        log.debug("Querying participant " + participantSessionId);

        this.lock.lock();

        try {
            Session retval = null;
            String sessionId = null;

            if (this.sessionParticipants.containsKey(participantSessionId)) {
                sessionId = this.sessionParticipants.get(participantSessionId);
            }
            if (sessionId != null && this.sessions.containsKey(sessionId)) {
                retval = this.sessions.get(sessionId);
            }

            log.debug("Found " + retval);
            return retval;
        } finally {
            this.lock.unlock();
        }
    }

    /*
     * (non-Javadoc)
     *
     * @see com.vmware.identity.session.SessionManager#remove(java.lang.String)
     */
    @Override
    public void remove(String sessionId) {
        Validate.notNull(sessionId);
        log.debug("Removing " + sessionId);

        this.lock.lock();

        try {
            if (this.sessions.containsKey(sessionId)) {
                this.sessions.remove(sessionId);
                if (this.sessionParticipants.containsValue(sessionId)) {
                    this.sessionParticipants.values().remove(sessionId);
                }
                if (this.sessionRequests.containsValue(sessionId)) {
                    this.sessionRequests.values().remove(sessionId);
                }
            }
        } finally {
            this.lock.unlock();
        }
    }

    /*
     * (non-Javadoc)
     *
     * @see com.vmware.identity.session.SessionManager#removeAll()
     */
    @Override
    public void clear() {
        log.debug("Removing all sessions");

        this.lock.lock();

        try {
            this.sessions.clear();
            this.sessionParticipants.clear();
            this.sessionRequests.clear();
        } finally {
            this.lock.unlock();
        }
    }

    @Override
    public void update(Session session) {
        Validate.notNull(session);
        log.debug("Updating " + session);

        String sessionId = session.getId();
        remove(sessionId);
        add(session);
    }

    @Override
    public Session getByLogoutRequestId(String logoutRequestId) {
        Validate.notNull(logoutRequestId);
        log.debug("Querying by logout request " + logoutRequestId);

        this.lock.lock();

        try {
            Session retval = null;
            String sessionId = null;

            if (this.sessionRequests.containsKey(logoutRequestId)) {
                sessionId = this.sessionRequests.get(logoutRequestId);
            }
            if (sessionId != null && this.sessions.containsKey(sessionId)) {
                retval = this.sessions.get(sessionId);
            }

            log.debug("Found " + retval);
            return retval;
        } finally {
            this.lock.unlock();
        }
    }

}
