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

package com.vmware.identity.sts.util;

import javax.servlet.ServletRequest;
import javax.xml.ws.handler.MessageContext;

import org.apache.commons.lang.Validate;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.AttributedString;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.UsernameTokenType;

import com.vmware.identity.sts.ws.WsConstants;

/**
 * Utility to extract needed information from context.
 * Will throw {@code MessageExtractionException} on any encountered exception
 * for the invoker to handler.
 * Primary for performance support currently
 *
 */
public class MessageExtractionUtil {

   /**
    * Parse the message context to get the user name
    * @param context cannot be null
    * @return
    * @throws MessageExtractionException
    */
   public static String extractUsernameFromMsgContext(MessageContext context) {

      Validate.notNull(context);
      ServletRequest req = (ServletRequest)context.get(MessageContext.SERVLET_REQUEST);
      assert req != null;
      return extractUsernameFromSevletRequest(req);
   }

   /**
    * Parse the servlet request to get the user name
    * @param req
    * @return
    */
   public static String extractUsernameFromSevletRequest(ServletRequest req)  {

      Validate.notNull(req);
      SecurityHeaderType secHdr = (SecurityHeaderType)req.getAttribute(WsConstants.SECURITY_HEADER_KEY);
      assert secHdr != null;
      return extractUsernameFromSecurityHeader(secHdr);
   }

   /**
    * Parse the security header to get the user name
    * @param secHdr cannot be null
    * @return user namec, could be null
    * @throws MessageExtractionException
    */
   public static String extractUsernameFromSecurityHeader(SecurityHeaderType secHdr) {

      Validate.notNull(secHdr);
      UsernameTokenType usernameToken = JAXBExtractor.extractFromSecurityHeader(secHdr, UsernameTokenType.class);
      if (usernameToken == null) {
         return null;
      }
      AttributedString attrUsername = usernameToken.getUsername();
      if (attrUsername == null) {
         return null;
      }
      return attrUsername.getValue();
   }
}
