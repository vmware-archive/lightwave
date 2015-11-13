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

package com.vmware.identity.interop;

import org.apache.commons.lang.SystemUtils;

import com.sun.jna.platform.win32.Shell32Util;
import com.sun.jna.platform.win32.ShlObj;

public class FileSystemPathLocator
{
	public static final String VMWARE_FOLDER_NAME          = "vmware";
	public static final String VMWARE_IDENTITY_FOLDER_NAME = "identity";
	public static final String LINUX_CACHE_FOLDER_ROOT     = "/var/lib";
	
	public static String getCacheFolderPath()
	{
		String cacheDirPath = null;
		
		if (SystemUtils.IS_OS_WINDOWS)
		{
			cacheDirPath = Shell32Util.getFolderPath(
											ShlObj.CSIDL_LOCAL_APPDATA);
		}
		else
		{
			cacheDirPath = LINUX_CACHE_FOLDER_ROOT;
		}
			
		if (cacheDirPath == null || cacheDirPath.isEmpty())
		{
			throw new IllegalStateException("Found empty cache dir path");
		}
		
		return String.format(
						"%s%s%s%s%s", 
						cacheDirPath,
						SystemUtils.FILE_SEPARATOR,
						VMWARE_FOLDER_NAME,
						SystemUtils.FILE_SEPARATOR,
						VMWARE_IDENTITY_FOLDER_NAME);
	}
	
	public static String getKrb5ConfFilePath()
	{
		return String.format(
						"%s%skrb5.conf", 
						getCacheFolderPath(), 
						SystemUtils.FILE_SEPARATOR);
	}
}
