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

import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.TimeZone;

import org.junit.Assert;
import org.junit.Test;
import org.oasis_open.docs.ws_sx.ws_trust._200512.LifetimeType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0.AttributedDateTime;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0.TimestampType;

import com.vmware.identity.sts.InvalidRequestException;
import com.vmware.identity.sts.InvalidSecurityException;
import com.vmware.identity.sts.InvalidTimeRangeException;
import com.vmware.identity.util.TimePeriod;

/**
 * Insert your comment for LifetimeConvertorTest here
 */
public final class LifetimeConvertorTest {

   private static final String DATE2_TXT = "2012-06-20T06:53:11.031Z";
   private static final String DATE1_TXT = "2012-06-20T06:53:10Z";
   private static final Date DATE1 = newDate(2012, 05, 20, 6, 53, 10, 0);
   private static final Date DATE2 = newDate(2012, 05, 20, 6, 53, 11, 31);

   /*****************************************
    *
    * Test for "fromTokenLifetime()"
    *
    *****************************************/
   @Test
   public void testFromTokenLifetimeOK() {
      Assert.assertEquals(new TimePeriod(DATE1, DATE2),
         fromTokenLifetime(DATE1_TXT, DATE2_TXT));
   }

   @Test
   public void testFromTokenLifetimeNoStartDate() {
      Assert.assertEquals(new TimePeriod(null, DATE2),
         fromTokenLifetime(null, DATE2_TXT));
   }

   @Test
   public void testFromTokenLifetimeNoEndDate() {
      Assert.assertEquals(new TimePeriod(DATE1, null),
         fromTokenLifetime(DATE1_TXT, null));
   }

   @Test
   public void testFromTokenLifetimeNoStartAndEndDate() {
      Assert.assertEquals(new TimePeriod(null, null),
         fromTokenLifetime(null, null));
   }

   @Test
   public void testFromTokenLifetimeInvalidDate() {
      try {
         fromTokenLifetime("invalid-date", DATE2_TXT);
         Assert.fail();
      } catch (InvalidRequestException e) {
         // expected
      }
   }

   @Test
   public void testFromTokenLifetimeInvalidPeriod() {
      testFromTokenLifetimeInvalidPeriodInt(DATE2_TXT, DATE1_TXT);

      testFromTokenLifetimeInvalidPeriodInt(DATE1_TXT, DATE1_TXT);
   }

   /*****************************************
    *
    * Test for "fromRequestLifetime()"
    *
    *****************************************/
   @Test
   public void testFromRequestLifetimeOK() {
      Assert.assertEquals(new TimePeriod(DATE1, DATE2),
         fromRequestLifetime(DATE1_TXT, DATE2_TXT));
   }

   @Test
   public void testFromRequestLifetimeInvalidDate() {
      testFromRequestLifetimeNOKInt(DATE1_TXT, "invasdc");
   }

   @Test
   public void testFromRequestLifetimeInvalidPeriod() {
      testFromRequestLifetimeNOKInt(DATE2_TXT, DATE1_TXT);

      testFromRequestLifetimeNOKInt(DATE1_TXT, DATE1_TXT);
   }

   private void testFromTokenLifetimeInvalidPeriodInt(String start, String end) {
      try {
         fromTokenLifetime(start, end);
         Assert.fail();
      } catch (InvalidTimeRangeException e) {
         // expected
      }
   }

   private TimePeriod fromTokenLifetime(String start, String end) {
      final LifetimeType lifetime = new LifetimeType();
      if (start != null) {
         AttributedDateTime value = new AttributedDateTime();
         value.setValue(start);
         lifetime.setCreated(value);
      }
      if (end != null) {
         AttributedDateTime value = new AttributedDateTime();
         value.setValue(end);
         lifetime.setExpires(value);
      }

      return LifetimeConvertor.fromRequestedTokenLifetime(lifetime);
   }

   private void testFromRequestLifetimeNOKInt(String start, String end) {
      try {
         fromRequestLifetime(start, end);
         Assert.fail();
      } catch (InvalidSecurityException e) {
         // expected
      }
   }

   private TimePeriod fromRequestLifetime(String start, String end) {
      final TimestampType lifetime = new TimestampType();
      {
         AttributedDateTime value = new AttributedDateTime();
         value.setValue(start);
         lifetime.setCreated(value);
      }
      {
         AttributedDateTime value = new AttributedDateTime();
         value.setValue(end);
         lifetime.setExpires(value);
      }

      return LifetimeConvertor.fromRequestLifetime(lifetime);
   }

   private static Date newDate(int year, int month, int day, int hours,
      int minutes, int seconds, int millis) {
      Calendar calendar = new GregorianCalendar(TimeZone.getTimeZone("GMT"));
      calendar.set(year, month, day, hours, minutes, seconds);
      calendar.set(Calendar.MILLISECOND, millis);
      return calendar.getTime();
   }
}
