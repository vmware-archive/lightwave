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
package com.vmware.vim.sso.admin;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Immutable data type representing lockout policy
 *
 * <p>
 * All value ranges below are inclusive. If not otherwise specified,
 * numeric values have the same limits as their datatypes.
 */
public final class LockoutPolicy {

   /**
    * Zero value for auto unlock interval means that only administrators will be
    * able unlock the account.
    */
   public static final long NO_AUTO_UNLOCK = 0L;

   public static final int DESCRIPTION_LENGTH_LIMIT = 120;
   public static final long FAILED_ATTEMPT_INTERVAL_LIMIT_SEC = 1000000000; // 1 bln.
   public static final long AUTO_UNLOCK_INTERVAL_LIMIT_SEC = 1000000000; // 1 bln.

   private final String _description;
   private final long _failedAttemptIntervalSec;
   private final int _failedAttempts;
   private final long _autoUnlockIntervalSec;

   /**
    * Lockout policy constructor
    *
    * @param description
    *           policy description; could be null or empty, max length is
    *           {@value #DESCRIPTION_LENGTH_LIMIT}
    * @param failedAttemptIntervalSec
    *           time window ( in seconds ) in which the failures must occur to
    *           trigger a lockout; a number is expected between 1 and
    *           {@value #FAILED_ATTEMPT_INTERVAL_LIMIT_SEC}
    * @param failedAttempts
    *           the number of failed logon attempts that causes a user account
    *           to be locked out; a positive number is expected
    * @param autoUnlockIntervalSec
    *           time ( in seconds ) that the account will remain locked or zero
    *           ( which is {link #NO_AUTO_UNLOCK} ) to make the only way an
    *           administrator to unlock it explicitly; non-negative number is
    *           expected less than {@value #AUTO_UNLOCK_INTERVAL_LIMIT_SEC}
    *
    * @throws IllegalArgumentException
    *            when any of the supplied arguments is invalid
    */
   public LockoutPolicy(String description, long failedAttemptIntervalSec,
      int failedAttempts, long autoUnlockIntervalSec) {

      if (description != null) {
         ValidateUtil.validateRange(description.length(), "length of description",
            0, DESCRIPTION_LENGTH_LIMIT);
      }
      ValidateUtil.validateRange(failedAttemptIntervalSec, "failedAttemptIntervalSec",
            1, FAILED_ATTEMPT_INTERVAL_LIMIT_SEC);
      ValidateUtil.validatePositiveNumber(failedAttempts, "failedAttempts");
      ValidateUtil.validateRange(autoUnlockIntervalSec, "autoUnlockIntervalSec",
            0, AUTO_UNLOCK_INTERVAL_LIMIT_SEC);

      _description = description;
      _failedAttemptIntervalSec = failedAttemptIntervalSec;
      _failedAttempts = failedAttempts;
      _autoUnlockIntervalSec = autoUnlockIntervalSec;
   }

   /**
    * Policy detailed description
    *
    * @return non-empty valid string for policy description
    */
   public String getDescription() {
      return _description;
   }

   /**
    * The time window ( in seconds ) in which the failures must occur to trigger
    * a lockout
    *
    * @return positive number for time window for login failures
    */
   public long getFailedAttemptIntervalSec() {
      return _failedAttemptIntervalSec;
   }

   /**
    * @see #getFailedAttempts()
    */
   @Deprecated
   public int getMaxFailedAttempts() {
      return getFailedAttempts();
   }

   /**
    * The maximum number of failed login attempts allowed before a lockout
    * occurs
    *
    * @return positive number for failed login attempts
    */
   public int getFailedAttempts() {
      return _failedAttempts;
   }

   /**
    * The time ( in seconds ) that the account will remain locked or zero to
    * make the only way an administrator to unlock it explicitly
    *
    * @return a non-negative value for the time the account will remain locked
    */
   public long getAutoUnlockIntervalSec() {
      return _autoUnlockIntervalSec;
   }
}
