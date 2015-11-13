/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.interop.domainmanager;

import com.sun.jna.Platform;

/**
 * Created by IntelliJ IDEA.
 * User: wfu
 * Date: 04/10/2013
 * Time: 10:59 PM
 */
public class DomainTrustInfo
{
	public DomainControllerInfo dcInfo = null;

	public boolean IsInforest = false;
	public boolean IsOutBound = false;
	public boolean IsInBound = false;
	public boolean IsRoot = false;
	public boolean IsPrimary = false;
	public boolean IsNativeMode = false;
	public boolean isExternal = false;

	public DomainTrustInfo(WinDomainTrustInfoNative trust, DomainControllerInfo dcInfo)
	{
		assert Platform.isWindows();

		this.dcInfo = dcInfo;

		this.IsInBound = trust.isInBound();
		this.IsOutBound = trust.isOutBound();
		this.IsInforest = trust.isInForest();
		this.IsRoot = trust.isRoot();
		this.IsPrimary = trust.isPrimary();
		this.IsNativeMode = trust.isNativeMode();
		this.isExternal = trust.isExternal();
	}

	public DomainTrustInfo(LinuxDomainTrust trust)
	{
		assert Platform.isLinux();

		this.dcInfo = trust.dcInfo;

		this.IsInforest = trust.IsInforest;
		this.IsOutBound = trust.IsOutBound;
		this.IsInBound = trust.IsInBound;
		this.IsRoot = trust.IsRoot;
		this.IsPrimary = trust.IsPrimary;
		this.IsNativeMode = trust.IsNativeMode;
	}
}

