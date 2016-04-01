/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *   use this file except in compliance with the License.  You may obtain a copy
 *   of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, without
 *   warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */
package com.vmware.identity.wstrust.client.impl;

import java.net.URL;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.TimeUnit;

import javax.xml.soap.Name;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Node;

import com.vmware.vim.sso.client.BundleMessageSource.Key;
import com.vmware.vim.sso.client.exception.InvalidTokenException;
import com.vmware.identity.wstrust.client.AccountLockedException;
import com.vmware.identity.wstrust.client.AuthenticationFailedException;
import com.vmware.identity.wstrust.client.CertificateValidationException;
import com.vmware.identity.wstrust.client.SsoRequestException;
import com.vmware.identity.wstrust.client.InvalidTokenRequestException;
import com.vmware.identity.wstrust.client.MalformedResponseException;
import com.vmware.identity.wstrust.client.PasswordExpiredException;
import com.vmware.identity.wstrust.client.RequestExpiredException;
import com.vmware.identity.wstrust.client.ServerCommunicationException;
import com.vmware.identity.wstrust.client.ServerSecurityException;
import com.vmware.identity.wstrust.client.TimeSynchronizationException;
import com.vmware.identity.wstrust.client.TokenRequestRejectedException;

/**
 * This class encapsulates the algorithm needed for send request/parse response
 *
 * <p>
 * Messages of thrown exceptions are localized.
 *
 * @param <T>
 *            type of response
 */
final class RequestResponseProcessor {
    private final WsSecuritySignature wsSecSignature;
    private final RequestBuilder requestBuilder;
    // private final ResponseHandler responseHandler;
    private final SoapBinding binding;

    private static final String AUTHENTICATION_FAULT = "FailedAuthentication";
    private static final String INVALID_TIME_RANGE_FAULT = "InvalidTimeRange";
    private static final String RENEW_NEEDED_FAULT = "RenewNeeded";
    private static final String UNABLE_TO_RENEW_FAULT = "UnableToRenew";
    private static final String MESSAGE_EXPIRED_FAULT = "MessageExpired";
    private static final String SIGNATURE_VALIDATION_FAULT = "FailedCheck";
    private static final String GENERAL_WSSE_FAULT = "InvalidSecurity";
    private static final String INVALID_REQUEST = "InvalidRequest";

    // This fault is not in RIAT spec. but is in WS-Trust 1.3,1.4
    private static final String EXPIRED_DATA_FAULT = "ExpiredData";

    private final static Logger log = LoggerFactory.getLogger(RequestResponseProcessor.class);

    public RequestResponseProcessor(RequestBuilder requestBuilder, WsSecuritySignature wsSecSignature,
            SoapBinding binding) {

        assert requestBuilder != null;
        // assert responseHandler != null;
        assert wsSecSignature != null;
        assert binding != null;

        this.wsSecSignature = wsSecSignature;
        this.requestBuilder = requestBuilder;
        // this.responseHandler = responseHandler;
        this.binding = binding;
    }

    /**
     * Executes roundtrip to the STS.
     *
     * @return
     * @throws InvalidTokenException
     *             if a token was successfully extracted from the STS response,
     *             but the token schema was not valid
     * @throws TokenRequestRejectedException
     *             usually if the credentials were not valid
     * @throws SsoRequestException
     * @throws ServerCommunicationException
     * @throws RequestExpiredException
     * @throws ServerSecurityException
     * @throws TimeSynchronizationException
     */
    public Node executeRoundtrip(URL serviceLocatioURL) throws InvalidTokenException, TokenRequestRejectedException,
            SsoRequestException, ServerCommunicationException, ServerSecurityException, RequestExpiredException,
            CertificateValidationException, TimeSynchronizationException {

        long roundtripStart = System.currentTimeMillis();

        if (log.isTraceEnabled()) {
            log.trace(String.format("Executing roundtrip with %s, %s", requestBuilder.getClass().getSimpleName(),
                    wsSecSignature.getClass().getSimpleName()));
        }

        SoapMessage request;
        try {
            request = requestBuilder.createRequest();
            wsSecSignature.sign(request);
        } catch (ParserException e) {
            String message = "Error occured during serialization of the token request";
            log.info(message, e);
            throw new SsoRequestException(message, Key.INTERNAL_CLIENT_ERROR, e, Key.PARSER_ERROR);
        } catch (SignatureException e) {
            String message = "Cannot sign request message";
            log.info(message, e);
            throw new SsoRequestException(message, Key.INTERNAL_CLIENT_ERROR, e, Key.FAILED_TO_SIGN_REQUEST);
        }

        try {
            Node result = sendRequest(request, serviceLocatioURL);
            return result;
        } catch (RequestExpiredException e) {
            checkTimeSyncronization(roundtripStart); // may throw
                                                     // TimeSyncronizationException
            throw e;
        } catch (ParserException e) {
            String message = "Error occured during deserialization of response" + " to the token request";
            log.info(message, e);
            throw new MalformedResponseException(message, Key.UNEXPECTED_RESPONSE_FORMAT, e);
        }
    }

    /**
     * Throws a relevant exception if we detect a situation that can only be
     * cause by a difference between client and server time
     *
     * @param roundtripStart
     *            millisecond timestamp of a point in time right before the
     *            roundtrip start
     * @throws TimeSynchronizationException
     *             if the situation was detected (message localized)
     */
    private void checkTimeSyncronization(long roundtripStart) throws TimeSynchronizationException {
        long roundtripEnd = System.currentTimeMillis();
        long roundtripEstimateSeconds = TimeUnit.MILLISECONDS.toSeconds(roundtripEnd - roundtripStart);

        // Our roundtrip interval estimate is definitely larger that actual
        // interval.
        if (roundtripEstimateSeconds < requestBuilder.getRequestValidityInSeconds()) {
            String message = "Server returned 'request expired' less than " + roundtripEstimateSeconds
                    + " seconds after request was issued, but it shouldn't have expired for " + "at least "
                    + requestBuilder.getRequestValidityInSeconds() + " seconds.";
            log.info(message);
            throw new TimeSynchronizationException(message, roundtripEstimateSeconds,
                    requestBuilder.getRequestValidityInSeconds());
        }
    }

    /**
     * Send the request through the transport
     *
     * @param message
     * @return the result of the SOAP method call
     * @throws ParserException
     * @throws ServerCommunicationException
     * @throws SsoRequestException
     * @throws CertificateValidationException
     * @throws InvalidCredentialException
     */
    private Node sendRequest(SoapMessage message, URL serviceLocationURL) throws ParserException,
            TokenRequestRejectedException, ServerCommunicationException, SsoRequestException, RequestExpiredException,
            ServerSecurityException, CertificateValidationException {

        SoapMessage response = null;
        try {
            long currentTime = System.currentTimeMillis();
            response = binding.sendMessage(message, serviceLocationURL);
            log.debug("Message received in: " + (System.currentTimeMillis() - currentTime) + " milliseconds.");
        } catch (SoapFaultException e) {
            handleFaultCondition(e.getFault());
        } catch (CertificateValidationException e) {
            // localize the message
            throw new CertificateValidationException(e.getMessage(), Key.BAD_SERVER_SSL_CERTIFICATE,
                    e.getCertificateChain(), e.getThumbprint());
        } catch (ServerCommunicationException e) {
            // localize the message
            throw new ServerCommunicationException(e.getMessage(), Key.FAILED_TO_CONNECT_TO_SERVER, e.getCause());
        }

        log.debug("Message successfully transported to the STS server");

        return response.getBody();
    }

    /**
     * Handle fault condition i.e. transform the SoapException to a more
     * specific "external" exception
     *
     * @param fault
     *            required
     * @throws ParserException
     * @throws TokenRequestRejectedException
     * @throws SsoRequestException
     * @throws RequestExpiredException
     * @throws ServerSecurityException
     */
    private void handleFaultCondition(SoapFault fault) throws ParserException, TokenRequestRejectedException,
            SsoRequestException, RequestExpiredException, ServerSecurityException {

        // Note that fault message = fault code + fault string
        log.debug("Processing fault: " + fault.getFaultMessage());

        assert fault.getFaultCodeAsName() != null;

        ErrorHandlerMap.handleSoapFault(fault);
    }

    private interface ErrorHandler {
        public void handleError(String message) throws ParserException, TokenRequestRejectedException,
                SsoRequestException, RequestExpiredException, ServerSecurityException;
    }

    private static class ErrorHandlerMap {
        private static Map<String, ErrorHandler> errorHandlers = new HashMap<>();

        static {
            errorHandlers.put(AUTHENTICATION_FAULT, new ErrorHandler() {

                @Override
                public void handleError(String message) throws ParserException, TokenRequestRejectedException,
                        SsoRequestException, RequestExpiredException, ServerSecurityException {

                    if (message != null && message.contains("locked")) {
                        log.info(message);
                        throw new AccountLockedException(message);

                    } else if (message != null && message.contains("expired")) {
                        /*
                         * That is the way to differentiate between two or more
                         * cases of Authentication fault, to read the fault
                         * message.
                         */
                        log.info(message);
                        throw new PasswordExpiredException(message);

                    } else {
                        String logMessage = "Provided credentials are not valid.";
                        log.info(logMessage);
                        throw new AuthenticationFailedException(logMessage, Key.INVALID_CREDENTIALS);
                    }

                }
            });

            errorHandlers.put(AUTHENTICATION_FAULT + "locked", new ErrorHandler() {

                @Override
                public void handleError(String message) throws ParserException, TokenRequestRejectedException,
                        SsoRequestException, RequestExpiredException, ServerSecurityException {
                    log.info(message);
                    throw new AccountLockedException(message);
                }
            });

            errorHandlers.put(AUTHENTICATION_FAULT + "expired", new ErrorHandler() {

                @Override
                public void handleError(String message) throws ParserException, TokenRequestRejectedException,
                        SsoRequestException, RequestExpiredException, ServerSecurityException {
                    log.info(message);
                    throw new PasswordExpiredException(message);
                }
            });

            errorHandlers.put(INVALID_TIME_RANGE_FAULT, new ErrorHandler() {

                @Override
                public void handleError(String message) throws ParserException, TokenRequestRejectedException,
                        SsoRequestException, RequestExpiredException, ServerSecurityException {

                    String logMessage = "Server rejected the provided time range. Cause: " + message;
                    /*
                     * We log at error as this is unexpected. The time range is
                     * created in code and not taken from the user, so it should
                     * be rejected only in case of time skew or bug
                     */
                    log.error(logMessage);

                    /*
                     * Ruling out start-after-end error, STS should return this
                     * fault if validity start is too far in the future, which
                     * also only be caused by time skew at this point.
                     */
                    throw new TimeSynchronizationException(logMessage, Key.REQUEST_AFFECTED_BY_TIME_SKEW);

                    /*
                     * RIAT, specifically, returns InvalidTimeRange also when
                     * the requested validity range has expired (requested end
                     * is in the past). According to the WS-Trust spec.
                     * ExpiredData fits better. See RIAT-1371 for details. Since
                     * we map both faults to the same exception, that doesn't
                     * affect the users.
                     */
                }
            });

            errorHandlers.put(EXPIRED_DATA_FAULT, new ErrorHandler() {
                @Override
                public void handleError(String faultMessage) throws ParserException, TokenRequestRejectedException,
                        SsoRequestException, RequestExpiredException, ServerSecurityException {
                    String logMessage = "Server rejected the request because the "
                            + "WS-Trust data was invalidated by time skew. Cause:" + faultMessage;
                    log.error(logMessage);
                    throw new TimeSynchronizationException(logMessage, Key.REQUEST_AFFECTED_BY_TIME_SKEW);
                }
            });

            errorHandlers.put(RENEW_NEEDED_FAULT, new ErrorHandler() {

                @Override
                public void handleError(String message) throws ParserException, TokenRequestRejectedException,
                        SsoRequestException, RequestExpiredException, ServerSecurityException {
                    String logMessage = "Token had expired. Renew is needed.";
                    log.info(logMessage);
                    throw new AuthenticationFailedException(logMessage, Key.RENEW_NEEDED);
                }
            });

            errorHandlers.put(UNABLE_TO_RENEW_FAULT, new ErrorHandler() {

                @Override
                public void handleError(String message) throws ParserException, TokenRequestRejectedException,
                        SsoRequestException, RequestExpiredException, ServerSecurityException {
                    String logMessage = "Unable to renew the security token: " + message;
                    log.info(logMessage);
                    throw new InvalidTokenRequestException(logMessage, message);
                }
            });

            errorHandlers.put(GENERAL_WSSE_FAULT, new ErrorHandler() {

                @Override
                public void handleError(String message) throws ParserException, TokenRequestRejectedException,
                        SsoRequestException, RequestExpiredException, ServerSecurityException {

                    String logMessage = "Server found errors in the WS-Security envelope: " + message;
                    log.info(logMessage);
                    throw new ServerSecurityException(logMessage, message);
                }
            });

            errorHandlers.put(MESSAGE_EXPIRED_FAULT, new ErrorHandler() {

                @Override
                public void handleError(String message) throws ParserException, TokenRequestRejectedException,
                        SsoRequestException, RequestExpiredException, ServerSecurityException {

                    String logMessage = "Request message has expired. Server message: " + message;
                    log.info(logMessage);
                    throw new RequestExpiredException(logMessage);
                }
            });

            errorHandlers.put(SIGNATURE_VALIDATION_FAULT, new ErrorHandler() {

                @Override
                public void handleError(String message) throws ParserException, TokenRequestRejectedException,
                        SsoRequestException, RequestExpiredException, ServerSecurityException {
                    String logMessage = "Request signature is not valid. Check if "
                            + "the confirmation certificate matches the given private key.";
                    log.info(logMessage);
                    throw new AuthenticationFailedException(logMessage, Key.SIGNATURE_VALIDATION_FAULT);
                }
            });

            errorHandlers.put(INVALID_REQUEST, new ErrorHandler() {

                @Override
                public void handleError(String message) throws ParserException, TokenRequestRejectedException,
                        SsoRequestException, RequestExpiredException, ServerSecurityException {
                    String logMessage = "Request is invalid: " + message;
                    log.info(logMessage);
                    throw new InvalidTokenRequestException(logMessage, message);
                }
            });

        }

        public static void handleSoapFault(SoapFault fault) throws ParserException, TokenRequestRejectedException,
                SsoRequestException, RequestExpiredException, ServerSecurityException {
            Name faultCodeName = fault.getFaultCodeAsName();

            ErrorHandler handler = errorHandlers.get(faultCodeName.getLocalName());

            if (handler != null)
                handler.handleError(fault.getFaultMessage());
            else {
                // The other WS-Security fault codes are considered
                // InternalErrors
                // as they can only be caused a bug in the client or
                // incompatibility
                // between the client and the server
                // WS-Security: UnsupportedSecurityToken, UnsupportedAlgorithm,
                // SecurityTokenUnavailable, InvalidSecurityToken
                // WS-Trust: BadRequest, InvalidRequest
                String message = "Failed trying to retrieve token: " + fault.getFaultMessage();
                log.info(message);

                throw new SsoRequestException(message, Key.UNEXPECTED_SERVER_ERROR, null, fault.getFaultMessage());
            }
        }
    }
}