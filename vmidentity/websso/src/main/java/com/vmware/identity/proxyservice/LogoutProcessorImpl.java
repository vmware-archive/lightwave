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

package com.vmware.identity.proxyservice;

import java.io.IOException;
import java.security.NoSuchAlgorithmException;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Map.Entry;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.commons.lang.Validate;
import org.joda.time.DateTime;
import org.opensaml.common.SignableSAMLObject;
import org.opensaml.saml2.core.LogoutResponse;
import org.opensaml.saml2.core.LogoutRequest;
import org.opensaml.saml2.core.Response;
import org.opensaml.ws.message.decoder.MessageDecodingException;
import org.opensaml.xml.ConfigurationException;
import org.opensaml.xml.io.UnmarshallingException;
import org.opensaml.xml.security.SecurityException;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.MessageSource;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.samlservice.AuthnRequestState;
import com.vmware.identity.samlservice.LogoutState;
import com.vmware.identity.samlservice.OasisNames;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.samlservice.LogoutState.ProcessingState;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.impl.SamlServiceImpl;
import com.vmware.identity.session.Session;
import com.vmware.identity.session.SessionManager;
import com.vmware.identity.websso.client.LogoutProcessor;
import com.vmware.identity.websso.client.Message;
import com.vmware.identity.websso.client.SamlUtils;
import com.vmware.identity.websso.client.SharedUtils;

/**
 * Proxying service Logout Processor Implementation
 *
 */
public class LogoutProcessorImpl implements LogoutProcessor {

    @Autowired
    private MessageSource messageSource;

    @Autowired
    private SessionManager sessionManager;

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
            .getLogger(LogoutProcessorImpl.class);

    private static final int THRESHHOLD_HOURS_FOR_MAP_CHECK = 1;
    private static final int THRESHHOLD_SIZE_FOR_MAP_CHECK = 5000;

    // When using nested synchronization, please doing so with same order to
    // avoid deadlock. The order is sync outgoingReqToIncomingReqMap first, followed by logoutStateMap.
    private final Map<String, LogoutState> logoutStateMap = Collections.synchronizedMap(new LinkedHashMap<String, LogoutState>());
    private final Map<String, String> outgoingReqToIncomingReqMap = Collections.synchronizedMap(new LinkedHashMap<String, String>());

    /**
     * Error callback in processing logout response or request from external
     * IDP.
     *
     * @see com.vmware.identity.websso.client.LogoutProcessor#logoutError(com.vmware.identity.websso.client.Message,
     *      javax.servlet.http.HttpServletResponse)
     */
    @Override
    public void logoutError(Message arg0, HttpServletRequest request,
            HttpServletResponse response) {
        Validate.notNull(arg0, "Message");

        com.vmware.identity.websso.client.ValidationResult clientVr = arg0
                .getValidationResult();
        ValidationResult vr = null;

        String message;

        LogoutState loState = this.findOriginalRequstState(request);
        Locale locale = (loState == null) ? null : loState.getLocale();

        // The error could be from external idp or from validation of response
        // from external idp
        if (arg0.getStatus() != null
                && !arg0.getStatus().equals(OasisNames.SUCCESS)) {
            // If it is external IDP authentication error.
            vr = new ValidationResult(HttpServletResponse.SC_BAD_REQUEST,
                    arg0.getStatus(), arg0.getSubstatus());

            message = vr.getMessage(messageSource, locale);
            logger.error(
                    "Error logout message from Externel IDP: {} message: {}",
                    vr.getResponseCode(), message);

        } else {
            // Else send the validation error to RP
            vr = new ValidationResult(clientVr.getResponseCode(),
                    clientVr.getStatus(), clientVr.getSubstatus());
            message = vr.getMessage(messageSource, locale);
            logger.error(
                    "Failed in validating logout response from Externel IDP: {} message: {}",
                    vr.getResponseCode(), message);
        }

        try {
            response.sendError(vr.getResponseCode(), message);
        } catch (IOException e) {
            logger.error("Caught IOException in sending error to logout request/response issurer");
        }
    }

    /**
     * Callback function at receiving a logout request from the trusted IDP.
     * This function will find corresponding lotus login session, send a slo
     * request to each of the session participants. The client library
     * sloconstroller will respond to slo requester when get back from this
     * function.
     *
     * @param message
     *            Websso message
     * @param request
     * @param response
     */
    @Override
    public void logoutRequest(Message arg0, HttpServletRequest request,
            HttpServletResponse response, String tenant) {
        Validate.notEmpty(tenant);
        // This is used only for lotus joining extern IDP federation (with more
        // than one SP). Saas used case.
        // incoming logout request from the IDP, need to clean up session info
        // on the server
        //

        // 1 create a new RequestState
        LogoutState loState = new LogoutState(request, response,
                sessionManager, null, null);

        // 2 setup validation state since this is already validated by client
        // lib
        ValidationResult vr = new ValidationResult(arg0.getValidationResult()
                .getResponseCode(), arg0.getStatus(), arg0.getSubstatus());
        loState.setValidationResult(vr);

        loState.setProcessingState(ProcessingState.PARSED);
        loState.getIdmAccessor().setTenant(tenant);

        // 4 Find lotus session id using ext IDP session id. Remove castle
        // session cookie.
        Session lotusSession = findSessionIdByExtSessionId(arg0
                .getSessionIndex());
        if (lotusSession == null) {
            // Ignore the request and log a warning.
            logger.warn("No session found matching external session: {}",
                    arg0.getSessionIndex());
            return;
        }
        // Now, we found the lotus session. Get loState populated with session
        // related infor.
        loState.setSessionId(lotusSession.getId());
        loState.setIssuerValue(lotusSession.getExtIDPUsed().getEntityID());
        loState.removeResponseHeaders();

        // 5 Send SLO request to each of the session participants.
        try {
            SamlServiceImpl.sendSLORequestsToOtherParticipants(tenant, loState);
        } catch (IOException e) {
            logger.error("Caught IOException in sending logout request to service providers.");
        }
    }

    /**
     *
     * @param sessionIndex
     *            Websso message
     * @return null or websso session string
     */
    private Session findSessionIdByExtSessionId(String extSessionIndex) {
        Validate.notNull(extSessionIndex, "extSessionIndex");
        Validate.notNull(sessionManager, "sessionManager");
        Collection<Session> sessions = sessionManager.getAll();

        Validate.notEmpty(sessions, "sessions");

        Session localSessionToLogout = null;

        for (Session localSession : sessions) {
            if (localSession.getExtIDPSessionID().equals(extSessionIndex))
                localSessionToLogout = localSession;
            else {
                logger.debug("Session matching hit miss: session= {}",
                        localSession.toString());
            }
        }

        if (localSessionToLogout != null) {
            logger.debug(
                    "Found a session requested in external IDP SLO request {}",
                    localSessionToLogout.toString());
            return localSessionToLogout;
        }
        return null;
    }

    /**
     * Callback function at receiving a validated successful logout response
     * from trusted IDP. It send logout success response to the service that
     * initiated the SLO request. And logout requests to other participants.
     *
     * @param message
     *            Websso message
     * @param request
     * @param response
     */
    @Override
    public void logoutSuccess(Message arg0, HttpServletRequest request,
            HttpServletResponse response) {

        LogoutState logoutState = this.findOriginalRequstState(request);
        if (logoutState == null) {
            logger.error("Unable to find LogoutState associated with original LO request from service provider. "
                    + "Logout at PSC is successful. But was unable to respond to participating service provider.");
            return;
        }

        String tenant = logoutState.getIdmAccessor().getTenant();

        // Send logout request to other participating SP's
        try {
            if (logoutState.needLogoutRequest()) {
                // send slo request to all non-initiating relying parties
                SamlServiceImpl.sendSLORequestsToOtherParticipants(tenant,
                        logoutState);
            }
        } catch (IOException e) {
            logger.error("Catch IOException in sending logout requests to other service providers.");
        }

        // Send logout response to other initiating SP
        try {
            if (logoutState.needLogoutResponse()) {
                String redirectUrl = SamlServiceImpl.buildResponseUrl(tenant,
                        logoutState);

                logger.info(
                        "Sending SAML logout response to SP. Redirect url is: {} ",
                        redirectUrl);

                if (redirectUrl != null) {
                    response.sendRedirect(redirectUrl);
                } else {
                    SamlServiceImpl.sendLogoutError(logoutState.getLocale(),
                            response, logoutState, messageSource);
                }
            }
        } catch (IOException e) {
            logger.error("Caught IOException in sending logout response to service provider.");
        }
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.vmware.identity.websso.client.LogoutProcessor#internalError(java.
     * lang.Exception, javax.servlet.http.HttpServletRequest)
     */
    @Override
    public void internalError(Exception internalError,
            HttpServletRequest request, HttpServletResponse response) {
        logger.error("internalError() is called in processing logout request/response from "
                + "external IDP. This means the request or response is understood and validated. "
                + "But responder can send the response or request to the IDP because of unexpected error.");

        LogoutState logoutState = this.findOriginalRequstState(request);
        try {
            String message;
            if (logoutState != null) {
                // Logout request was initiated by one of SP register to this
                // instance.
                // Send error response to the initiating SP

                message = logoutState.getMessageSource().getMessage(
                        "BadRequest", null, logoutState.getLocale());

            } else {
                message = "BadRequest";
            }
            response.sendError(HttpServletResponse.SC_INTERNAL_SERVER_ERROR,
                    message);

        } catch (IOException e) {
            logger.error("Caught IOException in sending processing error to service provider");
        }
    }

    /**
     * register LogoutState of the initial request.
     *
     * @param incomingReqID
     *            The request ID of LogoutState request came to PSC.
     * @param outGoingReqID
     *            The request ID of LogoutState request sending out to external
     *            IDP.
     * @param requestState
     *            LogoutState object for the logout request came to PSC.
     * @return void
     */
    public void registerRequestState(String incomingReqID,
            String outGoingReqID, LogoutState logoutState) {
        Validate.notEmpty(incomingReqID, "incomingReqID");
        Validate.notEmpty(outGoingReqID, "outGoingReqID");
        if (logoutState == null) {
            logger.warn("Skip null logoutState registration.");
            return;
        }
        Date noOlderThanDate = new Date(System.currentTimeMillis()
                - THRESHHOLD_HOURS_FOR_MAP_CHECK * 60 * 60 * 1000);
        this.cleanStaledLogout(noOlderThanDate);
        synchronized (this.outgoingReqToIncomingReqMap) {
            synchronized (this.logoutStateMap) {
                this.logoutStateMap.put(incomingReqID, logoutState);
                this.outgoingReqToIncomingReqMap.put(outGoingReqID, incomingReqID);
            }
        }
    }

    private void cleanStaledLogout(Date noOlderThanDate) {
        if (this.logoutStateMap.size() < THRESHHOLD_SIZE_FOR_MAP_CHECK) {
            return;
        }

        DateTime noOlderThanDateTime = new DateTime(
                noOlderThanDate.getTime());

        synchronized (this.outgoingReqToIncomingReqMap) {
            synchronized (this.logoutStateMap) {
                Iterator<Entry<String, String>> it = this.outgoingReqToIncomingReqMap
                        .entrySet().iterator();

                while (it.hasNext()) {
                    Entry<String, String> idMapEntry = it.next();
                    String inReqID = idMapEntry.getValue();
                    LogoutState state = this.logoutStateMap.get(inReqID);
                    LogoutRequest loRequest = state.getLogoutRequest();
                    if (loRequest == null
                            || loRequest.getIssueInstant().isBefore(
                                    noOlderThanDateTime)) {
                        this.logoutStateMap.remove(inReqID);
                        this.outgoingReqToIncomingReqMap
                                .remove(idMapEntry.getKey());
                    }
                }
            }
        }

    }

    /**
     * Helper function to decode HttpServletRequest to LogoutResponse
     *
     * @param request
     * @return
     * @throws Exception
     */
    private LogoutResponse decodeResponse(HttpServletRequest request)
            throws Exception {
        Validate.notNull(request, "request");

        String samlResponseStr = request
                .getParameter(SamlUtils.SAML_RESPONSE_PARAMETER);

        Validate.notEmpty(samlResponseStr, "Empty SSO response string");
        String decodedResponseStr = SamlUtils.extractResponse(samlResponseStr);
        Validate.notNull(decodedResponseStr, "decodedResponseStr");
        Document responseDoc = SharedUtils.createDOM(decodedResponseStr);

        LogoutResponse samlResponse = (LogoutResponse) SamlUtils
                .unmarshallSAMLObj(responseDoc);

        return samlResponse;
    }

    /**
     * This function find a match of SAML Authn Response received to an
     * AuthnRequestState for the original authentication request from relying
     * party. It also removes the AuthnRequestState and associated map entries.
     *
     * @param response
     *            HttpServletRequest that contain SAMLResponse
     * @return AuthnRequestState return null if unable to decode the response,
     *         or there is no inResponseTo attribute, or there was no match
     */
    private LogoutState findOriginalRequstState(HttpServletRequest request) {

        LogoutResponse samlResponse;
        LogoutState logoutState = null;

        try {
            samlResponse = decodeResponse(request);
        } catch (Exception e) {
            return null;
        }

        if (null == samlResponse) {
            return null;
        }

        String outReqID = samlResponse.getInResponseTo();

        if (null == outReqID) {
            // could be idp initiated request.
            return null;
        }
        synchronized (this.outgoingReqToIncomingReqMap) {
            synchronized (this.logoutStateMap) {
                String incomingReqID = this.outgoingReqToIncomingReqMap
                        .remove(outReqID);

                if (null == incomingReqID) {
                    logger.debug("No source authentication request was matched to the outgoing request id "
                            + outReqID);
                    return null;
                }
                logger.info("Removing request state for request id " + incomingReqID);
                logoutState = this.logoutStateMap.remove(incomingReqID);
            }
        }
        return logoutState;
    }

}
