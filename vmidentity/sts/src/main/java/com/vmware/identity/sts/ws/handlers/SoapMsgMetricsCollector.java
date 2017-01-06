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

package com.vmware.identity.sts.ws.handlers;

import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.concurrent.TimeUnit;

import javax.xml.namespace.QName;
import javax.xml.ws.handler.MessageContext;
import javax.xml.ws.handler.soap.SOAPHandler;
import javax.xml.ws.handler.soap.SOAPMessageContext;

import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.sun.xml.ws.transport.Headers;
import com.vmware.identity.performanceSupport.IPerfDataSink;
import com.vmware.identity.performanceSupport.PerfBucketKey;
import com.vmware.identity.performanceSupport.PerfMeasurementInterface;
import com.vmware.identity.performanceSupport.PerfMeasurementPoint;
import com.vmware.identity.sts.util.MessageExtractionUtil;

/**
 * This class is used for measuring / persistent of WS-Trust request SOAP
 * message processing time
 *
 */
public final class SoapMsgMetricsCollector
implements SOAPHandler<SOAPMessageContext>
{

   private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
         .getLogger(SoapMsgMetricsCollector.class);

   private static IPerfDataSink perfDataSink;
   private final ThreadLocal<Long> msgStartTs = new ThreadLocal<Long>();

   public SoapMsgMetricsCollector() {
   }

   @Override
   public boolean handleMessage(SOAPMessageContext context) {
      Validate.notNull(context);

      Boolean outbound =
            (Boolean) context.get(MessageContext.MESSAGE_OUTBOUND_PROPERTY);
      if (outbound != null && outbound.equals(Boolean.TRUE)) {

         reportTime(context, PerfMeasurementInterface.ISoapMsgHandlerOB);
         return true;
      }

      setMsgStartTs();
      return true;
   }

   @Override
   public void close(MessageContext context) {
      msgStartTs.remove();  // we are done using it
}

   private void reportTime(MessageContext context, PerfMeasurementInterface itf)
   {
      long delta = TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - msgStartTs.get().longValue());

      List<String> actions =
            ((Headers)context.get(MessageContext.HTTP_REQUEST_HEADERS)).get("soapaction");
      assert actions.size() == 1;

      //remove the double quote
      String action = actions.get(0);
      String canonicalAction = action.startsWith("\"")?
            action.substring(1, action.length()-1) : action;

      try {
            reportProcessingTime(
            delta,
            canonicalAction,
            MessageExtractionUtil.extractUsernameFromMsgContext(context),
            itf);
      }catch (Exception e){ //log & swallow
         logger.error(
            "exception occurred when reporting performance measurement data on itf {}: {}",
            itf, e.getMessage());
      }
   }

   @Override
   public boolean handleFault(SOAPMessageContext context) {
      return true;
   }

   @Override
   public Set<QName> getHeaders() {
      return Collections.emptySet();
   }

   private void setMsgStartTs() {
      long ms = System.nanoTime();
      msgStartTs.set(ms);
   }

   private void reportProcessingTime(long millis, String soapAction, String upn, PerfMeasurementInterface itf) {
      assert soapAction != null;

      perfDataSink.addMeasurement(
            new PerfBucketKey(PerfMeasurementPoint.lookupById(
                                 itf,
                                 soapAction),
                              upn),
            millis);
   }

   public static void setPerfDataSink(IPerfDataSink aPerfDataSink)
   {
      assert aPerfDataSink != null;
      perfDataSink = aPerfDataSink;
      logger.info("SoapMsgMetricsCollector.perfDataSink is set successfully: " + perfDataSink);
   }
}
