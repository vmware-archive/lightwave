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
using System.ComponentModel;
using System.Security.Permissions;
using System.Text;
using VMDNS.Common;
using VMDNSSnapIn.Nodes;
using VMwareMMCIDP.UI.Common.Utilities;

/*
 * @author Sumalatha Abhishek
 */

[assembly: PermissionSetAttribute(SecurityAction.RequestMinimum, Unrestricted = true)]

namespace VMDNSSnapIn
{

    [RunInstaller(true)]

    public class InstallUtilSupport : SnapInInstaller
    {
    }

    //[SnapInSettings("{29268362-32A8-42D8-BD70-ACD6262FB328}", DisplayName = VMDNSConstants.VMDNS_SNAPIN,

    //     Description = VMDNSConstants.VMDNS_DESCRIPTION)]

    public class VMDNSSnapIn : SnapIn
    {
        public VMDNSSnapIn()
        {
            VMDNSSnapInEnvironment.Instance.LoadLocalData();
            this.SmallImages.AddStrip(ResourceHelper.GetToolbarImage());
            this.RootNode = new VMDNSRootNode();
        }

        protected override void OnInitialize()
        {
            base.OnInitialize();
            MMCDlgHelper.snapIn = this;
            VMDNSSnapInEnvironment.Instance.SnapIn = this;
        }

        protected override void OnShutdown(AsyncStatus status)
        {
            VMDNSSnapInEnvironment.Instance.SaveLocalData();
        }
    }
}