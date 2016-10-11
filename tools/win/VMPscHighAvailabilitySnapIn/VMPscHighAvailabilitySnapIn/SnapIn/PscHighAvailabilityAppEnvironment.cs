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
using VmIdentity.CommonUtils;
using VMIdentity.CommonUtils.Log;
using VMPSCHighAvailability.Common;
using VMPSCHighAvailability.Common.Service;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMPscHighAvailabilitySnapIn.SnapIn
{
    /// <summary>
    /// App environment for the snapin.
    /// </summary>
    public class PscHighAvailabilityAppEnvironment : VMBaseSnapInEnvironment
    {
        /// <summary>
        /// Logger
        /// </summary>
        private ILogger _logger;

        /// <summary>
        /// The instance.
        /// </summary>
        private static PscHighAvailabilityAppEnvironment _instance;

        /// <summary>
        /// The service.
        /// </summary>
        private IPscHighAvailabilityService _service;

        /// <summary>
        /// Gets the file name path.
        /// </summary>
        /// <value>The file name path.</value>
        public override string FileNamePath
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
        public override string GetApplicationPath()
        {
            if (string.IsNullOrEmpty(ApplicationPath))
                ApplicationPath = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            return ApplicationPath;
        }

        /// <summary>
        /// Gets the instance.
        /// </summary>
        /// <value>The instance.</value>
        public static PscHighAvailabilityAppEnvironment Instance
        {
            get
            {
                if (_instance == null)
                {
                    _instance = new PscHighAvailabilityAppEnvironment();
                    _instance.SetSnapInDataFileName(Constants.LocalDataFileName);
                }
                return _instance;
            }
        }

        /// <summary>
        /// Holds snapin instance
        /// </summary>
        public VMPscHighAvailabilitySnapIn SnapIn { get; set; }

        /// <summary>
        /// Service instance
        /// </summary>
        public IPscHighAvailabilityService Service
        {
            get
            {
                if (_service == null)
                {
                    _service = new PscHighAvailabilityService(Logger, Instance);
                }
                return _service;
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
                    var logFolder = MMCUIConstants.GetLogFolder(Environment.UserName);
                    var filePath = string.Format("{0}{1}", logFolder, MMCUIConstants.PSC_LOG_FILE);
                    _logger = new FileLogger(filePath);
                }
                return _logger;
            }
        }
    }
}

