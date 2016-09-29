/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
using System.IO;
using System.Reflection;
using System.Xml.Serialization;
using AppKit;
using Foundation;
using VmIdentity.CommonUtils;
using VmIdentity.CommonUtils.Persistance;
using VMPSCHighAvailability.UI;
using VMPSCHighAvailability.Common;
using VMIdentity.CommonUtils.Log;

namespace VMPSCHighAvailability 
{
	/// <summary>
	/// App environment for the snapin.
	/// </summary>
	public class PscHighAvailabilityAppEnvironment : VMBaseSnapInEnvironment
	{
		/// <summary>
		/// The instance.
		/// </summary>
		private static PscHighAvailabilityAppEnvironment _instance;

		/// <summary>
		/// Logger
		/// </summary>
		private ILogger _logger;

		/// <summary>
		/// Gets the file name path.
		/// </summary>
		/// <value>The file name path.</value>
		public  override string FileNamePath
		{
			get
			{
				return Path.Combine(GetApplicationPath(), DataFileName);
			}
		}

		/// <summary>
		/// Gets the application path.
		/// </summary>
		/// <returns>The application path.</returns>
		public override string GetApplicationPath() {
			if (string.IsNullOrEmpty (ApplicationPath)) {
				string[] paths = NSSearchPath.GetDirectories (NSSearchPathDirectory.ApplicationSupportDirectory, NSSearchPathDomain.User);
				if (paths.Length > 0) {
					ApplicationPath = paths [0] + "/" + Constants.ToolsSuiteName;
					if(!File.Exists (ApplicationPath)) {
						Directory.CreateDirectory (ApplicationPath);
					}
				}
			}
			return ApplicationPath;
		}	

		/// <summary>
		/// Gets the instance.
		/// </summary>
		/// <value>The instance.</value>
		public static PscHighAvailabilityAppEnvironment Instance {
			get {
				if (_instance == null) {
					_instance = new PscHighAvailabilityAppEnvironment ();
					_instance.SetSnapInDataFileName (Constants.LocalDataFileName);
				}
				return _instance;
			}
		}

		/// <summary>
		/// Logger instance
		/// </summary>
		public ILogger Logger
		{
			get
			{
				if (_logger == null)
				{
					var filePath = Path.Combine(GetApplicationPath(), Constants.PscLogFileName);
					_logger = new FileLogger(filePath);
				}
				return _logger;
			}
		}
	}
}

