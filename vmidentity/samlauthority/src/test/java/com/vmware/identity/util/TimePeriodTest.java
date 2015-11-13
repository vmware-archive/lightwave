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
package com.vmware.identity.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;

import org.junit.Test;

public class TimePeriodTest {

   private Date pastPointOfTime;
   private Date startTime;
   private Date internalPointOfTime;
   private Date endTime;
   private Date futurePointOfTime;
   private long duration;

   public TimePeriodTest() {
      Calendar cal = new GregorianCalendar();

      pastPointOfTime = cal.getTime();
      cal.add(Calendar.MINUTE, 10);
      startTime = cal.getTime();
      cal.add(Calendar.MINUTE, 10);
      internalPointOfTime = cal.getTime();
      cal.add(Calendar.MINUTE, 10);
      endTime = cal.getTime();
      cal.add(Calendar.MINUTE, 10);
      futurePointOfTime = cal.getTime();

      duration = 20 * 60 * 1000;
   }

   @Test
   public void testCreateClosedPeriod() {
      TimePeriod period = new TimePeriod(startTime, endTime);

      assertNotNull(period);
      assertEquals(startTime, period.getStartTime());
      assertEquals(endTime, period.getEndTime());
   }

   @Test
   public void testCreateLeftOpenedPeriod() {
      TimePeriod period = new TimePeriod(null, endTime);

      assertNotNull(period);
      assertNull(period.getStartTime());
      assertEquals(endTime, period.getEndTime());
   }

   @Test
   public void testCreateRightOpenedPeriod() {
      TimePeriod period = new TimePeriod(startTime, null);

      assertNotNull(period);
      assertEquals(startTime, period.getStartTime());
      assertNull(period.getEndTime());
   }

   @Test
   public void testCreateOpenedPeriod() {
      TimePeriod period = new TimePeriod(null, null);

      assertNotNull(period);
      assertNull(period.getStartTime());
      assertNull(period.getEndTime());
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreatePeriodWithStartAfterEndDate() {
      new TimePeriod(endTime, startTime);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreatePeriodWithStartMatchingEndDate() {
      new TimePeriod(startTime, startTime);
   }

   @Test
   public void testCreatePeriodWithStartAndDuration() {
      TimePeriod period = new TimePeriod(startTime, duration);

      assertNotNull(period);
      assertEquals(startTime, period.getStartTime());
      assertEquals(endTime, period.getEndTime());
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreatePeriodWithStartAndZeroDuration() {
      new TimePeriod(startTime, 0);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreatePeriodWithNoStartAndDuration() {
      new TimePeriod(null, duration);
   }

   @Test
   public void testClosedPeriodContainsPointOfTime_InternalPoint() {
      TimePeriod period = new TimePeriod(startTime, endTime);
      assertTrue(period.contains(internalPointOfTime));
   }

   @Test
   public void testClosedPeriodContainsPointOfTime_PointMatchPeriodStart() {
      TimePeriod period = new TimePeriod(startTime, endTime);
      assertTrue(period.contains(startTime));
   }

   @Test
   public void testClosedPeriodDoesNotContainsPointOfTime_PointInFuture() {
      TimePeriod period = new TimePeriod(startTime, endTime);
      assertFalse(period.contains(futurePointOfTime));
   }

   @Test
   public void testClosedPeriodDoesNotContainsPointOfTime_PointMatchPeriodEnd() {
      TimePeriod period = new TimePeriod(startTime, endTime);
      assertFalse(period.contains(endTime));
   }

   @Test
   public void testClosedPeriodDoesNotContainsPointOfTime_PointInThePast() {
      TimePeriod period = new TimePeriod(startTime, endTime);
      assertFalse(period.contains(pastPointOfTime));
   }

   @Test
   public void testLeftOpenedPeriodContainsPointOfTime_InternalPoint() {
      TimePeriod period = new TimePeriod(null, endTime);
      assertTrue(period.contains(internalPointOfTime));
   }

   @Test
   public void testLeftOpenedPeriodDoesNotContainsPointOfTime_PointMatchPeriodEnd() {
      TimePeriod period = new TimePeriod(null, endTime);
      assertFalse(period.contains(endTime));
   }

   @Test
   public void testLeftOpenedPeriodDoesNotContainsPointOfTime_PointInFuture() {
      TimePeriod period = new TimePeriod(null, endTime);
      assertFalse(period.contains(futurePointOfTime));
   }

   @Test
   public void testRightOpenedPeriodContainsPointOfTime_PointMatchPeriodStart() {
      TimePeriod period = new TimePeriod(startTime, null);
      assertTrue(period.contains(startTime));
   }

   @Test
   public void testRightOpenedPeriodContainsPointOfTime_PointInternalPoint() {
      TimePeriod period = new TimePeriod(startTime, null);
      assertTrue(period.contains(internalPointOfTime));
   }

   @Test
   public void testRightOpenedPeriodDoesNotContainsPointOfTime_PointInThePast() {
      TimePeriod period = new TimePeriod(startTime, null);
      assertFalse(period.contains(pastPointOfTime));
   }

   @Test
   public void testOpenedPeriodContainsPointOfTime_PointInThePast() {
      TimePeriod period = new TimePeriod(null, null);
      assertTrue(period.contains(pastPointOfTime));
   }

   @Test
   public void testOpenedPeriodContainsPointOfTime_PointInTheFuture() {
      TimePeriod period = new TimePeriod(null, null);
      assertTrue(period.contains(futurePointOfTime));
   }

   @Test
   public void testOpenedPeriodContainsPointOfTime_InternalPoint() {
      TimePeriod period = new TimePeriod(null, null);
      assertTrue(period.contains(internalPointOfTime));
   }

   @Test(expected = IllegalArgumentException.class)
   public void testPeriodDoesNotContains_NullPointOfTime() {
      new TimePeriod(startTime, endTime).contains(null);
   }

   @Test
   public void testExpandClosedPeriod() {
      TimePeriod period = new TimePeriod(startTime, endTime);
      int duration = 1000;

      TimePeriod expandedPeriod = TimePeriod.expand(period, duration);

      assertEquals(new Date(period.getStartTime().getTime() - duration),
         expandedPeriod.getStartTime());
      assertEquals(new Date(period.getEndTime().getTime() + duration),
         expandedPeriod.getEndTime());
   }

   @Test
   public void testExpandLeftOpenedPeriod() {
      TimePeriod period = new TimePeriod(null, endTime);
      int duration = 1000;

      TimePeriod expandedPeriod = TimePeriod.expand(period, duration);

      assertNull(expandedPeriod.getStartTime());
      assertEquals(new Date(period.getEndTime().getTime() + duration),
         expandedPeriod.getEndTime());
   }

   @Test
   public void testExpandRightOpenedPeriod() {
      TimePeriod period = new TimePeriod(startTime, null);
      int duration = 1000;

      TimePeriod expandedPeriod = TimePeriod.expand(period, duration);

      assertEquals(new Date(period.getStartTime().getTime() - duration),
         expandedPeriod.getStartTime());
      assertNull(expandedPeriod.getEndTime());
   }

   @Test
   public void testExpandOpenedPeriod() {
      TimePeriod period = new TimePeriod(null, null);
      int duration = 1000;

      TimePeriod expandedPeriod = TimePeriod.expand(period, duration);

      assertEquals(period, expandedPeriod);
   }

   @Test
   public void testExpandPeriodWithZeroTolerance() {
      TimePeriod period = new TimePeriod(startTime, endTime);

      assertEquals(period, TimePeriod.expand(period, 0));
   }

   @Test(expected = IllegalArgumentException.class)
   public void testExpandPeriodWithNegativeTolerance() {
      TimePeriod.expand(new TimePeriod(startTime, endTime), -1);
   }

   @Test
   public void testEqualsClosedPeriod() {
      TimePeriod period1 = new TimePeriod(startTime, endTime);
      TimePeriod period2 = new TimePeriod(startTime, endTime);

      assertTrue(period1.equals(period2));
   }

   @Test
   public void testEqualsLeftOpenedPeriod() {
      TimePeriod period1 = new TimePeriod(null, endTime);
      TimePeriod period2 = new TimePeriod(null, endTime);

      assertTrue(period1.equals(period2));
   }

   @Test
   public void testEqualsRightOpenedPeriod() {
      TimePeriod period1 = new TimePeriod(startTime, null);
      TimePeriod period2 = new TimePeriod(startTime, null);

      assertTrue(period1.equals(period2));
   }

   @Test
   public void testEqualsOpenedPeriod() {
      TimePeriod period1 = new TimePeriod(null, null);
      TimePeriod period2 = new TimePeriod(null, null);

      assertTrue(period1.equals(period2));
   }

   @Test
   public void testNotEqualsClosedPeriods_SameEndTimes() {
      TimePeriod period1 = new TimePeriod(startTime, endTime);
      TimePeriod period2 = new TimePeriod(pastPointOfTime, endTime);

      assertFalse(period1.equals(period2));
   }

   @Test
   public void testNotEqualsClosedPeriods_SameStartTimes() {
      TimePeriod period1 = new TimePeriod(startTime, endTime);
      TimePeriod period2 = new TimePeriod(startTime, futurePointOfTime);

      assertFalse(period1.equals(period2));
   }

   @Test
   public void testNotEqualsClosedPeriods_CrossingPeriods() {
      TimePeriod period1 = new TimePeriod(startTime, endTime);
      TimePeriod period2 = new TimePeriod(pastPointOfTime, internalPointOfTime);

      assertFalse(period1.equals(period2));
   }

   @Test
   public void testNotEqualsClosedAndLeftOpenedPeriod_SameEndTimes() {
      TimePeriod period1 = new TimePeriod(startTime, endTime);
      TimePeriod period2 = new TimePeriod(null, endTime);

      assertFalse(period1.equals(period2));
   }

   @Test
   public void testNotEqualsClosedAndLeftOpenedPeriod_DifferentEndTimes() {
      TimePeriod period1 = new TimePeriod(startTime, endTime);
      TimePeriod period2 = new TimePeriod(null, pastPointOfTime);

      assertFalse(period1.equals(period2));
   }

   @Test
   public void testNotEqualsClosedAndRightOpenedPeriod_SameStartTimes() {
      TimePeriod period1 = new TimePeriod(startTime, endTime);
      TimePeriod period2 = new TimePeriod(startTime, null);

      assertFalse(period1.equals(period2));
   }

   @Test
   public void testNotEqualsClosedAndRightOpenedPeriod_DifferentStartTimes() {
      TimePeriod period1 = new TimePeriod(startTime, endTime);
      TimePeriod period2 = new TimePeriod(internalPointOfTime, null);

      assertFalse(period1.equals(period2));
   }

   @Test
   public void testNotEqualsLeftOpenedAndRightOpenedPeriod() {
      TimePeriod period1 = new TimePeriod(startTime, null);
      TimePeriod period2 = new TimePeriod(null, endTime);

      assertFalse(period1.equals(period2));
   }

   @Test
   public void testNotEqualsWithNull() {
      assertFalse(new TimePeriod(startTime, null).equals(null));
   }


   @Test
   public void testClosedPeriodHashCode() {
      new TimePeriod(startTime, endTime).hashCode();
   }

   @Test
   public void testLeftOpenedPeriodHashCode() {
      new TimePeriod(null, endTime).hashCode();
   }

   @Test
   public void testRightOpenedPeriodHashCode() {
      new TimePeriod(startTime, null).hashCode();
   }

   @Test
   public void testOpenedHashCode() {
      new TimePeriod(null, null).hashCode();
   }


   @Test
   public void testOpenedPeriodToString() {
      TimePeriod period = new TimePeriod(null, null);
      assertEquals("TimePeriod [startTime=-Inf, endTime=+Inf]",
         period.toString());
   }

   @Test
   public void testClosedPeriodToString() {
      TimePeriod period = new TimePeriod(startTime, endTime);
      assertEquals("TimePeriod [startTime=" + startTime + ", endTime="
         + endTime + "]", period.toString());
   }
}
