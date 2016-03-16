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
 * A password policy is a set of rules and restrictions on the
 * allowed passwords' format and the maximum password age.
 *
 * <p>
 * All value ranges below are inclusive. If not otherwise specified,
 * numeric values have the same limits as their datatypes.
 *
 */
public class PasswordPolicy {

   public static final int DESCRIPTION_LENGTH_LIMIT = 120;

   private final String _description;

   private final int _prohibitedPreviousPasswordsCount;

   private final PasswordFormat _passwordFormat;

   private final Integer _passwordLifetimeDays;

   /**
    * Create a password policy data object.
    *
    * @param description
    *           Policy's detailed description, could be null or empty, max length is
    *           {@value #DESCRIPTION_LENGTH_LIMIT}
    *
    * @param prohibitedPreviousPasswordsCount
    *           Restricts how many of the user's last used passwords cannot be
    *           selected; a non-negative number
    * @param passwordFormat
    *           Password format restrictions
    * @param passwordLifetimeDays
    *           Maximum password lifetime in days. Set to {@code null} to
    *           indicate infinite lifetime or provide a positive number
    */
   public PasswordPolicy(
      String description,
      int prohibitedPreviousPasswordsCount,
      PasswordFormat passwordFormat,
      Integer passwordLifetimeDays) {

      ValidateUtil.validateNotNull(passwordFormat, "passwordFormat");
      if (description != null) {
         ValidateUtil.validateRange(description.length(), "length of description",
            0, DESCRIPTION_LENGTH_LIMIT);
      }

      _description = description;
      _prohibitedPreviousPasswordsCount = prohibitedPreviousPasswordsCount;
      _passwordFormat = passwordFormat;
      _passwordLifetimeDays = passwordLifetimeDays;
   }

   /**
    * @return the policy's detailed description.
    */
   public String getDescription() {
      return _description;
   }

   /**
    * @return how many of the user's last used passwords cannot be selected
    */
   public int getProhibitedPreviousPasswordsCount() {
      return _prohibitedPreviousPasswordsCount;
   }

   /**
    * @return the password format restrictions
    */
   public PasswordFormat getPasswordFormat() {
      return _passwordFormat;
   }

   /**
    * @return a password's lifetime in days; {@code null} indicates infinite
    * life.
    */
   public Integer getPasswordLifetimeDays() {
      return _passwordLifetimeDays;
   }

   @Override
   public String toString() {
      return "PasswordPolicy [description=" + _description
            + ", prohibitedPreviousPasswordsCount="
            + _prohibitedPreviousPasswordsCount + ", passwordFormat="
            + _passwordFormat + ", passwordLifetimeDays="
            + _passwordLifetimeDays + "]";
   }


}
