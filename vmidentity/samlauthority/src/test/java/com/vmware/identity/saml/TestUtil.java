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

package com.vmware.identity.saml;

import java.util.Calendar;
import java.util.Date;

public class TestUtil {

   public static final long DEFAULT_MAXIMUM_TOKEN_LIFETIME = 12 * 60 * 1000;
   public static final long DEFAULT_CLOCK_TOLERANCE_2_MINUTES = 2 * 60 * 1000;
   public static final int TOKEN_DELEGATION_COUNT = 10;
   public static final int TOKEN_RENEW_COUNT = 10;
   public static final long DEFAULT_CLOCK_TOLERANCE = 10000L;
   public static final String TENANT_NAME = "tenant";

   public static Date createDate(int year, int month, int day, int hours,
      int minutes, int seconds, int miliseconds) {
      Calendar cal = Calendar.getInstance();

      cal.set(Calendar.YEAR, year);
      cal.set(Calendar.MONTH, month);
      cal.set(Calendar.DATE, day);
      cal.set(Calendar.HOUR_OF_DAY, hours);
      cal.set(Calendar.MINUTE, minutes);
      cal.set(Calendar.SECOND, seconds);
      cal.set(Calendar.MILLISECOND, miliseconds);

      return new Date(cal.getTimeInMillis());
   }

   public static Date shiftDate(Date base, long shift) {
      return new Date(base.getTime() + shift);
   }
}
