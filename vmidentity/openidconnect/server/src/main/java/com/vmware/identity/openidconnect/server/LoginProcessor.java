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

package com.vmware.identity.openidconnect.server;

import java.io.ByteArrayInputStream;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;

import org.apache.commons.lang3.StringUtils;
import org.apache.commons.lang3.tuple.Triple;
import org.springframework.context.MessageSource;

import com.vmware.identity.idm.GSSResult;
import com.vmware.identity.idm.IDMSecureIDNewPinException;
import com.vmware.identity.idm.RSAAMResult;
import com.vmware.identity.openidconnect.common.ErrorCode;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.StatusCode;
import com.vmware.identity.openidconnect.protocol.Base64Utils;
import com.vmware.identity.openidconnect.protocol.Header;
import com.vmware.identity.openidconnect.protocol.HttpRequest;
import com.vmware.identity.openidconnect.protocol.HttpResponse;

/**
 * @author Yehia Zayour
 */
public class LoginProcessor {
    private static final String REQUEST_LOGIN_PARAMETER = "CastleAuthorization";
    private static final String RESPONSE_AUTHZ_HEADER   = "CastleAuthorization";

    private final PersonUserAuthenticator personUserAuthenticator;
    private final SessionManager sessionManager;
    private final MessageSource messageSource;
    private final Locale locale;
    private final HttpRequest httpRequest;
    private final String tenant;

    public LoginProcessor(
            PersonUserAuthenticator personUserAuthenticator,
            SessionManager sessionManager,
            MessageSource messageSource,
            Locale locale,
            HttpRequest httpRequest,
            String tenant) {
        this.personUserAuthenticator = personUserAuthenticator;
        this.sessionManager = sessionManager;
        this.messageSource = messageSource;
        this.locale = locale;
        this.httpRequest = httpRequest;
        this.tenant = tenant;
    }

    public Triple<PersonUser, SessionID, LoginMethod> process() throws LoginException {
        String sessionIdString = this.httpRequest.getCookieValue(SessionManager.getSessionCookieName(this.tenant));
        if (sessionIdString != null) {
            SessionID sessionId = new SessionID(sessionIdString);
            SessionManager.Entry entry = this.sessionManager.get(sessionId);
            if (entry != null) {
                return Triple.of(entry.getPersonUser(), sessionId, (LoginMethod) null);
            }
        }

        String loginString = this.httpRequest.getParameters().get(REQUEST_LOGIN_PARAMETER);
        if (loginString == null) {
            return Triple.of((PersonUser) null, (SessionID) null, (LoginMethod) null);
        }

        String[] parts = loginString.split(" ");
        if (parts[0].isEmpty()) {
            throw new LoginException("invalid login string", localize(ErrorMessage.BAD_REQUEST));
        }

        LoginMethod loginMethod;
        try {
            loginMethod = LoginMethod.parse(parts[0]);
        } catch (ParseException e) {
            throw new LoginException("invalid login method", localize(ErrorMessage.BAD_REQUEST), e);
        }

        PersonUser personUser;
        switch (loginMethod) {
            case PASSWORD:
                personUser = processPasswordLogin(loginString);
                break;
            case PERSON_USER_CERTIFICATE:
                personUser = processPersonUserCertificateLogin();
                break;
            case GSS_TICKET:
                personUser = processGssTicketLogin(loginString);
                break;
            case SECURID:
                personUser = processSecurIDLogin(loginString);
                break;
            default:
                throw new IllegalStateException("unexpected login method: " + loginMethod);
        }

        return Triple.of(personUser, new SessionID(), loginMethod);
    }

    private PersonUser processPasswordLogin(String loginString) throws LoginException {
        // CastleAuthorization=Basic base64(username:password)
        String[] parts = loginString.split(" ");
        if (parts.length != 2) {
            throw new LoginException("malformed password login string", localize(ErrorMessage.BAD_REQUEST));
        }
        String unp = Base64Utils.decodeToString(parts[1]);

        int index = unp.indexOf(':');
        if (!(0 < index && index < unp.length() - 1)) {
            throw new LoginException("malformed username:password in login string", localize(ErrorMessage.BAD_REQUEST));
        }
        String username = unp.substring(0, index);
        String password = unp.substring(index + 1);

        try {
            return this.personUserAuthenticator.authenticateByPassword(this.tenant, username, password);
        } catch (InvalidCredentialsException e) {
            throw new LoginException("incorrect username or password", localize(ErrorMessage.UNAUTHORIZED), e);
        } catch (ServerException e) {
            throw new LoginException("error while authenticating username/password", localize(ErrorMessage.RESPONDER), e);
        }
    }

    private PersonUser processPersonUserCertificateLogin() throws LoginException {
        if (this.httpRequest.getCookieValue(SessionManager.getPersonUserCertificateLoggedOutCookieName(this.tenant)) != null) {
            throw new LoginException("already logged in once on this browser session", localize(ErrorMessage.LOGGED_OUT_TLS_SESSION));
        }

        List<X509Certificate> personUserCertificateChain;

        String certString64 = this.httpRequest.getHeaderValue("X-SSL-Client-Certificate");

        if (!StringUtils.isEmpty(certString64)) {
            byte[] certBytes = Base64Utils.decodeToBytes(certString64);
            ByteArrayInputStream inputStream = new ByteArrayInputStream(certBytes);
            try {
                CertificateFactory certFactory = CertificateFactory.getInstance("X.509");
                X509Certificate cert = (X509Certificate) certFactory.generateCertificate(inputStream);
                personUserCertificateChain = Arrays.asList(cert);
            } catch (CertificateException e) {
                throw new LoginException("failed to parse person user cert", localize(ErrorMessage.INVALID_CREDENTIAL), e);
            }
        } else {
            personUserCertificateChain = this.httpRequest.getClientCertificateChain();
            if (personUserCertificateChain == null || personUserCertificateChain.size() == 0) {
                throw new LoginException("missing person user cert", localize(ErrorMessage.NO_CLIENT_CERT));
            }
        }

        try {
            return this.personUserAuthenticator.authenticateByPersonUserCertificate(this.tenant, personUserCertificateChain);
        } catch (InvalidCredentialsException e) {
            throw new LoginException("invalid person user cert", localize(ErrorMessage.INVALID_CREDENTIAL), e);
        } catch (ServerException e) {
            throw new LoginException("error while authenticating person user cert", localize(ErrorMessage.RESPONDER), e);
        }
    }

    private PersonUser processGssTicketLogin(String loginString) throws LoginException {
        String contextId;
        String gssTicketString;
        boolean clientIntegrationPlugin = false; // Client Integration Plugin impl or built-in browser impl?

        String[] parts = loginString.split(" ");
        if (parts.length == 2) {
            String authzHeaderValue = this.httpRequest.getHeaderValue("Authorization");
            if (StringUtils.isEmpty(authzHeaderValue)) {
                Header serverLegHeader = new Header("WWW-Authenticate", LoginMethod.GSS_TICKET.getValue());
                throw new LoginException("authorization_header_needed", localize(ErrorMessage.UNAUTHORIZED), serverLegHeader);
            }
            contextId = parts[1];
            gssTicketString = authzHeaderValue.replaceFirst(LoginMethod.GSS_TICKET.getValue(), "").trim();
        } else if (parts.length == 3) {
            clientIntegrationPlugin = true;
            contextId = parts[1];
            gssTicketString = parts[2];
        } else {
            throw new LoginException("malformed gss login string", localize(ErrorMessage.BAD_REQUEST));
        }

        GSSResult result;
        try {
            result = this.personUserAuthenticator.authenticateByGssTicket(this.tenant, contextId, Base64Utils.decodeToBytes(gssTicketString));
        } catch (InvalidCredentialsException e) {
            throw new LoginException("invalid gss ticket", localize(ErrorMessage.UNAUTHORIZED), e);
        } catch (ServerException e) {
            throw new LoginException("error while doing gss authn", localize(ErrorMessage.RESPONDER), e);
        }

        if (result.complete()) {
            return new PersonUser(result.getPrincipalId(), this.tenant);
        } else {
            String serverLeg64 = Base64Utils.encodeToString(result.getServerLeg());
            Header serverLegHeader;
            if (clientIntegrationPlugin) {
                serverLegHeader = new Header(
                        RESPONSE_AUTHZ_HEADER,
                        String.format("%s %s %s", LoginMethod.GSS_TICKET.getValue(), contextId, serverLeg64));
            } else {
                serverLegHeader = new Header(
                        "WWW-Authenticate",
                        String.format("%s %s", LoginMethod.GSS_TICKET.getValue(), serverLeg64));
            }
            throw new LoginException("gss_continue_needed", localize(ErrorMessage.UNAUTHORIZED), serverLegHeader);
        }
    }

    private PersonUser processSecurIDLogin(String loginString) throws LoginException {
        // CastleAuthorization=RSAAM base64(username:passcode)
        // CastleAuthorization=RSAAM sessionId base64(username:passcode)
        String[] parts = loginString.split(" ");
        String sessionId;
        String unp;
        if (parts.length == 2) {
            sessionId = null;
            unp = Base64Utils.decodeToString(parts[1]);
        } else if (parts.length == 3) {
            sessionId = Base64Utils.decodeToString(parts[1]);
            unp = Base64Utils.decodeToString(parts[2]);
        } else {
            throw new LoginException("malformed securid login string", localize(ErrorMessage.BAD_REQUEST));
        }

        int index = unp.indexOf(':');
        if (!(0 < index && index < unp.length() - 1)) {
            throw new LoginException("malformed username:passcode in securid login string", localize(ErrorMessage.BAD_REQUEST));
        }
        String username = unp.substring(0, index);
        String passcode = unp.substring(index + 1);

        RSAAMResult result;
        try {
            result = this.personUserAuthenticator.authenticateBySecurID(this.tenant, username, passcode, sessionId);
        } catch (InvalidCredentialsException e) {
            throw new LoginException("incorrect securid username or passcode", localize(ErrorMessage.UNAUTHORIZED), e);
        } catch (IDMSecureIDNewPinException e) {
            throw new LoginException("new securid pin required", localize(ErrorMessage.SECURID_NEW_PIN_REQUIRED), e);
        } catch (ServerException e) {
            throw new LoginException("error while doing securid authn", localize(ErrorMessage.RESPONDER), e);
        }

        if (result.complete()) {
            return new PersonUser(result.getPrincipalId(), this.tenant);
        } else {
            String sessionId64 = Base64Utils.encodeToString(result.getRsaSessionID());
            Header serverLegHeader = new Header(
                    RESPONSE_AUTHZ_HEADER,
                    String.format("%s %s", LoginMethod.SECURID.getValue(), sessionId64));
            throw new LoginException("securid_next_code_required", localize(ErrorMessage.SECURID_NEXT_CODE), serverLegHeader);
        }
    }

    private String localize(ErrorMessage errorMessage) {
        return this.messageSource.getMessage(errorMessage.getValue(), (Object[]) null, this.locale);
    }

    public static class LoginException extends Exception {
        private static final String RESPONSE_ERROR_HEADER = "CastleError";
        private static final long serialVersionUID = 1L;

        private final ErrorObject errorObject;
        private final String localizedErrorMessage;
        private final Header serverLegHeader;

        private LoginException(String errorMessage, String localizedErrorMessage) {
            this(errorMessage, localizedErrorMessage, (Throwable) null, (Header) null);
        }

        private LoginException(String errorMessage, String localizedErrorMessage, Throwable cause) {
            this(errorMessage, localizedErrorMessage, cause, (Header) null);
        }

        private LoginException(String errorMessage, String localizedErrorMessage, Header serverLegHeader) {
            this(errorMessage, localizedErrorMessage, (Throwable) null, serverLegHeader);
        }

        private LoginException(String errorMessage, String localizedErrorMessage, Throwable cause, Header serverLegHeader) {
            super(cause);
            this.errorObject = new ErrorObject(ErrorCode.ACCESS_DENIED, errorMessage, StatusCode.UNAUTHORIZED);
            this.localizedErrorMessage = localizedErrorMessage;
            this.serverLegHeader = serverLegHeader;
        }

        public ErrorObject getErrorObject() {
            return this.errorObject;
        }

        public HttpResponse toHttpResponse() {
            HttpResponse httpResponse = HttpResponse.createErrorResponse(this.errorObject);
            httpResponse.addHeader(new Header(RESPONSE_ERROR_HEADER, Base64Utils.encodeToString(this.localizedErrorMessage)));
            if (this.serverLegHeader != null) {
                httpResponse.addHeader(this.serverLegHeader);
            }
            return httpResponse;
        }
    }
}