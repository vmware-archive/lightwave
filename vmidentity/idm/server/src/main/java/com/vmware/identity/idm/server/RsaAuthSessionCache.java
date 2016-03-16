/*
 *
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
 *
 */
package com.vmware.identity.idm.server;

import java.util.Iterator;
import java.util.Map.Entry;
import java.util.concurrent.ConcurrentHashMap;

import org.apache.commons.lang.Validate;

import com.vmware.identity.auth.passcode.spi.AuthenticationException;
import com.vmware.identity.auth.passcode.spi.AuthenticationSession;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;


public class RsaAuthSessionCache {

    private ConcurrentHashMap<String, ConcurrentHashMap<String, AuthenticationSession>> _rsaSessionCacheLookup; // lookup table for AuthSession cache keyed to tenant name.
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(RsaAuthSessionCache.class);

    RsaAuthSessionCache() {
        _rsaSessionCacheLookup = new ConcurrentHashMap<String, ConcurrentHashMap<String, AuthenticationSession>>();
    }

    /**
     * Removes RSA session cache for the given tenant
     * @param tenantName
     * @throws AuthenticationException
     */
    public void removeSessionCache(String tenantName) throws AuthenticationException {
        Validate.notEmpty(tenantName);

        //close all sessions in the session cache
        logger.debug("Removing RSA session cache ... tenant: "+tenantName);

        ConcurrentHashMap<String, AuthenticationSession> sessionCache = _rsaSessionCacheLookup.get(tenantName.toLowerCase());

        if (sessionCache == null) {
            logger.debug("No RSA session cache found ... tenant: "+tenantName);
            return;
        } else {
            Iterator<Entry<String, AuthenticationSession>> it = sessionCache.entrySet().iterator();
            while (it.hasNext()) {
                it.next().getValue().closeSession();
            }
        }

        _rsaSessionCacheLookup.remove(tenantName.toLowerCase());
        logger.debug("RSA session cache removed");
    }

    public AuthenticationSession getSession(String tenantName, String cachedSessionId) {
        ConcurrentHashMap<String, AuthenticationSession> sessionCache = _rsaSessionCacheLookup.get(tenantName.toLowerCase());

        if (sessionCache == null) {
            return null;
        }
        return sessionCache.get(cachedSessionId);
    }

    public void addSession(String tenantName, String sessionId, AuthenticationSession session) {
        String normalizedTenant = tenantName.toLowerCase();
        ConcurrentHashMap<String, AuthenticationSession> sessionCache = _rsaSessionCacheLookup.get(normalizedTenant);

        if (sessionCache == null) {
            sessionCache = new ConcurrentHashMap<String, AuthenticationSession>();
            _rsaSessionCacheLookup.put(normalizedTenant, sessionCache);
            logger.debug("Added RSA session cache for : "+tenantName);
        }
        sessionCache.put(sessionId, session);

        logger.debug("Added RSA session to cache. sessionID: "+sessionId);
    }

    public AuthenticationSession removeSession(String tenantName, String cachedSessionId) {
        ConcurrentHashMap<String, AuthenticationSession> sessionCache = _rsaSessionCacheLookup.get(tenantName.toLowerCase());

        if (sessionCache == null) {
            return null;
        }

        logger.debug("Removed cached RSA session. sessionID: "+cachedSessionId);

        return sessionCache.remove(cachedSessionId);
    }
}
