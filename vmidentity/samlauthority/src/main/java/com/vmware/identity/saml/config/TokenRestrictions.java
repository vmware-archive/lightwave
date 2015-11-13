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
package com.vmware.identity.saml.config;

import org.apache.commons.lang.Validate;

/**
 * This class will represent token restrictions for Saml authority issued
 * tokens.
 */
public final class TokenRestrictions {

   private final long maximumBearerTokenLifetime;
   private final long maximumHoKTokenLifetime;
   private final int delegationCount;
   private final int renewCount;

   /**
    * @param maximumBearerTokenLifetime
    *           in milliseconds. Positive number.
    * @param maximumHoKTokenLifetime
    *           in milliseconds Positive number.
    */
   public TokenRestrictions(long maximumBearerTokenLifetime,
      long maximumHoKTokenLifetime, int delegationCount, int renewCount) {
      Validate.isTrue(maximumBearerTokenLifetime > 0);
      Validate.isTrue(maximumHoKTokenLifetime > 0);
      Validate.isTrue(delegationCount >= 0);
      Validate.isTrue(renewCount >= 0);

      this.maximumBearerTokenLifetime = maximumBearerTokenLifetime;
      this.maximumHoKTokenLifetime = maximumHoKTokenLifetime;
      this.delegationCount = delegationCount;
      this.renewCount = renewCount;
   }

   /**
    * @return the maximumBearerTokenLifetime
    */
   public long getMaximumBearerTokenLifetime() {
      return maximumBearerTokenLifetime;
   }

   /**
    * @return the maximumHoKTokenLifetime
    */
   public long getMaximumHoKTokenLifetime() {
      return maximumHoKTokenLifetime;
   }

   /**
    * @return the token delegation count
    */
   public int getDelegationCount() {
      return delegationCount;
   }

   /**
    * @return the token renew count
    */
   public int getRenewCount() {
      return renewCount;
   }

   @Override
   public int hashCode() {
      final int prime = 31;
      int result = 1;
      result = prime
         * result
         + (int) (maximumBearerTokenLifetime ^ (maximumBearerTokenLifetime >>> 32));
      result = prime * result
         + (int) (maximumHoKTokenLifetime ^ (maximumHoKTokenLifetime >>> 32));
      result = prime * result + delegationCount;
      result = prime * result + renewCount;
      return result;
   }

   @Override
   public boolean equals(Object obj) {
      if (this == obj) {
         return true;
      }
      if (obj == null || getClass() != obj.getClass()) {
         return false;
      }

      TokenRestrictions other = (TokenRestrictions) obj;
      return maximumBearerTokenLifetime == other.maximumBearerTokenLifetime
         && maximumHoKTokenLifetime == other.maximumHoKTokenLifetime
         && delegationCount == other.delegationCount
         && renewCount == other.renewCount;
   }

}
