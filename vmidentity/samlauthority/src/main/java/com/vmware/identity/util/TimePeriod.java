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

import java.util.Date;

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.Validate;

/**
 * This class represents a period of time. The time interval can be restricted
 * [startTime, endTime), even from one side like [startTime, +Inf) or (-Inf,
 * endTime) or not restricted (-Inf,+Inf).
 */
public final class TimePeriod {

   private final Date startTime;
   private final Date endTime;

   /**
    * Creates time interval [startTime, endTime) if times are not null. Creates
    * time interval (-Inf, endTime) if start time is null. Creates time interval
    * [startTime, +Inf) if end time is null. Creates time interval (-Inf, +Inf)
    * if both times are null.
    *
    * @param startTime
    *           as UTC milliseconds from the epoch. Can be null.
    * @param endTime
    *           as UTC milliseconds from the epoch. Should be after start time,
    *           if it is not null. Can be null.
    * @throws IllegalArgumentException
    *            when start and end times are not null and the end time is not
    *            greater than start time
    */
   public TimePeriod(Date startTime, Date endTime) {
      if (startTime != null && endTime != null && !endTime.after(startTime)) {
         throw new IllegalArgumentException("EndTime: " + endTime
            + " is not after startTime: " + startTime);
      }

      this.startTime = startTime;
      this.endTime = endTime;
   }

   /**
    * Creates time interval [startTime, startTime + duration).
    *
    * @param startTime
    *           cannot be null.
    * @param duration
    *           time period length in milliseconds. Should be positive.
    * @throws IllegalArgumentException
    *            when the start time is null or duration is not positive number
    */
   public TimePeriod(Date startTime, long duration) {
      Validate.notNull(startTime);
      Validate.isTrue(duration > 0);

      this.startTime = startTime;
      this.endTime = new Date(startTime.getTime() + duration);
   }

   /**
    * @return period start time. Can be null, which means -Inf.
    */
   public Date getStartTime() {
      return startTime;
   }

   /**
    * @return period end time. Can be null, which means +Inf.
    */
   public Date getEndTime() {
      return endTime;
   }

   /**
    *
    * @param pointOfTime
    *           cannot be null
    * @return if given pointOfTime is in this time period. If this period has
    *         start and end times this point of time can match with start time
    *         but not with end time, meaning if start and end times are not null
    *         the point of time is compared to this interval inclusive the start
    *         time, exclusive the end time.
    */
   public boolean contains(Date pointOfTime) {
      Validate.notNull(pointOfTime);

      boolean result = true;

      if (startTime != null) {
         result &= (pointOfTime.compareTo(startTime) >= 0);
      }
      if (endTime != null) {
         result &= pointOfTime.before(endTime);
      }

      return result;
   }

   /**
    * @param period
    *           of time for expansion. Cannot be null.
    * @param tolerance
    *           in milliseconds. Non-negative.
    * @return left and right expanded interval with given time. If interval is
    *         opened expansion is applied only on closed side of interval.
    */
   public static TimePeriod expand(TimePeriod period, long tolerance) {
      Validate.notNull(period);
      Validate.isTrue(tolerance >= 0);

      if (tolerance == 0) {
         return period;
      }

      Date startTime = period.getStartTime();
      startTime = (startTime != null) ? new Date(startTime.getTime()
         - tolerance) : null;
      Date endTime = period.getEndTime();
      endTime = (endTime != null) ? new Date(endTime.getTime() + tolerance)
         : null;

      return new TimePeriod(startTime, endTime);
   }

   @Override
   public String toString() {
      return "TimePeriod [startTime="
         + (startTime == null ? "-Inf" : startTime) + ", endTime="
         + (endTime == null ? "+Inf" : endTime) + "]";
   }

   @Override
   public int hashCode() {
      final int prime = 31;
      int result = 1;
      result = prime * result + ObjectUtils.hashCode(endTime);
      result = prime * result + ObjectUtils.hashCode(startTime);
      return result;
   }

   @Override
   public boolean equals(Object obj) {
      if (this == obj) {
         return true;
      }
      if (obj == null || this.getClass() != obj.getClass()) {
         return false;
      }

      TimePeriod other = (TimePeriod) obj;
      return ObjectUtils.equals(startTime, other.startTime)
         && ObjectUtils.equals(endTime, other.endTime);
   }

}