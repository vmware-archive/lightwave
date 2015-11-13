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
package com.vmware.identity.idm.server.config.directory;

import java.io.Serializable;

public final class TokenPolicy implements Serializable
{
    private static final long serialVersionUID = 1995210895716413754L;


    /** @param maxTokenDelegationCount
    *           Maximum token delegation count. Should be
    *           non-negative.
    */
    private final int maxTokenDelegationCount;

    /** @param maxTokenRenewCount
    *           Maximum Token renewal count. Should be positive
    */
    private final int maxTokenRenewCount;

    /**
    * @param maxTokenClockTolerance
    *        Maximum token tolerance in milliseconds. Should be
    *        non-negative.
    */
    private final long maxTokenClockTolerance;

    /** @param maxHOKLifetime
     *          Maximum Holder-of-key token lifetime. Should be positive.
     */
    private final long maxHOKLifetime;

   /**
    * @param maxBearerTokenLifetime
    *           Maximum bearer token life time. Should be positive.
    */
    private final long maxBearerTokenLifetime;

    /** @param maxBearerRefreshTokenLifetime
     *          Maximum bearer refresh token life time. Should be positive.
     */
    private final long maxBearerRefreshTokenLifetime;

   /**
    * @param maxHoKRefreshTokenLifetime
    *           Maximum Holder-of-key refresh token lifetime. Should be positive.
    */
    private final long maxHoKRefreshTokenLifetime;

   public
   TokenPolicy(
       int maxTokenDelegationCount,
       int maxTokenRenewCount,
       long maxTokenClockTolerance,
       long maxHOKLifetime,
       long maxBearerTokenLifetime,
       long maxBearerRefreshTokenLifetime,
       long maxHoKRefreshTokenLifetime
        )
   {
      this.maxTokenDelegationCount = maxTokenDelegationCount;
      this.maxTokenRenewCount = maxTokenRenewCount;
      this.maxTokenClockTolerance = maxTokenClockTolerance;
      this.maxHOKLifetime = maxHOKLifetime;
      this.maxBearerTokenLifetime = maxBearerTokenLifetime;
      this.maxBearerRefreshTokenLifetime = maxBearerRefreshTokenLifetime;
      this.maxHoKRefreshTokenLifetime = maxHoKRefreshTokenLifetime;
   }

   /**
    * @return the maxTokenDelegationCount
    */
   public int getMaxTokenDelegationCount()
   {
      return maxTokenDelegationCount;
   }

   /**
    * @return the maxTokenRenewCount
    */
   public int getMaxTokenRenewCount()
   {
      return maxTokenRenewCount;
   }

   /**
    *
    * @return the maxTokenClockTolerance
    */
   public long getMaxTokenClockTolerance()
   {
       return maxTokenClockTolerance;
   }

   /**
    *
    * @return the maxHOKLifetime
    */
   public long getMaxHOKLifetime()
   {
       return maxHOKLifetime;
   }

   /**
    *
    * @return the maxBearerTokenLifetime
    */
   public long getMaxBearerTokenLifetime()
   {
       return maxBearerTokenLifetime;
   }

   public long getMaxBearerRefreshTokenLifetime()
   {
       return maxBearerRefreshTokenLifetime;
   }

   public long getMaxHoKRefreshTokenLifetime()
   {
       return maxHoKRefreshTokenLifetime;
   }

   @Override
   public int hashCode()
   {
      final int prime = 31;
      int result = 1;

      result = prime * result + maxTokenDelegationCount;
      result = prime * result + maxTokenRenewCount;
      result =
              prime
                      * result
                      + (int) (maxTokenClockTolerance ^ (maxTokenClockTolerance >>> 32));
      result =
              prime
                      * result
                      + (int) (maxHOKLifetime ^ (maxHOKLifetime >>> 32));
      result =
              prime
                      * result
                      + (int) (maxBearerTokenLifetime ^ (maxBearerTokenLifetime >>> 32));
      result =
              prime
                      * result
                      + (int) (maxBearerRefreshTokenLifetime ^ (maxBearerRefreshTokenLifetime >>> 32));
      result =
              prime
                      * result
                      + (int) (maxHoKRefreshTokenLifetime ^ (maxHoKRefreshTokenLifetime >>> 32));

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
       else if (other != null && other instanceof TokenPolicy)
       {
           TokenPolicy peer = (TokenPolicy)other;

           result = ( peer.maxTokenDelegationCount == maxTokenDelegationCount ) &&
                    ( peer.maxTokenRenewCount == maxTokenRenewCount ) &&
                    ( peer.maxTokenClockTolerance == maxTokenClockTolerance ) &&
                    ( peer.maxHOKLifetime == maxHOKLifetime ) &&
                    ( peer.maxBearerTokenLifetime == maxBearerTokenLifetime ) &&
                    ( peer.maxBearerRefreshTokenLifetime == maxBearerRefreshTokenLifetime ) &&
                    ( peer.maxHoKRefreshTokenLifetime == maxHoKRefreshTokenLifetime );
       }

       return result;
   }
}
