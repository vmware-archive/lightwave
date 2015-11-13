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

/**
 *
 * @author dmehta
 *
 */
public class SsoHealthStatsData implements Serializable {

	/**
	 * Serial version UID
	 */
	private static final long serialVersionUID = -7721320292891114497L;
	private String tenant;
	private int totalTokensGenerated;
	private int totalTokensRenewed;

	private int generatedTokensForTenant;
	private int renewedTokensForTenant;

	private long uptimeIDM;
	private long uptimeSTS;

	public SsoHealthStatsData(){

	}

	public SsoHealthStatsData(String tenant, int totalTokensGenerated,
			int totalTokensRenewed, int tokensGeneratedForTenant,
			int tokensRenewedForTenant, long upTimeIDM, long upTimeSTS) {

		this.tenant = tenant;
		this.totalTokensGenerated = totalTokensGenerated;
		this.totalTokensRenewed = totalTokensRenewed;
		this.generatedTokensForTenant = tokensGeneratedForTenant;
		this.renewedTokensForTenant = tokensRenewedForTenant;
		this.uptimeIDM = upTimeIDM;
		this.uptimeSTS = upTimeSTS;
	}

	public int getTotalTokensGenerated() {
		return totalTokensGenerated;
	}

	public int getTotalTokensRenewed() {
		return totalTokensRenewed;
	}

	public int getGeneratedTokensForTenant() {
		return generatedTokensForTenant;
	}

	public int getRenewedTokensForTenant() {
		return renewedTokensForTenant;
	}

	public long getUptimeIDM() {
		return uptimeIDM;
	}

	public long getUptimeSTS() {
		return uptimeSTS;
	}

	public String getTenant() {
		return tenant;
	}
}
