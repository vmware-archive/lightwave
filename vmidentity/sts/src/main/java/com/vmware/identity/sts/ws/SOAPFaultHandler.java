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
package com.vmware.identity.sts.ws;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import javax.xml.namespace.QName;
import javax.xml.soap.SOAPException;
import javax.xml.soap.SOAPFactory;
import javax.xml.soap.SOAPFault;
import javax.xml.ws.soap.SOAPFaultException;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

/**
 * This class is responsible for definition and execution of SOAP faults. This
 * class is thread safe.
 */
public final class SOAPFaultHandler {

   private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
      .getLogger(SOAPFaultHandler.class);
   private static final Map<FaultKey, FaultContent> faults = initFaults();
   static final String MSG_LEVELS_DELIMITER = " :: ";

   private SOAPFaultHandler() {
   }

   /**
    * @param key
    *           not null
    * @param logInfo
    *           not null
    */
   public static void throwSoapFault(WSFaultException cause) {
      assert cause != null;
      try {
         SOAPFactory fact = SOAPFactory.newInstance();
         SOAPFault fault = fact.createFault();
         FaultKey faultKey = cause.getFaultKey();
         fault.setFaultCode(faults.get(faultKey).getCode());
         fault.setFaultString(buildFaultText(cause));
         logger.debug("Ws Fault: ", cause);
         logger.info(
            "Returning a SOAP Fault with code: {} and description: {}",
            fault.getFaultCode(), fault.getFaultString());
         throw new SOAPFaultException(fault);
      } catch (SOAPException e) {
         logger.error("Unexpected error", e);
         throw new IllegalStateException(e);
      }
   }

   private static String buildFaultText(WSFaultException cause) {
      assert cause != null;

      final StringBuilder sb = new StringBuilder();
      appendMessageIfAny(cause, sb);
      Throwable ex = cause.getCause();
      while (ex != null) {
         appendMessageIfAny(ex, sb);
         ex = ex.getCause();
      }

      return sb.length() > 0 ? sb.toString() : faults.get(cause.getFaultKey())
         .getDescription();

   }

   private static void appendMessageIfAny(Throwable exc, StringBuilder sb) {
      assert exc != null;
      final String message = exc.getMessage();
      final Throwable cause = exc.getCause();
      if (message != null
         && (cause == null || !message.equals(cause.toString()))) {
         if (sb.length() > 0) {
            sb.append(MSG_LEVELS_DELIMITER);
         }
         sb.append(message);
      }
   }

   /**
    * @return pre-defined SOAP faults (as described by the standard - WSSE, WST)
    */
   private static Map<FaultKey, FaultContent> initFaults() {
      HashMap<FaultKey, FaultContent> result = new HashMap<SOAPFaultHandler.FaultKey, SOAPFaultHandler.FaultContent>();
      result.put(FaultKey.WSSE_FAILED_AUTHENTICATION, new FaultContent(
         new QName(WsConstants.WSSE_NS, "FailedAuthentication"),
         "The security token could not be authenticated or authorized"));

      result.put(FaultKey.WSSE_FAILED_CHECK, new FaultContent(new QName(
         WsConstants.WSSE_NS, "FailedCheck"),
         "The signature or decryption was invalid"));

      result.put(FaultKey.WSSE_INVALID_SECURITY, new FaultContent(new QName(
         WsConstants.WSSE_NS, "InvalidSecurity"),
         "An error was discovered processing the <wsse:Security> header."));

      result.put(FaultKey.WSSE_INVALID_SECURITY_TOKEN, new FaultContent(
         new QName(WsConstants.WSSE_NS, "InvalidSecurityToken"),
         "An invalid security token was provided"));

      result.put(FaultKey.WSSE_MESSAGE_EXPIRED, new FaultContent(new QName(
         WsConstants.WSSE_NS, "MessageExpired"), "The message has expired"));

      result.put(FaultKey.WSSE_SECURITY_TOKEN_UNAVAILABLE, new FaultContent(
         new QName(WsConstants.WSSE_NS, "SecurityTokenUnavailable"),
         "Referenced security token could not be retrieved"));

      result.put(FaultKey.WSSE_UNSUPPORTED_ALGORITHM, new FaultContent(
         new QName(WsConstants.WSSE_NS, "UnsupportedAlgorithm"),
         "An unsupported signature or encryption algorithm was used"));

      result.put(FaultKey.WSSE_UNSUPPORTED_SECURITY_TOKEN, new FaultContent(
         new QName(WsConstants.WSSE_NS, "UnsupportedSecurityToken"),
         "An unsupported token was provided"));

      result.put(FaultKey.WST_INVALID_REQUEST, new FaultContent(new QName(
         WsConstants.WST_NS, "InvalidRequest"),
         "The request was invalid or malformed."));

      result
         .put(FaultKey.WST_REQUEST_FAILED, new FaultContent(new QName(
            WsConstants.WST_NS, "RequestFailed"),
            "The specified request failed."));

      result.put(FaultKey.WST_AUTHENTICATION_FAILED, new FaultContent(
         new QName(WsConstants.WST_NS, "FailedAuthentication"),
         "Authentication failed."));

      result.put(FaultKey.WST_INVALID_TIME_RANGE, new FaultContent(new QName(
         WsConstants.WST_NS, "InvalidTimeRange"),
         "The requested time range is invalid or unsupported."));

      result.put(FaultKey.WST_UNABLE_TO_RENEW, new FaultContent(new QName(
         WsConstants.WST_NS, "UnableToRenew"),
         "The requested renewal failed."));

      return Collections.unmodifiableMap(result);
   }

   /**
    * Contains fault keys representing the standard (WSSE, WST) fault codes
    */
   public enum FaultKey {
      WSSE_UNSUPPORTED_SECURITY_TOKEN,
      WSSE_UNSUPPORTED_ALGORITHM,
      WSSE_INVALID_SECURITY,
      WSSE_INVALID_SECURITY_TOKEN,
      WSSE_FAILED_AUTHENTICATION,
      WSSE_FAILED_CHECK,
      WSSE_SECURITY_TOKEN_UNAVAILABLE,
      WSSE_MESSAGE_EXPIRED,
      WST_INVALID_REQUEST,
      WST_REQUEST_FAILED,
      WST_AUTHENTICATION_FAILED,
      WST_INVALID_TIME_RANGE,
      WST_UNABLE_TO_RENEW
   }

   /**
    * This class is a container for SOAP fault code + SOAP fault description
    */
   private static class FaultContent {

      private final QName code;
      private final String description;

      public FaultContent(QName code, String description) {
         assert code != null;
         assert description != null;
         this.code = code;
         this.description = description;
      }

      public QName getCode() {
         return code;
      }

      public String getDescription() {
         return description;
      }
   }
}
