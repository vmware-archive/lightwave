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

import java.io.Serializable;

/**
 * Immutable data type representing Sso Health Statisitics
 *
 * @author dmehta
 *
 */
public final class SsoHealthStats implements Serializable {

   private static final long serialVersionUID = -714920308572747869L;

   private String _tenant;

   private int _totalTokensGenerated;

   private int _totalTokensRenewed;

   private int _generatedTokensForTenant;

   private int _renewedTokensForTenant;

   // This gives the number of seconds IDM service is up for.
   private long _uptimeIDM;

   //This gives the number of seconds STS service is up for.

   private long _uptimeSTS;

   public SsoHealthStats() {

   }

   public SsoHealthStats(String tenant) {
      this._tenant = tenant;
   }

   public SsoHealthStats(String tenant, int totalTokensGenerated,
      int totalTokensRenewed, int generatedTokensForTenant,
      int renewedTokensForTenant, long uptimeIDM,
      long uptimeSTS) {

      this._tenant = tenant;
      this._totalTokensGenerated = totalTokensGenerated;
      this._totalTokensRenewed = totalTokensRenewed;
      this._generatedTokensForTenant = generatedTokensForTenant;
      this._renewedTokensForTenant = renewedTokensForTenant;
      this._uptimeIDM = uptimeIDM;
      this._uptimeSTS = uptimeSTS;
	}

   public String getTenant() {
      return _tenant;
   }

   public int getTotalTokensGenerated() {
      return _totalTokensGenerated;
   }

   public int getTotalTokensRenewed() {
      return _totalTokensRenewed;
   }

   public int getGeneratedTokensForTenant() {
      return _generatedTokensForTenant;
   }

   public int getRenewedTokensForTenant() {
      return _renewedTokensForTenant;
   }

   public long getUptimeIDM() {
      return _uptimeIDM;
   }

   public long getUptimeSTS() {
      return _uptimeSTS;
   }
}
