package com.vmware.identity.idm.server;

import java.util.Iterator;
import java.util.Map.Entry;
import java.util.concurrent.ConcurrentHashMap;

import org.apache.commons.lang.Validate;

import com.rsa.authagent.authapi.AuthAgentException;
import com.rsa.authagent.authapi.AuthSession;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;


public class RsaAuthSessionCache {

    private ConcurrentHashMap<String, ConcurrentHashMap<String, AuthSession>> _rsaSessionCacheLookup; // lookup table for AuthSession cache keyed to tenant name.
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(RsaAuthSessionCache.class);

    RsaAuthSessionCache() {
        _rsaSessionCacheLookup = new ConcurrentHashMap<String, ConcurrentHashMap<String, AuthSession>>();
    }

    /**
     * Removes RSA session cache for the given tenant
     * @param tenantName
     * @throws AuthAgentException
     */
    public void removeSessionCache(String tenantName) throws AuthAgentException {
        Validate.notEmpty(tenantName);

        //close all sessions in the session cache
        logger.debug("Removing RSA session cache ... tenant: "+tenantName);

        ConcurrentHashMap<String, AuthSession> sessionCache = _rsaSessionCacheLookup.get(tenantName.toLowerCase());

        if (sessionCache == null) {
            logger.debug("No RSA session cache found ... tenant: "+tenantName);
            return;
        } else {
            Iterator<Entry<String, AuthSession>> it = sessionCache.entrySet().iterator();
            while (it.hasNext()) {
                it.next().getValue().close();
            }
        }

        _rsaSessionCacheLookup.remove(tenantName.toLowerCase());
        logger.debug("RSA session cache removed");
    }

    public AuthSession getSession(String tenantName, String cachedSessionId) {
        ConcurrentHashMap<String, AuthSession> sessionCache = _rsaSessionCacheLookup.get(tenantName.toLowerCase());

        if (sessionCache == null) {
            return null;
        }
        return sessionCache.get(cachedSessionId);
    }

    public void addSession(String tenantName, String sessionId, AuthSession session) {
        String normalizedTenant = tenantName.toLowerCase();
        ConcurrentHashMap<String, AuthSession> sessionCache = _rsaSessionCacheLookup.get(normalizedTenant);

        if (sessionCache == null) {
            sessionCache = new ConcurrentHashMap<String, AuthSession>();
            _rsaSessionCacheLookup.put(normalizedTenant, sessionCache);
            logger.debug("Added RSA session cache for : "+tenantName);
        }
        sessionCache.put(sessionId, session);

        logger.debug("Added RSA session to cache. sessionID: "+sessionId);
    }

    public AuthSession removeSession(String tenantName, String cachedSessionId) {
        ConcurrentHashMap<String, AuthSession> sessionCache = _rsaSessionCacheLookup.get(tenantName.toLowerCase());

        if (sessionCache == null) {
            return null;
        }

        logger.debug("Removed cached RSA session. sessionID: "+cachedSessionId);

        return sessionCache.remove(cachedSessionId);
    }
}
