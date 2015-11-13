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
 * Advapi Java to Native Adapter
 * 
 * @author:  Sriram Nambakam <snambakam@vmware.com>
 * 
 * @version: 1.0
 * @since:   2011-12-7
 * 
 */

package com.vmware.identity.interop;

import com.sun.jna.Library;
import com.sun.jna.Native;

public class AdvapiAdapter extends NativeAdapter
{
	/**
	 * AdvapiLibraryNoThr is the JNA wrapper over C APIs in the library at liblwadvapi_nothr.so
	 * */
	public interface AdvapiLibraryNoThr extends Library
	{
	    AdvapiLibraryNoThr INSTANCE = 
	    		(AdvapiLibraryNoThr) Native.loadLibrary(
	    										"lwadvapi_nothr",
	    										AdvapiLibraryNoThr.class);
	
	    String 
	    LwWin32ExtErrorToName(
	    	int errorCode
	    	);
	    
	    String
	    LwWin32ExtErrorToDescription(
	    	int errorCode
	    	);
	}
}
