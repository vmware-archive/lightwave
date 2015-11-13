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
package com.vmware.identity.sts.impl;

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

import org.oasis_open.docs.ws_sx.ws_trust._200512.LifetimeType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0.AttributedDateTime;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0.TimestampType;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.sts.InvalidRequestException;
import com.vmware.identity.sts.InvalidSecurityException;
import com.vmware.identity.sts.InvalidTimeRangeException;
import com.vmware.identity.util.TimePeriod;

/**
 * Insert your comment for LifetimeConvertor here
 */
final class LifetimeConvertor {

   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(LifetimeConvertor.class);
   private static final String DEFAULT_TIMEZONE = "GMT";
   private static final String DATETIME_FORMAT_NO_MILLIS = "yyyy-MM-dd'T'HH:mm:ss'Z'";
   private static final String DATETIME_FORMAT_WITH_MILLIS = "yyyy-MM-dd'T'HH:mm:ss.SSS'Z'";

   // TODO unit test all visible methods

   /**
    * @param tokenValidity
    *           object containing saml token information. Cannot be null.
    * @return lifetime object which should be in response.
    */
   static LifetimeType toResponseLifetime(TimePeriod tokenValidity) {
      assert tokenValidity != null;

      LifetimeType lifetimeType = new LifetimeType();
      lifetimeType.setCreated(convertDateToAttributedDateTime(tokenValidity
         .getStartTime()));
      lifetimeType.setExpires(convertDateToAttributedDateTime(tokenValidity
         .getEndTime()));

      return lifetimeType;
   }

   /**
    * Extracts a time period from token lifetime requested by client.
    *
    * @param lifetime
    *           JAXB entity containing request info about token lifetime,
    *           required
    * @return not null time period
    * @throws InvalidRequestException
    *            when some of the dates is malformed or invalid
    * @throws InvalidTimeRangeException
    *            when the time period is not valid, e.g. the end date matches or
    *            is prior the start date
    */
   static TimePeriod fromRequestedTokenLifetime(LifetimeType lifetime)
      throws InvalidRequestException, InvalidTimeRangeException {
      assert lifetime != null;

      try {
         return newTimePeriod(lifetime.getCreated(), lifetime.getExpires());
      } catch (ParseException e) {
         throw new InvalidRequestException(
            "Cannot parse requested token lifetime [" + lifetime.getCreated() == null ? null
               : lifetime.getCreated().getValue() + "; "
                  + lifetime.getExpires() == null ? null : lifetime
                  .getExpires().getValue() + ")", e);
      } catch (IllegalArgumentException e) {
         throw new InvalidTimeRangeException("Invalid token lifetime", e);
      }
   }

   /**
    * Extracts a time period from request lifetime.
    *
    * @param lifetime
    *           JAXB entity containing request lifetime, required
    * @return not null time period
    * @throws InvalidSecurityException
    *            when some of the dates is malformed or invalid, or the time
    *            period is not valid, e.g. the end date matches or is prior the
    *            start date
    */
   static TimePeriod fromRequestLifetime(TimestampType lifetime)
      throws InvalidSecurityException {
      assert lifetime != null;

      try {
         return newTimePeriod(lifetime.getCreated(), lifetime.getExpires());
      } catch (ParseException e) {
         throw newInvalidSecurityException(lifetime, e,
            "Cannot parse request lifetime ");
      } catch (IllegalArgumentException e) {
         throw newInvalidSecurityException(lifetime, e,
            "Invalid request lifetime ");
      }

   }

   private static InvalidSecurityException newInvalidSecurityException(
      TimestampType lifetime, Exception cause, String msgStart) {
      assert lifetime != null && lifetime.getCreated() != null
         && lifetime.getExpires() != null : "Schema validation switched off";

      return new InvalidSecurityException(msgStart + "["
         + lifetime.getCreated().getValue() + "; "
         + lifetime.getExpires().getValue() + ")", cause);
   }

   private static TimePeriod newTimePeriod(AttributedDateTime created,
      AttributedDateTime expires) throws ParseException,
      IllegalArgumentException {

      final Date start = created == null ? null : toDate(created.getValue());
      final Date end = expires == null ? null : toDate(expires.getValue());
      return new TimePeriod(start, end);
   }

   private static AttributedDateTime convertDateToAttributedDateTime(
      Date dateTime) {
      if (dateTime == null) {
         return null;
      }

      log.trace(
         "About to create AttributedDateTime object for response. Datetime is: {}",
         dateTime);
      DateFormat formatter = newFormatter(DATETIME_FORMAT_WITH_MILLIS);
      String formattedDateTime = formatter.format(dateTime);
      log.trace("Formatted DateTime: {}", formattedDateTime);

      AttributedDateTime attributedDateTime = new AttributedDateTime();
      attributedDateTime.setValue(formattedDateTime);

      return attributedDateTime;
   }

   private static Date toDate(String dateTime) throws ParseException {
      assert dateTime != null;
      // TODO toString in all jaxb objects for debugging
      log.trace("Gets date object from request for datetime: {}", dateTime);

      Date result = null;
      try {
         result = newFormatter(DATETIME_FORMAT_WITH_MILLIS).parse(dateTime);
      } catch (ParseException e) {
         result = newFormatter(DATETIME_FORMAT_NO_MILLIS).parse(dateTime);
      }
      return result;
   }

   private static DateFormat newFormatter(String pattern) {
      assert pattern != null;

      DateFormat formatter = new SimpleDateFormat(pattern);
      formatter.setTimeZone(TimeZone.getTimeZone(DEFAULT_TIMEZONE));
      return formatter;
   }

}
