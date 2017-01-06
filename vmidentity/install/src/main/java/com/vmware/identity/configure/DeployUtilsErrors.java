/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.configure;

public enum DeployUtilsErrors {

	ERROR_ACCESS_DENIED(5),
	ERROR_INVALID_PARAMETER(87),
	ERROR_INVALID_NETNAME(1214),
	ERROR_PASSWORD_RESTRICTION(1325),
	LW_ERROR_PASSWORD_RESTRICTION(40127);
	private int errorCode;

	private DeployUtilsErrors(int errorCode) {
		this.errorCode = errorCode;
	}

	public int getErrorCode() {
		return errorCode;
	}
}
