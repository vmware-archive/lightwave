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

import java.util.Collection;
import java.util.Date;
import org.apache.commons.lang.Validate;
import org.springframework.beans.factory.annotation.Autowired;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
/**
 * Background thread which removes expired sessions
 *
 */
public final class SessionCleanupWrapper implements Runnable {

    /**
     * Sleep interval
     */
    public static final int SLEEP_SECONDS = 60;

    @Autowired
    private SessionManager sessionManager; // session manager service

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(SessionCleanupWrapper.class);

    /* (non-Javadoc)
     * @see java.lang.Runnable#run()
     */
    @Override
    public void run() {
        log.debug("Session cleanup thread started.");
        for (;;) {
            // sleep
            try {
                Thread.sleep(SLEEP_SECONDS*1000);
            } catch (InterruptedException e) {
                log.debug("Interrupt! Exiting session cleanup thread.");
                return;
            }

            log.debug("Check existing sessions");
            if (this.getSessionManager() == null) {
                log.debug("Session manager not yet set");
                continue;
            }
            // walk through all sessions
            Collection<Session> all = this.getSessionManager().getAll();
            for (Session session : all) {
                // remove expired sessions
                if (!session.isValid()) {
                    log.debug("Removing expired sessions {}", session.getId());
                    this.getSessionManager().remove(session.getId());
                    // TODO when supported, perform IDP-initiated SLO here
                }
            }
        }
    }

    /**
     * @return the sessionManager
     */
    public SessionManager getSessionManager() {
        return sessionManager;
    }

    /**
     * @param sessionManager the sessionManager to set
     */
    public void setSessionManager(SessionManager sessionManager) {
        Validate.notNull(sessionManager);
        this.sessionManager = sessionManager;
    }

}
