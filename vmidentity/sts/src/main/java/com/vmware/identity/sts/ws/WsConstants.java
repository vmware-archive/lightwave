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

/**
 * This class contains constants related to WS processing
 */
public class WsConstants {

   public static final String SECURITY_HEADER_KEY = "SecurityHeaderKey";
   public static final String DOM_REQUEST_KEY = "DOMRequestKey";
   public static final String SIGNATURE_KEY = "SignatureKey";
   public static final String WSA_ACTION_KEY = "WSA_Action";

   public static final String TENANT_MDC_KEY = "TenantNameMDCKey";
   public static final String CORRELATION_ID_MDC_KEY = "CorrelationIdMDCKey";
   public static final String DEFAULT_TENANT = "(DEFAULT_TENANT)";

   public static final String WST_NS = "http://docs.oasis-open.org/ws-sx/ws-trust/200512";
   public static final String WST14_NS = "http://docs.oasis-open.org/ws-sx/ws-trust/200802";
   public static final String WSSE_NS = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd";
   public static final String WSU_NS = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd";
   public static final String ASSERTION_NS = "urn:oasis:names:tc:SAML:2.0:assertion";
   public static final String WSA_NS = "http://www.w3.org/2005/08/addressing";
   public static final String DS_NS = "http://www.w3.org/2000/09/xmldsig#";

   public static final String WSA_ACTION_ELEMENT_NAME = "Action";
   public static final String WSSE_SECURITY_ELEMENT_NAME = "Security";
   public static final String WSSE_USERNAME_TOKEN_ELEMENT_NAME = "UsernameToken";
   public static final String WSSE_PASSWORD_ELEMENT_NAME = "Password";
   public static final String WSSE_PASSCODE_ELEMENT_NAME = "Passcode";
   public static final String WSSE_BINARY_SECURITY_TOKEN_ELEMENT_NAME = "BinarySecurityToken";
   public static final String ASSERTION_ELEMENT_NAME = "Assertion";
   public static final String WSU_TIMESTAMP_ELEMENT_NAME = "Timestamp";
   public static final String DS_SIGNATURE_ELEMENT_NAME = "Signature";

   public static final String VALIDATE_TOKEN_TYPE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RSTR/Status";
   public static final String VALIDATE_STATUS_CODE_VALID = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/status/valid";
   public static final String VALIDATE_STATUS_CODE_INVALID = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/status/invalid";

   public static final String REQUEST_TYPE_ISSUE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/Issue";
   public static final String REQUEST_TYPE_RENEW = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/Renew";
   public static final String REQUEST_TYPE_VALIDATE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/Validate";

   public static final String SOAP_ACTION_ISSUE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Issue";
   public static final String SOAP_ACTION_RENEW = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Renew";
   public static final String SOAP_ACTION_VALIDATE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Validate";

   public static final String ACTIVITY_CORRELATION_ID_CUSTOM_HEADER = "x-vmwsts-activityid";

}
