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

/**
 * VMware Identity Service
 * 
 * Exception from Native Library Calls
 * 
 * @author:  Sriram Nambakam <snambakam@vmware.com>
 * 
 * @version: 1.0
 * @since:   2011-12-7
 * 
 */

package com.vmware.identity.interop;

public class NativeCallException extends RuntimeException
{
	private static final long serialVersionUID = -7639587655177531496L;
	
	private int    _errCode = -1;
	private String _errName = "GENERIC_ERROR";
	private String _errMsg  = "Generic native error";

	public NativeCallException(int errCode)
	{
		_errCode = errCode;

		_errName = AdvapiAdapter.AdvapiLibraryNoThr.INSTANCE.LwWin32ExtErrorToName(errCode);
		_errMsg = AdvapiAdapter.AdvapiLibraryNoThr.INSTANCE.LwWin32ExtErrorToDescription(errCode);
	}

	public String getMessage()
	{
		return String.format("Native platform error [code: %d][%s][%s]", _errCode, _errName, _errMsg);
	}

	public int getErrCode()
	{
		return _errCode;
	}
}
