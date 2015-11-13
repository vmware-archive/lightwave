/* **********************************************************************
 * Copyright 2010-2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Data class representing password format restrictions.
 *
 * <p>
 * All value ranges below are inclusive. If not otherwise specified,
 * numeric values have the same limits as their datatypes.
 */
public final class PasswordFormat {

   /**
    * Restriction based on the password's length in characters.
    */
   public static final class LengthRestriction {

      private final int _minLength;

      private final int _maxLength;

      /**
       * Create an object representing password length restriction.
       *
       * @param minLength
       *           Minimum password length in characters. Should be
       *           non-negative.
       * @param maxLength
       *           Maximum password length in characters. Should be positive and
       *           greater than or equal to minLength,
       *           {@link AlphabeticRestriction#getMinAlphabeticCount()},
       *           {@link PasswordFormat#getMinNumericCount()} and
       *           {@link PasswordFormat#getMinSpecialCharCount()}.
       */
      public LengthRestriction(int minLength, int maxLength) {

         ValidateUtil.validateNotNegative(minLength, "minLength");
         ValidateUtil.validateNotNegative(maxLength - minLength,
               "Length tolerance (maxLength - minLength)");

         this._minLength = minLength;
         this._maxLength = maxLength;
      }

      /**
       * @return The minimum password length in characters
       */
      public int getMinLength() {
         return _minLength;
      }

      /**
       * @return The maximum password length in characters
       */
      public int getMaxLength() {
         return _maxLength;
      }

      @Override
      public String toString() {
         return "LengthRestriction [minLength=" + _minLength + ", maxLength="
               + _maxLength + "]";
      }
   }

   /**
    * Restrictions over the alphabetic characters in the password.
    */
   public static final class AlphabeticRestriction {

      private final int _minAlphabeticCount;

      private final int _minUppercaseCount;

      private final int _minLowercaseCount;

      /**
       * Create an object representing a restriction over the password's
       * alphabetic characters.
       *
       * @param minAlphabeticCount
       *           Require at least minAlphabeticCount alphabetic characters in
       *           the password. Should be non-negative and greater than or
       *           equal to the sum of minUppercaseCount and minLowercaseCount.
       * @param minUppercaseCount
       *           Require at least minUppercaseCount upper case characters in
       *           the password. Should be non-negative.
       * @param minLowercaseCount
       *           Require at least minLowercaseCount lower case characters in
       *           the password.
       */
      public AlphabeticRestriction(
         int minAlphabeticCount,
         int minUppercaseCount,
         int minLowercaseCount) {

         ValidateUtil.validateRange(minAlphabeticCount, "minAlphabeticCount",
               minUppercaseCount+minLowercaseCount, Integer.MAX_VALUE);
         ValidateUtil.validateNotNegative(minUppercaseCount, "minUppercaseCount");
         ValidateUtil.validateNotNegative(minLowercaseCount, "minLowercaseCount");

         _minAlphabeticCount = minAlphabeticCount;
         _minUppercaseCount = minUppercaseCount;
         _minLowercaseCount = minLowercaseCount;
      }

      /**
       * @return The minimum required alphabetic characters in the password.
       */
      public int getMinAlphabeticCount() {
         return _minAlphabeticCount;
      }

      /**
       * @return The minimum required upper case characters in the password.
       */
      public int getMinUppercaseCount() {
         return _minUppercaseCount;
      }

      /**
       * @return The minimum required lower case characters in the password.
       */
      public int getMinLowercaseCount() {
         return _minLowercaseCount;
      }

      @Override
      public String toString() {
         return "AlphabeticRestriction [minAlphabeticCount="
               + _minAlphabeticCount + ", minUppercaseCount="
               + _minUppercaseCount + ", minLowercaseCount="
               + _minLowercaseCount + "]";
      }

   }

   private final LengthRestriction _lengthRestriction;

   private final AlphabeticRestriction _alphabeticRestriction;

   private final int _minNumericCount;

   private final int _minSpecialCharCount;

   private final int _maxIdenticalAdjacentCharacters;

   /**
    * Create a set of password format restrictions.
    *
    * @param lengthRestriction
    *           Restriction on the password's length;
    * @param alphabeticRestriction
    *           Restriction on the alphabetic characters in the password.
    * @param minNumericCount
    *           Require at least minNumericCount numeric characters in password.
    *           Should be non-negative.
    * @param minSpecialCharCount
    *           Require at least minSpecialCharCount special characters (such as
    *           &#x40;, #, $, %, &amp;, ^ and *) in the password. Should be
    *           non-negative.
    * @param maxIdenticalAdjacentCharacters
    *           Restrict the maximum number of identical adjacent characters
    *           allowed in a password. Should be positive.
    */
   public PasswordFormat(
      LengthRestriction lengthRestriction,
      AlphabeticRestriction alphabeticRestriction,
      int minNumericCount,
      int minSpecialCharCount,
      int maxIdenticalAdjacentCharacters) {

      ValidateUtil.validateNotNull(lengthRestriction, "lengthRestriction");
      ValidateUtil.validateNotNull(alphabeticRestriction, "alphabeticRestriction");
      ValidateUtil.validatePositiveNumber(maxIdenticalAdjacentCharacters, "maxIdenticalAdjacentCharacters");

      ValidateUtil.validateRange(alphabeticRestriction._minAlphabeticCount +
                                 minNumericCount +
                                 minSpecialCharCount,
            "The sum of minAlphabeticCount, minNumericCount and minSpecialCharCount", 0, lengthRestriction._maxLength);

      _lengthRestriction = lengthRestriction;
      _alphabeticRestriction = alphabeticRestriction;
      _minNumericCount = minNumericCount;
      _minSpecialCharCount = minSpecialCharCount;
      _maxIdenticalAdjacentCharacters = maxIdenticalAdjacentCharacters;
   }

   /**
    * @return The restrictions over the password's length.
    */
   public LengthRestriction getLengthRestriction() {
      return _lengthRestriction;
   }

   /**
    * @return The restrictions over the alphabetic characters in the password.
    */
   public AlphabeticRestriction getAlphabeticRestriction() {
      return _alphabeticRestriction;
   }

   /**
    * @return The minimum required number of numeric characters in the password.
    */
   public int getMinNumericCount() {
      return _minNumericCount;
   }

   /**
    * @return The minimum required number of special characters in the password.
    */
   public int getMinSpecialCharCount() {
      return _minSpecialCharCount;
   }

   /**
    * @return The maximum number of adjacent identical characters allowed in a
    *         password.
    */
   public int getMaxIdenticalAdjacentCharacters() {
      return _maxIdenticalAdjacentCharacters;
   }

   @Override
   public String toString() {
      return "PasswordFormat [lengthRestriction=" + _lengthRestriction
            + ", alphabeticRestriction=" + _alphabeticRestriction
            + ", minNumericCount=" + _minNumericCount
            + ", minSpecialCharCount=" + _minSpecialCharCount
            + ", maxIdenticalAdjacentCharacters="
            + _maxIdenticalAdjacentCharacters + "]";
   }
}
