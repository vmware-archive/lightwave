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

import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.joda.time.DateTime;
import org.opensaml.common.SAMLVersion;
import org.opensaml.saml2.core.LogoutRequest;
import org.opensaml.saml2.core.LogoutResponse;
import org.opensaml.saml2.core.NameID;
import org.opensaml.saml2.core.SessionIndex;
import org.opensaml.saml2.core.Status;
import org.opensaml.saml2.core.StatusCode;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.samlservice.IdmAccessor;
import com.vmware.identity.samlservice.LogoutState;
import com.vmware.identity.samlservice.OasisNames;
import com.vmware.identity.samlservice.SamlValidator;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.session.Session;
import com.vmware.identity.session.SessionManager;

/**
 * Validate Logout request or response message
 *
 */
public class LogoutStateValidator implements SamlValidator<LogoutState> {
    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(LogoutStateValidator.class);

    @Override
    public com.vmware.identity.samlservice.SamlValidator.ValidationResult validate(
            LogoutState t) {
        log.debug("Validating request {}", t);

        ValidationResult vr = null;

        try {
            Validate.notNull(t);

            HttpServletRequest httpRequest = t.getRequest();
            Validate.notNull(httpRequest);

            IdmAccessor accessor = t.getIdmAccessor();
            Validate.notNull(accessor);
            Validate.notNull(accessor.getTenant());

            SessionManager sm = t.getSessionManager();
            Validate.notNull(sm);

            LogoutRequest request = t.getLogoutRequest();
            LogoutResponse response = t.getLogoutResponse();
            Validate.isTrue(request != null || response != null);
            if (request != null) {
                vr = validateLogoutRequest(vr, accessor, request);
            } else {
                vr = validateLogoutResponse(vr, accessor, response, sm);
            }

        } catch (Exception e) {
            vr = new ValidationResult(HttpServletResponse.SC_BAD_REQUEST,
                    "BadRequest", null);
            log.debug("Caught exception while Validating {} returning 400",
                    e.toString());
        }
        return vr;
    }

    /**
     * Validate LogoutResponse
     *
     * @param vr
     * @param accessor
     * @param response
     * @return
     */
    private com.vmware.identity.samlservice.SamlValidator.ValidationResult validateLogoutResponse(
            com.vmware.identity.samlservice.SamlValidator.ValidationResult vr,
            IdmAccessor accessor, LogoutResponse response, SessionManager sm) {
        Validate.notNull(response.getIssuer());

        // Validate single logout service first, if that is valid, we can send
        // SAML replies
        try {
            @SuppressWarnings("unused")
            String acsUrl = accessor.getSloForRelyingParty(response.getIssuer()
                    .getValue(), OasisNames.HTTP_REDIRECT);
        } catch (IllegalStateException e) {
            // set validation result to 400
            log.debug("Caught illegal state exception while Validating "
                    + e.toString() + ", returning 400");
            vr = new ValidationResult(HttpServletResponse.SC_BAD_REQUEST,
                    e.getMessage(), null);
        }

        // Validate ID
        if (vr == null && response.getID() == null) {
            vr = new ValidationResult(OasisNames.REQUESTER);
            log.debug("Validation FAILED - Request ID is missing");
        }

        // Validate version
        if (vr == null) {
            SAMLVersion version = response.getVersion();
            if ((version.getMajorVersion() > Shared.REQUIRED_SAML_VERSION
                    .getMajorVersion())
                    || version.getMajorVersion() == Shared.REQUIRED_SAML_VERSION
                            .getMajorVersion()
                    && version.getMinorVersion() > Shared.REQUIRED_SAML_VERSION
                            .getMinorVersion()) {
                // version too high
                vr = new ValidationResult(OasisNames.VERSION_MISMATCH,
                        OasisNames.REQUEST_VERSION_TOO_HIGH);
                log.debug("Validation FAILED - Version is too high");
            } else if ((version.getMajorVersion() < Shared.REQUIRED_SAML_VERSION
                    .getMajorVersion())
                    || version.getMajorVersion() == Shared.REQUIRED_SAML_VERSION
                            .getMajorVersion()
                    && version.getMinorVersion() < Shared.REQUIRED_SAML_VERSION
                            .getMinorVersion()) {
                // version too low
                vr = new ValidationResult(OasisNames.VERSION_MISMATCH,
                        OasisNames.REQUEST_VERSION_TOO_LOW);
                log.debug("Validation FAILED - Version is too low");
            }
        }

        // Validate IssueInstant
        if (vr == null) {
            DateTime dtPlus = response.getIssueInstant();
            DateTime dtMinus = response.getIssueInstant();
            DateTime instant = new DateTime();
            long clockTolerance = accessor.getClockTolerance();
            if (dtPlus == null) {
                vr = new ValidationResult(OasisNames.REQUESTER);
                log.debug("Validation FAILED - Issue Instant is missing");
            } else {
                dtPlus = dtPlus.plus(clockTolerance);
                dtMinus = dtMinus.minus(clockTolerance);
                // dtPlus must be after now and dtMinus must be before now
                // in order to satisfy clock tolerance
                if (dtPlus.isBefore(instant) || dtMinus.isAfter(instant)) {
                    vr = new ValidationResult(OasisNames.REQUESTER);
                    log.debug("Validation FAILED - Issue Instant outside of clock tolerance");
                    log.debug("clockTolerance {} ", clockTolerance);
                    log.debug("now {}", instant);
                    log.debug("dtPlus {}", dtPlus.toString());
                    log.debug("dtMinus {}", dtMinus.toString());
                }
            }
        }

        // Destination URL skipped, this is already done by OpenSAML when
        // parsing

        // validate inResponseTo (which is the corresponding SLO request ID that
        // this response is targetting at)
        if (vr == null) {
            String inResponseTo = response.getInResponseTo();
            if (inResponseTo == null) {
                vr = new ValidationResult(OasisNames.REQUESTER);
                log.debug("Validation FAILED - inResponseTo is missing");
            } else {
                // try to find a session by LogoutRequest id that we have
                Session session = sm.getByLogoutRequestId(inResponseTo);
                if (session == null) {
                    // No session found using the SLO request ID. This could
                    // happen due to
                    // fail-over (node switch). So here we ignore rather than
                    // throw error at browser
                    log.info("Unable to identify a session the SLO response is referring to. This could be caused by site-affinity switch.");
                }
            }
        }

        // check response status code
        if (vr == null) {
            Status status = null;
            StatusCode statusCode = null;
            if (vr == null) {
                // check LogoutResponse status code here
                status = response.getStatus();
                if (status == null) {
                    vr = new ValidationResult(OasisNames.REQUESTER);
                    log.debug("Validation FAILED - unable to find status code");
                }
            }
            if (vr == null) {
                statusCode = status.getStatusCode();
                if (statusCode == null) {
                    vr = new ValidationResult(OasisNames.REQUESTER);
                    log.debug("Validation FAILED - unable to find status code");
                }
            }
            if (vr == null) {
                String code = statusCode.getValue();
                if (!OasisNames.SUCCESS.equals(code)) {
                    vr = new ValidationResult(OasisNames.SUCCESS,
                            OasisNames.PARTIAL_LOGOUT);
                    log.debug("Validation FAILED - partially logged out session");
                }
            }
        }

        // validation done
        if (vr == null) {
            vr = new ValidationResult(); // success
        }
        return vr;
    }

    /**
     * Validate LogoutRequest
     *
     * @param vr
     * @param accessor
     * @param request
     * @return
     */
    private ValidationResult validateLogoutRequest(ValidationResult vr,
            IdmAccessor accessor, LogoutRequest request) {
        Validate.notNull(request.getIssuer());

        // Validate single logout service first, if that is valid, we can send
        // SAML replies
        try {
            @SuppressWarnings("unused")
            String acsUrl = accessor.getSloForRelyingParty(request.getIssuer()
                    .getValue(), OasisNames.HTTP_REDIRECT);
        } catch (IllegalStateException e) {
            // set validation result to 400
            log.debug("Caught illegal state exception while Validating "
                    + e.toString() + ", returning 400");
            vr = new ValidationResult(HttpServletResponse.SC_BAD_REQUEST,
                    e.getMessage(), null);
        }

        // Validate ID
        if (vr == null && request.getID() == null) {
            vr = new ValidationResult(OasisNames.REQUESTER);
            log.debug("Validation FAILED - Request ID is missing");
        }

        // Validate version
        if (vr == null) {
            SAMLVersion version = request.getVersion();
            if ((version.getMajorVersion() > Shared.REQUIRED_SAML_VERSION
                    .getMajorVersion())
                    || version.getMajorVersion() == Shared.REQUIRED_SAML_VERSION
                            .getMajorVersion()
                    && version.getMinorVersion() > Shared.REQUIRED_SAML_VERSION
                            .getMinorVersion()) {
                // version too high
                vr = new ValidationResult(OasisNames.VERSION_MISMATCH,
                        OasisNames.REQUEST_VERSION_TOO_HIGH);
                log.debug("Validation FAILED - Version is too high");
            } else if ((version.getMajorVersion() < Shared.REQUIRED_SAML_VERSION
                    .getMajorVersion())
                    || version.getMajorVersion() == Shared.REQUIRED_SAML_VERSION
                            .getMajorVersion()
                    && version.getMinorVersion() < Shared.REQUIRED_SAML_VERSION
                            .getMinorVersion()) {
                // version too low
                vr = new ValidationResult(OasisNames.VERSION_MISMATCH,
                        OasisNames.REQUEST_VERSION_TOO_LOW);
                log.debug("Validation FAILED - Version is too low");
            }
        }

        // Validate IssueInstant
        if (vr == null) {
            DateTime dtPlus = request.getIssueInstant();
            DateTime dtMinus = request.getIssueInstant();
            DateTime instant = new DateTime();
            long clockTolerance = accessor.getClockTolerance();
            if (dtPlus == null) {
                vr = new ValidationResult(OasisNames.REQUESTER);
                log.debug("Validation FAILED - Issue Instant is missing");
            } else {
                dtPlus = dtPlus.plus(clockTolerance);
                dtMinus = dtMinus.minus(clockTolerance);
                // dtPlus must be after now and dtMinus must be before now
                // in order to satisfy clock tolerance
                if (dtPlus.isBefore(instant) || dtMinus.isAfter(instant)) {
                    vr = new ValidationResult(OasisNames.REQUESTER);
                    log.debug("Validation FAILED - Issue Instant outside of clock tolerance");
                    log.debug("clockTolerance {}", clockTolerance);
                    log.debug("now {}", instant);
                    log.debug("dtPlus {}", dtPlus.toString());
                    log.debug("dtMinus {}", dtMinus.toString());
                }
            }
        }

        // Destination URL skipped, this is already done by OpenSAML when
        // parsing

        // Validate NotOnOrAfter
        if (vr == null) {
            DateTime notOnOrAfter = request.getNotOnOrAfter();
            if (notOnOrAfter != null) {
                DateTime instant = new DateTime();
                if (!instant.isBefore(notOnOrAfter)) {
                    vr = new ValidationResult(OasisNames.REQUESTER,
                            OasisNames.REQUEST_DENIED);
                    log.debug("Validation FAILED - NotOnOrAfter condition violated");
                    log.debug("now {}", instant);
                    log.debug("notOnOrAfter {}", notOnOrAfter.toString());
                }
            }
        }

        // validate NameID
        if (vr == null) {
            NameID nameID = request.getNameID();
            if (nameID == null || nameID.getFormat() == null
                    || nameID.getValue() == null) {
                log.debug("Validation FAILED for NameID: node, format or value missing");
                vr = new ValidationResult(OasisNames.REQUESTER);
            }
        }

        // validate session index
        if (vr == null) {
            List<SessionIndex> sessionList = request.getSessionIndexes();
            if (sessionList == null || sessionList.size() == 0) {
                log.debug("Validation FAILED for session indices: at least one session index is required");
                vr = new ValidationResult(OasisNames.REQUESTER);
            }
        }

        // validation done
        if (vr == null) {
            vr = new ValidationResult(); // success
        }
        return vr;
    }

}
