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
package com.vmware.identity.samlservice.impl;

import java.security.NoSuchAlgorithmException;
import java.util.List;

import javax.servlet.http.HttpServletRequest;

import org.apache.commons.lang.Validate;
import org.opensaml.common.impl.SecureRandomIdentifierGenerator;
import org.opensaml.saml2.core.LogoutRequest;
import org.opensaml.saml2.core.LogoutResponse;
import org.opensaml.saml2.core.SessionIndex;
import org.springframework.beans.factory.annotation.Autowired;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.proxyservice.LogoutProcessorImpl;
import com.vmware.identity.samlservice.IdmAccessor;
import com.vmware.identity.samlservice.LogoutState;
import com.vmware.identity.samlservice.OasisNames;
import com.vmware.identity.samlservice.ProcessingFilter;
import com.vmware.identity.samlservice.SamlServiceException;
import com.vmware.identity.session.LogoutRequestData;
import com.vmware.identity.session.Session;
import com.vmware.identity.session.SessionManager;
import com.vmware.identity.websso.client.IDPConfiguration;
import com.vmware.identity.websso.client.MetadataSettings;
import com.vmware.identity.websso.client.SamlUtils;
import com.vmware.identity.websso.client.SloRequestSettings;
import com.vmware.identity.websso.client.endpoint.SloRequestSender;

/**
 * LogoutRequest/Response processing.
 *
 * We talk to SessionManager here.
 *
 */
public class LogoutStateProcessingFilter implements
        ProcessingFilter<LogoutState> {
    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(LogoutStateProcessingFilter.class);

    @Autowired
    private MetadataSettings metadataSettings;

    @Autowired
    private SloRequestSender sloRequestSender;

    @Override
    public void preProcess(LogoutState t) throws SamlServiceException {
    }

    private static SecureRandomIdentifierGenerator generator;

    static {
        try {
            generator = new SecureRandomIdentifierGenerator();
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Unexpected error in creating SecureRandomIdentifierGenerator", e);
        }
    }

    /**
     * Validate logout request/response session id, cache in LogoutState. In
     * case of a request for logging out an external authenticated session,
     * issue composed a lotus initiated logout request to the IDP.
     */
    @Override
    public void process(LogoutState t) throws SamlServiceException {
        log.debug("LogoutStateProcessingFilter.process is called");

        Validate.notNull(t);
        IdmAccessor accessor = t.getIdmAccessor();
        Validate.notNull(accessor);
        HttpServletRequest request = t.getRequest();
        Validate.notNull(request);
        LogoutRequest logoutRequest = t.getLogoutRequest();
        LogoutResponse logoutResponse = t.getLogoutResponse();
        Validate.isTrue(logoutRequest != null || logoutResponse != null);
        SessionManager sessionManager = t.getSessionManager();
        Validate.notNull(sessionManager);

        String sessionId = null; // server side session id

        if (logoutRequest != null) {
            sessionId = processLogoutRequest(t, logoutRequest, sessionManager);
        } else {
            sessionId = processLogoutResponse(logoutResponse, sessionManager);
        }

        Validate.notNull(sessionId, "sessionId");
        t.setSessionId(sessionId);

        // Remove Castle session cookie
        t.removeResponseHeaders();
        // send the logout request to external IDP
        if (logoutRequest != null
                && sessionManager.get(sessionId).isUsingExtIDP()) {
            sendRequestToIDP(t);
        }
    }

    // Consruct and send a logout request to exteral IDP. Should be called only
    // when
    // the target session is authenticated with external idp
    private void sendRequestToIDP(LogoutState t) throws SamlServiceException {
        log.debug("LogoutStateProcessingFilter.sendRequestToIDP is called");
        try {
            String sessionId = t.getSessionId();
            Validate.notEmpty(sessionId, "sessionId");
            SessionManager sessionManager = t.getSessionManager();
            Validate.notNull(sessionManager, "sessionManager");

            Session session = sessionManager.get(sessionId);

            Validate.isTrue(session.isUsingExtIDP(),
                    "Not expected to call sendRequestToIDP!");

            IDPConfig extIDPConfig = session.getExtIDPUsed();
            Validate.notNull(extIDPConfig, "extIDPConfig");
            String spAlias = t.getIdmAccessor().getTenant();
            Validate.notEmpty(spAlias, "spAlias");

            if (null == metadataSettings
                    .getIDPConfigurationByEntityID(extIDPConfig.getEntityID())
                    || null == metadataSettings.getSPConfiguration(spAlias)) {
                SamlServiceImpl.initMetadataSettings(metadataSettings,
                        extIDPConfig, t.getIdmAccessor());
            }

            IDPConfiguration extIDPConfiguration = metadataSettings
                    .getIDPConfigurationByEntityID(extIDPConfig.getEntityID());
            Validate.notNull(extIDPConfiguration, "extIDPConfiguration");

            String idpAlias = extIDPConfiguration.getAlias();
            Validate.notEmpty(idpAlias, "idpAlias");

            /*
             * Skip if the slo request was sent from external idp itself. This
             * could happen this lotus instance is one of multiple SP
             * participated in the external IDP session.
             */
            if (extIDPConfiguration.getEntityID().equals(t.getIssuerValue()))
                return;

            SloRequestSettings extSLORequestSettings = new SloRequestSettings(
                    spAlias, idpAlias, true, session.getPrincipalId().getUPN(), // subject
                    OasisNames.IDENTITY_FORMAT_UPN,
                    session.getExtIDPSessionID(), null);

            LogoutProcessorImpl logoutProcessorImpl = (LogoutProcessorImpl) getSloRequestSender()
                    .getLogoutProcessor();

            String outGoingReqID = generator.generateIdentifier();
            String redirectUrl = getSloRequestSender().getRequestUrl(
                    extSLORequestSettings, outGoingReqID);
            logoutProcessorImpl.registerRequestState(t.getLogoutRequest()
                    .getID(), outGoingReqID, t);

            if (SamlUtils.isIdpSupportSLO(metadataSettings,
                    extSLORequestSettings)) {
                Validate.notEmpty(redirectUrl, "redirectUrl");
                t.getResponse().sendRedirect(redirectUrl);
            } else {
                log.warn(String
                        .format("SLO end point does not exist for external IDP: %s, SLO request is not sent.",
                                extSLORequestSettings.getIDPAlias()));
            }
        } catch (Exception e) {
            // failed to authenticate with via proxying.
            log.debug(
                    "Caught exception in proxying logout request to external IDP: {}",
                    e.getMessage());
            throw new SamlServiceException(
                    "Caught error in proxying logout request to external IDP:",
                    e);
        }

    }

    /**
     * Process LogoutResponse
     *
     * @param logoutResponse
     * @param sessionManager
     * @return
     */
    private String processLogoutResponse(LogoutResponse response,
            SessionManager sm) throws SamlServiceException {
        String sessionId = null;

        // get the session object
        String inResponseTo = response.getInResponseTo();
        Validate.notNull(inResponseTo);

        Session session = sm.getByLogoutRequestId(inResponseTo);
        Validate.notNull(session);

        try {
            session.getLock().lock();
            LogoutRequestData logoutRequestData = session
                    .getLogoutRequestData();
            if (logoutRequestData == null) {
                throw new SamlServiceException(
                        "Unable to find logout request data");
            }

            String currentRelyingParty = logoutRequestData.getCurrent();
            if (currentRelyingParty == null
                    || !session
                            .containsSessionParticipantUrl(currentRelyingParty)) {
                throw new SamlServiceException(
                        "Unable to find current relying party");
            }

            // log off current relying party, the rest will be done in
            // LogoutState methods
            session.removeSessionParticipantByUrl(currentRelyingParty);

            sessionId = session.getId();

        } finally {
            session.getLock().unlock();
        }

        return sessionId;
    }

    /**
     * Process LogoutRequest return the session found or throw
     * SamlServiceException
     *
     * @param t
     * @param logoutRequest
     * @param sessionManager
     * @return session id found
     * @throws SamlServiceException
     */
    private String processLogoutRequest(LogoutState t,
            LogoutRequest logoutRequest, SessionManager sessionManager)
            throws SamlServiceException {
        String sessionId = null;

        // read session indexes
        List<SessionIndex> sessionList = logoutRequest.getSessionIndexes();
        Validate.notNull(sessionList, "sessionList");
        Validate.isTrue(sessionList.size() > 0);

        /*
         * Process each session. Every session index must refer to 1) session
         * that exists 2) session that is open for the calling relying party 3)
         * session that belongs to the same server session (really, there
         * shouldn't be a case where there is more than one session index for
         * the same relying party, but we do allow that in the code below as a
         * corner case)
         *
         * If all conditions above are satisfied, then every relying party
         * session will be terminated when server session is closed down.
         */
        for (SessionIndex sessionIndex : sessionList) {
            Validate.notNull(sessionIndex);
            String participantSession = sessionIndex.getSessionIndex();
            Validate.notNull(participantSession, "participantSession");

            Session session = sessionManager
                    .getByParticipant(participantSession);
            if (session == null) {
                // requester specified non-existent session
                throw new SamlServiceException("Server session not found");
            }
            if (!session.containsSessionParticipantUrl(t.getIssuerValue())) {
                throw new SamlServiceException(
                        "Issuer URL does not match session participant URL");
            }
            if (sessionId == null) {
                sessionId = session.getId();
            } else {
                // all participant sessions must belong to same server session
                if (!sessionId.equals(session.getId())) {
                    // we cannot log out multiple server sessions at once
                    throw new SamlServiceException(
                            "Participant requested multiple server sessions to be terminated");
                }
            }
        }
        return sessionId;
    }

    public SloRequestSender getSloRequestSender() {
        return sloRequestSender;
    }

    public void setSloRequestSender(SloRequestSender sloRequestSender) {
        this.sloRequestSender = sloRequestSender;
    }
}
