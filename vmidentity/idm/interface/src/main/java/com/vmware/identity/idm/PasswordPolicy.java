/*
 *
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
 *
 */
package com.vmware.identity.idm;

import java.io.Serializable;

public final class PasswordPolicy implements Serializable
{
   private static final long serialVersionUID = 2732368191461052823L;

   private final String description;

   private final int prohibitedPreviousPasswordsCount;
   /** @param minLength
   *           Minimum password length in characters. Should be
   *           non-negative.
   */
   private final int minLength;
   /** @param maxLength
   *           Maximum password length in characters. Should be positive and
   *           greater than or equal to minLength
   */
   private final int maxLength;
   /*
    * @param minAlphabeticCount
    *        Require at least minAlphabeticCount alphabetic characters in
    *        the password. Should be non-negative and greater than or
    *        equal to the sum of minUppercaseCount and minLowercaseCount.
    */
   private final int minAlphabeticCount;
   /** @param minUppercaseCount
    *           Require at least minUppercaseCount upper case characters in
    *           the password. Should be non-negative.
    */
   private final int minUppercaseCount;
   /**
    * @param minLowercaseCount
    *           Require at least minLowercaseCount lower case characters in
    *           the password.
    */
   private final int minLowercaseCount;
   /**
    * @param minNumericCount
    *           Require at least minNumericCount numeric characters in
    *           the password.
    */
   private final int minNumericCount;
   /**
    * @param minSpecialCharCount
    *           Require at least minSpecialCharCount special characters in
    *           the password.
    */
   private final int minSpecialCharCount;
   /**
    * @param maxIdenticalAdjacentCharacters
    *           Require no more than maxIdenticalAdjacentCharacters identical
    *           adjacent characters in the password.
    */
   private final int maxIdenticalAdjacentCharacters;
   /**
    * @param passwordLifetimeDays
    *           Maximum password age in days.
    */
   private final int passwordLifetimeDays;

   public
   PasswordPolicy(
		String description,
		int    prohibitedPreviousPasswordCount,
		int    minLength,
		int    maxLength,
		int    minAlphabeticCount,
		int    minUppercaseCount,
		int    minLowercaseCount,
		int    minNumericCount,
		int    minSpecialCharCount,
		int    maxIdenticalAdjacentCharacters,
		int    passwordLifetimeDays
		)
   {
      this.description = description;
      this.prohibitedPreviousPasswordsCount = prohibitedPreviousPasswordCount;
      this.minLength = minLength;
      this.maxLength = maxLength;
      this.minAlphabeticCount = minAlphabeticCount;
      this.minUppercaseCount = minUppercaseCount;
      this.minLowercaseCount = minLowercaseCount;
      this.minNumericCount = minNumericCount;
      this.minSpecialCharCount = minSpecialCharCount;
      this.maxIdenticalAdjacentCharacters = maxIdenticalAdjacentCharacters;
      this.passwordLifetimeDays = passwordLifetimeDays;
   }

   /**
    * @return the description
    */
   public String getDescription()
   {
      return description;
   }

   /**
    * @return the prohibitedPreviousPasswordsCount
    */
   public int getProhibitedPreviousPasswordsCount()
   {
      return prohibitedPreviousPasswordsCount;
   }

   /**
    *
    * @return the minimum required length
    */
   public int getMinimumLength()
   {
	   return minLength;
   }

   /**
    *
    * @return the maximum required length
    */
   public int getMaximumLength()
   {
	   return maxLength;
   }

   /**
    *
    * @return the minimum number of alphabets required
    */
   public int getMinimumAlphabetCount()
   {
	   return minAlphabeticCount;
   }

   /**
    *
    * @return the minimum number of upper case characters
    */
   public int getMinimumUppercaseCount()
   {
	   return minUppercaseCount;
   }

   /**
    *
    * @return the minimum number of lower case characters
    */
   public int getMinimumLowercaseCount()
   {
	   return minLowercaseCount;
   }

   /**
    *
    * @return the minimum number of numeric characters
    */
   public int getMinimumNumericCount()
   {
	   return minNumericCount;
   }

   /**
    *
    * @return the minimum number of special characters required
    */
   public int getMinimumSpecialCharacterCount()
   {
	   return minSpecialCharCount;
   }

   /**
    *
    * @return the maximum number of adjacent identical characters allowed
    */
   public int getMaximumAdjacentIdenticalCharacterCount()
   {
	   return maxIdenticalAdjacentCharacters;
   }

   /**
    * @return the passwordLifetimeDays
    */
   public int getPasswordLifetimeDays()
   {
      return passwordLifetimeDays;
   }

   @Override
   public int hashCode()
   {
      final int prime = 31;
      int result = 1;
      result =
          prime * result
                     + ((description == null) ? 0 : description.hashCode());
      result = prime * result + maxIdenticalAdjacentCharacters;
      result = prime * result + maxLength;
      result = prime * result + minAlphabeticCount;
      result = prime * result + minLength;
      result = prime * result + minLowercaseCount;
      result = prime * result + minNumericCount;
      result = prime * result + minSpecialCharCount;
      result = prime * result + minUppercaseCount;
      result = prime * result + passwordLifetimeDays;
      result = prime * result + prohibitedPreviousPasswordsCount;
      return result;
   }

   @Override
   public boolean equals(Object other)
   {
	   boolean result = false;

	   if (this == other)
	   {
		   result = true;
	   }
	   else if (other != null && other instanceof PasswordPolicy)
	   {
		   PasswordPolicy peer = (PasswordPolicy)other;

		   result = isEquals(peer.description, description) &&
				    ( peer.minLength == minLength ) &&
				    ( peer.maxLength == maxLength ) &&
		            ( peer.minAlphabeticCount == minAlphabeticCount ) &&
		            ( peer.minUppercaseCount == minUppercaseCount ) &&
		            ( peer.minLowercaseCount == minLowercaseCount ) &&
		            ( peer.minNumericCount == minNumericCount ) &&
		            ( peer.minSpecialCharCount == minSpecialCharCount ) &&
		            ( peer.maxIdenticalAdjacentCharacters == maxIdenticalAdjacentCharacters ) &&
			   	  	(peer.passwordLifetimeDays == passwordLifetimeDays) &&
			   	  	(peer.prohibitedPreviousPasswordsCount ==
			   	  						prohibitedPreviousPasswordsCount);
	   }

	   return result;
   }

   private static <T> boolean isEquals(T src, T dst)
   {
	   	return (src == dst) ||
	   		   (src == null && dst == null) ||
	   		   (src != null && src.equals(dst));
   }
}
