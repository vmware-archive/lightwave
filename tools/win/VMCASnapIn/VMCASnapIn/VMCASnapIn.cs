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
using System.ComponentModel;
using System.Security.Permissions;
using System.Windows.Forms;
using Microsoft.ManagementConsole;
using VMCASnapIn.Nodes;
using VMCASnapIn.Utilities;
using VMwareMMCIDP.UI.Common.Utilities;

[assembly: PermissionSetAttribute(SecurityAction.RequestMinimum, Unrestricted = true)]
namespace VMCASnapIn
{
    [RunInstaller(true)]
    public class InstallUtilSupport : SnapInInstaller
    {
    }

    //[SnapInSettings("{D2608952-F746-496c-BCDE-A0A91277F711}", DisplayName = "VMware certificate server")]
    //uncomment above line for local testing
    public class VMCASnapIn : SnapIn
    {
        public VMCASnapIn()
        {
            VMCASnapInEnvironment.Instance.LoadLocalData();
            this.SmallImages.AddStrip(MiscUtilsService.GetToolbarImage());
            this.RootNode = new VMCARootNode();
        }

        protected override void OnInitialize()
        {
            base.OnInitialize();
            VMCASnapInEnvironment.Instance.SnapIn = this;
            MMCDlgHelper.snapIn = this;
        }

        protected override void OnShutdown(AsyncStatus status)
        {
            VMCASnapInEnvironment.Instance.SaveLocalData();
        }
    }
}