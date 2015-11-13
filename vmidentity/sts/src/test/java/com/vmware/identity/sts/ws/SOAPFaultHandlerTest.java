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

import javax.xml.ws.soap.SOAPFaultException;

import org.junit.Assert;
import org.junit.Test;

import com.vmware.identity.sts.ws.SOAPFaultHandler.FaultKey;

/**
 * Insert your comment for SOAPFaultHandlerTest here
 */
public final class SOAPFaultHandlerTest {

   private static final FaultKey FAILED_CHECK_WSSE = FaultKey.WSSE_FAILED_CHECK;
   private static final String FAILED_CHECK_MSG = "ns0:FailedCheck";
   private static final String EXC_MESSAGE_L0 = "exc_message_0";
   private static final String EXC_MESSAGE_L1 = "exc_message_1";
   private static final String EXC_MESSAGE_L2 = "exc_message_2";

   @Test
   public void testThrowSoapFaultZeroNesting() {
      try {
         SOAPFaultHandler.throwSoapFault(new WSFaultException(
            FAILED_CHECK_WSSE, EXC_MESSAGE_L0));
         Assert.fail();
      } catch (SOAPFaultException e) {
         Assert.assertEquals(FAILED_CHECK_MSG, e.getFault().getFaultCode());
         Assert.assertEquals(EXC_MESSAGE_L0, e.getFault().getFaultString());
      }
   }

   @Test
   public void testThrowSoapFaultOneLevelNesting() {
      try {
         SOAPFaultHandler.throwSoapFault(new WSFaultException(
            FAILED_CHECK_WSSE, new Exception(EXC_MESSAGE_L1)));
         Assert.fail();
      } catch (SOAPFaultException e) {
         Assert.assertEquals(FAILED_CHECK_MSG, e.getFault().getFaultCode());
         Assert.assertEquals(EXC_MESSAGE_L1, e.getFault().getFaultString());
      }
   }

   @Test
   public void testThrowSoapFaultTwoLevelsNesting() {
      try {
         SOAPFaultHandler.throwSoapFault(new WSFaultException(
            FAILED_CHECK_WSSE, new Exception(EXC_MESSAGE_L1, new Exception(
               EXC_MESSAGE_L2))));
         Assert.fail();
      } catch (SOAPFaultException e) {
         Assert.assertEquals(FAILED_CHECK_MSG, e.getFault().getFaultCode());
         Assert.assertEquals(EXC_MESSAGE_L1
            + SOAPFaultHandler.MSG_LEVELS_DELIMITER + EXC_MESSAGE_L2, e
            .getFault().getFaultString());
      }
   }
}
