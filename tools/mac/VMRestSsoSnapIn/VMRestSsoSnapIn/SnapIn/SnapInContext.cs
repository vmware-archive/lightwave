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


using AppKit;
using Foundation;
using System;
using System.IO;
using System.Reflection;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Cache;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace Vmware.Tools.RestSsoAdminSnapIn
{
    public class SnapInContext
    {
        private string _applicationPath;
        private static SnapInContext _instance;
        private ServiceGateway _serviceGateway;
        private SnapInContext()
        {
            SessionId = string.Format("{0}", DateTime.Now.ToString("MMddyy_hhmmssfff"));
        }
        public static SnapInContext Instance
        {
            get { return _instance ?? (_instance = new SnapInContext()); }
        }
        public string SessionId { get; private set; }
       
        public string ApplicationPath
        {
			get {
				if (string.IsNullOrEmpty (_applicationPath)) {
					NSFileManager fileManager = NSFileManager.DefaultManager;
					string[] paths = NSSearchPath.GetDirectories (NSSearchPathDirectory.ApplicationSupportDirectory, NSSearchPathDomain.User);
					if (paths.Length > 0) {
						_applicationPath = paths [0] + "/LightwaveTools";
						if (!Directory.Exists (_applicationPath)) {
							Directory.CreateDirectory (_applicationPath);
						}
					}
				}
				return _applicationPath;
			}
        }
        public AuthTokenManager AuthTokenManager { get; set; }
        public ServiceGateway ServiceGateway
        {
            get
            {
				if (_serviceGateway == null) {
					var serviceConfig = new ServiceConfigManager();
					_serviceGateway = new ServiceGateway (serviceConfig);
				}
				return _serviceGateway;
            }
			set{
				_serviceGateway = value;
			}
        }
    }
}
