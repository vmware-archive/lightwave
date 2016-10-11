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

using Microsoft.ManagementConsole;
using VMPSCHighAvailability.Common;
using VMPscHighAvailabilitySnapIn.ScopeNodes;
using VMPscHighAvailabilitySnapIn.Utils;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMPscHighAvailabilitySnapIn.SnapIn
{
    /// <summary>
    /// Snapin
    /// </summary>
    //[SnapInSettings("{3D203C08-4574-4A05-9E74-ACFEAC0082FE}", DisplayName = Constants.Company + " " + Constants.ToolName + " " + Constants.SnapIn)]
    //uncomment above line for local testing
    public class VMPscHighAvailabilitySnapIn : Microsoft.ManagementConsole.SnapIn
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public VMPscHighAvailabilitySnapIn()
        {
            RootNode = new RootNode();
        }

        /// <summary>
        /// On snapin initialization
        /// </summary>
        protected override void OnInitialize()
        {
            base.OnInitialize();

            PscHighAvailabilityAppEnvironment.Instance.LoadLocalData();
            this.SmallImages.AddStrip(ResourceHelper.GetToolbarImage());
            PscHighAvailabilityAppEnvironment.Instance.SnapIn = this;
            MMCDlgHelper.snapIn = this;
            ((RootNode)RootNode).RefreshAll();
        }

        /// <summary>
        /// On snapin shutdown
        /// </summary>
        /// <param name="status"></param>
        protected override void OnShutdown(AsyncStatus status)
        {
            PscHighAvailabilityAppEnvironment.Instance.SaveLocalData();
        }
    }
}
