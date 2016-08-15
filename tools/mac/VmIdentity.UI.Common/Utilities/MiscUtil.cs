﻿/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System;
using AppKit;
using System.Configuration;
using System.IO;
using VMIdentity.CommonUtils;

namespace VmIdentity.UI.Common
{
	public static class MiscUtil
	{
		public static string GetBrandConfig (string key)
		{
			try
			{
				return CommonConstants.GetConfigValue(key);
			}
			catch(Exception e) {
				var alert = new NSAlert (){
					AlertStyle=NSAlertStyle.Critical,
					InformativeText=CommonConstants.CONFIG_NOT_FOUND
				};
				alert.RunModal ();
				return string.Empty;
			}
		}
	}
}

