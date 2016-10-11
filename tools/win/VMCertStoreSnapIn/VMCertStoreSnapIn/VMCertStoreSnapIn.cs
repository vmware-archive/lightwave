/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */using System.ComponentModel;
using System.Security.Permissions;
using System.Text;
using System.Windows.Forms;
using Microsoft.ManagementConsole;
using VMCertStoreSnapIn.ListViews;
using VMCertStoreSnapIn.Nodes;
using VMCertStoreSnapIn.UI;
using VMCertStoreSnapIn.Utilities;
using VMCertStore;

using VMwareMMCIDP.UI.Common.Utilities;

[assembly: PermissionSetAttribute(SecurityAction.RequestMinimum, Unrestricted = true)]
namespace VMCertStoreSnapIn
{

    [RunInstaller(true)]
    public class InstallUtilSupport : SnapInInstaller
    {
    }

    //[SnapInSettings("{01E67CB5-7A76-4ea4-99F7-FBF720C19A81}", DisplayName = "VMware Local Authentication Service")]
    //uncomment above line for local testing
    public class VMCertStoreSnapIn : SnapIn
    {
        public VMCertStoreSnapIn()
        {
            VMCertStoreSnapInEnvironment.Instance.LoadLocalData();

            var il = new ImageList();
            this.SmallImages.AddStrip(MiscUtilsService.GetToolbarImage());

            this.RootNode = new VMCertStoreRootNode();

            this.RootNode.ImageIndex = this.RootNode.SelectedImageIndex = (int)VMCertStoreImageIndex.VecsStore;
        }

        protected override void OnInitialize()
        {
            base.OnInitialize();
            VMCertStoreSnapInEnvironment.Instance.SnapIn = this;
            MMCDlgHelper.snapIn = this;
        }

        protected override void OnShutdown(AsyncStatus status)
        {
            VMCertStoreSnapInEnvironment.Instance.SaveLocalData();
        }
    }
}