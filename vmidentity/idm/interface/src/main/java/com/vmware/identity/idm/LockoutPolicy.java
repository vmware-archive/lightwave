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

public final class LockoutPolicy implements Serializable {

   private static final long serialVersionUID = -4189118766233559992L;

   private final String description;
   private final long failedAttemptIntervalSec;
   private final int maxFailedAttempts;
   private final long autoUnlockIntervalSec;

   /**
    * @param _description
    * @param _failedAttemptIntervalSec
    * @param _maxFailedAttempts
    * @param _autoUnlockIntervalSec
    */
   public LockoutPolicy(String description, long failedAttemptIntervalSec,
      int maxFailedAttempts, long autoUnlockIntervalSec) {

      this.description = description;
      this.failedAttemptIntervalSec = failedAttemptIntervalSec;
      this.maxFailedAttempts = maxFailedAttempts;
      this.autoUnlockIntervalSec = autoUnlockIntervalSec;
   }

   /**
    * @return the description
    */
   public String getDescription() {
      return description;
   }

   /**
    * @return the failedAttemptIntervalSec
    */
   public long getFailedAttemptIntervalSec() {
      return failedAttemptIntervalSec;
   }

   /**
    * @return the maxFailedAttempts
    */
   public int getMaxFailedAttempts() {
      return maxFailedAttempts;
   }

   /**
    * @return the autoUnlockIntervalSec
    */
   public long getAutoUnlockIntervalSec() {
      return autoUnlockIntervalSec;
   }

   @Override
   public int hashCode()
   {
       final int prime = 31;
       int result = 1;
       result =
               prime
                       * result
                       + (int) (autoUnlockIntervalSec ^ (autoUnlockIntervalSec >>> 32));
       result =
               prime * result
                       + ((description == null) ? 0 : description.hashCode());
       result =
               prime
                       * result
                       + (int) (failedAttemptIntervalSec ^ (failedAttemptIntervalSec >>> 32));
       result = prime * result + maxFailedAttempts;
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
	   else if (other != null && other instanceof LockoutPolicy)
	   {
		   LockoutPolicy peer = (LockoutPolicy)other;

		   result =
				 (isEquals(peer.description, description) &&
			   	  (peer.autoUnlockIntervalSec == autoUnlockIntervalSec) &&
			   	  (peer.failedAttemptIntervalSec == failedAttemptIntervalSec) &&
			   	  (peer.maxFailedAttempts == maxFailedAttempts)
			   	 );
	   }

	   return result;
   }

   private static boolean isEquals(String src, String dst)
   {
	   	return (src == dst) ||
	   		   (src == null && dst == null) ||
	   		   (src != null && src.equals(dst));
   }
}
