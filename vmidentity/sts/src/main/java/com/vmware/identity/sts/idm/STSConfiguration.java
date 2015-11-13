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
package com.vmware.identity.sts.idm;

import org.apache.commons.lang.Validate;

/**
 * Insert your comment for STSConfiguration here
 */
public final class STSConfiguration {
   // TODO re-think this object purpose and if it is needed(holds only 1 param).

   private final long clockTolerance;

   /**
    *
    * Creates a new configuration
    *
    * @param clockTolerance
    *           Maximum allowed clock skew between token sender and consumer, in
    *           milliseconds, required.
    * @throws IllegalArgumentException
    *            when some of the arguments are invalid
    */
   public STSConfiguration(long clockTolerance) {
      Validate.isTrue(clockTolerance >= 0);

      this.clockTolerance = clockTolerance;
   }

   /**
    * @return the clockTolerance
    */
   public long getClockTolerance() {
      return clockTolerance;
   }

   @Override
   public int hashCode() {
      final int prime = 31;
      int result = 1;
      result = prime * result
         + (int) (clockTolerance ^ (clockTolerance >>> 32));
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

      STSConfiguration other = (STSConfiguration) obj;
      return (clockTolerance == other.clockTolerance);
   }

}
